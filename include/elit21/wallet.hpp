#pragma once

#include "elit21/transaction.hpp"

#include <cstdint>
#include <string>

namespace elit21 {

struct SignedTransaction {
    Transaction tx;
    std::string signature;
};

class Wallet {
  public:
    Wallet(std::string address, std::string secret, std::uint64_t initial_balance = 0);

    [[nodiscard]] const std::string& address() const;
    [[nodiscard]] std::uint64_t balance() const;
    [[nodiscard]] std::uint64_t nonce() const;

    [[nodiscard]] SignedTransaction create_signed_payment(const std::string& to,
                                                          std::uint64_t amount,
                                                          std::uint64_t fee,
                                                          const std::string& memo = "");

    [[nodiscard]] bool can_afford(std::uint64_t amount, std::uint64_t fee) const;
    void apply_debit(std::uint64_t amount, std::uint64_t fee);
    void apply_credit(std::uint64_t amount);

    [[nodiscard]] bool verify_signature(const SignedTransaction& signed_tx) const;

  private:
    [[nodiscard]] std::string sign(const Transaction& tx) const;

    std::string address_;
    std::string secret_;
    std::uint64_t balance_;
    std::uint64_t nonce_;
};

}  // namespace elit21
