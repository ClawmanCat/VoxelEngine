#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/graphics/texture/texture_utils.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/io/paths.hpp>
#include <VoxelEngine/utility/io/file_io.hpp>
#include <VoxelEngine/utility/color.hpp>


test_result test_main(void) {
    test_result result = VE_TEST_SUCCESS;


    ve::image_rgba8 red16x16  { .data = std::vector<ve::RGBA8>(16 * 16, ve::colors::PURE_RED),   .size = { 16, 16 } };
    ve::image_rgba8 green32x8 { .data = std::vector<ve::RGBA8>(32 * 8,  ve::colors::PURE_GREEN), .size = { 32, 8  } };
    ve::image_rgba8 blue4x4   { .data = std::vector<ve::RGBA8>(4  * 4,  ve::colors::PURE_BLUE),  .size = { 4,  4  } };

    ve::image_rgba8 mixed_16x16  { .data = std::vector<ve::RGBA8>(32 * 8,  ve::colors::BLACK), .size = { 16, 16 } };
    mixed_16x16.foreach([&] (const auto& where, auto& pixel) {
        pixel = (where.x < mixed_16x16.size.x / 2) ? ve::colors::PURE_RED : ve::colors::PURE_GREEN;
    });


    // Test without scaling: Move red16x16 red channel into RGB, result should be white.
    auto result_image_1 = ve::gfx::combine_images({
        ve::gfx::combine_image_data { .src = &red16x16, .source_channels = { 1, 0, 0, 0 }, .dest_channels = { 1, 0, 0, 0 } },
        ve::gfx::combine_image_data { .src = &red16x16, .source_channels = { 1, 0, 0, 0 }, .dest_channels = { 0, 1, 0, 0 } },
        ve::gfx::combine_image_data { .src = &red16x16, .source_channels = { 1, 0, 0, 0 }, .dest_channels = { 0, 0, 1, 0 } }
    }, red16x16.size);

    result_image_1.foreach([&] (const auto& where, const auto& pixel) {
        if (pixel != ve::colors::WHITE) result |= VE_TEST_FAIL("Pixel ", where, " is in an incorrect state: expected ", ve::colors::WHITE, ", got ", pixel);
    });

    if (result_image_1.size != red16x16.size) result |= VE_TEST_FAIL("Combined image has an incorrect size");

    if (result != VE_TEST_SUCCESS) {
        ve::io::write_image(ve::io::paths::PATH_LOGS / "test_texture_utils_merge_output.png", result_image_1);
        return result;
    }


    // Test with downscaling: Move mixed16x16 into RGB, and resize to 4x8.
    // Left half should be red, right half should be green.
    auto result_image_2 = ve::gfx::combine_images({
        ve::gfx::combine_image_data { .src = &mixed_16x16, .source_channels = { 1, 0, 0, 0 }, .dest_channels = { 1, 0, 0, 0 } },
        ve::gfx::combine_image_data { .src = &mixed_16x16, .source_channels = { 0, 1, 0, 0 }, .dest_channels = { 0, 1, 0, 0 } },
        ve::gfx::combine_image_data { .src = &mixed_16x16, .source_channels = { 0, 0, 1, 0 }, .dest_channels = { 0, 0, 1, 0 } }
    }, ve::vec2ui { 4, 8 });

    result_image_2.foreach([&] (const auto& where, const auto& pixel) {
        const auto& expected = (where.x < result_image_2.size.x / 2) ? ve::colors::PURE_RED : ve::colors::PURE_GREEN;
        if (pixel != expected) result |= VE_TEST_FAIL("Pixel ", where, " is in an incorrect state: expected ", expected, ", got ", pixel);
    });

    if (result_image_2.size != ve::vec2ui { 4, 8 }) result |= VE_TEST_FAIL("Combined image has an incorrect size");

    if (result != VE_TEST_SUCCESS) {
        ve::io::write_image(ve::io::paths::PATH_LOGS / "test_texture_utils_merge_output.png", result_image_2);
        return result;
    }


    // Test with upscaling: Move mixed16x16 into RGB, and resize to 32x32.
    // Left half should be red, right half should be green.
    auto result_image_3 = ve::gfx::combine_images({
        ve::gfx::combine_image_data { .src = &mixed_16x16, .source_channels = { 1, 0, 0, 0 }, .dest_channels = { 1, 0, 0, 0 } },
        ve::gfx::combine_image_data { .src = &mixed_16x16, .source_channels = { 0, 1, 0, 0 }, .dest_channels = { 0, 1, 0, 0 } },
        ve::gfx::combine_image_data { .src = &mixed_16x16, .source_channels = { 0, 0, 1, 0 }, .dest_channels = { 0, 0, 1, 0 } }
    }, ve::vec2ui { 32, 32 });

    result_image_3.foreach([&] (const auto& where, const auto& pixel) {
        const auto& expected = (where.x < result_image_3.size.x / 2) ? ve::colors::PURE_RED : ve::colors::PURE_GREEN;
        if (pixel != expected) result |= VE_TEST_FAIL("Pixel ", where, " is in an incorrect state: expected ", expected, ", got ", pixel);
    });

    if (result_image_3.size != ve::vec2ui { 32, 32 }) result |= VE_TEST_FAIL("Combined image has an incorrect size");

    if (result != VE_TEST_SUCCESS) {
        ve::io::write_image(ve::io::paths::PATH_LOGS / "test_texture_utils_merge_output.png", result_image_3);
        return result;
    }


    // Test with multiple differently sized images. Result should be white.
    auto result_image_4 = ve::gfx::combine_images({
        ve::gfx::combine_image_data { .src = &red16x16,  .source_channels = { 1, 0, 0, 0 }, .dest_channels = { 0, 0, 1, 0 } },
        ve::gfx::combine_image_data { .src = &green32x8, .source_channels = { 0, 1, 0, 0 }, .dest_channels = { 0, 1, 0, 0 } },
        ve::gfx::combine_image_data { .src = &blue4x4,   .source_channels = { 0, 0, 1, 0 }, .dest_channels = { 1, 0, 0, 0 } }
    }, ve::vec2ui { 20, 20 });

    result_image_4.foreach([&] (const auto& where, const auto& pixel) {
        if (pixel != ve::colors::WHITE) result |= VE_TEST_FAIL("Pixel ", where, " is in an incorrect state: expected", ve::colors::WHITE, ", got ", pixel);
    });

    if (result_image_4.size != ve::vec2ui { 20, 20 }) result |= VE_TEST_FAIL("Combined image has an incorrect size");

    if (result != VE_TEST_SUCCESS) {
        ve::io::write_image(ve::io::paths::PATH_LOGS / "test_texture_utils_merge.png", result_image_4);
        return result;
    }


    return VE_TEST_SUCCESS;
}