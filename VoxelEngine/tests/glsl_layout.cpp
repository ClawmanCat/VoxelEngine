#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/graphics/shader/glsl_layout.hpp>
#include <VoxelEngine/graphics/shader/compiler.hpp>
#include <VoxelEngine/graphics/shader/reflect.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(shader/shader_helpers.hpp)

#include <ctti/nameof.hpp>
#include <magic_enum.hpp>

using namespace ve::defs;


struct S1 {
    float a;
    vec3f b;
    float c;
    float d;
    mat3f e;
    vec3f f;
    float g;
};

std::string_view S1_glsl = R"GLSL(
layout(std140) uniform S1 {
    float a;
    vec3  b;
    float c;
    float d;
    mat3  e;
    vec3  f;
    float g;
};
)GLSL";


struct S2_A { float a; float b; };
struct S2_B { vec3f a; };

struct S2 {
    S2_A  a;
    float b;
    S2_B  c;
    S2_B  d;
    float e;
};

std::string_view S2_glsl = R"GLSL(
struct S2_A { float a; float b; };
struct S2_B { vec3  a; };

layout(std140) uniform S2 {
    S2_A  a;
    float b;
    S2_B  c;
    S2_B  d;
    float e;
};
)GLSL";


struct S3 {
    float a;
    std::array<float, 7> b;
    float c;
};

std::string_view S3_glsl = R"GLSL(
layout(std140) uniform S3 {
    float a;
    float b[7];
    float c;
};
)GLSL";


template <ve::gfx::glsl_layout Layout, typename T>
test_result test_with_layout(std::string glsl, const T& object) {
    glsl = ve::cat(
        "#version 430\n",
        glsl, "\n",
        "void main() {}"
    );

    if (Layout == ve::gfx::glsl_layout::STD430) {
        glsl = ve::replace_substring(glsl, "layout(std140) uniform", "layout(std430) buffer");
    }


    // Compile GLSL to SPIRV so we can reflect over it.
    shaderc::Compiler compiler {};
    auto spirv = compiler.CompileGlslToSpv(glsl, shaderc_vertex_shader, "test_shader", ve::gfxapi::shader_helpers::default_compile_options());

    if (spirv.GetCompilationStatus() != shaderc_compilation_status_success) {
        return VE_TEST_FAIL("Failed to compile shader:", spirv.GetErrorMessage(), "\nShader source:\n", glsl);
    }

    auto reflection = ve::gfx::reflect::generate_stage_reflection(
        &ve::gfxapi::shader_stages[0],
        ve::gfx::spirv_blob { spirv.begin(), spirv.end() }
    );


    // If too much padding or not enough padding was inserted, sizes should not match.
    auto field = (Layout == ve::gfx::glsl_layout::STD140)
        ? &ve::gfx::reflect::stage::uniform_buffers
        : &ve::gfx::reflect::stage::storage_buffers;

    std::size_t expected = (reflection.*field)[0].struct_size;
    std::size_t observed = ve::gfx::to_glsl_layout<Layout>(object).size();

    if (expected == observed) return VE_TEST_SUCCESS;
    else {
        return VE_TEST_FAIL(
            "Converted object ", ctti::nameof<T>(), " did not match expected size for GLSL layout ", magic_enum::enum_name(Layout),
            " (Expected ", expected, ", got ", observed, ")"
        );
    }
}


test_result test_main(void) {
    test_result result = VE_TEST_SUCCESS;

    std::tuple test_structs {
        std::pair { S1{}, S1_glsl },
        std::pair { S2{}, S2_glsl },
        std::pair { S3{}, S3_glsl }
    };


    ve::tuple_foreach(test_structs, [&] (const auto& pair) {
        result |= test_with_layout<ve::gfx::glsl_layout::STD140>(std::string { pair.second }, pair.first);
        // result |= test_with_layout<ve::gfx::glsl_layout::STD430>(std::string { pair.second }, pair.first);
    });


    return result;
}