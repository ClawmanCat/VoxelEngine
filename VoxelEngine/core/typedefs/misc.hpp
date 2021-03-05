#pragma once

#include <VoxelEngine/core/traits.hpp>

#include <absl/hash/hash.h>
#include <tl/expected.hpp>
#include <tl/optional.hpp>
#include <boost/pfr.hpp>

#include <memory>
#include <utility>
#include <array>
#include <iterator>
#include <stdexcept>


namespace ve {
    // C++-style templates for weird C syntax.
    template <typename Ret, typename... Args>
    using Fn = Ret(*)(Args...);
    
    template <typename Class, typename Ret, typename... Args>
    using MemFn = Ret(Class::*)(Args...);
    
    template <typename Class, typename Ret, typename... Args>
    using ConstMemFn = Ret(Class::*)(Args...) const;
    
    template <typename Class, typename T>
    using MemVar = T(Class::*);
    
    template <typename T, std::size_t N>
    using ArrayRef = T(&)[N];
    
    
    // Smart Pointers
    template <typename T, typename Deleter = std::default_delete<T>>
    using unique = std::unique_ptr<T>;
    
    template <typename T> using shared = std::shared_ptr<T>;
    template <typename T> using weak   = std::weak_ptr<T>;
    
    
    // Monadic std::optional / std::expected
    template <typename T> using optional = tl::optional<T>;
    constexpr inline auto nullopt  = tl::nullopt;
    
    
    using exception_t = std::runtime_error;
    
    template <typename T> using expected = tl::expected<T, exception_t>;
    using unexpected = tl::unexpected<exception_t>;
    
    
    // CTAD doesn't work on alias templates, these can be used instead.
    template <typename T> inline optional<T> make_optional(T&& val) {
        return optional<std::remove_reference_t<T>>(std::forward<T>(val));
    }
    
    template <typename T> inline expected<T> make_expected(T&& val) {
        return expected<std::remove_reference_t<T>>(std::forward<T>(val));
    }
    
    template <typename... Ts> inline unexpected make_unexpected(Ts&&... args) {
        return unexpected { exception_t { std::forward<Ts>(args)... } };
    }
    
    
    // std::reference_wrapper is too long of a name.
    template <typename T> using ref = std::reference_wrapper<T>;
    
    
    // Merge a set of callables into a single functor.
    template <typename... Fns> struct visitor : public Fns... {
        explicit visitor(Fns&&... fns) : Fns(std::forward<Fns>(fns))... {}
        using Fns::operator()...;
        
        // Allow using this type as a heterogeneous comparator for Abseil types.
        using is_transparent = void;
    };
    
    
    // Iterate over each class member. Requires that the class can be decomposed using Boost PFR.
    // Pred should be a type such that at least one of the following statements is valid:
    // - Pred{}.operator()<std::size_t Index>(auto& value)
    // - Pred{}(auto& value)
    template <typename Cls, typename Pred>
    constexpr inline void iterate_class_members(Cls& cls, const Pred& pred) {
        [&] <std::size_t... Indices> (std::index_sequence<Indices...>) {
            ([&] <std::size_t Index> (value<Index>) {
                using member_type   = boost::pfr::tuple_element_t<Index, Cls>;
                member_type& member = boost::pfr::get<Index>();
                
                if constexpr (requires (const Pred& p, member_type& m) { p.template operator()<Index>(m); }) {
                    pred.template operator()<Index>(member);
                } else if constexpr (requires (const Pred& p, member_type& m) { p(m); }) {
                    pred(member);
                } else {
                    // Equivalent to ve::always_false_v.
                    static_assert(!sizeof(member_type*), "Predicate for iterate_class_member cannot be invoked.");
                }
            }(value<Indices>{}), ...);
        }(std::make_index_sequence<boost::pfr::tuple_size_v<Cls>>());
    }
    
    
    // Equivalent to above, but used when no instance of the class is available.
    // Pred should be a type such that at least one of the following statements is valid:
    // - Pred{}.operator()<std::size_t Index, typename T>()
    // - Pred{}.operator()<typename T>()
    template <typename Cls, typename Pred>
    constexpr inline void iterate_class_members(const Pred& pred) {
        [&] <std::size_t... Indices> (std::index_sequence<Indices...>) {
            ([&] <std::size_t Index> (value<Index>) {
                using member_type = boost::pfr::tuple_element_t<Index, Cls>;
            
                if constexpr (requires (const Pred& p) { p.template operator()<Index, member_type>(); }) {
                    pred.template operator()<Index, member_type>();
                } else if constexpr (requires (const Pred& p) { p.template operator()<member_type>(); }) {
                    pred.template operator()<member_type>();
                } else {
                    // Equivalent to ve::always_false_v.
                    static_assert(!sizeof(member_type*), "Predicate for iterate_class_member cannot be invoked.");
                }
            }(value<Indices>{}), ...);
        }(std::make_index_sequence<boost::pfr::tuple_size_v<Cls>>());
    }
}