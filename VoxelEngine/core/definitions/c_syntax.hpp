#pragma once

#include <cstddef>


namespace ve {
    /** A function pointer with return type 'Ret' and arguments 'Args'. */
    template <typename Ret, typename... Args>
    using fn = Ret(*)(Args...);

    /** A member function pointer of class 'Cls' with return type 'Ret' and arguments 'Args'. */
    template <typename Cls, typename Ret, typename... Args>
    using mem_fn = Ret(Cls::*)(Args...);

    /** A const member function pointer of class 'Cls' with return type 'Ret' and arguments 'Args'. */
    template <typename Cls, typename Ret, typename... Args>
    using const_mem_fn = Ret(Cls::*)(Args...) const;

    /** A member variable pointer of class 'Cls' with type 'T'. */
    template <typename Cls, typename T>
    using mem_var = T(Cls::*);

    /** A reference to an array with elements of type 'T' and size 'N'. */
    template <typename T, std::size_t N>
    using array_reference = T(&)[N];
}