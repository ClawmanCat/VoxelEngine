#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>


template <typename... Ts> using pack = ve::meta::pack<Ts...>;


TEST(pack, size) {
    ASSERT_EQ(pack<>::size, 0);
    ASSERT_EQ(pack<int>::size, 1);
    ASSERT_EQ((pack<int, char, bool>::size), 3);

    ASSERT_TRUE(pack<>::empty);
    ASSERT_FALSE(pack<int>::empty);
}


TEST(pack, head) {
    ASSERT_TYPE_EQ(pack<>::head, ve::meta::null_type);
    ASSERT_TYPE_EQ(pack<int>::head, int);
    ASSERT_TYPE_EQ(pack<int, char>::head, int);
}


TEST(pack, tail) {
    ASSERT_TYPE_EQ(pack<>::tail, pack<>);
    ASSERT_TYPE_EQ(pack<int>::tail, pack<>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::tail, pack<char, bool>);
}


TEST(pack, rhead) {
    ASSERT_TYPE_EQ(pack<>::rhead, ve::meta::null_type);
    ASSERT_TYPE_EQ(pack<int>::rhead, int);
    ASSERT_TYPE_EQ(pack<int, char>::rhead, char);
}


TEST(pack, rtail) {
    ASSERT_TYPE_EQ(pack<>::rtail, pack<>);
    ASSERT_TYPE_EQ(pack<int>::rtail, pack<>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::rtail, pack<int, char>);
}


TEST(pack, reverse) {
    ASSERT_TYPE_EQ(pack<>::reverse, pack<>);
    ASSERT_TYPE_EQ(pack<int>::reverse, pack<int>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::reverse, pack<bool, char, int>);
}


TEST(pack, append) {
    ASSERT_TYPE_EQ(pack<>::template append<>, pack<>);
    ASSERT_TYPE_EQ(pack<char, long>::template append<>, pack<char, long>);
    ASSERT_TYPE_EQ(pack<>::template append<int>, pack<int>);
    ASSERT_TYPE_EQ(pack<bool>::template append<int>, pack<bool, int>);
    ASSERT_TYPE_EQ(pack<char, long>::template append<bool, int>, pack<char, long, bool, int>);
}


TEST(pack, prepend) {
    ASSERT_TYPE_EQ(pack<>::template prepend<>, pack<>);
    ASSERT_TYPE_EQ(pack<char, long>::template prepend<>, pack<char, long>);
    ASSERT_TYPE_EQ(pack<>::template prepend<int>, pack<int>);
    ASSERT_TYPE_EQ(pack<bool>::template prepend<int>, pack<int, bool>);
    ASSERT_TYPE_EQ(pack<char, long>::template prepend<bool, int>, pack<bool, int, char, long>);
}


TEST(pack, append_pack) {
    ASSERT_TYPE_EQ(pack<>::template append_pack<pack<>>, pack<>);
    ASSERT_TYPE_EQ(pack<char, long>::template append_pack<pack<>>, pack<char, long>);
    ASSERT_TYPE_EQ(pack<>::template append_pack<pack<int>>, pack<int>);
    ASSERT_TYPE_EQ(pack<bool>::template append_pack<pack<int>>, pack<bool, int>);
    ASSERT_TYPE_EQ(pack<char, long>::template append_pack<pack<bool, int>>, pack<char, long, bool, int>);
}


TEST(pack, prepend_pack) {
    ASSERT_TYPE_EQ(pack<>::template prepend_pack<pack<>>, pack<>);
    ASSERT_TYPE_EQ(pack<char, long>::template prepend_pack<pack<>>, pack<char, long>);
    ASSERT_TYPE_EQ(pack<>::template prepend_pack<pack<int>>, pack<int>);
    ASSERT_TYPE_EQ(pack<bool>::template prepend_pack<pack<int>>, pack<int, bool>);
    ASSERT_TYPE_EQ(pack<char, long>::template prepend_pack<pack<bool, int>>, pack<bool, int, char, long>);
}


TEST(pack, expand_inside) {
    ASSERT_TYPE_EQ(pack<>::template expand_inside<std::tuple>, std::tuple<>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::template expand_inside<std::tuple>, std::tuple<int, char, bool>);
}


TEST(pack, expand_outside) {
    ASSERT_TYPE_EQ(pack<>::template expand_outside<std::tuple>, pack<>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::template expand_outside<std::tuple>, pack<std::tuple<int>, std::tuple<char>, std::tuple<bool>>);
}


TEST(pack, contains) {
    ASSERT_FALSE(pack<>::template contains<int>);
    ASSERT_FALSE((pack<bool, char>::template contains<int>));
    ASSERT_TRUE((pack<bool, int, char>::template contains<int>));
}


TEST(pack, contains_all) {
    ASSERT_TRUE((pack<>::contains_all<>));
    ASSERT_TRUE((pack<int, char>::contains_all<>));
    ASSERT_TRUE((pack<int, char, bool>::contains_all<bool>));
    ASSERT_TRUE((pack<int, char, bool>::contains_all<int, char, bool>));
    ASSERT_FALSE((pack<int, char, bool>::contains_all<long>));
    ASSERT_FALSE((pack<int, char, bool>::contains_all<int, char, bool, long>));
}


TEST(pack, contains_any) {
    ASSERT_FALSE((pack<>::contains_any<>));
    ASSERT_FALSE((pack<int, char>::contains_any<>));
    ASSERT_TRUE((pack<int, char, bool>::contains_any<bool>));
    ASSERT_TRUE((pack<int, char, bool>::contains_any<int, char, bool>));
    ASSERT_FALSE((pack<int, char, bool>::contains_any<long>));
    ASSERT_TRUE((pack<int, char, bool>::contains_any<int, char, bool, long>));
    ASSERT_TRUE((pack<int, char, bool>::contains_any<bool, long>));
}


TEST(pack, pop_front) {
    ASSERT_TYPE_EQ(pack<>::pop_front<0>, pack<>);
    ASSERT_TYPE_EQ(pack<>::pop_front<8>, pack<>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::pop_front<0>, pack<int, char, bool>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::pop_front<1>, pack<char, bool>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::pop_front<8>, pack<>);
}


TEST(pack, pop_back) {
    ASSERT_TYPE_EQ(pack<>::pop_back<0>, pack<>);
    ASSERT_TYPE_EQ(pack<>::pop_back<8>, pack<>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::pop_back<0>, pack<int, char, bool>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::pop_back<1>, pack<int, char>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::pop_back<8>, pack<>);
}


TEST(pack, subrange) {
    ASSERT_TYPE_EQ(pack<>::template subrange<0, 0>, pack<>);
    ASSERT_TYPE_EQ(pack<int>::template subrange<0, 1>, pack<int>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::template subrange<1, 2>, pack<char, bool>);
}


TEST(pack, first_n) {
    ASSERT_TYPE_EQ(pack<>::first_n<0>, pack<>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::template first_n<0>, pack<>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::template first_n<1>, pack<int>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::template first_n<3>, pack<int, char, bool>);
}


TEST(pack, last_n) {
    ASSERT_TYPE_EQ(pack<>::last_n<0>, pack<>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::template last_n<0>, pack<>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::template last_n<1>, pack<bool>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::template last_n<3>, pack<int, char, bool>);
}


TEST(pack, nth) {
    ASSERT_TYPE_EQ(pack<int, char, bool, long>::template nth<0>, int);
    ASSERT_TYPE_EQ(pack<int, char, bool, long>::template nth<1>, char);
    ASSERT_TYPE_EQ(pack<int, char, bool, long>::template nth<3>, long);
}


TEST(pack, find) {
    ASSERT_EQ((pack<int, char, bool, int>::template find_first<int>), 0);
    ASSERT_EQ((pack<int, char, bool, int>::template find_last<int>), 3);
    ASSERT_EQ((pack<int, char, bool, int>::template find_all<int>), (std::array<std::size_t, 2> { 0, 3 }));
}


TEST(pack, flatten) {
    ASSERT_TYPE_EQ(pack<>::flatten<>, pack<>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::flatten<>, pack<int, char, bool>);
    ASSERT_TYPE_EQ(pack<int, pack<char, bool>>::flatten<>, pack<int, char, bool>);
    ASSERT_TYPE_EQ(pack<pack<int>, pack<pack<char>, bool>>::flatten<>, pack<int, char, bool>);
}


TEST(pack, flatten_nonrecursive) {
    ASSERT_TYPE_EQ(pack<>::flatten_nonrecursive<>, pack<>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::flatten_nonrecursive<>, pack<int, char, bool>);
    ASSERT_TYPE_EQ(pack<int, pack<char, bool>>::flatten_nonrecursive<>, pack<int, char, bool>);
    ASSERT_TYPE_EQ(pack<pack<int>, pack<pack<char>, bool>>::flatten_nonrecursive<>, pack<int, pack<char>, bool>);
}


TEST(pack, unique) {
    ASSERT_TYPE_EQ(pack<>::template unique<>, pack<>);
    ASSERT_TYPE_EQ(pack<int, char, bool>::template unique<>, pack<int, char, bool>);
    ASSERT_TYPE_EQ(pack<int, char, bool, int>::template unique<>, pack<int, char, bool>);
    ASSERT_TYPE_EQ(pack<int, char, bool, int, char, long, bool, int>::template unique<>, pack<int, char, bool, long>);
}


TEST(pack, all) {
    ASSERT_TRUE((pack<>::all([] <typename T> { return false; })));
    ASSERT_TRUE((pack<int, char, bool>::all([] <typename T> { return std::is_integral_v<T>; })));
    ASSERT_FALSE((pack<int, char, bool, float>::all([] <typename T> { return std::is_integral_v<T>; })));
}


TEST(pack, any) {
    ASSERT_FALSE((pack<>::any([] <typename T> { return true; })));
    ASSERT_TRUE((pack<int, char, float>::any([] <typename T> { return std::is_integral_v<T>; })));
    ASSERT_FALSE((pack<float, double>::any([] <typename T> { return std::is_integral_v<T>; })));
}


TEST(pack, foreach_indexed) {
    ve::hash_set<std::size_t> seen;


    pack<int, char, bool>::foreach_indexed([&] <typename T, std::size_t I> {
        for (std::size_t j = 0; j < I; ++j) EXPECT_TRUE(seen.contains(j));

        switch (I) {
            case 0: {
                EXPECT_TYPE_EQ(T, int);
                break;
            }
            case 1: {
                EXPECT_TYPE_EQ(T, char);
                break;
            }
            case 2: {
                EXPECT_TYPE_EQ(T, bool);
                break;
            }
            default: { ADD_FAILURE(); }
        };

        seen.insert(I);
    });
}


TEST(pack, foreach) {
    ve::hash_set<ve::type_index_t> seen;


    pack<int, char, bool>::foreach([&] <typename T> {
        if constexpr (std::is_same_v<T, int>) {
            EXPECT_TRUE(seen.empty());
        }

        else if constexpr (std::is_same_v<T, char>) {
            EXPECT_TRUE(seen.contains(ve::type_index<int>()));
        }

        else if constexpr (std::is_same_v<T, bool>) {
            EXPECT_TRUE(seen.contains(ve::type_index<char>()));
        }

        else { ADD_FAILURE(); };

        seen.insert(ve::type_index<T>());
    });
}


struct filter_integers {
    template <typename T> constexpr bool operator()(void) const {
        return std::is_integral_v<T>;
    }
};

TEST(pack, filter_type) {
    ASSERT_TYPE_EQ(
        typename pack<int, float, long, double>::template filter_type<filter_integers>,
        pack<int, long>
    );
}


TEST(pack, filter_value) {
    ASSERT_TYPE_EQ(
        typename pack<int, float, long, double>::template filter_value<filter_integers {}>,
        pack<int, long>
    );
}


TEST(pack, filter_trait) {
    ASSERT_TYPE_EQ(
        typename pack<int, float, long, double>::template filter_trait<std::is_integral>,
        pack<int, long>
    );
}


TEST(pack, apply) {
    pack<int, char, bool>::apply([] <typename... Ts> {
        ASSERT_TYPE_EQ(pack<Ts...>, pack<int, char, bool>);
    });
}


TEST(pack, erase) {
    ASSERT_TYPE_EQ(typename pack<int, char, bool>::template erase<1>, pack<int, bool>);
}


TEST(pack, erase_first) {
    ASSERT_TYPE_EQ(
        typename pack<int, char, bool, char, char, int>::template erase_first<char>,
        pack<int, bool, char, char, int>
    );
}


TEST(pack, erase_all) {
    ASSERT_TYPE_EQ(
        typename pack<int, char, bool, char, char, int>::template erase_all<char>,
        pack<int, bool, int>
    );
}