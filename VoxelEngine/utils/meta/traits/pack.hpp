#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/traits/null_type.hpp>
#include <VoxelEngine/utils/meta/traits/is_of_template.hpp>


namespace ve::meta {
    namespace detail {
        // Num types = 0
        template <template <typename...> typename Pack, typename...> struct splitter {
            using head = null_type;
            using tail = Pack<>;
        };
        
        // Num types = 1
        template <template <typename...> typename Pack, typename T0, typename... Ts> struct splitter<Pack, T0, Ts...> {
            using head = T0;
            using tail = Pack<>;
        };
        
        // Num types > 1
        template <template <typename...> typename Pack, typename T0, typename T1, typename... Ts> struct splitter<Pack, T0, T1, Ts...> {
            using head = T0;
            using tail = Pack<T1, Ts...>;
        };
    
    
    
        // Num types = 0
        template <template <typename...> typename Pack, typename...> struct reverser {
            using type = Pack<>;
        };
    
        // Num types = 1
        template <template <typename...> typename Pack, typename T0, typename... Ts> struct reverser<Pack, T0, Ts...> {
            using type = Pack<T0>;
        };
    
        // Num types > 1
        template <template <typename...> typename Pack, typename T0, typename T1, typename... Ts> struct reverser<Pack, T0, T1, Ts...> {
            using type = typename reverser<Pack, T1, Ts...>::type::template append<T0>;
        };
    }
    
    
    template <typename... Ts> class pack {
    public:
        template <typename... Xs> friend class pack;
        
        
        using self = pack<Ts...>;
        
        constexpr static std::size_t size = sizeof...(Ts);
        
        using head    = typename detail::splitter<pack, Ts...>::head;
        using tail    = typename detail::splitter<pack, Ts...>::tail;
        using reverse = typename detail::reverser<pack, Ts...>::type;
        
        
        template <template <typename...> typename F> using expand_inside  = F<Ts...>;
        template <template <typename...> typename F> using expand_outside = pack<F<Ts>...>;
    
    
        template <typename... Xs> using append  = pack<Ts..., Xs...>;
        template <typename... Xs> using prepend = pack<Xs..., Ts...>;
        
        template <typename Pack> using append_pack  = typename Pack::template expand_inside<append>;
        template <typename Pack> using prepend_pack = typename Pack::template expand_inside<prepend>;
        
    private:
        template <std::size_t N> constexpr static auto pop_front_impl(void) {
            if constexpr (size == 0 || N == 0) return self { };
            else return tail::template pop_front_impl<N - 1>();
        }
    
        template <std::size_t N> constexpr static auto pop_back_impl(void) {
            if constexpr (N >= size) return pack<> { };
            else if constexpr (N == 0) return self { };
            else return typename pack<head>::template join<decltype(tail::template pop_back_impl<N>())> { };
        }
    public:
        template <std::size_t N = 1> using pop_front = decltype(pop_front_impl<N>());
        template <std::size_t N = 1> using pop_back  = decltype(pop_back_impl<N>());
        
        
        template <typename T> constexpr static bool contains(void) {
            if constexpr (size > 0) {
                return std::is_same_v<head, T> || tail::template contains<T>();
            } else return false;
        }
        
        
        // Returns pred(pred(pred(Init, E0), E1), E2) ...
        // where E[N] is a new pack containing the Nth element of the current pack.
        template <typename Pred, typename Init>
        constexpr static auto left_fold(Init&& init, const Pred& pred) {
            if constexpr (size == 0) return std::forward<Init>(init);
            else return pred(
                pop_front<>::template left_fold(std::forward<Init>(init), pred),
                pack<head>()
            );
        }
    
        // Returns pred(Init, pred(E0, pred(E1, pred(E2, ...)))
        // where E[N] is a new pack containing the Nth element of the current pack.
        template <typename Pred, typename Init>
        constexpr static auto right_fold(Init&& init, const Pred& pred) {
            if constexpr (size == 0) return std::forward<Init>(init);
            else return pred(std::forward<Init>(init), tail::template right_fold(pack<head> { }, pred));
        }
        
        
        template <typename Pred>
        constexpr static bool all(const Pred& pred) {
            if constexpr (size == 0) return true;
            else return pred.template operator()<head>() && tail::template all(pred);
        }
    
    
        template <typename Pred>
        constexpr static bool any(const Pred& pred) {
            if constexpr (size == 0) return false;
            else return pred.template operator()<head>() || tail::template all(pred);
        }
    };
    
    
    template <typename T> concept type_pack = is_of_template_v<pack, T>;
}