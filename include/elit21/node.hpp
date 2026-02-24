#pragma once

#include "elit21/blockchain.hpp"
#include "elit21/mempool.hpp"
#include "elit21/wallet.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace elit21 {

class Node {
  public:
    explicit Node(std::string preferred_codec = "RLE");

    void register_wallet(const std::string& address, const std::string& secret, std::uint64_t initial_balance);
    [[nodiscard]] Wallet& wallet(const std::string& address);
    [[nodiscard]] const Wallet& wallet(const std::string& address) const;

    void submit(const SignedTransaction& signed_tx);
    [[nodiscard]] std::size_t mempool_size() const;

    [[nodiscard]] Block forge_block_from_mempool(std::size_t max_transactions);
    void commit_local_block(const Block& block);

    [[nodiscard]] const Blockchain& chain() const;

  private:
    [[nodiscard]] static std::string encode_transactions(const std::vector<Transaction>& txs);
    [[nodiscard]] static std::vector<Transaction> decode_transactions(const std::string& payload);

    Blockchain blockchain_;
    Mempool mempool_;
    std::map<std::string, Wallet> wallets_;
};

}  // namespace elit21
