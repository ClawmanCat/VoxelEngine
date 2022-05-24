#pragma once


namespace ve {
    // Indicates an overload of a function can be overridden, while some other overload cannot. E.g.:
    //
    // struct some_cls {
    //    void my_fn(int x, int y) { do_something(x, y); my_fn(x, y, overridable_function_tag{}); }
    //    virtual void my_fn(int x, int y, overridable_function_tag) = 0;
    // };
    struct overridable_function_tag {};
}