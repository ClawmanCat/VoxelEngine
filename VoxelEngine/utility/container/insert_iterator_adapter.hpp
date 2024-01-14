#pragma once

#include <VoxelEngine/core/core.hpp>

#include <iterator>


namespace ve {
    /**
     * Adapter for std::*_insert_iterator to provide a method returning an iterator to the previously inserted element.
     * @tparam Inserter The template of an *_insert_iterator class.
     */
    template <template <typename> typename Inserter> struct insert_iterator_adapter_type {};


    template <> struct insert_iterator_adapter_type<std::back_insert_iterator> {
        template <typename Container> class type : public std::back_insert_iterator<Container> {
        public:
            using base = std::back_insert_iterator<Container>;

            using base::base;

            /** Returns an iterator to the previously inserted element. */
            typename Container::iterator where(void) const {
                return std::prev(base::container->end());
            }
        };
    };


    template <> struct insert_iterator_adapter_type<std::front_insert_iterator> {
        template <typename Container> class type : public std::front_insert_iterator<Container> {
        public:
            using base = std::front_insert_iterator<Container>;

            using base::base;

            /** Returns an iterator to the previously inserted element. */
            typename Container::iterator where(void) const {
                return base::container->begin();
            }
        };
    };


    template <> struct insert_iterator_adapter_type<std::insert_iterator> {
        template <typename Container> class type : public std::insert_iterator<Container> {
        public:
            using base = std::insert_iterator<Container>;

            using base::base;

            /** Returns an iterator to the previously inserted element. */
            typename Container::iterator where(void) const {
                return std::prev(base::iter);
            }
        };
    };


    template <typename Container> using retrievable_back_insert_iterator  = typename insert_iterator_adapter_type<std::back_insert_iterator>::template type<Container>;
    template <typename Container> using retrievable_front_insert_iterator = typename insert_iterator_adapter_type<std::front_insert_iterator>::template type<Container>;
    template <typename Container> using retrievable_insert_iterator       = typename insert_iterator_adapter_type<std::insert_iterator>::template type<Container>;
}