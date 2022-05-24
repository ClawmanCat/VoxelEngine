#pragma once

#include <VoxelEngine/core/platform.hpp>
#include <VoxelEngine/core/preprocessor.hpp>

#include <boost/preprocessor.hpp>
#include <boost/vmd/is_empty.hpp>

#include <memory>
#include <type_traits>


// Casts the current object to the given base and invokes the postconstructor on it.
// Note: if we received the empty tuple as our base list, the macro will be called with E empty.
#define ve_impl_postconstructor_basecall(R, D, E)       \
BOOST_PP_EXPR_IF(                                       \
    BOOST_PP_NOT(BOOST_VMD_IS_EMPTY(E)),                \
    E::ve_impl_postconstructor()                        \
);


// Calls F if F is non-empty.
#define ve_impl_call_if_nonempty(F)                     \
BOOST_PP_IF(                                            \
    BOOST_PP_NOT(BOOST_VMD_IS_EMPTY(F)),                \
    F,                                                  \
    ve::detail::no_op                                   \
)

namespace ve::detail { inline void no_op(void) {} }


// Makes it so that the class it is in can only be constructed as a smart pointer, through the create method.
// This is useful e.g. for classes that use std::shared_from_this or need to call some virtual method after the constructor.
// Arguments are as follows:
//    ptr_type:    The type of pointer to construct. Can be "unique" or "shared", or any other pointer type,
//                 as long as ve::ptr_type<T> and ve::make_ptr_type<T>(...) are defined.
//    cls:         The name of the current class.
//    base_tpl:    Either an empty tuple, a single base class, or a tuple of base classes.
//                 These are required for invoking the postconstructor actions on the provided base classes.
//    post_action: The name of a zero-argument class member function which will be called after the constructor completes.
//                 This is useful for doing things that cannot be done in a constructor, like making virtual calls.
//    __VA_ARGS__: The argument list for the constructor, exactly as one would provide it normally.
//                 Usage of default arguments is supported.
// The body of the constructor (including any initializer list) should be appended after the macro.
// Different versions of this macro are provided below, which default one of more of the macro arguments.
// Additional constructors can be provided with ve_additional_ctor.
// Note: usage of this macro sets the visibility after it to public, even if this is not the default.
// You should make sure to place this macro in a public section of your class, or restore the visibility afterwards.
#define ve_impl_smart_ptr_only(ptr_type, cls, base_tpl, post_action, ...)                               \
private:                                                                                                \
    /* Private & explicit-only constructable, so cannot be constructed outside this class. */           \
    struct ve_impl_hidden_t { explicit ve_impl_hidden_t(void) = default; };                             \
    using ve_impl_ptr_type = ve::ptr_type<cls>;                                                         \
protected:                                                                                              \
    /* Optional post-construct action. Also calls post-construct actions for base classes. */           \
    VE_NO_IDE_ONLY(                                                                                     \
        void ve_impl_postconstructor(void) {                                                            \
            BOOST_PP_SEQ_FOR_EACH(                                                                      \
                ve_impl_postconstructor_basecall,                                                       \
                _,                                                                                      \
                BOOST_PP_TUPLE_TO_SEQ((BOOST_PP_REMOVE_PARENS(base_tpl)))                               \
            );                                                                                          \
                                                                                                        \
            ve_impl_call_if_nonempty(post_action)();                                                    \
        }                                                                                               \
    )                                                                                                   \
                                                                                                        \
    /* Protected constructor so derived classes can be constructed.   */                                \
    /* Will show actual arguments instead of "auto&&... args" in IDEs */                                \
    VE_IDE_ONLY(cls(__VA_ARGS__);)                                                                      \
    VE_NO_IDE_ONLY(                                                                                     \
        /* First type cannot be the hidden type, or constructor would be ambiguous. */                  \
        template <typename... Ts>                                                                       \
        requires (!std::is_same_v<std::decay_t<Ts>, ve_impl_hidden_t> && ...)                           \
        cls(Ts&&... ts) : cls(ve_impl_hidden_t { }, fwd(ts)...) {}                                      \
    )                                                                                                   \
                                                                                                        \
public:                                                                                                 \
    /* Create method can be used to actually construct the object. */                                   \
    /* Will show actual arguments instead of "auto&&... args" in IDEs */                                \
    VE_IDE_ONLY(ve::ptr_type<cls> create(__VA_ARGS__);)                                                 \
    VE_NO_IDE_ONLY(                                                                                     \
        static ve::ptr_type<cls> create(auto&&... args) {                                               \
            auto ptr = ve::make_##ptr_type<cls>(ve_impl_hidden_t { }, fwd(args)...);                    \
            ptr->ve_impl_postconstructor();                                                             \
            return ptr;                                                                                 \
        }                                                                                               \
    );                                                                                                  \
                                                                                                        \
    /* Constructor is public so we can call make_shared / make_unique.        */                        \
    /* It cannot actually be called because ve_impl_hidden_t cannot be named. */                        \
    /* Body of the constructor is provided after the macro.                   */                        \
    cls(ve_impl_hidden_t, __VA_ARGS__)


#define ve_unique_only(cls, ...)                                   ve_impl_smart_ptr_only(unique, cls, (),    /* No Action */, __VA_ARGS__)
#define ve_unique_only_then(cls, then, ...)                        ve_impl_smart_ptr_only(unique, cls, (),    then,            __VA_ARGS__)
#define ve_derived_unique_only(cls, bases, ...)                    ve_impl_smart_ptr_only(unique, cls, bases, /* No Action */, __VA_ARGS__)
#define ve_derived_unique_only_then(cls, bases, then, ...)         ve_impl_smart_ptr_only(unique, cls, bases, then,            __VA_ARGS__)

#define ve_shared_only(cls, ...)                                   ve_impl_smart_ptr_only(shared, cls, (),    /* No Action */, __VA_ARGS__)
#define ve_shared_only_then(cls, then, ...)                        ve_impl_smart_ptr_only(shared, cls, (),    then,            __VA_ARGS__)
#define ve_derived_shared_only(cls, bases, ...)                    ve_impl_smart_ptr_only(shared, cls, bases, /* No Action */, __VA_ARGS__)
#define ve_derived_shared_only_then(cls, bases, then, ...)         ve_impl_smart_ptr_only(shared, cls, bases, then,            __VA_ARGS__)


// Defines an additional constructor for a class already marked with ve_impl_smart_ptr_only.
// Note: if a postconstructor action was provided with ve_impl_smart_ptr_only, attempting to call create for this constructor
// will still invoke that action.
#define ve_additional_ctor(cls, ...)                                                                    \
VE_IDE_ONLY(ve_impl_ptr_type create(__VA_ARGS__);)                                                      \
cls(ve_impl_hidden_t, __VA_ARGS__)