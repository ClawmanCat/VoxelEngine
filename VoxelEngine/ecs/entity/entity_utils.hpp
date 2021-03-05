#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pack.hpp>


#define ve_entity_class(name, entity_base, ...)                             \
class name :                                                                \
    public VE_UNWRAP_BASE(entity_base),                                     \
    public ve::detail::ve_impl_base_recorder<entity_base>                   \
    __VA_OPT__(,) __VA_ARGS__                                               \
{                                                                           \
public:                                                                     \
    using ve::detail::ve_impl_base_recorder<entity_base>::ve_impl_bases;    \
    using ve_impl_this_t = name;                                            \
                                                                            \
private:


namespace ve::detail {
    // Used for asserting an entity class was made using ve_entity_class.
    template <typename Self> struct ve_impl_assert_entity_def_used { };
    
    
    // Keeps track of all entity bases of the current class.
    template <typename Base> struct ve_impl_base_recorder {
        constexpr static auto ve_impl_get_bases(void) {
            if constexpr (requires { Base::ve_impl_get_bases(); }) {
                using old_bases = decltype(Base::ve_impl_get_bases());
                using new_bases = typename old_bases::template append<Base>;
        
                return new_bases { };
            } else {
                return meta::pack<Base> { };
            }
        }
        
        using ve_impl_bases = decltype(ve_impl_get_bases());
    };
}