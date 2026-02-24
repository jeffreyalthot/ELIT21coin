#include "elit21/blockchain.hpp"

#include <cassert>
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

    std::cout << "All tests passed.\n";
    return 0;
}
