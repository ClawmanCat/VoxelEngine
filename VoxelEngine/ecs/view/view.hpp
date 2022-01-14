#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/empty_storage.hpp>
#include <VoxelEngine/ecs/component/common_component.hpp>
#include <VoxelEngine/utility/traits/negate_trait.hpp>
#include <VoxelEngine/utility/traits/bind.hpp>
#include <VoxelEngine/utility/traits/nest.hpp>
#include <VoxelEngine/utility/traits/value.hpp>
#include <VoxelEngine/utility/traits/function_traits.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>

#include <entt/entt.hpp>


namespace ve {
    namespace detail {
        // Future versions will have the ability to transform some component types based on their tags,
        // this is here for future compatibility.
        template <typename T> using resolve_component_type = T;


        template <typename... Included> struct entt_view_helper {
            template <typename... Excluded> struct with_exclude {
                // Since ENTT doesn't support exclude-only views, we just use the common component if the include list is empty.
                template <typename Entity> using type = std::conditional_t<
                    sizeof...(Included) == 0,
                    entt::basic_view<Entity, entt::exclude_t<Excluded...>, detail::common_component>,
                    entt::basic_view<Entity, entt::exclude_t<Excluded...>, Included...>
                >;
            };
        };


        template <typename Entity, meta::pack_of_types Included, meta::pack_of_types Excluded>
        using entt_view_type = typename Excluded
            ::template expand_outside<resolve_component_type>
            ::template expand_inside<
                Included
                    ::template expand_outside<resolve_component_type>
                    ::template expand_inside<entt_view_helper>
                    ::template with_exclude
            >::template type<Entity>;



        template <typename> struct view_info {};

        template <typename Entity, typename... Included, typename... Excluded>
        struct view_info<entt::basic_view<Entity, entt::exclude_t<Excluded...>, Included...>> {
            using entity_type    = Entity;
            using included_types = meta::pack<Included...>;
            using excluded_types = meta::pack<Excluded...>;
            using type           = entt::basic_view<Entity, entt::exclude_t<Excluded...>, Included...>;
        };
    }


    // Wraps entt::basic_view and resolves empty components and components that are stored as smart pointers transparently.
    // TODO: Implement iterable views.
    template <typename Entity, meta::pack_of_types Included, meta::pack_of_types Excluded>
    class basic_view {
    public:
        using wrapped_view_t   = detail::entt_view_type<Entity, Included, Excluded>;
        using ve_view_tag      = void;
        using included_types   = Included;
        using excluded_types   = Excluded;
        using entity_type      = typename wrapped_view_t::entity_type;
        using size_type        = typename wrapped_view_t::size_type;
        using iterator         = typename wrapped_view_t::iterator;
        using reverse_iterator = typename wrapped_view_t::reverse_iterator;


        basic_view(void) = default;
        explicit basic_view(wrapped_view_t wrapped) : wrapped_view(std::move(wrapped)) {}


        template <typename... Components> decltype(auto) get(const entity_type entt) const {
            auto get_type = [&] <typename T> (meta::type_wrapper<T>) -> decltype(auto) {
                // If T is empty, return a fake object.
                if constexpr (std::is_empty_v<T>) {
                    return empty_storage_for<T>();
                }

                // If T is a resolved pointer component type, resolve it.
                else if constexpr (!std::is_same_v<T, detail::resolve_component_type<T>>) {
                    return *(wrapped_view.template get<detail::resolve_component_type<T>>(entt));
                }

                // Otherwise just forward the call.
                else {
                    return wrapped_view.template get<T>(entt);
                }
            };


            if constexpr (sizeof...(Components) == 1) return get_type(meta::type_wrapper<Components...>{});
            else return std::forward_as_tuple(get_type(meta::type_wrapper<Components>{})...);
        }


        template <typename Func>
        void each(Func func) const { each_impl<Func>(std::move(func)); }

        template <typename Component, typename Func>
        void each(Func func) const { each_impl<Func, Component>(std::move(func)); }


        iterator begin(void) const { return wrapped_view.begin(); }
        iterator end(void)   const { return wrapped_view.end();   }

        reverse_iterator rbegin(void) const { return wrapped_view.rbegin(); }
        reverse_iterator rend(void)   const { return wrapped_view.rend();   }

        iterator find(const entity_type entt) const { return wrapped_view.find(entt); }
        bool contains(const entity_type entt) const { return wrapped_view.contains(entt); }

        entity_type front(void) const { return wrapped_view.front(); }
        entity_type back(void)  const { return wrapped_view.back();  }

        void use(void) const { wrapped_view.use(); }
        size_type size_hint(void) const { return wrapped_view.size_hint(); }


        explicit operator bool(void) const { return (bool) wrapped_view; }


        VE_GET_MREF(wrapped_view);
    private:
        wrapped_view_t wrapped_view;


        template <typename Func, typename... Component> void each_impl(Func func) const {
            using function_arguments = typename meta::function_traits<Func>::arguments;

            using forwarded_components = typename function_arguments
                ::template expand_outside<std::remove_cvref_t>
                ::template filter_trait<meta::bind_types<std::is_same, entity_type>::template front>
                ::template expand_outside<detail::resolve_component_type>;


            auto get_type = [] <typename T, typename T0, typename... Ts> (auto self, meta::type_wrapper<T> w, T0& first, Ts&... rest) -> decltype(auto) {
                if constexpr (std::is_same_v<T, T0>) return first;
                else return self(self, w, rest...);
            };


            [&] <typename... FwdArgs> (meta::pack<FwdArgs...>) {
                // Forward arguments that the wrapped view can use (non-empty and pointer-unresolved) and transform them before invoking the predicate.
                wrapped_view.template each<Component...>([&] (const entity_type entt, FwdArgs&... args) {
                    auto get_argument = [&] <typename T> (meta::type_wrapper<T>) -> decltype(auto) {
                        if constexpr (std::is_empty_v<T>) {
                            return empty_storage_for<T>();
                        }

                        else if constexpr (!std::is_same_v<T, detail::resolve_component_type<T>>) {
                            return *get_type(get_type, meta::type_wrapper<detail::resolve_component_type<T>>{}, args...);
                        }

                        else {
                            return get_type(get_type, meta::type_wrapper<T>{}, args...);
                        }
                    };


                    [&] <typename... Args> (meta::pack<Args...>) {
                        std::invoke(func, get_argument(meta::type_wrapper<Args>{})...);
                    } (function_arguments {});
                });
            } (forwarded_components {});
        }
    };


    template <meta::pack_of_types Included, meta::pack_of_types Excluded>
    using view = basic_view<entt::entity, Included, Excluded>;


    template <typename Entity, typename... LArgs, typename... RArgs>
    auto operator|(const basic_view<Entity, LArgs...>& lhs, const basic_view<Entity, RArgs...>& rhs) {
        return entt::view_pack { lhs.get_wrapped_view(), rhs.get_wrapped_view() };
    }

    template <typename Entity, typename... LArgs, typename... RArgs>
    auto operator|(const basic_view<Entity, LArgs...>& lhs, const entt::basic_view<Entity, RArgs...>& rhs) {
        return entt::view_pack { lhs.get_wrapped_view(), rhs };
    }

    template <typename Entity, typename... LArgs, typename... RArgs>
    auto operator|(const entt::basic_view<Entity, LArgs...>& lhs, const basic_view<Entity, RArgs...>& rhs) {
        return entt::view_pack { lhs, rhs.get_wrapped_view() };
    }

    template <typename Entity, typename... LArgs, typename... RArgs>
    auto operator|(const basic_view<Entity, LArgs...>& lhs, const entt::view_pack<RArgs...>& rhs) {
        return lhs.get_wrapped_view() | rhs;
    }


    template <meta::pack_of_types Included, meta::pack_of_types Excluded = meta::pack<>>
    inline auto view_registry(auto& registry) {
        // Performs pointer & empty view resolution.
        using ViewInfo = typename detail::view_info<detail::entt_view_type<entt::entity, Included, Excluded>>;

        return [&] <typename... I> (meta::pack<I...>) {
            return [&] <typename... E> (meta::pack<E...>) {
                return ve::view<Included, Excluded> { registry.template view<I...>(entt::exclude_t<E...> {}) };
            } (typename ViewInfo::excluded_types {});
        } (typename ViewInfo::included_types {});
    }


    template <meta::pack_of_types Included, meta::pack_of_types Excluded = meta::pack<>>
    inline auto view_storage(auto& storage) {
        // Performs pointer & empty view resolution.
        using ViewInfo = typename detail::view_info<detail::entt_view_type<entt::entity, Included, Excluded>>;

        return [&] <typename... I> (meta::pack<I...>) {
            return [&] <typename... E> (meta::pack<E...>) {
                return ve::view<Included, Excluded> { typename ViewInfo::type { storage } };
            } (typename ViewInfo::excluded_types {});
        } (typename ViewInfo::included_types {});
    }
}