#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>
#include <VoxelEngine/graphics/texture/aligned_texture_atlas.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/io/file_io.hpp>
#include <VoxelEngine/utility/io/paths.hpp>
#include <VoxelEngine/utility/color.hpp>


test_result test_main(void) {
    // Create graphics context to prevent writing to the atlas from failing.
    auto window = ve::gfx::window::create(ve::gfx::window::arguments { .title = "Test Window" });


    ve::image_rgba8 image {
        .data = std::vector<ve::RGBA8>{ 8 * 8, ve::colors::PURE_RED },
        .size = ve::vec2ui { 8, 8 }
    };

    // 32x32 atlas should store (32 / 8)^2 = 16 textures.
    ve::gfx::aligned_texture_atlas atlas { "Atlas", ve::vec2ui { 32 }, 8 };

    try {
        // Run twice to check that clearing the atlas actually works.
        for (std::size_t i = 0; i < 2; ++i) {
            std::vector<ve::gfx::subtexture> subtextures;

            for (std::size_t j = 0; j < 16; ++j) {
                subtextures.push_back(atlas.store(image));
            }

            for (const auto &st : subtextures) atlas.remove(st);
        }
    } catch (const ve::gfx::atlas_full_error& e) {
        ve::io::write_image(
            ve::io::paths::PATH_LOGS / "test_texture_atlas_packing_output.png",
            atlas.get_texture()->read()
        );

        return VE_TEST_FAIL("Atlas marked itself as full but not all textures that should've fit were inserted.");
    }


    return VE_TEST_SUCCESS;
}