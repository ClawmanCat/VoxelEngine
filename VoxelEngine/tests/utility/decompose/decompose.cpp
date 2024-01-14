#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/decompose/decompose.hpp>

#include <utility>
#include <tuple>
#include <string>


template <typename T> inline void decomposition_test(T& value, auto get_first, auto get_second) {
    ASSERT_TRUE(ve::decomposable<T>);


    auto const_refs = ve::decompose_as_const_references(value);

    ASSERT_EQ(std::get<0>(const_refs), 3);
    ASSERT_EQ(std::get<1>(const_refs), "Meow");

    get_first(value)  = 4;
    get_second(value) = "Woof";

    ASSERT_EQ(std::get<0>(const_refs), 4);
    ASSERT_EQ(std::get<1>(const_refs), "Woof");


    auto mut_refs   = ve::decompose_as_references(value);
    ASSERT_EQ(std::get<0>(mut_refs), 4);
    ASSERT_EQ(std::get<1>(mut_refs), "Woof");

    std::get<0>(mut_refs) = 3;
    std::get<1>(mut_refs) = "Meow";

    ASSERT_EQ(std::get<0>(mut_refs), 3);
    ASSERT_EQ(std::get<1>(mut_refs), "Meow");


    auto copy = ve::decompose_as_copy(value);
    ASSERT_EQ(std::get<0>(copy), 3);
    ASSERT_EQ(std::get<1>(copy), "Meow");

    get_first(value)  = 4;
    get_second(value) = "Woof";

    ASSERT_EQ(std::get<0>(copy), 3);
    ASSERT_EQ(std::get<1>(copy), "Meow");
}


TEST(decompose, pair) {
    std::pair<int, std::string> p { 3, "Meow" };

    decomposition_test(
        p,
        [] (auto& p) -> auto& { return p.first; },
        [] (auto& p) -> auto& { return p.second; }
    );
}


TEST(decompose, tuple) {
    std::tuple<int, std::string> p { 3, "Meow" };

    decomposition_test(
        p,
        [] (auto& p) -> auto& { return std::get<0>(p); },
        [] (auto& p) -> auto& { return std::get<1>(p); }
    );
}


TEST(decompose, struct) {
    struct my_decomposable {
        int first;
        std::string second;
    };

    my_decomposable p { 3, "Meow" };


    decomposition_test(
        p,
        [] (auto& p) -> auto& { return p.first; },
        [] (auto& p) -> auto& { return p.second; }
    );
}


TEST(decompose, custom_decomposer) {
    struct my_decomposable {
    public:
        my_decomposable(int f, std::string s) : first(f), second(std::move(s)) {}

        int first;

        VE_DECOMPOSE_INTO(my_decomposable, first, second);
        VE_GET_MREFS(second);
    private:
        std::string second;
    };


    my_decomposable p { 3, "Meow" };


    decomposition_test(
        p,
        [] (auto& p) -> auto& { return p.first; },
        [] (auto& p) -> auto& { return p.get_second(); }
    );
}


TEST(decompose, non_decomposable) {
    struct non_decomposable {
    public:
        int first;
    private:
        std::string second;
    };


    ASSERT_FALSE(ve::decomposable<non_decomposable>);
}