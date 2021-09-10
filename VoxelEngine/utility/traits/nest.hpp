#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    namespace detail {
        template <
            template <typename...> typename... Templates,
            typename Nested
        > struct nest_impl { using type = Nested; };
        
        template <
            template <typename...> typename Template,
            template <typename...> typename... Templates,
            typename Nested
        > struct nest_impl<Template, Templates..., Nested> {
            using type = Template<typename nest_impl<Templates..., Nested>::type>;
        };
    }
    
    
    template <template <typename...> typename... Templates, typename Nested>
    using nest = detail::nest_impl<Templates..., Nested>;
}