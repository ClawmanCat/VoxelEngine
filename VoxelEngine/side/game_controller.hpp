#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/side/client/client.hpp>
#include <VoxelEngine/side/server/server.hpp>

#include <vector>


namespace ve {
    class game_controller {
    public:
        template <typename T, typename... Args> requires std::is_base_of_v<server, T>
        weak<server> create_server(Args&&... args) {
            server = std::make_shared<T>(std::forward<Args>(args)...);
            return server;
        }
        
        void remove_server(void) {
            server = nullptr;
        }
    
        [[nodiscard]] weak<server> get_server(void) { return server; }
        [[nodiscard]] weak<const server> get_server(void) const { return server; }
        
        
        template <typename T, typename... Args> requires std::is_base_of_v<client, T>
        weak<client> create_client(Args&&... args) {
            auto [it, success] = clients.emplace(std::make_shared<T>(std::forward<Args>(args)...));
            return *it;
        }
        
        void remove_client(client* c) {
            clients.erase(c);
        }
    private:
        shared<server> server;
        hash_set<shared<client>> clients;
    };
}