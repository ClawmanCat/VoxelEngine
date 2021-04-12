#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    template <typename Derived> class externally_serializable {
    public:
        using externally_serializable_tag = void;
        
        
        void on_serialized(void) {
            VE_CRTP_CHECK(Derived, on_serialized);
            static_cast<Derived*>(this)->on_serialized();
        }
    
    
        void on_deserialized(void) {
            VE_CRTP_CHECK(Derived, on_deserialized);
            static_cast<Derived*>(this)->on_deserialized();
        }
    };
}