#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    namespace detail {
        template <
            typename Nested,
            template <typename...> typename... Templates
        > struct nest_impl { using type = Nested; };
        
        template <
            typename Nested,
            template <typename...> typename Template,
            template <typename...> typename... Templates
        > struct nest_impl<Nested, Template, Templates...> {
            using type = Template<typename nest_impl<Nested, Templates...>::type>;
        };
    }
    
    
    template <typename Nested, template <typename...> typename... Templates>
    using nest = detail::nest_impl<Nested, Templates...>;
}