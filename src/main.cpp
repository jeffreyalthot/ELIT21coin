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

    const auto report = chain.validate_with_metrics();
    std::cout << "Validation blocks checked: " << report.blocks_checked << "\n";
    std::cout << "Validation latency (us): " << report.elapsed_microseconds << "\n";
    if (!report.valid) {
        std::cout << "Validation failed at block: " << report.failed_block_index << "\n";
        std::cout << "Validation reason: " << report.failure_reason << "\n";
    }
    return report.valid ? 0 : 1;
}
