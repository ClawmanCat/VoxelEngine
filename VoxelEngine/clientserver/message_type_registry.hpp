#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/simple_event_dispatcher.hpp>
#include <VoxelEngine/event/subscribe_only_view.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/heterogeneous_key.hpp>

#include <ctti/type_id.hpp>


namespace ve {
    using mtr_id = u16;
    struct mtr_friend_access;


    struct message_type {
        std::string name;
        u64 type_hash;
        mtr_id id;
        bool is_core;


        template <typename T> bool holds(void) const {
            return type_hash == ve::type_hash<T>();
        }
    };


    struct message_type_registered_event { const message_type* type; };


    class message_type_registry : public subscribe_only_view<simple_event_dispatcher<>> {
    public:
        template <typename T>
        const message_type& register_type(std::string name, bool is_core = false) {
            return register_type(std::move(name), type_hash<T>(), is_core);
        }


        const message_type& register_type(std::string name, u64 type_id, bool is_core = false) {
            VE_DEBUG_ASSERT(!is_discontinuous, "Cannot manually register types into MTR used to register remote types.");
            return register_type(std::move(name), type_id, next_id++, is_core);
        }


        bool contains(mtr_id id) const {
            return types_by_id.contains(id);
        }


        bool contains(std::string_view name) const {
            return types_by_name.contains(name);
        }


        const message_type& get_type(std::string_view name) const {
            return types_by_name.at(name);
        }


        const message_type& get_type(mtr_id id) const {
            return *types_by_id.at(id);
        }


        const message_type* try_get_type(std::string_view name) const {
            auto it = types_by_name.find(name);
            return (it == types_by_name.end()) ? nullptr : &it->second;
        }


        const message_type* try_get_type(mtr_id id) const {
            auto it = types_by_id.find(id);
            return (it == types_by_id.end()) ? nullptr : it->second;
        }


        VE_GET_CREF(types_by_name);
        VE_GET_CREF(types_by_id);
        VE_GET_VAL(num_core_types);
    private:
        friend struct mtr_friend_access;

        stable_hash_map<std::string, message_type> types_by_name;
        hash_map<mtr_id, const message_type*> types_by_id;

        mtr_id next_id = 0;
        mtr_id num_core_types = 0;
        VE_DEBUG_ONLY(bool is_discontinuous = false);


        const message_type& register_type(std::string name, u64 type_id, mtr_id id, bool is_core = false) {
            VE_DEBUG_ONLY(is_discontinuous |= (id + 1 != next_id));

            auto [it, success] = types_by_name.emplace(name, message_type {
                .name      = name,
                .type_hash = type_id,
                .id        = id,
                .is_core   = is_core
            });

            if (!success) {
                throw std::runtime_error(cat("Attempt to re-register existing message type ", name));
            }


            types_by_id.emplace(id, &it->second);


            if (is_core) {
                VE_ASSERT(num_core_types == id, "Core message types must be registered before all others.");
                ++num_core_types;
            }


            dispatch_event(message_type_registered_event { &it->second });
            return it->second;
        }
    };
}