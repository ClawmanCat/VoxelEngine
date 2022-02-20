#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/input/binder/bindable_input.hpp>
#include <VoxelEngine/utility/bit.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve {
    namespace binding_error_handlers {
        const static inline auto ignore = [] {};

        const static inline auto warn = [] {
            VE_LOG_WARN("Binding does not support events of provided type. Event will be ignored.");
        };

        const static inline auto error = [] {
            VE_ASSERT(false, "Binding does not support events of provided type.");
        };
    }


    class input_binder {
    private:
        using binding_t = std::function<void(const void*, std::size_t)>;
        struct alias_data;
        struct type_data;

    public:
        struct binding_handle {
            binding_handle(std::string key, std::size_t index) : key(std::move(key)), index(index) {}
            ve_rt_move_only(binding_handle);

        private:
            friend class input_binder;

            std::string key;
            std::size_t index;
        };


        input_binder(void) = default;
        ve_immovable(input_binder);


        ~input_binder(void) {
            while (!aliases.empty()) {
                const auto& [name, alias] = *aliases.begin();
                remove_alias(name);
            }
        }


        // Create an alias for the given input, overwriting any existing alias under the given name.
        template <typename Input> requires std::is_base_of_v<input, Input>
        void create_alias(std::string_view name, Input value) {
            if (aliases.contains(name)) remove_alias(name);
            maybe_add_type(value);

            type_data* type = &input_types[value.type_hash];
            auto [it, success] = aliases.emplace(
                std::string { name },
                alias_data {
                    .name  = std::string { name },
                    .input = make_unique<Input>(std::move(value)),
                    .type  = type
                }
            );

            type->aliases.push_back(&it->second);
        }


        // Removes an alias previously added with create_alias.
        // Any bindings to the given alias remain, and will be re-enabled if a new alias with the given name is added later.
        void remove_alias(std::string_view name) {
            auto it = aliases.find(name);

            unique<input> i = std::move(it->second.input);
            std::erase_if(it->second.type->aliases, [&] (const auto* a) { return a->name == name; });

            aliases.erase(it);
            maybe_remove_type(*i);
        }


        // Adds a function to execute when the input aliased to the given name is entered by the user.
        // Pred must either be invocable as pred() or take a const void* and an std::size_t as arguments, which are a pointer to the event
        // wrapped by the input associated with the given alias and the type hash of said event respectively.
        template <typename Pred> requires (std::is_invocable_v<Pred> || std::is_invocable_v<Pred, const void*, std::size_t>)
        binding_handle add_binding(std::string_view name, Pred binding) {
            if constexpr (std::is_invocable_v<Pred>) {
                return add_binding(name, [binding = std::move(binding)] (const void*, std::size_t) { std::invoke(binding); });
            } else {
                bindings[name].emplace_back(std::move(binding));
                return binding_handle { std::string { name }, bindings[name].size() - 1 };
            }
        }


        // Equivalent to above, except the binding callable accepts the event argument directly.
        // A behaviour must be chosen for when the event type does not match the expected argument.
        // If the event type is not deducible from pred (e.g. its parameter is auto), a list of allowed event types must be provided.
        template <
            typename Pred,
            meta::pack_of_types AllowedEvents = typename meta::function_traits<Pred>::arguments::template expand_outside<std::remove_cvref_t>,
            typename Handler = decltype(binding_error_handlers::warn)
        >
        requires (AllowedEvents::all([] <typename E> { return std::is_invocable_v<Pred, const E&>; }))
        binding_handle add_specialized_binding(std::string_view name, Pred binding, Handler on_fail = binding_error_handlers::warn) {
            return add_binding(
                name,
                [binding = std::move(binding), on_fail = std::move(on_fail)] (const void* event, std::size_t type) {
                    bool no_overloads_found = AllowedEvents::foreach([&] <typename E> {
                        if (type == type_hash<E>()) {
                            std::invoke(binding, *((const E*) event));
                            return meta::PACK_FOREACH_BREAK;
                        }

                        return meta::PACK_FOREACH_CONTINUE;
                    });

                    if (no_overloads_found) std::invoke(on_fail);
                }
            );
        }


        // Overload of add_specialized_binding to prevent having to specify the type of pred when the AllowedEvents list is manually provided.
        template <meta::pack_of_types AllowedEvents, typename Handler = decltype(binding_error_handlers::warn)>
        binding_handle add_specialized_binding(std::string_view name, auto binding, Handler on_fail = binding_error_handlers::warn) {
            return add_specialized_binding<decltype(binding), AllowedEvents>(name, std::move(binding), std::move(on_fail));
        }


        // Removes a binding function previously added with add_binding.
        void remove_binding(const binding_handle& handle) {
            bindings.at(handle.key).at(handle.index) = nullptr;
        }


        // Removes all bindings for the given alias.
        void remove_bindings(std::string_view alias) {
            if (auto it = bindings.find(alias); it != bindings.end()) {
                it->second.clear();
            }
        }


        // Returns the input aliased to the given name, or null if no such input exists.
        const input* get_input_for_alias(std::string_view alias) const {
            auto it = aliases.find(alias);
            return it != aliases.end() ? it->second.input.get() : nullptr;
        }


        // Returns the type hash of the input aliased to the given name, or null if no such input exists.
        std::optional<std::size_t> get_input_type_for_alias(std::string_view alias) const {
            auto it = aliases.find(alias);
            return it != aliases.end() ? std::optional { it->second.type->type_hash } : std::nullopt;
        }
    private:
        struct alias_data {
            std::string name;
            unique<input> input;
            type_data* type;
        };


        struct type_data {
            struct invoker_data {
                const input_binder* self;
                const type_data* type;
            };

            std::vector<event_handler_id_t> event_handlers;
            invoker_data data;
            std::size_t type_hash;

            std::vector<alias_data*> aliases;
        };


        stable_hash_map<std::size_t, type_data> input_types;
        stable_hash_map<std::string, alias_data> aliases;
        hash_map<std::string, std::vector<binding_t>> bindings;


        void maybe_add_type(const input& i) {
            if (auto [it, success] = input_types.try_emplace(i.type_hash, type_data { }); success) {
                auto& type = it->second;

                type.type_hash = i.type_hash;
                type.data      = type_data::invoker_data { .self = this, .type = &type };

                auto invoker = detail::type_erased_handler {
                    .wrapped = [] (const void* event, const void* data, std::size_t event_type) {
                        const auto* idata = (const type_data::invoker_data*) data;

                        for (const auto* alias : idata->type->aliases) {
                            if (alias->input->should_trigger(event, event_type)) {
                                if (auto it = idata->self->bindings.find(alias->name); it != idata->self->bindings.end()) {
                                    for (const auto& binding : it->second) {
                                        if (binding) std::invoke(binding, event, event_type);
                                    }
                                }
                            }
                        }
                    },
                    .data = &type.data
                };

                type.event_handlers = i.register_handlers(invoker);
            }
        }


        void maybe_remove_type(const input& i) {
            auto it = input_types.find(i.type_hash);

            if (it->second.aliases.empty()) {
                i.unregister_handlers(it->second.event_handlers);
                input_types.erase(it);
            }
        }
    };
}