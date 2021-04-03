#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>
#include <VoxelEngine/utility/traits/pack.hpp>
#include <VoxelEngine/ecs/component/static_component_info.hpp>
#include <VoxelEngine/ecs/component/component.hpp>
#include <VoxelEngine/ecs/scene.hpp>
#include <VoxelEngine/side/side.hpp>

#include <entt/entt.hpp>
#include <ctti/nameof.hpp>


namespace ve {
    // Non-templated base class for entity types.
    // Don't derive from this directly, use ve_entity_class and derive from entity<YourEntityType>
    // or a class derived from entity<YourEntityType>.
    template <side Side> class entity_base {
    public:
        virtual ~entity_base(void) = default;
        ve_move_only(entity_base);
        
        VE_GET_VAL(id);
        VE_GET_VAL(scene);
        VE_GET_VAL(owner);
        
        [[nodiscard]] bool has_dynamic_behaviour(void) const { return dynamic_behaviour; }
        
    protected:
        [[nodiscard]] entt::registry& get_storage(void) {
            return scene->storage;
        }
    
        [[nodiscard]] const entt::registry& get_storage(void) const {
            return scene->storage;
        }
        
    private:
        template <typename, side> friend class entity;
        template <side> friend class scene;
        
        
        // Inaccessible type to prevent construction of entity_base except by scene.
        struct secret_type {};
        
        
        // TODO: Rework this? Contain parent of scene then decide on per component basis.
        scene<Side>* scene = nullptr;
        entt::entity id = entt::null;
        actor_id owner = no_actor_id;
        
        // If all function components for this entity have their default implementation,
        // we don't have to check the ECS when calling them.
        bool dynamic_behaviour = false;
        
    public:
        entity_base(secret_type, ve::scene<Side>* scene, entt::entity id, ve_default_actor(owner))
            : scene(scene), id(id), owner(owner) {}
    };
    
    
    template <typename Derived, side Side> class entity : public entity_base<Side> {
    public:
        using entity_base<Side>::entity_base;
        using most_derived_t = Derived;
        
        
        template <typename... Args> void init(Args&&... args) {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, template init<Args...>)) {
                static_cast<Derived*>(this)->init(std::forward<Args>(args)...);
            }
        }
        
        
        template <component_type Component>
        constexpr void set_component(universal<Component> auto&& cmp) {
            if constexpr (has_static_component<Component>()) {
                // If this is a function component being changed to a non-default value,
                // we can no longer optimize away the ECS lookup + indirect function call for this entity
                // when calling it as if it were a member function.
                constexpr auto info = get_static_component_info<Component>();
                if constexpr (info.functional) this->dynamic_behaviour = true;
            }
    
            this->get_storage().template emplace_or_replace<Component>(this->id, std::forward<Component>(cmp));
        }
        
        
        template <component_type Component>
        [[nodiscard]] constexpr const Component& get_component(void) const {
            VE_ASSERT(
                has_component<Component>(), // No-op if component is static.
                "Cannot get non-existent component "s + ctti::nameof<Component>().cppstring() + " from entity."
            );
    
            return this->get_storage().template get<Component>(this->id);
        }
    
        template <component_type Component>
        [[nodiscard]] constexpr Component& get_component(void) {
            return const_cast<Component&>(
                ((const entity<Derived, Side>*) this)->template get_component<Component>()
            );
        }
    
    
        template <component_type Component>
        [[nodiscard]] constexpr optional<const Component&> try_get_component(void) const {
            if constexpr (has_static_component<Component>()) return get_component<Component>();
    
            auto ptr = this->get_storage().template try_get<Component>(this->id);
    
            if (ptr) return *ptr;
            else return nullopt;
        }
    
        template <component_type Component>
        [[nodiscard]] constexpr optional<Component&> try_get_component(void) {
            return const_cast<Component&>(
                *((const entity<Derived, Side>&) *this).try_get_component<Component>()
            );
        }
        
        
        template <component_type Component>
        [[nodiscard]] constexpr bool has_component(void) const {
            if constexpr (has_static_component<Component>()) return true;
            else return this->get_storage().template has<Component>(this->id);
        }
    private:
        // Gets information about the component of type T if it is known to exist at compile time.
        //
        // Returns either:
        // An instance of static_component_value_info if Derived has a value component of type T.
        // An instance of static_component_fn_info if Derived has a function component of type T.
        // void if neither of the above apply.
        template <typename T>
        consteval static auto get_static_component_info(void) {
            if constexpr (requires { Derived::template ve_impl_component_info<T>(); }) {
                return Derived::template ve_impl_component_info<T>();
            }
        }
        
        
        // Checks if Derived has a component of type T.
        template <typename T> consteval static bool has_static_component(void) {
            using invoke_t = decltype(get_static_component_info<T>());
            return !std::is_same_v<invoke_t, void>;
        }
    };
}