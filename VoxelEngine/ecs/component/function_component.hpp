#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>
#include <VoxelEngine/utility/traits/pack.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/ecs/component/component.hpp>
#include <VoxelEngine/ecs/component/named_component.hpp>
#include <VoxelEngine/side/side.hpp>

#include <ctti/type_id.hpp>

#include <type_traits>


namespace ve {
    namespace detail {
        template <typename Ret, typename... Args>
        inline std::string signature_string(void) {
            std::string result;
            
            for (const auto& arg : { ctti::nameof<Args>()... }) result += arg.cppstring() + " ";
            result += "=> " + ctti::nameof<Ret>().cppstring();
            
            return result;
        }
    }
    
    
    struct signature {
        ctti::type_id_t return_type;
        small_vector<ctti::type_id_t> argument_types;
        VE_DEBUG_ONLY(std::string signature_string);
        
        
        template <typename Ret, typename... Args>
        explicit signature(Fn<Ret, Args...>) :
            return_type(ctti::type_id<Ret>()),
            argument_types({ ctti::type_id<Args>()... })
        {
            VE_DEBUG_ONLY(signature_string = detail::signature_string<Ret, Args...>());
        }
        
        
        template <typename Ret, typename... Args>
        explicit signature(meta::pack<Ret, Args...>) :
            return_type(ctti::type_id<Ret>()),
            argument_types({ ctti::type_id<Args>()... })
        {
            VE_DEBUG_ONLY(signature_string = detail::signature_string<Ret, Args...>());
        }
        
        
        [[nodiscard]] bool operator==(const signature& o) const {
            if (return_type != o.return_type) return false;
            
            for (const auto& [my_arg, other_arg] : views::zip(argument_types, o.argument_types)) {
                if (my_arg != other_arg) return false;
            }
            
            return true;
        }
    };
    
    
    template <side Side = side::SERVER>
    class function_component : public component<function_component<Side>, Side, component_serialization_mode::BINARY> {
    public:
        template <typename Ret, typename... Args>
        explicit function_component(Fn<Ret, Args...> fn) :
            fn_signature(fn),
            fn_pointer((void*) fn)
        {}
    
        
        // Automatic deduction of Args is prevented here since it is very easy to pass T& instead of T, or vice versa.
        template <typename Ret, typename... Args>
        Ret invoke_unchecked(dont_deduce<Args> auto... args) const {
            return ((Fn<Ret, Args...>) fn_pointer)(std::forward<Args>(args)...);
        }
    
    
        // Automatic deduction of Args is prevented here since it is very easy to pass T& instead of T, or vice versa.
        template <typename Ret, typename... Args>
        Ret invoke_checked(dont_deduce<Args> auto... args) const {
            VE_DEBUG_ONLY(
                auto sigstring = detail::signature_string<Ret, Args...>();
                
                VE_ASSERT(
                    (fn_signature == signature { meta::pack<Ret, Args...>{ } }),
                    "Attempt to call function component with invalid return type or arguments:\n"s +
                    "expected " + fn_signature.signature_string + ", got " + sigstring + "."
                );
            );
    
            VE_RELEASE_ONLY(
                VE_ASSERT(
                    (fn_signature == signature { meta::pack<Ret, Args...>{ } }),
                    "Attempt to call function component with invalid return type or arguments."
                );
            );
            
            
            return invoke_unchecked<Ret, Args...>(std::forward<Args>(args)...);
        }
    
    
        // Automatic deduction of Args is prevented here since it is very easy to pass T& instead of T, or vice versa.
        template <typename Ret, typename... Args>
        Ret operator()(dont_deduce<Args> auto... args) const {
            VE_DEBUG_ONLY(invoke_checked<Ret, Args...>(std::forward<Args>(args)...));
            VE_RELEASE_ONLY(invoke_unchecked<Ret, Args...>(std::forward<Args>(args)...));
        }
        
    
        [[nodiscard]] std::vector<u8> to_bytes(void) const {
            // TODO: ID Lookup
        }
        
    
        static function_component<Side> from_bytes(std::span<u8> bytes) {
            // TODO: ID Lookup
        }
        
    private:
        signature fn_signature;
        void* fn_pointer;
    };
    
    
    template <meta::string_arg Name, side Side>
    using named_function_component = named_component<Name, function_component<Side>>;
}