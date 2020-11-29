#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/render/buffer/vertex_array.hpp>
#include <VoxelEngine/graphics/render/shader/shader_program.hpp>
#include <VoxelEngine/graphics/render/generic_uniform.hpp>

#include <functional>
#include <vector>
#include <tuple>
#include <string>


namespace ve {
    class graphics_pipeline {
    public:
        template <typename T> using uniform_producer = std::function<T(void)>;
        
        
        // Renders the pipeline to the currently bound target.
        void draw(void) {
            for (auto& [shader, vertex_arrays] : buffers) {
                shader->bind();
                GLuint id = shader->get_id();
    
                // Set global uniforms.
                for (const auto& [name, setter] : uniforms) setter(id, name);
                
                // Set local uniforms & draw buffers.
                for (auto& array : vertex_arrays) array->draw(id);
                
                shader->unbind();
            }
        }
        
        
        template <typename T, universal<std::string> String>
        void set_global_uniform_val(String&& name, T&& value) {
            uniforms.insert_or_assign(
                std::forward<String>(name),
                [value = std::forward<T>(value)](GLuint program, const std::string& name) {
                    generic_uniform::set_uniform(program, name.c_str(), value);
                }
            );
        }
    
    
        template <universal<std::string> String, typename Producer>
        requires std::is_invocable_v<std::remove_cvref_t<Producer>>
        void set_global_uniform_fn(String&& name, Producer&& producer) {
            uniforms.insert_or_assign(
                std::forward<String>(name),
                [producer = std::forward<Producer>(producer)](GLuint program, const std::string& name) {
                    generic_uniform::set_uniform(program, name.c_str(), producer());
                }
            );
        }
        
        
        void add_buffer(shared<shader_program>&& shader, unique<vertex_array_base>&& buffer) {
            VE_ASSERT(shader != nullptr);
            auto it = buffers.find(shader);
            
            if (it != buffers.end()) {
                it->second.push_back(std::move(buffer));
            } else {
                // Can't use an initializer list because buffer is move only.
                std::vector<unique<vertex_array_base>> v;
                v.push_back(std::move(buffer));
                
                buffers.insert({ std::move(shader), std::move(v) });
            }
        }
    private:
        using uniform_fn = std::function<void(GLuint, const std::string&)>;
        
        
        hash_map<shared<shader_program>, std::vector<unique<vertex_array_base>>> buffers;
        hash_map<std::string, uniform_fn> uniforms;
    };
}