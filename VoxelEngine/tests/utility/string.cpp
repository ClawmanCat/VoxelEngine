#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/string.hpp>
#include <VoxelEngine/utility/services/logger.hpp>

#include <string>
#include <string_view>
#include <exception>
#include <vector>


using namespace std::string_literals;
using namespace std::string_view_literals;


TEST(to_string, recursion_prevention) {
    struct recursive {
        std::string to_string(void) const {
            return std::format("[ This is {}! ]", ve::to_string(*this));
        }
    };


    ASSERT_EQ(ve::to_string(recursive { }), "[ This is ...! ]"s);
}


TEST(to_string, convertible) {
    auto v1 = ve::to_string("Meow"sv);
    auto v2 = ve::to_string("Meow");

    ASSERT_EQ(v1, "Meow"s);
    ASSERT_TYPE_EQ(decltype(v1), std::string);

    ASSERT_EQ(v2, "Meow"s);
    ASSERT_TYPE_EQ(decltype(v2), std::string);
}


TEST(to_string, member_function) {
    struct method_to_string    { std::string to_string   (void) const { return "method_to_string";    } };
    struct method_to_str       { std::string to_str      (void) const { return "method_to_str";       } };
    struct method_to_cppstring { std::string to_cppstring(void) const { return "method_to_cppstring"; } };
    struct method_string       { std::string string      (void) const { return "method_string";       } };
    struct method_str          { std::string str         (void) const { return "method_str";          } };
    struct method_cppstring    { std::string cppstring   (void) const { return "method_cppstring";    } };
    
    ASSERT_EQ(ve::to_string(method_to_string    { }), "method_to_string"s);
    ASSERT_EQ(ve::to_string(method_to_str       { }), "method_to_str"s);
    ASSERT_EQ(ve::to_string(method_to_cppstring { }), "method_to_cppstring"s);
    ASSERT_EQ(ve::to_string(method_string       { }), "method_string"s);
    ASSERT_EQ(ve::to_string(method_str          { }), "method_str"s);
    ASSERT_EQ(ve::to_string(method_cppstring    { }), "method_cppstring"s);


    struct multi_string_1 : method_to_string, method_string {};
    ASSERT_EQ(ve::to_string(multi_string_1 { }), "method_to_string"s);

    struct multi_string_2 : method_str, method_to_str {};
    ASSERT_EQ(ve::to_string(multi_string_2 { }), "method_to_str"s);

    struct multi_string_3 : method_cppstring, method_str {};
    ASSERT_EQ(ve::to_string(multi_string_3 { }), "method_str"s);
}


TEST(to_string, numeric) {
    ASSERT_EQ(ve::to_string(ve::u8  { +69 }),     "69");
    ASSERT_EQ(ve::to_string(ve::i8  { -69 }),     "-69");
    ASSERT_EQ(ve::to_string(ve::u32 { +420420 }), "420420");
    ASSERT_EQ(ve::to_string(ve::i32 { 0       }), "0");


    ASSERT_TRUE(
        ve::to_string(33.580f).starts_with("33.580") ||
        ve::to_string(33.580f).starts_with("33.579")
    );

    ASSERT_TRUE(
        ve::to_string(-0.12345).starts_with("-0.12345") ||
        ve::to_string(-0.12345).starts_with("-0.12344")
    );

    // Standard allows both "inf" and "infinity".
    ASSERT_TRUE(ve::to_string(ve::infinity<ve::f64>).starts_with("inf"));
    ASSERT_TRUE(ve::to_string(-1 * ve::infinity<ve::f64>).starts_with("-inf"));
}


TEST(to_string, glm) {
    ASSERT_EQ(
        ve::to_string(ve::vec3ui { 0, 100, 32000 }),
        "[0, 100, 32000]"
    );

    ASSERT_MATCHES(
        ve::to_string(ve::vec2f { -100.0f, 32.5f }),
        R"RGX(\[-100\.?(0*), 32.5(0*)\])RGX"
    );

    ASSERT_MATCHES(
        ve::to_string(ve::quatd { 0.25, 0.25, 0.25, 0.25 }),
        R"RGX(\[0.25(0*), 0.25(0*), 0.25(0*), 0.25(0*)\])RGX"
    );

    ASSERT_MATCHES(
        ve::to_string(ve::mat2f { { 25, 30 }, { 0.5, 12 } }),
        R"RGX(\[\[25\.?(0*), 30\.?(0*)\], \[0.5(0*), 12\.?(0*)\]\])RGX"
    );
}


TEST(to_string, boolean) {
    ASSERT_EQ(ve::to_string(true), "true");
    ASSERT_EQ(ve::to_string(false), "false");
}


TEST(to_string, exception) {
    struct my_exc : std::exception {
        const char* what(void) const override { return "Failed to meow!"; }
    };

    std::runtime_error err1 { "Failed to woof!" };
    my_exc err2 { };

    ASSERT_EQ(ve::to_string(err1), "[Exception std::runtime_error] { Failed to woof! }");
    ASSERT_EQ(ve::to_string(err2), "[Exception my_exc] { Failed to meow! }");
}


TEST(to_string, from_container) {
    std::vector<std::string> c1 { "Meow", "Woof", "Awoo" };
    ve::tree_set<int> c2 { 8, 16, 32 };
    ve::tree_map<int, std::string> c3 { { 8, "Meow" }, { 16, "Woof" }, { 32, "Awoo" } };

    ASSERT_EQ(ve::to_string(c1), "[Meow, Woof, Awoo]");
    ASSERT_EQ(ve::to_string(c2), "[8, 16, 32]");
    ASSERT_EQ(ve::to_string(c3), "[[Pair] { 8, Meow }, [Pair] { 16, Woof }, [Pair] { 32, Awoo }]");
}


TEST(to_string, from_pointer) {
    int x = 1;
    int* xp = &x;
    auto yp = std::make_unique<int>(2);
    auto zp = std::make_shared<int>(3);

    ASSERT_MATCHES(ve::to_string(xp), R"RGX(\[Raw Pointer @ 0x[A-F0-9]+\] \{ 1 \})RGX");
    ASSERT_MATCHES(ve::to_string(yp), R"RGX(\[Unique Pointer @ 0x[A-F0-9]+\] \{ 2 \})RGX");
    ASSERT_MATCHES(ve::to_string(zp), R"RGX(\[Shared Pointer @ 0x[A-F0-9]+\] \{ 3 \})RGX");


    xp = nullptr;
    yp = nullptr;
    zp = nullptr;

    ASSERT_MATCHES(ve::to_string(xp), R"RGX(\[Raw Pointer @ 0x(0+)\] \{ NULL \})RGX");
    ASSERT_MATCHES(ve::to_string(yp), R"RGX(\[Unique Pointer @ 0x(0+)\] \{ NULL \})RGX");
    ASSERT_MATCHES(ve::to_string(zp), R"RGX(\[Shared Pointer @ 0x(0+)\] \{ NULL \})RGX");


    std::optional<int> va = std::nullopt, vb = 4;

    ASSERT_MATCHES(ve::to_string(va), R"RGX(\[Optional @ 0x(0+)\] \{ NULL \})RGX");
    ASSERT_MATCHES(ve::to_string(vb), R"RGX(\[Optional @ 0x[A-F0-9]+\] \{ 4 \})RGX");
}


TEST(to_string, from_variant) {
    std::variant<int, std::string, bool> v;

    v = 33;
    ASSERT_MATCHES(ve::to_string(v), R"RGX(\[std::variant<.+> holding int\] \{ 33 \})RGX");

    v = "Meow"s;
    ASSERT_MATCHES(ve::to_string(v), R"RGX(\[std::variant<.+> holding .+\] \{ Meow \})RGX");

    v = true;
    ASSERT_MATCHES(ve::to_string(v), R"RGX(\[std::variant<.+> holding bool\] \{ true \})RGX");
}


// Must be outside test body: friend functions not allowed in local class.
struct streamable {
    int x, y, z;

    friend std::ostream& operator<<(std::ostream& stream, const streamable& obj) {
        stream << std::format("[x = {}, y = {}, z = {}]", obj.x, obj.y, obj.z);
        return stream;
    }
};

TEST(to_string, from_streamable) {
    ASSERT_EQ(
        ve::to_string(streamable { 1, 2, 3 }),
        "[x = 1, y = 2, z = 3]"s
    );
}


TEST(to_string, from_decomposable) {
    struct decomposable_1 {
        int x = 1, y = 2, z = 3;
    };

    struct decomposable_2 {
        std::string a = "Meow", b =  "Woof", c = "Awoo";
    };

    struct decomposable_3 {
        decomposable_1 d1 = {};
        decomposable_2 d2 = {};
    };


    ASSERT_EQ(
        ve::to_string(decomposable_1 { }),
        "[decomposable_1] { 1, 2, 3 }"s
    );

    ASSERT_EQ(
        ve::to_string(decomposable_2 { }),
        "[decomposable_2] { Meow, Woof, Awoo }"s
    );

    ASSERT_EQ(
        ve::to_string(decomposable_3 { }),
        "[decomposable_3] { [decomposable_1] { 1, 2, 3 }, [decomposable_2] { Meow, Woof, Awoo } }"s
    );
}


TEST(cat, concatenate) {
    ASSERT_EQ(
        ve::cat("Meow", 123, "Awoo", 456),
        "Meow123Awoo456"s
    );

    ASSERT_EQ(ve::cat(), "");
}


TEST(cat, concatenate_with_separator) {
    ASSERT_EQ(
        ve::cat_with(", ", "Meow", 123, "Awoo", 456),
        "Meow, 123, Awoo, 456"
    );

    ASSERT_EQ(ve::cat_with(", "), "");
}


TEST(string_conversion, to_lowercase) {
    ASSERT_EQ(
        ve::to_lowercase("AwooMeow123"),
        "awoomeow123"s
    );

    ASSERT_EQ(ve::to_lowercase(""), "");
}


TEST(string_conversion, to_uppercase) {
    ASSERT_EQ(
        ve::to_uppercase("AwooMeow123"),
        "AWOOMEOW123"s
    );

    ASSERT_EQ(ve::to_uppercase(""), "");
}


TEST(string_conversion, to_hex) {
    ASSERT_EQ(ve::to_hex(0x0000u, 4), "0000");
    ASSERT_EQ(ve::to_hex(0xFA54u, 4), "FA54");
    ASSERT_EQ(ve::to_hex(0xABCDu, 3), "BCD");
    ASSERT_EQ(ve::to_hex(0xABCDu, 0), "");
    ASSERT_EQ(ve::to_hex(0xFAFAFAFAu), "FAFAFAFA");
    ASSERT_EQ(ve::to_hex(0xFAFAFAFAu, 1), "A");
    ASSERT_EQ(ve::to_hex(0xFAu, 4), "00FA");
}


TEST(string_conversion, thousand_separator) {
    ASSERT_EQ(ve::thousand_separate(0), "0");
    ASSERT_EQ(ve::thousand_separate(1), "1");
    ASSERT_EQ(ve::thousand_separate(-1), "-1");
    ASSERT_EQ(ve::thousand_separate(999), "999");
    ASSERT_EQ(ve::thousand_separate(-999), "-999");
    ASSERT_EQ(ve::thousand_separate(1'000), "1'000");
    ASSERT_EQ(ve::thousand_separate(-1'000), "-1'000");
    ASSERT_EQ(ve::thousand_separate(1'000'000), "1'000'000");
    ASSERT_EQ(ve::thousand_separate(-1'000'000), "-1'000'000");
}