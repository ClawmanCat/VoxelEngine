#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/meta/stop_token.hpp>
#include <VoxelEngine/utility/meta/sequence.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>
#include <VoxelEngine/utility/container/container_utils.hpp>


using pack = ve::meta::pack<std::string, int, bool>;
auto  seq  = ve::meta::make_integer_sequence<std::size_t, 5, 10, 1>();
auto  tpl  = std::tuple<std::string, int, bool> { "Meow", 33, true };


TEST(stop_token, no_token_return_void) {
    ASSERT_TYPE_EQ(
        decltype(pack::foreach_indexed([] <typename T, std::size_t I> {})),
        void
    );

    ASSERT_TYPE_EQ(
        decltype(ve::meta::foreach_in_sequence(seq, [] <std::size_t I> {})),
        void
    );

    ASSERT_TYPE_EQ(
        decltype(ve::tuple_foreach(tpl, [] (const auto& e) {})),
        void
    );
}


TEST(stop_token, no_token_invokes_all) {
    std::string s1;

    pack::foreach_indexed([&] <typename T, std::size_t I> {
        if constexpr (std::is_same_v<T, std::string>) s1 += "S";
        if constexpr (std::is_same_v<T, int        >) s1 += "I";
        if constexpr (std::is_same_v<T, bool       >) s1 += "B";
    });

    ASSERT_EQ(s1, "SIB");


    std::string s2;

    ve::meta::foreach_in_sequence(seq, [&] <std::size_t I> {
        s2 += std::to_string(I);
    });

    ASSERT_EQ(s2, "56789");


    std::string s3;

    ve::tuple_foreach(tpl, [&] <typename T> (const T& e) {
        if constexpr (std::is_same_v<T, std::string>) s3 += "S";
        if constexpr (std::is_same_v<T, int        >) s3 += "I";
        if constexpr (std::is_same_v<T, bool       >) s3 += "B";
    });

    ASSERT_EQ(s3, "SIB");
}


TEST(stop_token, token_returns_variant) {
    auto r1 = pack::foreach_indexed([] <typename T, std::size_t I> {
        return ve::meta::stop_token<T*>{ nullptr };
    });

    ASSERT_TYPE_EQ(decltype(r1), std::variant<std::string*, int*, bool*, ve::meta::null_type>);


    auto r2 = ve::meta::foreach_in_sequence(seq, [] <std::size_t I> {
        return ve::meta::stop_token<ve::meta::value<(int) I>>{ };
    });

    ASSERT_TYPE_EQ(decltype(r2), std::variant<ve::meta::value<5>, ve::meta::value<6>, ve::meta::value<7>, ve::meta::value<8>, ve::meta::value<9>, ve::meta::null_type>);


    auto r3 = ve::tuple_foreach(tpl, [] <typename T> (T& e) {
        return ve::meta::stop_token<T*>{ &e };
    });

    ASSERT_TYPE_EQ(decltype(r3), std::variant<std::string*, int*, bool*, ve::meta::null_type>);
}


TEST(stop_token, void_token_not_in_variant) {
    auto r1 = pack::foreach_indexed([] <typename T, std::size_t I> {
        if constexpr (std::is_same_v<T, std::string>) return ve::meta::stop_token<void>{};
        else return ve::meta::stop_token<T*>{ nullptr };
    });

    ASSERT_TYPE_EQ(decltype(r1), std::variant<int*, bool*, ve::meta::null_type>);


    auto r2 = ve::meta::foreach_in_sequence(seq, [] <std::size_t I> {
        if constexpr (I <= 7) return ve::meta::stop_token<void>{};
        else return ve::meta::stop_token<ve::meta::value<(int) I>>{ };
    });

    ASSERT_TYPE_EQ(decltype(r2), std::variant<ve::meta::value<8>, ve::meta::value<9>, ve::meta::null_type>);


    auto r3 = ve::tuple_foreach(tpl, [] <typename T> (T& e) {
        if constexpr (std::is_same_v<T, std::string>) return ve::meta::stop_token<void>{};
        else return ve::meta::stop_token<T*>{ &e };
    });

    ASSERT_TYPE_EQ(decltype(r3), std::variant<int*, bool*, ve::meta::null_type>);
}


TEST(stop_token, void_function_not_in_variant) {
    auto r1 = pack::foreach_indexed([] <typename T, std::size_t I> {
        if constexpr (!std::is_same_v<T, std::string>) return ve::meta::stop_token<T*>{ nullptr };
    });

    ASSERT_TYPE_EQ(decltype(r1), std::variant<int*, bool*, ve::meta::null_type>);


    auto r2 = ve::meta::foreach_in_sequence(seq, [] <std::size_t I> {
        if constexpr (I > 7) return ve::meta::stop_token<ve::meta::value<(int) I>>{ };
    });

    ASSERT_TYPE_EQ(decltype(r2), std::variant<ve::meta::value<8>, ve::meta::value<9>, ve::meta::null_type>);


    auto r3 = ve::tuple_foreach(tpl, [] <typename T> (T& e) {
        if constexpr (!std::is_same_v<T, std::string>) return ve::meta::stop_token<T*>{ &e };
    });

    ASSERT_TYPE_EQ(decltype(r3), std::variant<int*, bool*, ve::meta::null_type>);
}


TEST(stop_token, variant_holds_first_stopping_token) {
    auto r1 = pack::foreach_indexed([] <typename T, std::size_t I> {
        return ve::meta::stop_token<T*>{ .payload = nullptr, .stop = (std::is_same_v<T, int>) };
    });

    ASSERT_TRUE(std::holds_alternative<int*>(r1));
    ASSERT_EQ(std::get<int*>(r1), nullptr);


    auto r2 = ve::meta::foreach_in_sequence(seq, [] <std::size_t I> {
        return ve::meta::stop_token<ve::meta::value<(int) I>>{ .stop = (I == 7) };
    });

    ASSERT_TRUE(std::holds_alternative<ve::meta::value<7>>(r2));


    auto r3 = ve::tuple_foreach(tpl, [] <typename T> (T& e) {
        return ve::meta::stop_token<T*>{ .payload = &e, .stop = (std::is_same_v<T, int>) };
    });

    ASSERT_TRUE(std::holds_alternative<int*>(r3));
    ASSERT_EQ(*std::get<int*>(r3), 33);
}


TEST(stop_token, no_stopping_token_results_in_null_variant) {
    auto r1 = pack::foreach_indexed([] <typename T, std::size_t I> {
        return ve::meta::stop_token<T*>{ .payload = nullptr, .stop = false };
    });

    ASSERT_TRUE(std::holds_alternative<ve::meta::null_type>(r1));


    auto r2 = ve::meta::foreach_in_sequence(seq, [] <std::size_t I> {
        return ve::meta::stop_token<ve::meta::value<(int) I>>{ .stop = false };
    });

    ASSERT_TRUE(std::holds_alternative<ve::meta::null_type>(r2));


    auto r3 = ve::tuple_foreach(tpl, [] <typename T> (T& e) {
        return ve::meta::stop_token<T*>{ .payload = &e, .stop = false };
    });

    ASSERT_TRUE(std::holds_alternative<ve::meta::null_type>(r3));
}