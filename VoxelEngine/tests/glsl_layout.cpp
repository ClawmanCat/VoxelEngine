#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/graphics/graphics.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>

using namespace ve::defs;


// ===========================================================================
//                                  Test Data
// ===========================================================================
struct S1 {
    float a = 3.5f;
    vec3f b = { 1.0f, 2.0f, 3.0f };
    float c = 100.0f;
    float d = 200.0f;
    mat3f e = glm::identity<mat3f>();
    vec3f f = { 1.0f, 2.0f, 3.0f };
    float g = 1e6f;
};

std::string_view S1_glsl = R"GLSL(
%LAYOUT% S1 {
    float a;
    vec3  b;
    float c;
    float d;
    mat3  e;
    vec3  f;
    float g;
} ubo;
)GLSL";


struct S2_A { float a = 2.0f; float b = 4.0f; };
struct S2_B { vec3f a = { 1.0f, 2.0f, 3.0f }; };

struct S2 {
    S2_A  a = S2_A { };
    float b = 32.0f;
    S2_B  c = S2_B { };
    S2_B  d = S2_B { };
    float e = 1000.0f;
};

std::string_view S2_glsl = R"GLSL(
struct S2_A { float a; float b; };
struct S2_B { vec3  a; };

%LAYOUT% S2 {
    S2_A  a;
    float b;
    S2_B  c;
    S2_B  d;
    float e;
} ubo;
)GLSL";


struct S3 {
    float a = 300.0f;
    std::array<float, 5> b = { 100.0f, 200.0f, 300.0f, 400.0f, 500.0f };
    float c = 0.0f;
    float d = 1.0f;
    float e = 2.0f;
    std::array<float, 3> f = { 100.0f, 200.0f, 300.0f };
};

std::string_view S3_glsl = R"GLSL(
%LAYOUT% S3 {
    float a;
    float b[5];
    float c;
    float d;
    float e;
    float f[3];
} ubo;
)GLSL";


struct S4_member {
    vec3f a = { 20.0f, 40.0f, 60.0f };
    vec3f b = { 0.0f,  1.0f,  2.0f  };
    float c = 1e6f;
};

struct S4 {
    std::array<S4_member, 8> a = ve::create_filled_array<8>(ve::produce(S4_member { }));
    u32   b = 256'256'256;
    vec3f c = { 3.0f, 6.0f, 9.0f };
    float d = 255.0f;
    float e = 0.125f;
};

std::string_view S4_glsl = R"GLSL(
struct S4_member {
    vec3 a, b;
    float c;
};

struct S4_struct {
    S4_member a[8];
    uint b;
    vec3 c;
    float d, e;
};

%LAYOUT% S4 { S4_struct a; } ubo;
)GLSL";


// ===========================================================================
//                               Shader Sources
// ===========================================================================
std::string_view vertex_shader = R"GLSL(
#version 430

#include "utility/multipass.util.glsl"

%UNIFORM_DEF%
%UNIFORM_EXPECT_TABLE%

out flat float difference;

void main() {
    difference  = check_uniforms();
    gl_Position = vec4(quad_vertices[gl_VertexID], 0.0, 1.0);
}
)GLSL";


std::string_view fragment_shader = R"GLSL(
#version 430

in flat float difference;
out vec4 color;

void main() { color = (difference > 1e-3) ? vec4(1.0, 0.0, 0.0, 1.0) : vec4(0.0, 1.0, 0.0, 1.0); }
)GLSL";


// ===========================================================================
//                                  Test Code
// ===========================================================================
template <ve::gfx::glsl_layout Layout> void insert_uniform_def(std::string& glsl, std::string_view uniform_def) {
    glsl = ve::replace_substring(glsl, "%UNIFORM_DEF%", uniform_def);

    glsl = ve::replace_substring(
        glsl,
        "%LAYOUT%",
        Layout == ve::gfx::glsl_layout::STD140 ? "layout (std140) uniform" : "layout (std430) buffer"
    );
}


template <typename UBO> void insert_uniform_expect_table(std::string& glsl, const UBO& elem, std::string_view ubo_name = "") {
    std::stringstream stream;


    auto insert_element = [&] <typename Elem> (const Elem& elem, std::string_view name, auto self) {
        constexpr std::string_view names = "abcdefghijklmnopqrstuvwxyz";
        using traits = ve::meta::glm_traits<Elem>;

        if constexpr (std::is_fundamental_v<Elem>) {
            stream << "result += diff(" << name << ", " << elem << ");\n";
        }

        else if constexpr (traits::is_glm) {
            for (std::size_t i = 0; i < traits::num_cols; ++i) {
                for (std::size_t j = 0; j < traits::num_rows; ++j) {
                    typename traits::value_type value;

                    if constexpr (traits::is_matrix) value = elem[i][j];
                    else value = elem[j];

                    stream << "result += diff(" << name;
                    if (traits::is_matrix) stream << "[" << i << "]";
                    stream << "[" << j << "], " << value << ");\n";
                }
            }
        }

        else if constexpr (ve::meta::is_std_array_v<Elem>) {
            for (std::size_t i = 0; i < elem.size(); ++i) {
                std::invoke(self, elem[i], ve::cat(name, "[", i, "]"), self);
            }
        }

        else {
            using decomposer = ve::decomposer_for<Elem>;

            ve::meta::create_pack::from_decomposable<Elem>::foreach_indexed([&] <typename T, std::size_t I> {
                const T& value = decomposer::template get<I>(elem);

                std::string next_name = ve::cat(name, name.empty() ? "" : ".", std::string { } + names[I]);
                std::invoke(self, value, next_name, self);
            });
        }
    };


    stream << "float diff(float x, float y) { return abs(x - y); }\n";

    stream << "float check_uniforms() {\n";
    stream << "float result = 0.0;\n";
    insert_element(elem, ubo_name, insert_element);
    stream << "return result;\n";
    stream << "}\n";

    glsl = ve::replace_substring(glsl, "%UNIFORM_EXPECT_TABLE%", stream.str());
}


// Renders to a single pixel texture using a shader that checks the uniform value against the value of object.
// If the values match, vec4(1) is rendered, otherwise vec4(0) is rendered.
template <ve::gfx::glsl_layout Layout, typename T>
test_result test_with_layout(std::string uniform_def, const T& object, std::string_view prefix, std::string_view name) {
    std::string vs = std::string { vertex_shader };
    insert_uniform_def<Layout>(vs, uniform_def);
    insert_uniform_expect_table(vs, object, ve::cat("ubo", prefix));
    while (!vs.empty() && isspace(vs[0])) vs.erase(0, 1); // Cannot have whitespace before #version.

    std::string fs = std::string { fragment_shader };
    insert_uniform_def<Layout>(fs, uniform_def);
    insert_uniform_expect_table(fs, object, ve::cat("ubo", prefix));
    while (!fs.empty() && isspace(fs[0])) fs.erase(0, 1); // Cannot have whitespace before #version.


    using src_list = typename ve::gfx::shader_cache::source_list;
    const auto* vs_stage = &*ranges::find(ve::gfxapi::shader_stages, ".vert.glsl", &ve::gfxapi::shader_stage::file_extension);
    const auto* fs_stage = &*ranges::find(ve::gfxapi::shader_stages, ".frag.glsl", &ve::gfxapi::shader_stage::file_extension);

    auto cache_id = ve::cat(name, "_", Layout == ve::gfx::glsl_layout::STD140 ? "std140" : "std430");
    auto shader   = ve::gfx::shader_cache::instance().get_or_load<ve::gfx::vertex_types::no_vertex>(src_list { std::pair { vs_stage, vs }, std::pair { fs_stage, fs } }, cache_id);
    auto tmpl     = ve::gfxapi::framebuffer_attachment_template { "color" };
    auto target   = ve::make_shared<ve::gfxapi::render_target>(std::vector { tmpl }, ve::produce(vec2ui { 3 }), ve::produce(true));
    auto pipeline = ve::gfxapi::single_pass_pipeline { target, shader };
    auto vbo      = ve::gfxapi::g_input_quad();
    auto context  = ve::gfxapi::render_context { };

    pipeline.set_uniform_value<T>(std::string { name }, object);
    pipeline.draw(ve::gfxapi::pipeline::draw_data { .buffers = { vbo.get() }, .ctx = &context });

    auto texture = std::static_pointer_cast<ve::gfxapi::texture>(target->get_attachments().at("color").texture);
    auto result  = texture->read()[{ 1, 1 }];


    if (result == vec4ub { 0, 255, 0, 255 }) {
        return VE_TEST_SUCCESS;
    } else {
        return VE_TEST_FAIL(
            "Uniform value for ", name,
            " with layout ", (Layout == ve::gfx::glsl_layout::STD140 ? "STD140" : "STD430"),
            " observed in shader differed from the value of the CPU-side object."
        );
    }
}


test_result test_main(void) {
    // Required to create API context.
    static auto window = ve::gfx::window::create(ve::gfx::window::arguments { .title = "Test" });


    std::tuple test_structs {
        std::tuple { S1{}, S1_glsl, "S1", ""   },
        std::tuple { S2{}, S2_glsl, "S2", ""   },
        std::tuple { S3{}, S3_glsl, "S3", ""   },
        std::tuple { S4{}, S4_glsl, "S4", ".a" }
    };


    test_result result = VE_TEST_SUCCESS;

    ve::tuple_foreach(test_structs, [&] (const auto& elem) {
        const auto& [obj, glsl, name, prefix] = elem;

        result |= test_with_layout<ve::gfx::glsl_layout::STD140>(std::string { glsl }, obj, prefix, name);
        // result |= test_with_layout<ve::gfx::glsl_layout::STD430>(std::string { glsl }, obj, prefix, name);
    });

    return result;
}