#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/entity/entity_defaults.hpp>
#include <VoxelEngine/utility/bit.hpp>
#include <VoxelEngine/utility/meta/common_concepts.hpp>

#include <concepts>
#include <bit>
#include <string>
#include <format>


namespace ve::ecs {
    /** An entity_type can be any unsigned integral type. */
    template <typename T> concept entity_type = std::unsigned_integral<T>;


    /**
     * Concept indicating a class can be used as entity traits for an ECS container.
     * See @ref default_entity_traits for an example implementation of an entity traits class.
     */
    template <typename T> concept entity_traits = requires {
        { T::index_bits      } -> meta::const_member_type<u8>;
        { T::version_bits    } -> meta::const_member_type<u8>;
        { T::index_stability } -> meta::const_member_type<bool>;
        { T::page_size       } -> meta::const_member_type<std::size_t>;
        // Entity type must be a valid entity type.
        entity_type<typename T::type>;
        // Index, version and unassigned bits must add up to size of entity type.
        (T::index_bits + T::version_bits) <= 8 * sizeof(T);
        // Page size must be a power of two.
        std::popcount(T::page_size) == 1;
    };


    /** Returns true if two entity traits classes represent the same traits, even if they are different classes. */
    template <entity_traits T1, entity_traits T2> constexpr inline bool entity_traits_equivalent = (
        std::is_same_v<typename T1::type, typename T2::type> &&
        T1::index_bits      == T2::index_bits                &&
        T1::version_bits    == T2::version_bits              &&
        T1::index_stability == T2::index_stability           &&
        T1::page_size       == T2::page_size
    );


    /** Default entity_traits class used if no other entity traits are provided to an ECS container. */
    template <entity_type T> struct default_entity_traits {
        /** The type of entity these traits are associated with. */
        using type = T;

        /**
         * Each entity ID is split into an index and a version, with any leftover unassigned bits free for user defined purposes.
         * The number of index bits controls how many entities can exist at the same time.
         * The number of version bits controls how many times an entity ID can be reused before it becomes unusable.
         * (Note: reuse of entity IDs is required to keep the memory usage of the ECS from blowing up.)
         * Data is stored in the order index, version, unassigned bits from LSB to MSB.
         */
        constexpr static inline u8 index_bits      = 5 * sizeof(T) - (VE_DEFAULT_ENTITY_UBITS);
        /** @copydoc index_bits */
        constexpr static inline u8 version_bits    = 3 * sizeof(T);

        /**
         * If set to true, the index of entities in the dense vector of the sparse set will not change.
         * This does not guarantee any kind of iterator or reference stability. Just that the index remains constant.
         * This is required to enable reference stability for components, however.
         * Enabling this option allows the existence of tombstone entities in the dense vector of the sparse set,
         * which may negatively impact performance.
         */
        constexpr static inline bool index_stability = false;

        /**
         * Size for pages in the sparse set, in number of entities. Must be a power of two.
         * Usage of paging reduces memory usage from large unoccupied entity ID ranges.
         */
        constexpr static inline std::size_t page_size = 1024ull;
    };


    /**
     * Utility to generate a new entity traits class by changing an existing one.
     * @tparam Traits The entity traits to transform.
     */
    template <entity_traits Traits> struct transform_entity_traits {
        /** Transformed traits class with new settings. */
        template <entity_type ET, u8 ib, u8 vb, bool is, std::size_t ps> struct traits_builder {
            using type = ET;

            constexpr static inline u8 index_bits         = ib;
            constexpr static inline u8 version_bits       = vb;
            constexpr static inline bool index_stability  = is;
            constexpr static inline std::size_t page_size = ps;
        };


        template <entity_type T> using with_entity_type = traits_builder<
            T,
            Traits::index_bits,
            Traits::version_bits,
            Traits::index_stability,
            Traits::page_size
        >;

        template <u8 IndexBits, u8 VersionBits> using with_bit_distribution = traits_builder<
            typename Traits::type,
            IndexBits,
            VersionBits,
            Traits::index_stability,
            Traits::page_size
        >;

        template <bool IndexStability> using with_index_stability = traits_builder<
            typename Traits::type,
            Traits::index_bits,
            Traits::version_bits,
            IndexStability,
            Traits::page_size
        >;

        template <std::size_t PageSize> using with_page_size = traits_builder<
            typename Traits::type,
            Traits::index_bits,
            Traits::version_bits,
            Traits::index_stability,
            PageSize
        >;
    };


    /** Utility functions associated with a set of entity traits. */
    template <entity_traits Traits> struct entity_utils {
        using traits_type = Traits;
        using entity_type = typename Traits::type;


        /** Returns the tombstone value. That is, the entity ID associated with the absence of an entity. */
        [[nodiscard]] consteval static entity_type tombstone(void) {
            return max_value<entity_type>;
        }


        /** Returns a bitmask to get the index part of an entity ID. */
        [[nodiscard]] consteval static entity_type index_mask(void) {
            return set_n_bits<entity_type>(traits_type::index_bits);
        }


        /** Returns a bitmask to get the version part of an entity ID. */
        [[nodiscard]] consteval static entity_type version_mask(void) {
            return set_n_bits<entity_type>(traits_type::version_bits, traits_type::index_bits);
        }


        /** Returns a bitmask to get the part of an entity ID not used for the index or the version of the entity. */
        [[nodiscard]] consteval static entity_type unassigned_bits_mask(void) {
            return max_value<entity_type>
                & ~index_mask()
                & ~version_mask();
        }


        [[nodiscard]] constexpr static entity_type index_of(entity_type entity) {
            return entity & index_mask();
        }


        [[nodiscard]] constexpr static entity_type version_of(entity_type entity) {
            return (entity & version_mask()) >> traits_type::index_bits;
        }


        [[nodiscard]] constexpr static entity_type unassigned_bits_of(entity_type entity) {
            return entity >> (traits_type::index_bits + traits_type::version_bits);
        }


        [[nodiscard]] constexpr static entity_type make_entity(entity_type index, entity_type version) {
            return index | (version << traits_type::index_bits);
        }


        [[nodiscard]] constexpr static entity_type make_entity(entity_type index, entity_type version, entity_type unassigned) requires (unassigned_bits_mask() != 0) {
            return (((unassigned << traits_type::version_bits) | version) << traits_type::index_bits) | index;
        }


        [[nodiscard]] constexpr static entity_type next_version(entity_type entity) {
            return (entity & ~version_mask()) | ((version_of(entity) + 1) << traits_type::index_bits);
        }


        /** Returns true if incrementing the version of this entity gives another valid entity. */
        [[nodiscard]] constexpr static bool has_next_version(entity_type entity) {
            return version_of(entity) != (version_mask() >> traits_type::index_bits);
        }


        [[nodiscard]] constexpr static entity_type next_index(entity_type entity) {
            return entity + 1;
        }


        [[nodiscard]] constexpr static bool has_next_index(entity_type entity) {
            return index_of(entity) != index_mask();
        }


        /** Deconstructs an entity ID into an index and a version. */
        [[nodiscard]] constexpr static std::tuple<entity_type, entity_type> decompose(entity_type entity) {
            return { index_of(entity), version_of(entity) };
        }


        /** Deconstructs an entity ID into an index, a version and the remaining unassigned bits. */
        [[nodiscard]] constexpr static std::tuple<entity_type, entity_type, entity_type> decompose_complete(entity_type entity) requires (unassigned_bits_mask() != 0) {
            return { index_of(entity), version_of(entity), unassigned_bits_of(entity) };
        }


        /** Returns a printable string containing the entity's index, version and unassigned bits. */
        [[nodiscard]] static std::string entity_string(entity_type entity) {
            if (entity == tombstone()) return std::format("[Entity ({})] {{ Tombstone }}", typename_of<entity_type>());

            if constexpr (unassigned_bits_mask() != 0) {
                return std::format(
                    "[Entity ({})] {{ ID = {}, Index = {}, Version = {}, Unassigned = {:#X} }}",
                    typename_of<entity_type>(),
                    entity,
                    index_of(entity),
                    version_of(entity),
                    unassigned_bits_of(entity)
                );
            } else {
                return std::format(
                    "[Entity ({})] {{ ID = {}, Index = {}, Version = {} }}",
                    typename_of<entity_type>(),
                    entity,
                    index_of(entity),
                    version_of(entity)
                );
            }
        }
    };
}