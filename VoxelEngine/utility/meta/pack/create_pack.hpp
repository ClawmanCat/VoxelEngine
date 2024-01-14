#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>


namespace ve::meta::create_pack {
    namespace detail {
        template <typename> struct from_template {};
        template <template <typename...> typename Tmpl, typename... Ts> struct from_template<Tmpl<Ts...>> { using type = pack<Ts...>; };

        template <type_pack...> struct from_many { using type = pack<>; };
        template <type_pack P, type_pack... Ps> struct from_many<P, Ps...> { using type = typename P::template append_pack<typename from_many<Ps...>::type>; };
    }


    /** Construct a pack from a template instantiation like std::tuple or std::variant. */
    template <typename T> using from_template = typename detail::from_template<T>::type;

    /** Construct a pack from zero or more other packs. */
    template <type_pack... Packs> using from_many = typename detail::from_many<Packs...>::type;
}