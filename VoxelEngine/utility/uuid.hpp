#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/random.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>


namespace ve {
    using uuid = boost::uuids::uuid;


    inline uuid nil_uuid(void) {
        boost::uuids::nil_generator gen;
        return gen();
    }


    inline uuid random_uuid(void) {
        static thread_local boost::uuids::basic_random_generator gen { goodrand::global_generator };
        return gen();
    }


    inline uuid uuid_from_string(std::string_view sv) {
        boost::uuids::string_generator gen;
        return gen(sv.begin(), sv.end());
    }
}