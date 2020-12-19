#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/render/generic_uniform.hpp>

#include <GL/glew.h>

#include <type_traits>
#include <string>
#include <utility>
#include <functional>
#include <vector>


namespace ve {
    class uniform_storage {
    public:
        template <typename T> using uniform_producer = std::function<T(void)>;
        
        
        // Sets the uniform to have the given value until changed.
        template <typename T, universal<std::string> String>
        void set_global_uniform_val(String&& name, T&& value) {
            uniforms.insert_or_assign(
                std::forward<String>(name),
                [value = std::forward<T>(value)](GLuint program, const std::string& name, u32& state) {
                    generic_uniform::set_uniform(program, name.c_str(), value, state);
                }
            );
        }
    
    
        // Sets the uniform to be refreshed by calling producer every time it is used.
        template <universal<std::string> String, typename Producer>
        requires std::is_invocable_v<std::remove_cvref_t<Producer>>
        void set_global_uniform_fn(String&& name, Producer&& producer) {
            uniforms.insert_or_assign(
                std::forward<String>(name),
                [producer = std::forward<Producer>(producer)](GLuint program, const std::string& name, u32& state) {
                    generic_uniform::set_uniform(program, name.c_str(), producer(), state);
                }
            );
        }
        
    protected:
        void bind_uniforms(GLuint shader, u32& uniform_state) {
            for (const auto& [name, setter] : uniforms) setter(shader, name, uniform_state);
        }
        
    private:
        using uniform_fn = std::function<void(GLuint, const std::string&, u32&)>;
        
        hash_map<std::string, uniform_fn> uniforms;
    };
}