#include <VoxelEngine/graphics/window_manager.hpp>



namespace ve {
    [[nodiscard]] window_manager& window_manager::instance(void) {
        static window_manager i;
        return i;
    }
    
    
    void window_manager::on_actor_destroyed(ve::actor_id id) {
        std::vector<shared<window>> removed_windows;
        
        // Move windows out of the manager because they will remove themselves when being destroyed.
        // (so we don't want the mutex to be locked when they do.)
        {
            std::lock_guard lock{mtx};
        
            for (i32 i = windows.size() - 1; i >= 0; --i) {
                auto& [owner, win] = windows[i];
                
                if (owner == id) {
                    VE_LOG_DEBUG(
                        "Removing window "s +
                        win->get_name() +
                        " (" +
                        std::to_string(win->get_id()) +
                        ") because its owner was destroyed."
                    );
                    
                    removed_windows.push_back(std::move(win));
                    swap_erase(windows, windows.begin() + i);
                }
            }
        }
    }
    
    
    weak<window> window_manager::create_window(const window::arguments& args, actor_id owner) {
        std::lock_guard lock { mtx };
        
        // Can't use make shared because constructor is only exposed through friendship.
        auto win = shared<window> { new window { args } };
        
        windows.push_back({ owner, win });
        return win;
    }
    
    
    void window_manager::remove_window(u32 id) {
        std::lock_guard lock { mtx };
        
        auto it = std::find_if(
            windows.begin(),
            windows.end(),
            [&](const auto& pair) { return pair.second->get_id() == id; }
        );
        
        if (it != windows.end()) windows.erase(it);
    }
    
    
    weak<window> window_manager::get_window(u32 id) {
        auto it = std::find_if(
            windows.begin(),
            windows.end(),
            [&](const auto& pair) { return pair.second->get_id() == id; }
        );
        
        return it == windows.end() ? weak<window> { } : it->second;
    }
}