#include "elit21/blockchain.hpp"

#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <vector>

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
        bad.bytes = "abc";  // odd length -> corruption
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
        bool caught = false;
        elit21::CompressedBlock invalid;
        invalid.codec = "UNSUPPORTED";
        invalid.bytes = "abc";
        try {
            (void)elit21::decompress_block(invalid);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
    }


    {
        bool caught = false;
        elit21::CompressedBlock oversized_raw;
        oversized_raw.codec = "RAW";
        oversized_raw.bytes = "012345";
        try {
            (void)elit21::decompress_block(oversized_raw, 4);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
    }

    {
        bool caught = false;
        elit21::Blockchain chain;
        auto block = chain.create_block("AAAAAAAAAAAA");
        auto compressed = chain.compress_for_transport(block);
        try {
            elit21::Blockchain strict_chain("RLE", 8);
            strict_chain.accept_from_network(compressed);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
    }

    {
        bool caught = false;
        try {
            (void)elit21::Blockchain("RLE", 0);
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


    {
        elit21::Blockchain chain;
        auto block = chain.create_block("tx:alice|bob\nmetadata:1");
        auto compressed = chain.compress_for_transport(block, {"RAW"});
        chain.accept_from_network(compressed);
        assert(chain.chain().back().payload == "tx:alice|bob\nmetadata:1");
        assert(chain.is_valid());
    }

    {
        bool caught = false;
        elit21::Block block;
        block.header.index = 1;
        block.header.timestamp = 42;
        block.header.previous_hash = "prev";
        block.payload = "abc";
        block.hash = elit21::compute_hash(block.header, block.payload);

        auto raw = block.serialize();
        const auto marker = std::string("|3|");
        const auto pos = raw.find(marker);
        assert(pos != std::string::npos);
        raw.replace(pos, marker.size(), "|10|");

        try {
            (void)elit21::Block::deserialize(raw);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
    }


    {
        elit21::Blockchain chain;
        auto report = chain.validate_with_metrics();
        assert(report.valid);
        assert(report.blocks_checked == 1);
        assert(report.failure_reason.empty());
    }

    std::cout << "All tests passed.\n";
    return 0;
}
