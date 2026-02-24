#include "elit21/blockchain.hpp"

#include <iostream>
#include <vector>

int main() {
    elit21::Blockchain chain;

    auto candidate = chain.create_block("transaction:alice->bob:42");
    const std::vector<std::string> peer_codecs{"RAW", "RLE"};
    auto compressed = chain.compress_for_transport(candidate, peer_codecs);
    chain.accept_from_network(compressed);

    std::cout << "Chain size: " << chain.chain().size() << "\n";
    std::cout << "Chain valid: " << (chain.is_valid() ? "yes" : "no") << "\n";
    std::cout << "Negotiated codec: " << compressed.codec << "\n";
    std::cout << "Transport bytes: " << compressed.bytes.size() << "\n";
    return chain.is_valid() ? 0 : 1;
}
