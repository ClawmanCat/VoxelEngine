#pragma once

#include <VoxelEngine/core/preprocessor.hpp>

#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW
#define GLM_SWIZZLE_RGBA
#define GLM_FORCE_SIZE_T_LENGTH
#include <glm/glm.hpp>
#include <glm/gtx/vec_swizzle.hpp>

#include <boost/preprocessor.hpp>

#include <cstdint>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <set>
#include <chrono>
#include <vector>
#include <utility>
#include <string>
#include <string_view>
#include <span>
#include <numbers>


namespace ve {
    // Literal Namespaces
    using namespace std::string_literals;
    using namespace std::string_view_literals;
    using namespace std::chrono_literals;
    
    
    // Scalar Types
    using i8  =   int8_t;
    using u8  =  uint8_t;
    using i16 =  int16_t;
    using u16 = uint16_t;
    using i32 =  int32_t;
    using u32 = uint32_t;
    using i64 =  int64_t;
    using u64 = uint64_t;
    using f32 = float;
    using f64 = double;
    
    
    static_assert(sizeof(float)  == 32 / 8);
    static_assert(sizeof(double) == 64 / 8);
    
    
    // Constants
    constexpr inline double pi     = std::numbers::pi;
    constexpr inline double sqrt_2 = std::numbers::sqrt2;
    constexpr inline double sqrt_3 = std::numbers::sqrt3;
    
    
    // Weird C-Syntax
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
    using unique = std::unique_ptr<T, Deleter>;
    
    template <typename T> using shared = std::shared_ptr<T>;
    template <typename T> using weak   = std::weak_ptr<T>;
    
    
    // Associative Containers
    template <typename K, typename V, typename Hash = std::hash<K>, typename Equality = std::equal_to<K>>
    using hash_map = std::unordered_map<K, V, Hash, Equality>;
    
    template <typename K, typename Hash = std::hash<K>, typename Equality = std::equal_to<K>>
    using hash_set = std::unordered_set<K, Hash, Equality>;
    
    template <typename K, typename V, typename Compare = std::less<K>>
    using tree_map = std::map<K, V, Compare>;
    
    template <typename K, typename Compare = std::less<K>>
    using tree_set = std::set<K, Compare>;
    
    template <typename K, typename V>
    using vec_map = std::vector<std::pair<K, V>>;
    
    
    // Chrono
    using std::chrono::steady_clock;
    using std::chrono::high_resolution_clock;
    
    template <typename To, typename Rep, typename Period>
    [[nodiscard]] inline auto duration_cast(const std::chrono::duration<Rep, Period>& from) {
        return std::chrono::duration_cast<To>(from);
    }
    
    constexpr inline steady_clock::time_point steady_epoch;
    constexpr inline high_resolution_clock::time_point high_res_epoch;
    
    
    // Missing from the STL.
    inline std::string operator+(const std::string& lhs, std::string_view rhs) {
        return lhs + std::string(rhs);
    }
    
    inline std::string operator+(const std::string_view& lhs, std::string_view rhs) {
        return std::string(lhs).append(rhs);
    }
    
    
    // Enable universal reference behaviour when the template type is known.
    // e.g. a method with signature template<universal<std::string> Str> auto fn(Str&&) { ... }
    // will accept std::string&, const std::string& and std::string&&.
    template <typename T, typename DT = std::remove_cvref_t<T>> concept universal =
        std::is_same_v<T, DT> ||
        std::is_same_v<T, DT&> ||
        std::is_same_v<T, const DT&>;
    
    
    // Equivalent to above, but does not accept const T&.
    template <typename T, typename DT = std::remove_cvref_t<T>> concept mutable_universal =
        std::is_same_v<T, DT> ||
        std::is_same_v<T, DT&>;
    
    
    // Accepts spans with both dynamic and static extents.
    template <typename V, typename T>
    concept span = std::is_same_v<
        std::span<std::remove_const_t<T>, V::extent>,
        std::span<std::remove_const_t<typename V::value_type>, V::extent>
    > && (std::is_const_v<T> || !std::is_const_v<typename V::value_type>);
    
    
    // Accepts any string or string-like object, can be used as universal reference.
    template <typename T, typename DT = std::remove_cvref_t<T>>
    concept stringlike = universal<std::string> || std::is_same_v<DT, std::string_view> || std::is_same_v<DT, const char*>;
    
    
    // GLM
    #define VE_SCALAR_TYPE_SEQ \
    ((i8, b))((u8, ub))((i16, s))((u16, us))((i32, i))((u32, ui))((i64, l))((u64, ul))((f32, f))((f64, d))((bool, bl))
    
    
    template <std::size_t R, typename T>                using vec = glm::vec<R, T>;
    template <std::size_t C, std::size_t R, typename T> using mat = glm::mat<C, R, T>;
    
    template <typename T> using vec2 = vec<2, T>;
    template <typename T> using vec3 = vec<3, T>;
    template <typename T> using vec4 = vec<4, T>;
    
    template <typename T> using mat2 = mat<2, 2, T>;
    template <typename T> using mat3 = mat<3, 3, T>;
    template <typename T> using mat4 = mat<4, 4, T>;

    
    #define VE_GEN_GENERIC_MAT(Rep, Data, Elem)                                                                 \
    template <typename T> using                                                                                 \
    BOOST_PP_SEQ_CAT((mat)(BOOST_PP_TUPLE_ELEM(0, Elem))(x)(BOOST_PP_TUPLE_ELEM(1, Elem)))                      \
    = mat<BOOST_PP_TUPLE_ELEM(0, Elem), BOOST_PP_TUPLE_ELEM(1, Elem), T>;

    BOOST_PP_SEQ_FOR_EACH(
        VE_GEN_GENERIC_MAT,
        _,
        ((2, 2))((2, 3))((2, 4))((3, 2))((3, 3))((3, 4))((4, 2))((4, 3))((4, 4))
    );
    

    #define VE_VEC_TYPES(Type)                                                                                  \
    using BOOST_PP_CAT(vec2, BOOST_PP_TUPLE_ELEM(1, Type)) = vec2<BOOST_PP_TUPLE_ELEM(0, Type)>;                \
    using BOOST_PP_CAT(vec3, BOOST_PP_TUPLE_ELEM(1, Type)) = vec3<BOOST_PP_TUPLE_ELEM(0, Type)>;                \
    using BOOST_PP_CAT(vec4, BOOST_PP_TUPLE_ELEM(1, Type)) = vec4<BOOST_PP_TUPLE_ELEM(0, Type)>;
    
    // Generate typedefs for every vector size for every scalar type in VE_SCALAR_TYPE_SEQ.
    #define VE_GEN_VEC_TYPES(R, D, E) VE_VEC_TYPES(E)
    BOOST_PP_SEQ_FOR_EACH(
        VE_GEN_VEC_TYPES,
        _,
        VE_SCALAR_TYPE_SEQ
    );

    
    // Generate typedefs for quaternions for every scalar type in VE_SCALAR_TYPE_SEQ.
    // (Yes, it is really 'qua' and not 'quat'.)
    #define VE_GEN_QUAT_TYPE(R, D, E) using BOOST_PP_CAT(quat, BOOST_PP_TUPLE_ELEM(1, E)) = glm::qua<BOOST_PP_TUPLE_ELEM(0, E)>;
    BOOST_PP_SEQ_FOR_EACH(
        VE_GEN_QUAT_TYPE,
        _,
        VE_SCALAR_TYPE_SEQ
    );
    
    
    // Generate typedefs for every matrix size for every scalar type in VE_SCALAR_TYPE_SEQ.
    #define VE_SQUARE_MAT_TYPE(S, T)                                                                            \
    using BOOST_PP_CAT(mat##S, BOOST_PP_TUPLE_ELEM(1, T)) = glm::mat<S, S, BOOST_PP_TUPLE_ELEM(0, T)>;
    
    #define VE_MAT_TYPE(C, R, T)                                                                                \
    using BOOST_PP_CAT(mat##C##x##R, BOOST_PP_TUPLE_ELEM(1, T)) = glm::mat<C, R, BOOST_PP_TUPLE_ELEM(0, T)>;    \
                                                                                                                \
    BOOST_PP_IF(                                                                                                \
        BOOST_PP_EQUAL(C, R),                                                                                   \
        VE_SQUARE_MAT_TYPE,                                                                                     \
        VE_EMPTY                                                                                                \
    )(C, T)

    #define VE_MAT_TYPES_C(C, T)                                                                                \
    VE_MAT_TYPE(C, 2, T); VE_MAT_TYPE(C, 3, T); VE_MAT_TYPE(C, 4, T);

    #define VE_MAT_TYPES(T)                                                                                     \
    VE_MAT_TYPES_C(2, T); VE_MAT_TYPES_C(3, T); VE_MAT_TYPES_C(4, T);

    #define VE_GEN_MAT_TYPES(R, D, E) VE_MAT_TYPES(E)
    BOOST_PP_SEQ_FOR_EACH(
        VE_GEN_MAT_TYPES,
        _,
        VE_SCALAR_TYPE_SEQ
    );
}