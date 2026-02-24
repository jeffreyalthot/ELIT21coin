#include "elit21/mempool.hpp"

#include <algorithm>
#include <stdexcept>

namespace elit21 {

Mempool::Mempool(std::size_t max_transactions) : max_transactions_(max_transactions) {
    if (max_transactions_ == 0) {
        throw std::runtime_error("invalid mempool capacity");
    }
}

void Mempool::add(const Transaction& tx) {
    if (!is_valid_transaction(tx)) {
        throw std::runtime_error("refusing invalid transaction");
    }
    if (contains(tx.id())) {
        throw std::runtime_error("duplicate transaction");
    }
    if (transactions_.size() >= max_transactions_) {
        throw std::runtime_error("mempool full");
    }
    transactions_.push_back(tx);
}

bool Mempool::contains(const std::string& tx_id) const {
    return std::any_of(transactions_.begin(), transactions_.end(), [&](const Transaction& tx) {
        return tx.id() == tx_id;
    });
}

std::size_t Mempool::size() const {
    return transactions_.size();
}

std::vector<Transaction> Mempool::select_for_block(std::size_t limit) const {
    auto selected = transactions_;
    std::stable_sort(selected.begin(), selected.end(), [](const Transaction& a, const Transaction& b) {
        if (a.fee != b.fee) {
            return a.fee > b.fee;
        }
        return a.nonce < b.nonce;
    });
    if (selected.size() > limit) {
        selected.resize(limit);
    }
    return selected;
}

void Mempool::remove_committed(const std::vector<Transaction>& committed) {
    for (const auto& tx : committed) {
        const auto tx_id = tx.id();
        transactions_.erase(
            std::remove_if(transactions_.begin(), transactions_.end(), [&](const Transaction& pending) {
                return pending.id() == tx_id;
            }),
            transactions_.end());
    }
}

}  // namespace elit21
