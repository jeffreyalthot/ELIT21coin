#pragma once

#include <cstdint>
#include <string>

namespace elit21 {

struct CompressedBlock {
    std::uint8_t version{1};
    std::string codec{"RLE"};
    std::string bytes;
};

[[nodiscard]] CompressedBlock compress_block(const std::string& raw_block);
[[nodiscard]] std::string decompress_block(const CompressedBlock& compressed);

}  // namespace elit21
