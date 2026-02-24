#include "elit21/codec.hpp"

#include <algorithm>
#include <stdexcept>

namespace elit21 {

std::vector<std::string> supported_codecs() {
    return {"RLE", "RAW"};
}

bool is_supported_codec(const std::string& codec) {
    const auto codecs = supported_codecs();
    return std::find(codecs.begin(), codecs.end(), codec) != codecs.end();
}

CompressedBlock compress_block(const std::string& raw_block, const std::string& codec) {
    if (!is_supported_codec(codec)) {
        throw std::runtime_error("unsupported codec");
    }

    CompressedBlock out;
    out.codec = codec;

    if (codec == "RAW") {
        out.bytes = raw_block;
        return out;
    }

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

std::string decompress_block(const CompressedBlock& compressed, std::size_t max_output_bytes) {
    if (compressed.version != 1) {
        throw std::runtime_error("unsupported compressed block version");
    }
    if (!is_supported_codec(compressed.codec)) {
        throw std::runtime_error("unsupported codec");
    }

    if (compressed.codec == "RAW") {
        if (compressed.bytes.size() > max_output_bytes) {
            throw std::runtime_error("decompressed payload exceeds configured limit");
        }
        return compressed.bytes;
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
        if (raw.size() + count > max_output_bytes) {
            throw std::runtime_error("decompressed payload exceeds configured limit");
        }
        raw.append(count, value);
    }
    return raw;
}

}  // namespace elit21
