#pragma once

#include "elit21/block.hpp"
#include "elit21/codec.hpp"

#include <vector>

namespace elit21 {

class Blockchain {
  public:
    Blockchain();

    [[nodiscard]] const std::vector<Block>& chain() const { return chain_; }
    [[nodiscard]] Block create_block(const std::string& payload) const;
    [[nodiscard]] CompressedBlock compress_for_transport(const Block& block) const;

    void accept_from_network(const CompressedBlock& compressed_block);
    [[nodiscard]] bool is_valid() const;

  private:
    std::vector<Block> chain_;
};

}  // namespace elit21
