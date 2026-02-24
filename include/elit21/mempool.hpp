#pragma once

#include "elit21/transaction.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace elit21 {

class Mempool {
  public:
    explicit Mempool(std::size_t max_transactions = 10'000);

    void add(const Transaction& tx);
    [[nodiscard]] bool contains(const std::string& tx_id) const;
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] std::vector<Transaction> select_for_block(std::size_t limit) const;
    void remove_committed(const std::vector<Transaction>& committed);

  private:
    std::size_t max_transactions_;
    std::vector<Transaction> transactions_;
};

}  // namespace elit21
