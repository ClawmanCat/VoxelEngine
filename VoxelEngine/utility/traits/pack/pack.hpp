#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/decompose.hpp>
#include <VoxelEngine/utility/traits/pack/pack_helpers.hpp>


namespace ve::meta {
    template <typename T> concept pack_of_types = requires { typename T::pack_tag; };
    
    
    template <typename... Ts> struct pack {
        using pack_tag = void;
    
    
        constexpr static inline std::size_t size = sizeof...(Ts);
        
        using self     = pack<Ts...>;
        using empty    = pack<>;
        using head     = typename detail::splitter<pack, Ts...>::head;
        using tail     = typename detail::splitter<pack, Ts...>::tail;
    
    
        template <template <typename...> typename P> using expand_inside  = P<Ts...>;
        template <template <typename...> typename P> using expand_outside = pack<P<Ts>...>;
        template <template <typename...> typename P> using transform      = P<self>;
        
    
        template <typename... Xs> using append  = pack<Ts..., Xs...>;
        template <typename... Xs> using prepend = pack<Xs..., Ts...>;
    
        template <typename Pack> using append_pack  = typename Pack::template expand_inside<append>;
        template <typename Pack> using prepend_pack = typename Pack::template expand_inside<prepend>;
        
        
        
        template <typename T> consteval static bool contains_impl(void) {
            if constexpr (size == 0) return false;
            else if constexpr (std::is_same_v<T, head>) return true;
            else return tail::template contains_impl<T>();
        }
    
        template <typename T> constexpr static bool contains = contains_impl<T>();
    
    
    
        template <typename Pack> consteval static bool contains_any_impl(void) {
            if constexpr (size == 0 || Pack::size == 0) return false;
            else if constexpr (contains<typename Pack::head>) return true;
            else return contains_any<typename Pack::tail>();
        }
    
        template <typename Pack> constexpr static bool contains_any = contains_any_impl<Pack>();
    
    
        
        template <typename Pack> consteval static bool contains_all_impl(void) {
            if constexpr (Pack::size == 0) return true;
            else if constexpr (!contains<typename Pack::head>) return false;
            else return contains_all<typename Pack::tail>();
        }
    
        template <typename Pack> constexpr static bool contains_all = contains_all_impl<Pack>();


        template <typename Pack> consteval static bool is_same_impl(void) {
            if constexpr (size != Pack::size) return false;
            else {
                constexpr bool head_same = std::is_same_v<head, typename Pack::head>;

                if constexpr (size == 1) return head_same;
                else return head_same && tail::template is_same<typename Pack::tail>;
            }
        }

        template <typename Pack> constexpr static bool is_same = is_same_impl<Pack>();
        
    
    
        template <std::size_t N> consteval static auto pop_front_impl(void) {
            if constexpr (N >= size) return ve_beptr(empty){};
            else if constexpr (N == 0) return ve_beptr(self){};
            else return tail::template pop_front_impl<N - 1>();
        }
    
        template <std::size_t N> using pop_front = ve_deptr(pop_front_impl, N);
        template <std::size_t N> using get = typename pop_front<N>::head;
    
    
        
        template <std::size_t N> consteval static auto pop_back_impl(void) {
            if constexpr (N >= size) return ve_beptr(empty){};
            else if constexpr (N == 0) return ve_beptr(self){};
        
            else {
                using tail_t = ve_deptr(tail::template pop_back_impl, N);
                return ve_beptr(typename pack<head>::template append_pack<tail_t>){};
            }
        }
    
        template <std::size_t N> using pop_back = ve_deptr(pop_back_impl, N);


        template <std::size_t N> using first = pop_back<size - N>;
        template <std::size_t N> using last  = pop_front<size - N>;



        constexpr static auto reverse_impl(void) {
            if constexpr (size == 0) return ve_beptr(empty){};
            else {
                using tail_t = ve_deptr(pop_back<1>::reverse_impl);
                return ve_beptr(typename last<1>::template append_pack<tail_t>){};
            }
        }

        using reverse = ve_deptr(reverse_impl);


        constexpr static auto unique_impl(void) {
            if constexpr (size == 0) return ve_beptr(empty){};
            else if constexpr (tail::template contains<head>) return tail::unique_impl();
            else return ve_beptr(typename pack<head>::template append_pack<ve_deptr(tail::unique_impl)>){};
        }

        using unique = typename ve_deptr(reverse::unique_impl)::reverse;


        // Returns the index of the first occurrence of T in the pack. T must be a member of the pack.
        template <typename T> constexpr static std::size_t find_impl(void) {
            if constexpr (std::is_same_v<head, T>) return 0;
            else return tail::template find_impl<T>() + 1;
        }

        template <typename T> constexpr static std::size_t find = find_impl<T>();

        
        // Invokes pred for each element in the pack. Returns early if pred returns false.
        template <typename Pred>
        constexpr static void foreach(Pred&& pred) {
            if constexpr (size == 0) return;
            else {
                using invoke_t = decltype(pred.template operator()<head>());
                
                if constexpr (std::is_same_v<invoke_t, bool>) {
                    if (pred.template operator()<head>()) {
                        tail::template foreach(fwd(pred));
                    }
                } else {
                    pred.template operator()<head>();
                    tail::template foreach(fwd(pred));
                }
            }
        }


        // Equivalent to above, but pred is invoked as pred<T, Index> rather than pred<T>.
        template <typename Pred, std::size_t I = 0>
        constexpr static void foreach_indexed(Pred&& pred) {
            if constexpr (size == 0) return;
            else {
                using invoke_t = decltype(pred.template operator()<head, I>());

                if constexpr (std::is_same_v<invoke_t, bool>) {
                    if (pred.template operator()<head, I>()) {
                        tail::template foreach_indexed<Pred, I + 1>(fwd(pred));
                    }
                } else {
                    pred.template operator()<head, I>();
                    tail::template foreach_indexed<Pred, I + 1>(fwd(pred));
                }
            }
        }
        
        
        template <typename Pred> constexpr static bool all(Pred&& pred) {
            return (pred.template operator()<Ts>() && ...);
        }
    
        template <typename Pred> constexpr static bool any(Pred&& pred) {
            return (pred.template operator()<Ts>() || ...);
        }
    };
    
    
    namespace create_pack {
        namespace detail {
            template <typename> struct from_template {};


            // Constructs a pack from the types in a templated class.
            // E.g.: std::tuple<int, int, int> => pack<int, int, int>.
            template <template <typename...> typename Tmpl, typename... Args>
            struct from_template<Tmpl<Args...>> {
                using type = pack<Args...>;
            };
            

            // Given a decomposable class, construct a pack from all its member types.
            template <typename T> requires is_decomposable_v<T>
            struct from_decomposable {
                using decomposer = decomposer_for<T>;

                constexpr static auto gen_type(void) {
                    // He who rewrites this method shall be doomed to crash Clang.
                    return [] <std::size_t... Is> (std::index_sequence<Is...>) {
                        return pack<typename decomposer::template element_t<Is>...>{};
                    }(std::make_index_sequence<decomposer::size>());
                }

                using type = decltype(gen_type());
            };
        }
        
        
        template <typename T> using from_template     = typename detail::from_template<T>::type;
        template <typename T> using from_decomposable = typename detail::from_decomposable<T>::type;
    }
}