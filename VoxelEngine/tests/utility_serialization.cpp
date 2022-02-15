#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/io/serialize/binary_serializable.hpp>

#include <string>
#include <vector>
#include <list>
#include <forward_list>

using namespace ve::defs;


struct A {
    i32 ia, ib, ic;
    float fa, fb, fc;
    std::string sa, sb;
    
    std::vector<std::string> vs;
    std::array<i32, 5> a;
    std::list<i32> la;
    std::forward_list<i32> lb;
    
    hash_map<std::string, std::string> hm;
    hash_set<int> hs;
    tree_map<std::string, std::string> tm;
    tree_set<int> ts;
    
    small_vector<int> sv;
    small_vec_map<std::string, std::string> svm;
    small_vec_set<int> svs;
    
    
    struct X {
        i32 a, b;
        ve_eq_comparable(X);
    } x;
    
    struct Y {
        i32 a, b;
        bool was_serialized = false;
        
        constexpr bool operator==(const Y& o) const {
            // During the test, the serialized object will be compared with the original.
            // If the serialization methods were used correctly, the serialized version will have was_serialized = true.
            if (was_serialized == o.was_serialized) return false;
            return a == o.a && b == o.b;
        }
        
        void to_bytes(std::vector<u8>& vec) const {
            ve::serialize::push_serializer ser { vec };
            
            ser.push(a);
            ser.push(b);
            ser.push(true); // Set was_serialized to true.
        }
        
        static Y from_bytes(std::span<const u8> s) {
            ve::serialize::pop_deserializer ser { s };
            Y result;
            
            ser.pop_into(result.was_serialized);
            ser.pop_into(result.b);
            ser.pop_into(result.a);
            
            return result;
        }
    } y;
    
    struct Z { // External serializer.
        i32 a, b;
        bool was_serialized = false;
    
        constexpr bool operator==(const Z& o) const {
            // See Y::operator==.
            if (was_serialized == o.was_serialized) return false;
            return a == o.a && b == o.b;
        }
    } z;
    
    
    ve_rt_eq_comparable(A);
};


template <> struct ve::serialize::binary_serializer<typename A::Z> {
    static void to_bytes(const A::Z& val, std::vector<u8>& vec) {
        ve::serialize::push_serializer ser { vec };
    
        ser.push(val.a);
        ser.push(val.b);
        ser.push(true); // Set was_serialized to true.
    }
    
    static typename A::Z from_bytes(std::span<const u8> s) {
        ve::serialize::pop_deserializer ser { s };
        typename A::Z result;
    
        ser.pop_into(result.was_serialized);
        ser.pop_into(result.b);
        ser.pop_into(result.a);
    
        return result;
    }
};


test_result test_main(void) {
    A object {
        .ia = 1, .ib = 2, .ic = 3,
        .fa = 0.5f, .fb = 1.0f, .fc = infinity<float>,
        .sa = "Meow", .sb = "Old Cat",
        .vs = { "A", "B", "C", "D" },
        .a  = {  10,  20,  30,  40,  50 },
        .la = { -10, -20, -30, -40, -50 },
        .lb = { 1, 2, 3, 4, 5 },
        .hm = { { "A", "1" }, { "B", "2" }, { "C", "3" } },
        .hs = { 1, 2, 3, 4, 5 },
        .tm = { { "A", "1" }, { "B", "2" }, { "C", "3" } },
        .ts = { 1, 2, 3, 4, 5 },
        .sv = { 1, 2, 3, 4, 5 },
        .svm = { { "A", "1" }, { "B", "2" }, { "C", "3" } },
        .svs = { 1, 2, 3, 4, 5 },
        .x = { 1, 2 }, .y = { 1, 2 }, .z = { 1, 2 }
    };
    
    
    std::vector<u8> bytes;
    ve::serialize::to_bytes(object, bytes);
    
    std::span<const u8> span { bytes.begin(), bytes.end() };
    A new_object = ve::serialize::from_bytes<A>(span);
    
    
    if (object == new_object) return VE_TEST_SUCCESS;
    else return VE_TEST_FAIL("Deserialized object does not compare equal to original object.");
}