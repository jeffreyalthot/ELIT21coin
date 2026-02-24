#include "elit21/blockchain.hpp"

#include <chrono>
#include <stdexcept>

namespace elit21 {

Blockchain::Blockchain() {
    Block genesis;
    genesis.header.index = 0;
    genesis.header.timestamp = 0;
    genesis.header.previous_hash = "GENESIS";
    genesis.payload = "ELIT21coin genesis";
    genesis.hash = compute_hash(genesis.header, genesis.payload);
    chain_.push_back(genesis);
}

Block Blockchain::create_block(const std::string& payload) const {
    Block block;
    block.header.index = static_cast<std::uint32_t>(chain_.size());
    block.header.timestamp = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
    block.header.previous_hash = chain_.back().hash;
    block.payload = payload;
    block.hash = compute_hash(block.header, block.payload);
    return block;
}

CompressedBlock Blockchain::compress_for_transport(const Block& block) const {
    return compress_block(block.serialize());
}

void Blockchain::accept_from_network(const CompressedBlock& compressed_block) {
    const auto raw = decompress_block(compressed_block);
    const auto block = Block::deserialize(raw);

    if (block.header.index != chain_.size()) {
        throw std::runtime_error("index mismatch");
    }
    if (block.header.previous_hash != chain_.back().hash) {
        throw std::runtime_error("previous hash mismatch");
    }
    if (block.hash != compute_hash(block.header, block.payload)) {
        throw std::runtime_error("hash mismatch");
    }

    chain_.push_back(block);
}

bool Blockchain::is_valid() const {
    if (chain_.empty()) {
        return false;
    }
    for (std::size_t i = 1; i < chain_.size(); ++i) {
        const auto& previous = chain_[i - 1];
        const auto& current = chain_[i];
        if (current.header.previous_hash != previous.hash) {
            return false;
        }
        if (compute_hash(current.header, current.payload) != current.hash) {
            return false;
        }
    }
    return true;
}

}  // namespace elit21
