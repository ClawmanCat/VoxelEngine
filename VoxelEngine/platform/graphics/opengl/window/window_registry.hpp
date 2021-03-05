#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>
#include <VoxelEngine/platform/graphics/opengl/window/window.hpp>


namespace ve::graphics {
    class window_registry : public resource_owner<window_registry> {
    public:
        static window_registry& instance(void);
    
    
        void on_actor_destroyed(actor_id id) {
            auto it = windows.find(id);
            if (it == windows.end()) return;
    
            for (auto* win : it->second) {
                VE_LOG_DEBUG("Closing window " + win->get_window_title() + " because its owner was destroyed.");
                win->close();
                window_ids.erase(win->get_window_id());
            }
            
            windows.erase(it);
        }
        
        
        void add_window(window* win, ve_default_actor(owner)) {
            windows[owner].push_back(win);
            window_ids[win->get_window_id()] = win;
        }
        
        
        void remove_window(window* win, ve_default_actor(owner)) {
            auto windows_for_actor_it = windows.find(owner);
            if (windows_for_actor_it == windows.end()) return;
            
            auto& windows_for_actor = windows_for_actor_it->second;
            auto it = std::find(
                windows_for_actor.begin(),
                windows_for_actor.end(),
                win
            );
            
            if (it != windows_for_actor.end()) windows_for_actor.erase(it);
            window_ids.erase(win->get_window_id());
        }
        
        
        void draw_all(void) {
            for (auto& [owner, windows_for_owner] : windows) {
                for (auto& window : windows_for_owner) window->draw();
            }
        }
        
        
        expected<window*> get_window(u32 id) {
            auto it = window_ids.find(id);
            
            if (it == window_ids.end()) return make_unexpected("No window with ID "s + std::to_string(id));
            else return it->second;
        }
    private:
        window_registry(void) = default;
        
        small_flat_map<actor_id, std::vector<window*>> windows;
        small_flat_map<u32, window*> window_ids;
    };
}