#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>
#include <VoxelEngine/utility/traits/pack.hpp>
#include <VoxelEngine/ecs/component/static_component_info.hpp>
#include <VoxelEngine/ecs/component/component.hpp>
#include <VoxelEngine/ecs/entity/entity_utils.hpp>

#include <entt/entt.hpp>
#include <ctti/nameof.hpp>


namespace ve {
    // Non-templated base class for entity types.
    // Don't derive from this directly, use ve_entity_class and derive from entity<YourEntityType>
    // or a class derived from entity<YourEntityType>.
    class entity_base {
    public:
        virtual ~entity_base(void) {
            if (storage && id != entt::null) storage->destroy(id);
        }
        
        ve_swap_move_only(entity_base, id, storage);
        
        VE_GET_VAL(id);
        VE_GET_VAL(storage);
        
        [[nodiscard]] bool has_dynamic_behaviour(void) const { return dynamic_behaviour; }
    private:
        template <typename Derived> friend class entity;
        
        entt::entity id = entt::null;
        entt::registry* storage = nullptr;
        // If all function components for this entity have their default implementation,
        // we don't have to check the ECS when calling them.
        bool dynamic_behaviour = false;
        
        explicit entity_base(entt::registry* storage) : id(storage->create()), storage(storage) {}
    };
    
    
    // Note: use the ve_entity_class macro when extending this class.
    template <typename Derived> class entity : public entity_base {
    private:
        void ve_impl_check_entity_def_usage() {
            // Make sure the aforementioned macro was actually used.
            // This cannot be at class-scope due to Derived being incomplete in that context.
            static_assert(
                std::is_base_of_v<ve::detail::ve_impl_assert_entity_def_used<Derived>, Derived>,
                "Please use ve_entity_class when defining a new entity class."
            );
        }
        
    public:
        using entity_base::entity_base;
        
        
        template <component_type Component>
        constexpr void set_component(universal<Component> auto&& cmp) {
            if constexpr (has_static_component<Component>()) {
                // If this is a function component being changed to a non-default value,
                // we can no longer optimize away the ECS lookup + indirect function call for this entity
                // when calling it as if it were a member function.
                constexpr auto info = get_static_component_info<Component>();
                if constexpr (info.functional) dynamic_behaviour = true;
            }
            
            storage->emplace_or_replace<Component>(id, std::forward<Component>(cmp));
        }
        
        
        template <component_type Component>
        [[nodiscard]] constexpr const Component& get_component(void) const {
            VE_ASSERT(
                has_component<Component>(), // No-op if component is static.
                "Cannot get non-existent component "s + ctti::nameof<Component>().cppstring() + " from entity."
            );
    
            return storage->get<Component>(id);
        }
    
        template <component_type Component>
        [[nodiscard]] constexpr Component& get_component(void) {
            return const_cast<Component&>(
                ((const entity<Derived>*) this)->template get_component<Component>()
            );
        }
    
    
        template <component_type Component>
        [[nodiscard]] constexpr optional<const Component&> try_get_component(void) const {
            if constexpr (has_static_component<Component>()) return get_component<Component>();
    
            auto ptr = storage->try_get<Component>(id);
    
            if (ptr) return *ptr;
            else return nullopt;
        }
    
        template <component_type Component>
        [[nodiscard]] constexpr optional<Component&> try_get_component(void) {
            return const_cast<Component&>(
                *((const entity<Derived>&) *this).try_get_component<Component>()
            );
        }
        
        
        template <component_type Component>
        [[nodiscard]] constexpr bool has_component(void) const {
            if constexpr (has_static_component<Component>()) return true;
            else return storage->has<Component>(id);
        }
    private:
        // Gets information about the component of type T if it is known to exist at compile time.
        //
        // Returns either:
        // An instance of static_component_value_info if Derived has a value component of type T.
        // An instance of static_component_fn_info if Derived has a function component of type T.
        // void if neither of the above apply.
        //
        // To be exact, returns these if Derived has a value or function which is marked as a static component
        // that would be of type T if it was wrapped in a value_component or function_component respectively.
        // e.g. if Derived had a member "int myvar" marked as a value component, the type of T would have to
        //      be named_value_component<"myvar", int> to retrieve the info struct.
        template <typename T>
        consteval static auto get_static_component_info(void) {
            // Every type created by ve_entity_class(...) will have a function to retrieve all classes
            // from which Derived inherits that themselves (possibly indirectly) inherit from entity.
            // This type will be a specialization of ve::meta::pack.
            using bases = typename Derived::ve_impl_bases;
            
            return bases::for_each([] <typename Base> () {
                // Static components will define a function of the following signature in their enclosing class:
                //
                // template <typename T> requires std::is_same_v<T, ComponentT>
                // constexpr static auto ve_impl_component_info(void) { ... }
                //
                // returning either a static_component_value_info or a static_component_fn_info.
                if constexpr (requires { Base::template ve_impl_component_info<T>(); }) {
                    return Base::template ve_impl_component_info<T>();
                }
            });
        }
        
        
        // Checks if Derived has a component of type T.
        // Same rules apply as when using get_static_component_info.
        template <typename T> consteval static bool has_static_component(void) {
            using invoke_t = decltype(get_static_component_info<T>());
            return !std::is_same_v<invoke_t, void>;
        }
    };
}