#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace elit21 {

struct BlockHeader {
    std::uint32_t index{};
    std::uint64_t timestamp{};
    std::string previous_hash;
};

struct Block {
    BlockHeader header;
    std::string payload;
    std::string hash;

    [[nodiscard]] std::string serialize() const;
    static Block deserialize(const std::string& raw);
};

[[nodiscard]] std::string compute_hash(const BlockHeader& header, const std::string& payload);

}  // namespace elit21
