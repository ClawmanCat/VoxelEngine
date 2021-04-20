#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/side/side.hpp>
#include <VoxelEngine/ecs/component/component.hpp>
#include <VoxelEngine/ecs/component/value_component.hpp>
#include <VoxelEngine/ecs/component/named_component.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>
#include <VoxelEngine/utility/traits/member_function_traits.hpp>


namespace ve::detail {
    // If T is a component, returns T, otherwise returns named_value_component<T>.
    template <
        meta::string_arg Name,
        typename T,
        side Side                        = side::SERVER,
        component_serialization_mode CSM = component_serialization_mode::BINARY,
        typename Serializer              = meta::null_type
    > using underlying_component_t = std::conditional_t<
        (requires { typename T::component_tag_t; }),
        T,
        named_value_component<Name, T, Side, CSM, Serializer>
    >;
    
    
    // Invokes the provided function component with args,
    // reconstructing the function signature from the provided member function type.
    template <typename MemFn> struct fc_invoke_wrapper {
        using Traits = ve::meta::mft<MemFn>;
        
        template <typename... Args> struct inner {
            constexpr static typename Traits::return_type invoke(auto& cmp, void* self, Args&&... args) {
                cmp.template invoke_unchecked<
                    typename Traits::return_type,
                    void*,
                    Args...
                >(self, std::forward<Args>(args)...);
            }
            
            constexpr static typename Traits::return_type invoke(auto& cmp, const void* self, Args&&... args) {
                cmp.template invoke_unchecked<
                    typename Traits::return_type,
                    const void*,
                    Args...
                >(self, std::forward<Args>(args)...);
            }
        };
        
        using type = typename Traits::argument_types::template expand_inside<inner>;
    };
    
    
    // Class for managing assiging to a VE_COMPONENT as if it was a class member.
    // This allows syntax like int VE_COMPONENT(my_int) = 3;
    // Properties cannot have a default initializer, so we have to run a lambda at object creation to invoke the setter.
    template <typename Assigner>
    struct assignment_helper {
        Assigner assigner;
        
        explicit assignment_helper(Assigner&& assigner) : assigner(std::move(assigner)) {}
        
        
        // In the component macro, there will be a definition like:
        // [[no_unique_address]] null_type impl = some_lambda() = user provided arg;
        // or, if no arguments are provided after the macro:
        // [[no_unique_address]] null_type impl = some_lambda();
        // where some lambda returns an instance of this class, initialized with the setter as assigner,
        // which is then invoked with the user provided argument by this operator,
        // before being cast to null_type and assigned to impl.
        template <typename Arg>
        meta::null_type operator=(Arg arg) const {
            assigner(std::forward<Arg>(arg));
            return {};
        }
        
        // Handle the case where no argument is provided.
        operator meta::null_type(void) const {
            assigner();
            return {};
        }
    };
}