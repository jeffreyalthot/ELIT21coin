#include "elit21/codec.hpp"

#include <stdexcept>

namespace elit21 {

CompressedBlock compress_block(const std::string& raw_block) {
    CompressedBlock out;
    if (raw_block.empty()) {
        return out;
    }

    out.bytes.reserve(raw_block.size());
    char current = raw_block.front();
    std::uint8_t run = 1;

    for (std::size_t i = 1; i < raw_block.size(); ++i) {
        if (raw_block[i] == current && run < 255) {
            ++run;
        } else {
            out.bytes.push_back(static_cast<char>(run));
            out.bytes.push_back(current);
            current = raw_block[i];
            run = 1;
        }
    }

    out.bytes.push_back(static_cast<char>(run));
    out.bytes.push_back(current);
    return out;
}

std::string decompress_block(const CompressedBlock& compressed) {
    if (compressed.version != 1) {
        throw std::runtime_error("unsupported compressed block version");
    }
    if (compressed.codec != "RLE") {
        throw std::runtime_error("unsupported codec");
    }
    if (compressed.bytes.size() % 2 != 0) {
        throw std::runtime_error("corrupted compressed bytes");
    }

    std::string raw;
    for (std::size_t i = 0; i < compressed.bytes.size(); i += 2) {
        const auto count = static_cast<unsigned char>(compressed.bytes[i]);
        if (count == 0) {
            throw std::runtime_error("invalid run-length 0");
        }
        const char value = compressed.bytes[i + 1];
        raw.append(count, value);
    }
    return raw;
}

}  // namespace elit21
