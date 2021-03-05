#pragma once

#include <VoxelEngine/core/typedefs/misc.hpp>

#include <boost/preprocessor.hpp>
#include <boost/container_hash/hash.hpp>
#include <absl/hash/hash.h>

#include <string>
#include <string_view>
#include <compare>
#include <tuple>
#include <utility>
#include <type_traits>


#define ve_eq_comparable(cls)                                           \
[[nodiscard]] bool operator==(const cls&) const = default;              \
[[nodiscard]] bool operator!=(const cls&) const = default;


#define ve_comparable(cls)                                              \
[[nodiscard]] auto operator<=>(const cls&) const = default;             \
[[nodiscard]] bool operator== (const cls&) const = default;             \
[[nodiscard]] bool operator!= (const cls&) const = default;


#define VE_IMPL_NEQ_RETURN(Rep, Data, Elem) if (Elem != o.Elem) return false;

#define ve_eq_comparable_fields(cls, ...)                               \
[[nodiscard]] bool operator==(const cls& o) const {                     \
    BOOST_PP_SEQ_FOR_EACH(                                              \
        VE_IMPL_NEQ_RETURN,                                             \
        _,                                                              \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                           \
    );                                                                  \
                                                                        \
    return true;                                                        \
}


#define VE_IMPL_NCMP_RETURN(Rep, Data, Elem) \
if (auto cmp = Elem <=> o.Elem; cmp != std::strong_ordering::equal) return cmp;

#define ve_comparable_fields(cls, ...)                                  \
[[nodiscard]] std::strong_ordering operator<=>(const cls& o) const {    \
    BOOST_PP_SEQ_FOR_EACH(                                              \
        VE_IMPL_NCMP_RETURN,                                            \
        _,                                                              \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                           \
    );                                                                  \
                                                                        \
    return std::strong_ordering::equal;                                 \
}                                                                       \
                                                                        \
ve_eq_comparable_fields(cls, __VA_ARGS__)


// Allow overloading the hash operator from within the class so we can use ve_hashable
// where out-of-class definitions are not possible.
template <typename T> requires requires (T t, std::size_t h) { h = t.hash(); }
struct std::hash<T> {
    std::size_t operator()(const T& val) const {
        return val.hash();
    }
};


#define VE_IMPL_HASH_COMBINE(Rep, Data, Elem) boost::hash_combine(seed, this->Elem);

#define ve_hashable(...)                                \
[[nodiscard]] std::size_t hash(void) const {            \
    std::size_t seed = 0;                               \
                                                        \
    BOOST_PP_SEQ_FOR_EACH(                              \
        VE_IMPL_HASH_COMBINE,                           \
        _,                                              \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)           \
    );                                                  \
                                                        \
    return seed;                                        \
}


// Marks the given class as being non-copyable.
#define ve_move_only(cls)                                                       \
cls(const cls&) = delete;                                                       \
cls& operator=(const cls&) = delete;                                            \
                                                                                \
cls(cls&&) = default;                                                           \
cls& operator=(cls&&) = default;


#define VE_IMPL_SWAP_MEMBER(R, D, E) std::swap(E, o.E);

// Marks the given class as being non-copyable,
// and makes moving the type require swapping the given members.
// (The default move constructor only copies for trivial types.)
#define ve_swap_move_only(cls, ...)                                             \
cls(const cls&) = delete;                                                       \
cls& operator=(const cls&) = delete;                                            \
                                                                                \
cls(cls&& o) { *this = std::move(o); }                                          \
                                                                                \
cls& operator=(cls&& o) {                                                       \
    BOOST_PP_SEQ_FOR_EACH(                                                      \
        VE_IMPL_SWAP_MEMBER,                                                    \
        _,                                                                      \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                   \
    );                                                                          \
                                                                                \
    return *this;                                                               \
}


// Marks the given class as being non-copyable and non-movable.
#define ve_immovable(cls)                                                       \
cls(const cls&) = delete;                                                       \
cls& operator=(const cls&) = delete;                                            \
                                                                                \
cls(cls&&) = delete;                                                            \
cls& operator=(cls&&) = delete;


// Can be used to easily copy the const version of a function into a non-const one, having the same behaviour.
//
// e.g. given a function const RetType my_fn(Args...) const { ... }, generates a function RetType my_fn(Args...)
// which calls the original version of the function and casts away the const qualifier of the result.
namespace ve::detail {
    template <typename> struct cmf_signature { };
    
    template <typename Cls, typename Ret, typename... Args>
    struct cmf_signature<ConstMemFn<Cls, Ret, Args...>> {
        using class_type  = Cls;
        using return_type = Ret;
    };
}


#define VE_IMPL_OVERLOAD_NON_CONST(fn, ret_t, const_this_t)                 \
template <typename... Args>                                                 \
VE_UNWRAP(ret_t) fn(Args&&... args) {                                       \
    return const_cast<VE_UNWRAP(ret_t)>(((const_this_t*) this)              \
        ->fn(std::forward<Args>(args)...));                                 \
}

#define VE_OVERLOAD_NON_CONST(cls, fn)                                      \
VE_IMPL_OVERLOAD_NON_CONST(                                                 \
    fn,                                                                     \
    std::remove_const_t<                                                    \
        typename ve::detail::cmf_signature<decltype(&cls::fn)>::return_type \
    >,                                                                      \
    std::add_const_t<                                                       \
        typename ve::detail::cmf_signature<decltype(&cls::fn)>::class_type  \
    >                                                                       \
)


// Wrap the given variable.
#define VE_WRAP_MEMBER(value)                                           \
operator decltype(value)& (void) { return value; }                      \
operator const decltype(value)& (void) const { return value; }          \
                                                                        \
decltype(value)& operator*(void) { return value; }                      \
const decltype(value)& operator*(void) const { return value; }          \
                                                                        \
decltype(value)* operator->(void) { return &value; }                    \
const decltype(value)* operator->(void) const { return &value; }


namespace ve {
    inline std::string operator+(const std::string& lhs, const std::string_view& rhs) {
        return lhs + std::string(rhs.begin(), rhs.end());
    }
    
    inline std::string operator+(const std::string_view& lhs, const std::string& rhs) {
        return std::string(lhs.begin(), lhs.end()) + rhs;
    }
    
    inline std::string operator+(const std::string_view& lhs, const std::string_view& rhs) {
        return std::string(lhs.begin(), lhs.end()) + std::string(rhs.begin(), rhs.end());
    }
    
    
    // Bitwise operations on enums.
    template <typename Enum> requires std::is_enum_v<Enum>
    constexpr inline Enum operator|(Enum a, Enum b) {
        using utype = std::underlying_type_t<Enum>;
        
        return (Enum) ((utype) a | (utype) b);
    }
    
    template <typename Enum> requires std::is_enum_v<Enum>
    constexpr inline Enum operator&(Enum a, Enum b) {
        using utype = std::underlying_type_t<Enum>;
        
        return (Enum) ((utype) a & (utype) b);
    }
}