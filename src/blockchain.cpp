#include "elit21/blockchain.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <utility>

namespace elit21 {

Blockchain::Blockchain(std::string preferred_codec,
                       std::size_t max_transport_block_bytes,
                       std::uint64_t max_future_drift_seconds)
    : preferred_codec_(std::move(preferred_codec)),
      max_transport_block_bytes_(max_transport_block_bytes),
      max_future_drift_seconds_(max_future_drift_seconds) {
    if (!is_supported_codec(preferred_codec_)) {
        throw std::runtime_error("unsupported preferred codec");
    }
    if (max_transport_block_bytes_ == 0) {
        throw std::runtime_error("max transport block bytes must be > 0");
    }
    if (max_future_drift_seconds_ == 0) {
        throw std::runtime_error("max future drift must be > 0");
    }

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
    return compress_block(block.serialize(), preferred_codec_);
}

CompressedBlock Blockchain::compress_for_transport(const Block& block, const std::vector<std::string>& peer_codecs) const {
    return compress_block(block.serialize(), negotiate_codec(peer_codecs));
}

std::string Blockchain::negotiate_codec(const std::vector<std::string>& peer_codecs) const {
    if (std::find(peer_codecs.begin(), peer_codecs.end(), preferred_codec_) != peer_codecs.end()) {
        return preferred_codec_;
    }

    for (const auto& codec : peer_codecs) {
        if (is_supported_codec(codec)) {
            return codec;
        }
    }

    throw std::runtime_error("no mutually supported codec");
}

void Blockchain::accept_from_network(const CompressedBlock& compressed_block) {
    const auto now = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
    const auto raw = decompress_block(compressed_block, max_transport_block_bytes_);
    const auto block = Block::deserialize(raw);

    if (block.header.index != chain_.size()) {
        throw std::runtime_error("index mismatch");
    }
    if (block.header.previous_hash != chain_.back().hash) {
        throw std::runtime_error("previous hash mismatch");
    }
    if (block.header.timestamp < chain_.back().header.timestamp) {
        throw std::runtime_error("timestamp regression");
    }
    if (block.header.timestamp > now + max_future_drift_seconds_) {
        throw std::runtime_error("timestamp too far in the future");
    }
    if (block.hash != compute_hash(block.header, block.payload)) {
        throw std::runtime_error("hash mismatch");
    }

    chain_.push_back(block);
}

bool Blockchain::is_valid() const {
    return validate_with_metrics().valid;
}

ValidationReport Blockchain::validate_with_metrics() const {
    const auto start = std::chrono::steady_clock::now();

    ValidationReport report;
    report.blocks_checked = chain_.size();

    if (chain_.empty()) {
        report.failure_reason = "empty chain";
    } else if (chain_.front().header.index != 0 || chain_.front().header.previous_hash != "GENESIS") {
        report.failure_reason = "invalid genesis header";
    } else if (compute_hash(chain_.front().header, chain_.front().payload) != chain_.front().hash) {
        report.failure_reason = "invalid genesis hash";
    } else {
        report.valid = true;
        const auto now = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count());

        for (std::size_t i = 1; i < chain_.size(); ++i) {
            const auto& previous = chain_[i - 1];
            const auto& current = chain_[i];
            if (current.header.index != i) {
                report.valid = false;
                report.failed_block_index = i;
                report.failure_reason = "index mismatch";
                break;
            }
            if (current.header.timestamp < previous.header.timestamp) {
                report.valid = false;
                report.failed_block_index = i;
                report.failure_reason = "timestamp regression";
                break;
            }
            if (current.header.timestamp > now + max_future_drift_seconds_) {
                report.valid = false;
                report.failed_block_index = i;
                report.failure_reason = "timestamp too far in the future";
                break;
            }
            if (current.header.previous_hash != previous.hash) {
                report.valid = false;
                report.failed_block_index = i;
                report.failure_reason = "previous hash mismatch";
                break;
            }
            if (compute_hash(current.header, current.payload) != current.hash) {
                report.valid = false;
                report.failed_block_index = i;
                report.failure_reason = "hash mismatch";
                break;
            }
        }
    }

    const auto end = std::chrono::steady_clock::now();
    report.elapsed_microseconds = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    return report;
}

}  // namespace elit21
