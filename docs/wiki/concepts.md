# Engine Concepts
This article covers common concepts used throughout the design of the engine. 


### Traits
Some classes, for example `ve::ecs::basic_sparse_set`, `ve::ecs::basic_component_storage` and `ve::ecs::basic_registry` accept a traits class as a template parameter to customize their behaviour.  
For example, the `entity_traits` provided to `basic_sparse_set` allow control over what integral type is used to store entities, and how the bits of this type are allocated between the entity's ID, its version and reserved bits for used-defined purposes.  
Typically, for some set of traits, the engine will provide the following:
- A C++20 concept defining the requirements a traits class must fulfil.
- A default implementation of these traits, typically called `default_*_traits`.
- A traits-transformer class which allows the modification of an existing traits class, typically called `transform_*_traits`.
- A utility class which provides common utility methods associated with the given traits (Only where applicable).
- At least one class which accepts a class fulfilling the concept as a template parameter (The consumer).

```c++
using my_entity_traits = ve::ecs::default_entity_traits<std::uint64_t>;


// Definition of a component_traits class.
struct my_component_traits {
    using type = my_component;
    
    constexpr static inline bool        reference_stability = false;
    constexpr static inline bool        elude_storage       = false;
    constexpr static inline std::size_t page_size           = 1024ull;
};

static_assert(ve::ecs::component_traits<my_component_traits>, "Concept component_traits not fulfilled!");


// Using the traits transformer to change the associated component_type.
using my_other_component_traits = typename transform_component_traits<my_component_traits>
    ::template with_component_type<my_other_component>;


// Traits class can be used with its associated consumer.
ve::ecs::basic_component_storage<
    my_entity_traits,
    my_component_traits
> my_component_storage;

ve::ecs::basic_component_storage<
    my_entity_traits,
    my_other_component_traits
> my_other_component_storage;
```


### Mixins
In scenarios where making an object observable would have a too great performance impact, the engine uses mixins to provide the possibility to add callbacks to a class.  
For each class that accepts a mixin, there is also a CRTP-interface which can be implemented to provide callbacks. The class implementing this interface can then be passed to the observed object as a template parameter:

```c++
using my_entity_traits    = ...;
using my_component_traits = ...;

struct component_mixin : ve::ecs::component_storage_mixin<component_mixin, my_entity_traits, my_component_traits> {
    using entity    = typename my_entity_traits::type;
    using component = typename my_component_traits::type;
    
    void on_insert(entity entt, component& cmp, std::size_t index) {
        // Handle component added to storage.
    }
    
    void on_erase(entity entt, component&& cmp, std::size_t index) {
        // Handle component removed from storage.
    }
};

// Mixin must be added to its parent as a template parameter.
ve::ecs::basic_component_storage<
    my_entity_traits, 
    my_component_traits, 
    ve::ecs::no_sparse_set_mixin<my_entity_traits>, 
    component_mixin
> storage;

// Will call component_mixin::on_insert
storage.emplace(some_entity, my_component_args...);
```

Engine classes that internally use a class that accepts a mixin will usually accept a template parameter (Either directly or through some traits class)
which allows users to set the mixin.


### Type Packs
The engine provides the class template `ve::meta::pack`, which serves as a list of types and provides several utility methods to work with this list.
Many classes and methods that require a list of typenames as an argument use this class, as it does not require the weird semantics and limitations that come with normal variadic templates (e.g. not being able to `typedef` them).  
The concept `ve::meta::type_pack` indicates that a template parameter should be filled with a type pack. The type pack may also be used by users of the engine to perform template metaprogramming.

```c++
template <ve::meta::type_pack P> void some_engine_fn(...) { ... }


using my_types = ve::meta::pack<int, char, bool, std::string>;

using my_const_pointers = typename my_types
    ::template map<std::add_const_t>
    ::template map<std::add_pointer_t>;


some_engine_fn<my_const_pointers>(...);
```


### Typename / Constant Wrapping
`ve::meta::value` and `ve::meta::type` are typedefs respectively for `std::integral_constant` (with a deduced value type) and `std::type_identity`. 
They are used to easily pass typenames as parameters when metaprogramming (in a similar way to how `std::integer_sequence` is used), for example by allowing template argument deduction when used as a parameter, or to return a typename from a function.

```c++
template <typename Head, typename... Tail> consteval void foreach_type(auto fn) {
    std::invoke(fn, ve::meta::type<Head>{});
    if constexpr (sizeof...(Tail) > 0) foreach_type<Tail...>(fn);
}

foreach_type<int, char, bool, long, std::string>([] <typename T> (ve::meta::type<T>) {
    ve::get_service<ve::engine_logger>().info("Iterating over type {}", ve::typename_of<T>());
});
```


### Stop Tokens
The engine provides various functionalities to perform `foreach` operations on data structures that do not typically support it,
like `std::tuple`, `meta::pack` and `std::index_sequence`.  
When using these functionalities, the class `meta::stop_token<T>` can be used to return to break from the loop and/or return a value.
Given a function `f` used as a callback within such a foreach method, this works as follows:
- If `f` always returns void the return type of the foreach function is also void.
- If `f` returns a stop token with `.stop = true`, this will break from the loop.
- If said stop token has a value (i.e. it is not `meta::stop_token<void>`), that value will be returned from the foreach function as a variant (see below).
- Given `R`, the set of all non-void value types of stop tokens that could be returned from `f`, the foreach function has a return type of `std::variant<R..., meta::null_type>`.
- If none of the stop tokens returned by `f` have `.stop = true`, the returned variant has a value of `meta::null_type`.

```c++
auto tpl = std::tuple<std::string, bool, int, char> { ... };

auto result = ve::tuple_foreach(tpl, [] <typename E> (const E& e) {
    if constexpr (std::is_same_v<E, std::string>) {
        return meta::stop_token<std::string*> { .payload = &e, .stop = (e == "Meow") };
    }
    
    if constexpr (std::is_same_v<E, int>) {
        return meta::stop_token<void> { .stop = (e == 3) };
    }
    
    if constexpr (std::is_same_v<E, char>) {
        return meta::stop_token<char> { .payload = e };
    }
});
```

In the above example, the type of `result` is `std::variant<std::string*, char, meta::null_type>`.
- If the value of the string-element of `tpl` was `"Meow"`, the foreach method will break after that element and the variant will contain a pointer to that string.
- If the value of the int-element of `tpl` was `3`, the foreach method will break after that element and the variant will contain `meta::null_type`.
- Otherwise, the variant will contain the char-element of `tpl`.

