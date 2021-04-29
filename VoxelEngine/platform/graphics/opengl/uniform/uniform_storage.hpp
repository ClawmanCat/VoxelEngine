#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>
#include <VoxelEngine/platform/graphics/opengl/context.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/set_uniform.hpp>

#include <functional>


namespace ve::graphics {
    // Extending uniform_storage allows a class to easily store a set of uniforms, either as constant values
    // or as predicates which produce the value of the uniform when called.
    class uniform_storage : public resource_owner<uniform_storage> {
        struct uniform_setter {
            std::function<void(const std::string&, context&)> setter;
            actor_id owner;
            
            void operator()(const std::string& name, context& ctx) const {
                setter(name, ctx);
            }
        };
        
    public:
        template <typename T> using producer_fn = std::function<T(void)>;
        
        uniform_storage(void) = default;
        ve_move_only(uniform_storage);
        
        
        void on_actor_destroyed(actor_id id) {
            remove_all_owned_from_vector(
                id,
                uniforms,
                ve_get_field(second.owner),
                ve_get_field(first),
                "uniform"
            );
        }
        
        
        template <typename T>
        void set_uniform_value(std::string&& name, T&& value, ve_default_actor(owner)) {
            auto setter = uniform_setter {
                .setter = [value = std::forward<T>(value)](const std::string& name, context& ctx) {
                    set_uniform(name.c_str(), value, ctx);
                },
                .owner = owner
            };
            
            uniforms[std::move(name)] = std::move(setter);
        }
    
    
        template <typename T>
        void set_uniform_value(std::string_view name, T&& value, ve_default_actor(owner)) {
            set_uniform_value(std::string { name }, std::forward<T>(value), owner);
        }
        
    
        template <typename T, typename Pred> requires requires (Pred p) { producer_fn<T> { p }; }
        void set_uniform_producer(std::string&& name, Pred&& fn, ve_default_actor(owner)) {
            auto setter = uniform_setter {
                .setter = [fn = producer_fn<T> { std::forward<Pred>(fn) }](const std::string& name, context& ctx) {
                    set_uniform(name.c_str(), fn(), ctx);
                },
                .owner = owner
            };
    
            uniforms[std::move(name)] = std::move(setter);
        }
    
    
        template <typename T, typename Pred> requires requires (Pred p) { producer_fn<T> { p }; }
        void set_uniform_producer(std::string_view name, Pred&& fn, ve_default_actor(owner)) {
            set_uniform_producer(std::string { name }, std::forward<Pred>(fn), owner);
        }
        
        
        void clear_uniforms(void) {
            uniforms.clear();
        }
        
    protected:
        void bind_uniforms(context& ctx) const {
            for (const auto& [name, setter] : uniforms) setter(name, ctx);
        }
        
    private:
        flat_map<std::string, uniform_setter> uniforms;
    };
}