#include "elit21/block.hpp"

#include <cstddef>
#include <functional>
#include <sstream>
#include <stdexcept>

namespace elit21 {

std::string Block::serialize() const {
    std::ostringstream os;
    os << header.index << '|'
       << header.timestamp << '|'
       << header.previous_hash.size() << '|'
       << header.previous_hash << '|'
       << payload.size() << '|'
       << payload << '|'
       << hash;
    return os.str();
}

Block Block::deserialize(const std::string& raw) {
    std::size_t cursor = 0;
    auto consume_token = [&](const char* field_name) {
        const auto separator = raw.find('|', cursor);
        if (separator == std::string::npos) {
            throw std::runtime_error(std::string("invalid block: ") + field_name);
        }
        const auto token = raw.substr(cursor, separator - cursor);
        cursor = separator + 1;
        return token;
    };

    auto consume_sized_field = [&](const std::size_t size, const char* field_name) {
        if (raw.size() - cursor < size + 1) {
            throw std::runtime_error(std::string("invalid block: ") + field_name);
        }
        const auto token = raw.substr(cursor, size);
        cursor += size;
        if (raw[cursor] != '|') {
            throw std::runtime_error(std::string("invalid block: ") + field_name);
        }
        ++cursor;
        return token;
    };

    std::string token;
    Block block;

    token = consume_token("index");
    block.header.index = static_cast<std::uint32_t>(std::stoul(token));

    token = consume_token("timestamp");
    block.header.timestamp = static_cast<std::uint64_t>(std::stoull(token));

    token = consume_token("previous_hash_size");
    const auto previous_hash_size = static_cast<std::size_t>(std::stoull(token));
    block.header.previous_hash = consume_sized_field(previous_hash_size, "previous_hash");

    token = consume_token("payload_size");
    const auto payload_size = static_cast<std::size_t>(std::stoull(token));
    block.payload = consume_sized_field(payload_size, "payload");

    if (cursor >= raw.size()) {
        throw std::runtime_error("invalid block: hash");
    }
    block.hash = raw.substr(cursor);
    if (block.hash.empty()) {
        throw std::runtime_error("invalid block: hash");
    }

    return block;
}

std::string compute_hash(const BlockHeader& header, const std::string& payload) {
    const std::string seed = std::to_string(header.index) +
                             std::to_string(header.timestamp) +
                             header.previous_hash +
                             payload;
    return std::to_string(std::hash<std::string>{}(seed));
}

}  // namespace elit21
