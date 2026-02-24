#include "elit21/blockchain.hpp"

#include <iostream>

int main() {
    elit21::Blockchain chain;

    auto candidate = chain.create_block("transaction:alice->bob:42");
    auto compressed = chain.compress_for_transport(candidate);
    chain.accept_from_network(compressed);

    std::cout << "Chain size: " << chain.chain().size() << "\n";
    std::cout << "Chain valid: " << (chain.is_valid() ? "yes" : "no") << "\n";
    std::cout << "Compressed payload bytes: " << compressed.bytes.size() << "\n";
    return chain.is_valid() ? 0 : 1;
}
