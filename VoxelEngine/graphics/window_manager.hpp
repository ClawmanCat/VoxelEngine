#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/window.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>
#include <VoxelEngine/utils/container_utils.hpp>
#include <VoxelEngine/utils/immovable.hpp>
#include <VoxelEngine/utils/iteratable.hpp>

#include <string>
#include <mutex>
#include <algorithm>
#include <vector>


namespace ve {
    class window_manager : public resource_owner<window_manager> {
    public:
        [[nodiscard]] static window_manager& instance(void);
        
        void on_actor_destroyed(ve::actor_id id);
        
        
        weak<window> VE_RESOURCE_FN(create_window, const window::arguments& args);
        
        void remove_window(u32 id);
        weak<window> get_window(u32 id);
    private:
        window_manager(void) = default;
        ve_make_immovable;
        
        
        vec_map<actor_id, shared<window>> windows;
        std::mutex mtx;
    public:
        ve_make_iteratable(windows);
    };
}