#include "elit21/blockchain.hpp"
#include "elit21/mempool.hpp"
#include "elit21/node.hpp"
#include "elit21/transaction.hpp"
#include "elit21/wallet.hpp"

#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>

int main() {
    {
        elit21::Blockchain chain;
        auto block = chain.create_block("tx:1");
        auto compressed = chain.compress_for_transport(block);
        chain.accept_from_network(compressed);
        assert(chain.chain().size() == 2);
        assert(chain.is_valid());
    }

    {
        bool caught = false;
        elit21::CompressedBlock bad;
        bad.bytes = "abc";
        try {
            (void)elit21::decompress_block(bad);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
    }

    {
        bool caught = false;
        elit21::Blockchain chain;
        auto block = chain.create_block("tx:2");
        auto compressed = chain.compress_for_transport(block);
        auto tampered_raw = elit21::decompress_block(compressed);
        tampered_raw.back() = tampered_raw.back() == '0' ? '1' : '0';
        compressed = elit21::compress_block(tampered_raw);

        try {
            chain.accept_from_network(compressed);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
    }

    {
        elit21::Blockchain chain("RLE");
        auto block = chain.create_block("tx:raw-fallback");
        auto compressed = chain.compress_for_transport(block, {"RAW", "UNKNOWN"});
        assert(compressed.codec == "RAW");
        chain.accept_from_network(compressed);
        assert(chain.chain().size() == 2);
    }

    {
        bool caught = false;
        elit21::Blockchain chain;
        auto block = chain.create_block("tx:no-common-codec");
        try {
            (void)chain.compress_for_transport(block, {"UNKNOWN"});
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
    }

    {
        elit21::Transaction tx;
        tx.from = "alice";
        tx.to = "bob";
        tx.amount = 10;
        tx.fee = 1;
        tx.nonce = 7;
        tx.memo = "memo::with|pipes";

        const auto raw = tx.serialize();
        const auto decoded = elit21::Transaction::deserialize(raw);
        assert(decoded.from == tx.from);
        assert(decoded.to == tx.to);
        assert(decoded.amount == tx.amount);
        assert(decoded.fee == tx.fee);
        assert(decoded.nonce == tx.nonce);
        assert(decoded.memo == tx.memo);
    }

    {
        bool caught = false;
        elit21::Mempool mempool(3);

        elit21::Transaction low_fee{"alice", "bob", 2, 1, 2, "low"};
        elit21::Transaction high_fee{"alice", "bob", 2, 9, 1, "high"};
        elit21::Transaction mid_fee{"alice", "bob", 2, 4, 3, "mid"};

        mempool.add(low_fee);
        mempool.add(high_fee);
        mempool.add(mid_fee);
        assert(mempool.size() == 3);

        const auto selected = mempool.select_for_block(2);
        assert(selected.size() == 2);
        assert(selected[0].fee >= selected[1].fee);

        try {
            mempool.add(high_fee);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
    }

    {
        elit21::Wallet alice("alice", "s3cr3t", 100);
        auto signed_tx = alice.create_signed_payment("bob", 30, 2, "invoice#42");
        assert(alice.verify_signature(signed_tx));
        assert(alice.nonce() == 1);

        bool caught = false;
        try {
            (void)alice.create_signed_payment("bob", 10'000, 1);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
    }

    {
        elit21::Node node;
        node.register_wallet("alice", "alice-secret", 1000);
        node.register_wallet("bob", "bob-secret", 10);

        auto payment1 = node.wallet("alice").create_signed_payment("bob", 120, 3, "lot-1");
        auto payment2 = node.wallet("alice").create_signed_payment("bob", 40, 1, "lot-2");
        node.submit(payment1);
        node.submit(payment2);
        assert(node.mempool_size() == 2);

        auto block = node.forge_block_from_mempool(100);
        node.commit_local_block(block);

        assert(node.mempool_size() == 0);
        assert(node.wallet("alice").balance() == (1000 - 120 - 3 - 40 - 1));
        assert(node.wallet("bob").balance() == (10 + 120 + 40));
        assert(node.chain().chain().size() == 2);
        assert(node.chain().is_valid());
    }

    {
        bool caught = false;
        elit21::Node node;
        node.register_wallet("alice", "alice-secret", 50);
        node.register_wallet("bob", "bob-secret", 0);

        auto signed_tx = node.wallet("alice").create_signed_payment("bob", 10, 1);
        signed_tx.signature = "tampered";

        try {
            node.submit(signed_tx);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
    }

    {
        bool caught = false;
        try {
            (void)elit21::Blockchain("RLE", 1024 * 1024, 0);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
    }

    {
        bool caught = false;
        elit21::Blockchain chain("RAW", 1024 * 1024, 5);
        const auto& previous = chain.chain().back();

        elit21::Block forged;
        forged.header.index = 1;
        const auto now = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count());
        forged.header.timestamp = now + 3600;
        forged.header.previous_hash = previous.hash;
        forged.payload = "tx:future";
        forged.hash = elit21::compute_hash(forged.header, forged.payload);

        auto compressed = chain.compress_for_transport(forged, {"RAW"});
        try {
            chain.accept_from_network(compressed);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
    }

    std::cout << "All tests passed.\n";
    return 0;
}
