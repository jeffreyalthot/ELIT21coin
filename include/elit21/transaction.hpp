#pragma once

#include <cstdint>
#include <string>

namespace elit21 {

struct Transaction {
    std::string from;
    std::string to;
    std::uint64_t amount{0};
    std::uint64_t fee{0};
    std::uint64_t nonce{0};
    std::string memo;

    [[nodiscard]] std::string id() const;
    [[nodiscard]] std::string serialize() const;
    static Transaction deserialize(const std::string& raw);
};

[[nodiscard]] bool is_valid_transaction(const Transaction& tx);

}  // namespace elit21
