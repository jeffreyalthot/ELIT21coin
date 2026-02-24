#pragma once

#include "elit21/blockchain.hpp"
#include "elit21/codec.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace elit21 {

struct ReadinessGate {
    std::string name;
    bool passed{false};
    std::string detail;
};

struct ReadinessReport {
    bool ready_for_development{false};
    std::size_t wallets_registered{0};
    std::size_t mempool_size{0};
    std::size_t chain_height{0};
    ValidationReport validation;
    std::vector<ReadinessGate> gates;
    std::map<std::string, std::uint64_t> balances;

    [[nodiscard]] std::string to_markdown() const;
};

[[nodiscard]] ReadinessReport evaluate_readiness(const Blockchain& chain,
                                                 std::size_t mempool_size,
                                                 const std::map<std::string, std::uint64_t>& balances,
                                                 std::size_t min_wallets,
                                                 std::size_t max_mempool_threshold,
                                                 std::size_t min_chain_height,
                                                 const std::vector<std::string>& required_codecs = {"RLE", "RAW"});

}  // namespace elit21
