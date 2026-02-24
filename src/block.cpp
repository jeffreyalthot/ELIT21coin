#include "elit21/block.hpp"

#include <functional>
#include <sstream>
#include <stdexcept>

namespace elit21 {

std::string Block::serialize() const {
    std::ostringstream os;
    os << header.index << '|'
       << header.timestamp << '|'
       << header.previous_hash << '|'
       << payload << '|'
       << hash;
    return os.str();
}

Block Block::deserialize(const std::string& raw) {
    std::istringstream is(raw);
    std::string token;
    Block block;

    if (!std::getline(is, token, '|')) throw std::runtime_error("invalid block: index");
    block.header.index = static_cast<std::uint32_t>(std::stoul(token));

    if (!std::getline(is, token, '|')) throw std::runtime_error("invalid block: timestamp");
    block.header.timestamp = static_cast<std::uint64_t>(std::stoull(token));

    if (!std::getline(is, block.header.previous_hash, '|')) throw std::runtime_error("invalid block: previous_hash");
    if (!std::getline(is, block.payload, '|')) throw std::runtime_error("invalid block: payload");
    if (!std::getline(is, block.hash)) throw std::runtime_error("invalid block: hash");

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
