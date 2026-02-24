#include "elit21/node.hpp"

#include <sstream>
#include <stdexcept>

namespace elit21 {

Node::Node(std::string preferred_codec)
    : blockchain_(std::move(preferred_codec)), mempool_(10'000) {}

void Node::register_wallet(const std::string& address, const std::string& secret, std::uint64_t initial_balance) {
    if (wallets_.find(address) != wallets_.end()) {
        throw std::runtime_error("wallet already exists");
    }
    wallets_.emplace(address, Wallet(address, secret, initial_balance));
}

Wallet& Node::wallet(const std::string& address) {
    auto it = wallets_.find(address);
    if (it == wallets_.end()) {
        throw std::runtime_error("wallet not found");
    }
    return it->second;
}

const Wallet& Node::wallet(const std::string& address) const {
    auto it = wallets_.find(address);
    if (it == wallets_.end()) {
        throw std::runtime_error("wallet not found");
    }
    return it->second;
}

void Node::submit(const SignedTransaction& signed_tx) {
    auto sender_it = wallets_.find(signed_tx.tx.from);
    if (sender_it == wallets_.end()) {
        throw std::runtime_error("unknown sender");
    }
    auto receiver_it = wallets_.find(signed_tx.tx.to);
    if (receiver_it == wallets_.end()) {
        throw std::runtime_error("unknown receiver");
    }
    if (!sender_it->second.verify_signature(signed_tx)) {
        throw std::runtime_error("invalid signature");
    }
    if (!sender_it->second.can_afford(signed_tx.tx.amount, signed_tx.tx.fee)) {
        throw std::runtime_error("insufficient sender balance");
    }

    mempool_.add(signed_tx.tx);
}

std::size_t Node::mempool_size() const {
    return mempool_.size();
}

Block Node::forge_block_from_mempool(std::size_t max_transactions) {
    const auto chosen = mempool_.select_for_block(max_transactions);
    const auto payload = encode_transactions(chosen);
    return blockchain_.create_block(payload);
}

void Node::commit_local_block(const Block& block) {
    const auto txs = decode_transactions(block.payload);
    for (const auto& tx : txs) {
        auto& sender = wallet(tx.from);
        auto& receiver = wallet(tx.to);
        sender.apply_debit(tx.amount, tx.fee);
        receiver.apply_credit(tx.amount);
    }
    mempool_.remove_committed(txs);

    const auto compressed = blockchain_.compress_for_transport(block, {"RLE", "RAW"});
    blockchain_.accept_from_network(compressed);
}

const Blockchain& Node::chain() const {
    return blockchain_;
}

ReadinessReport Node::readiness_report(std::size_t min_wallets,
                                       std::size_t max_mempool_threshold,
                                       std::size_t min_chain_height) const {
    std::map<std::string, std::uint64_t> balances;
    for (const auto& [address, wallet] : wallets_) {
        balances[address] = wallet.balance();
    }

    return evaluate_readiness(blockchain_,
                              mempool_.size(),
                              balances,
                              min_wallets,
                              max_mempool_threshold,
                              min_chain_height,
                              {"RLE", "RAW"});
}


std::string Node::encode_transactions(const std::vector<Transaction>& txs) {
    std::ostringstream os;
    os << txs.size() << '\n';
    for (const auto& tx : txs) {
        const auto raw = tx.serialize();
        os << raw.size() << '\n' << raw << '\n';
    }
    return os.str();
}

std::vector<Transaction> Node::decode_transactions(const std::string& payload) {
    std::istringstream is(payload);
    std::size_t count = 0;
    if (!(is >> count)) {
        if (payload.empty()) {
            return {};
        }
        throw std::runtime_error("invalid block payload transaction count");
    }
    is.get();

    std::vector<Transaction> txs;
    txs.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        std::size_t size = 0;
        if (!(is >> size)) {
            throw std::runtime_error("invalid block payload transaction size");
        }
        is.get();

        std::string raw(size, '\0');
        is.read(raw.data(), static_cast<std::streamsize>(size));
        if (is.gcount() != static_cast<std::streamsize>(size)) {
            throw std::runtime_error("invalid block payload transaction body");
        }
        if (is.get() != '\n') {
            throw std::runtime_error("invalid block payload delimiter");
        }

        txs.push_back(Transaction::deserialize(raw));
    }
    return txs;
}

}  // namespace elit21
