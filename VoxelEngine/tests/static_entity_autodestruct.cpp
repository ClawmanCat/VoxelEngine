#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/ecs/ecs.hpp>


struct destroyable_entity : public ve::static_entity {
    using ve::static_entity::static_entity;


    static inline ve::u32 destroy_count = 0;
    bool moved = false;


    destroyable_entity(destroyable_entity&& o) : static_entity(std::move(o)) { o.moved = true; }

    destroyable_entity& operator=(destroyable_entity&& o) {
        static_entity::operator=(std::move(o));
        o.moved = true;

        return *this;
    }

    ~destroyable_entity(void) {
        if (!moved) ++destroy_count;
    }
};


test_result test_main(void) {
    {
        ve::registry storage { };

        // If the underlying ENTT entity is destroyed, the static_entity should be destroyed as well.
        for (ve::u32 i = 0; i < 10; ++i) {
            auto& e = storage.store_static_entity(destroyable_entity { storage });
            storage.destroy_entity(e.get_id());

            if (destroyable_entity::destroy_count != (i + 1)) {
                return VE_TEST_FAIL("static_entity was not destroyed after underlying entt::entity was destroyed.");
            }
        }

        // If the registry is destroyed the same should happen to any remaining static entities.
        destroyable_entity::destroy_count = 0;

        for (ve::u32 i = 0; i < 10; ++i) {
            storage.store_static_entity(destroyable_entity { storage });
        }
    }


    if (destroyable_entity::destroy_count != 10) {
        return VE_TEST_FAIL("static_entity was not destroyed after registry was destroyed.");
    }


    return VE_TEST_SUCCESS;
}