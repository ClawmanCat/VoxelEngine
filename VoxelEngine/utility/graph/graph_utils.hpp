#pragma once

#include <VoxelEngine/core/core.hpp>

#include <range/v3/view.hpp>


namespace ve::graph {
    enum class visitor_state { CONTINUE, STOP };


    /**
     * Transforms a view that returns the connected vertices of a given vertex to return edges instead.
     * @param from A pointer to the vertex to which other vertices connect.
     * @return A range-adapter that accepts vertex-pointers and transforms them to pairs [from, to].
     */
    constexpr inline auto connected_vertices_to_edges(auto* from) {
        return views::transform([from] (auto* to) { return std::pair { from, to }; });
    }


    // Equivalent to VE_TRY_CRTP_CALL, but if the derived method returns void 'Default' is returned.
    #define VE_IMPL_GRAPH_CRTP_VISITOR_METHOD(Self, Derived, Fn, ArgTuple, Default)                         \
    return [&] {                                                                                            \
        if constexpr (VE_CRTP_IS_OVERRIDDEN_WITH_ANY_SIGNATURE(Self, Derived, Fn)) {                        \
            using result_t = decltype(VE_CRTP_CALL(Self, Derived, Fn, BOOST_PP_TUPLE_ENUM(ArgTuple)));      \
                                                                                                            \
            if constexpr (std::is_void_v<result_t>) {                                                       \
                VE_CRTP_CALL(Self, Derived, Fn, BOOST_PP_TUPLE_ENUM(ArgTuple));                             \
                return Default;                                                                             \
            } else {                                                                                        \
                return VE_CRTP_CALL(Self, Derived, Fn, BOOST_PP_TUPLE_ENUM(ArgTuple));                      \
            }                                                                                               \
        } else return Default;                                                                              \
    } ()
}