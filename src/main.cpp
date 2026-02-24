#include "elit21/node.hpp"

#include <cassert>
#include <iostream>

int main() {
    elit21::Node node("RLE");
    node.register_wallet("alice", "alice-secret", 1'000);
    node.register_wallet("bob", "bob-secret", 200);

    auto signed_tx = node.wallet("alice").create_signed_payment("bob", 150, 2, "paiement demo");
    node.submit(signed_tx);

    auto block = node.forge_block_from_mempool(100);
    node.commit_local_block(block);

    assert(node.chain().chain().size() == 2);
    assert(node.wallet("alice").balance() == 848);
    assert(node.wallet("bob").balance() == 350);

    const auto report = node.chain().validate_with_metrics();
    std::cout << "ELIT21 demo node: valid=" << std::boolalpha << report.valid
              << ", blocks_checked=" << report.blocks_checked
              << ", elapsed_us=" << report.elapsed_microseconds << '\n';

    const auto readiness = node.readiness_report();
    std::cout << readiness.to_markdown() << '\n';

    return 0;
}
