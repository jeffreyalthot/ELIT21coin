#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace elit21 {

struct CompressedBlock {
    std::uint8_t version{1};
    std::string codec{"RLE"};
    std::string bytes;
};

[[nodiscard]] std::vector<std::string> supported_codecs();
[[nodiscard]] bool is_supported_codec(const std::string& codec);
[[nodiscard]] CompressedBlock compress_block(const std::string& raw_block, const std::string& codec = "RLE");
[[nodiscard]] std::string decompress_block(const CompressedBlock& compressed);

}  // namespace elit21
