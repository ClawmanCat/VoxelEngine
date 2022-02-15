#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/stack_polymorph.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


struct Base {
    enum derived_type { A, B, C, D };

    virtual ~Base(void) = default;
    virtual derived_type get_derived_type(void) const = 0;
};

struct A : public Base {
    derived_type get_derived_type(void) const override { return Base::A; }
};

struct B : public Base {
    derived_type get_derived_type(void) const override { return Base::B; }
};

struct C : public Base {
    derived_type get_derived_type(void) const override { return Base::C; }
    ve::u8 buffer[64];
};

struct alignas(32) D : public Base {
    derived_type get_derived_type(void) const override { return Base::D; }
};


template <std::size_t Capacity> test_result test_with_size(void) {
    test_result result = VE_TEST_SUCCESS;


    using poly_t = ve::stack_polymorph<Base, Capacity, ve::common_alignment<A, B, C>>; // D omitted intentionally, it will be stored remotely.

    poly_t local_value;
    if (local_value.has_value()) result |= VE_TEST_FAIL("Stack polymorph incorrectly reports itself as having a value.");


    // Test value overwrite.
    ve::meta::pack<A, B, C, D>::foreach([&] <typename T> () {
        local_value = T { };

        if (!local_value.has_value()) result |= VE_TEST_FAIL("Stack polymorph incorrectly reports itself as not having a value.");
        if (local_value->get_derived_type() != T{}.get_derived_type()) result |= VE_TEST_FAIL("Stack polymorph contains incorrect value.");

        if constexpr (poly_t::template can_contain_locally<T>()) {
            if (!local_value.has_local_storage()) result |= VE_TEST_FAIL("Stack polymorph does not have local storage for type that is small enough to be contained locally.");
        } else {
            if (local_value.has_local_storage()) result |= VE_TEST_FAIL("Stack polymorph has local storage for type that too big to be contained locally.");
        }
    });


    // Test move assignment.
    ve::meta::pack<A, B, C, D>::foreach([&] <typename T> () {
        auto other = poly_t { T{} };
        local_value = std::move(other);

        if (!local_value.has_value()) result |= VE_TEST_FAIL("Stack polymorph incorrectly reports itself as not having a value.");
        if (local_value->get_derived_type() != T{}.get_derived_type()) result |= VE_TEST_FAIL("Stack polymorph contains incorrect value.");

        if constexpr (poly_t::template can_contain_locally<T>()) {
            if (!local_value.has_local_storage()) result |= VE_TEST_FAIL("Stack polymorph does not have local storage for type that is small enough to be contained locally.");
        } else {
            if (local_value.has_local_storage()) result |= VE_TEST_FAIL("Stack polymorph has local storage for type that too big to be contained locally.");
        }
    });


    // Test value clearing.
    ve::meta::pack<A, B, C, D>::foreach([&] <typename T> () {
        local_value = T { };
        if (!local_value.has_value()) result |= VE_TEST_FAIL("Stack polymorph incorrectly reports itself as not having a value.");

        local_value.delete_value();
        if (local_value.has_value()) result |= VE_TEST_FAIL("Stack polymorph incorrectly reports itself as having a value.");
    });


    return result;
}


test_result test_main(void) {
    test_result result = VE_TEST_SUCCESS;

    result |= test_with_size<1>();
    result |= test_with_size<sizeof(A)>();
    result |= test_with_size<sizeof(C)>();

    return result;
}