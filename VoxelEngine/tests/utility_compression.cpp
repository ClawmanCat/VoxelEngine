#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/compression.hpp>
#include <VoxelEngine/utility/random.hpp>


using namespace ve::defs;


test_result test_main(void) {
    std::vector<std::string> test_strings {
        "This is a test test test string.",
        "",
        "Wow!",
        "Bing Chilling Bing Chilling Bing Chilling Bing Chilling Bing Chilling Bing Chilling"
    };


    std::string random_string;
    random_string.reserve(1 << 16);

    for (std::size_t i = 0; i < (1 << 16); ++i) {
        random_string += ve::cheaprand::random_element("ABCDEFGHIJKLMNOPQRSTUVWXYZ "s);
    }

    test_strings.push_back(std::move(random_string));


    for (const auto& str : test_strings) {
        std::vector<u8> bytes(str.size(), 0x00);
        memcpy(bytes.data(), str.data(), str.size());

        auto compressed   = ve::compress(std::span<const u8> { bytes.begin(), bytes.end() });
        auto decompressed = ve::decompress(std::span<const u8> { compressed.begin(), compressed.end() });

        if (decompressed != bytes) {
            VE_LOG_DEBUG(ve::cat("i: ", bytes.size(), ", o: ", decompressed.size(), ", c: ", compressed.size()));

            for (std::size_t i = 0; i < bytes.size(); ++i) {
                VE_LOG_DEBUG(ve::cat(i, ": ", decompressed[i] == bytes[i] ? "T" : "F"));
            }

            return VE_TEST_FAIL("Decompressed data did not match original data.");
        }
    }


    return VE_TEST_SUCCESS;
}