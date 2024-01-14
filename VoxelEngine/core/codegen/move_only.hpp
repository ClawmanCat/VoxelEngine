#pragma once


#define VE_IMPL_GENERATE_CONSTRUCTORS(Class, CopyOp, MoveOp, KwConstexpr, KwNoexcept)   \
KwConstexpr Class(const Class&) KwNoexcept = CopyOp;                                    \
KwConstexpr Class(Class&&) KwNoexcept = MoveOp;                                         \
                                                                                        \
KwConstexpr Class& operator=(const Class&) KwNoexcept = CopyOp;                         \
KwConstexpr Class& operator=(Class&&) KwNoexcept = MoveOp;


#define VE_COPYABLE(Class)           VE_IMPL_GENERATE_CONSTRUCTORS(Class, default, default, constexpr,           noexcept)
#define VE_RT_COPYABLE(Class)        VE_IMPL_GENERATE_CONSTRUCTORS(Class, default, default, /* not constexpr */, noexcept)
#define VE_THROWING_COPYABLE(Class)  VE_IMPL_GENERATE_CONSTRUCTORS(Class, default, default, /* not constexpr */, /* not noexcept */)

#define VE_MOVE_ONLY(Class)          VE_IMPL_GENERATE_CONSTRUCTORS(Class, delete,  default, constexpr,           noexcept)
#define VE_RT_MOVE_ONLY(Class)       VE_IMPL_GENERATE_CONSTRUCTORS(Class, delete,  default, /* not constexpr */, noexcept)
#define VE_THROWING_MOVE_ONLY(Class) VE_IMPL_GENERATE_CONSTRUCTORS(Class, delete,  default, /* not constexpr */, /* not noexcept */)

#define VE_IMMOVABLE(Class)          VE_IMPL_GENERATE_CONSTRUCTORS(Class, delete,  delete,  constexpr,           noexcept)