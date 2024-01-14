#pragma once

#include <memory>


namespace ve {
    template <typename T, typename Deleter = std::default_delete<T>>
    using unique = std::unique_ptr<T, Deleter>;

    template <typename T>
    using shared = std::shared_ptr<T>;

    template <typename T>
    using weak = std::weak_ptr<T>;


    using std::make_unique;
    using std::make_shared;
}