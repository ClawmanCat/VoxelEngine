#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    template <typename Derived> struct binary_serializable {
        using binary_serializable_tag = void;
        
        [[nodiscard]] std::vector<u8> to_bytes(void) const {
            VE_CRTP_CHECK(Derived, to_bytes);
            return static_cast<const Derived*>(this)->to_bytes();
        }
        
        static Derived from_bytes(std::span<u8> bytes) {
            VE_STATIC_CRTP_CHECK(Derived, from_bytes);
            return Derived::from_bytes(bytes);
        }
        
        // Optional method: estimate the size of an object of this class.
        // Default implementation recursively calls size_estimate on this class' members,
        // using sizeof(member) if the member does not implement this method.
        constexpr static u32 size_estimate(void) {
            if constexpr (VE_STATIC_CRTP_IS_IMPLEMENTED(Derived, size_estimate)) {
                // Derived class has its own implementation, use that.
                return Derived::size_estimate();
            } else {
                // Otherwise sum member size estimates.
                std::size_t accumulated_size = 0;
                
                iterate_class_members<Derived>([&] <typename Member> () {
                    if constexpr (requires { Member::size_estimate(); }) {
                        accumulated_size += Member::size_estimate();
                    } else {
                        accumulated_size += sizeof(Member);
                    }
                });
                
                return accumulated_size;
            }
        }
    };
    
    
    
}