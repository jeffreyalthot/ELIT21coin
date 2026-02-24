#pragma once

#include "elit21/block.hpp"
#include "elit21/codec.hpp"

#include <cstddef>
#include <vector>

namespace elit21 {

class Blockchain {
  public:
    explicit Blockchain(std::string preferred_codec = "RLE", std::size_t max_transport_block_bytes = 1024 * 1024);

    [[nodiscard]] const std::vector<Block>& chain() const { return chain_; }
    [[nodiscard]] Block create_block(const std::string& payload) const;
    [[nodiscard]] CompressedBlock compress_for_transport(const Block& block) const;
    [[nodiscard]] CompressedBlock compress_for_transport(const Block& block, const std::vector<std::string>& peer_codecs) const;

    void accept_from_network(const CompressedBlock& compressed_block);
    [[nodiscard]] bool is_valid() const;

  private:
    [[nodiscard]] std::string negotiate_codec(const std::vector<std::string>& peer_codecs) const;

    std::vector<Block> chain_;
    std::string preferred_codec_;
    std::size_t max_transport_block_bytes_;
};

}  // namespace elit21
