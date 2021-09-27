#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/bit.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/buffer.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/render_context.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    struct vertex_buffer {
        virtual ~vertex_buffer(void) = default;
        virtual void draw(render_context& ctx) const = 0;
    };


    template <typename Vertex> requires has_vertex_layout<Vertex>
    class unindexed_vertex_buffer : public vertex_buffer {
    public:
        unindexed_vertex_buffer(void) = default;


        explicit unindexed_vertex_buffer(std::span<const Vertex> vertices) {
            update(vertices);
        }


        void draw(render_context& ctx) const override {
            VkDeviceSize offset = 0;

            ctx.cmd_buffer->record(vkCmdBindVertexBuffers, 0, 1, &vertex_storage.get_handle(), &offset);
            ctx.cmd_buffer->record(vkCmdDraw, (u32) vertex_count, 1, 0, 0);
        }


        void reserve(std::size_t capacity) {
            if (vertex_storage.get_size() < capacity * sizeof(Vertex)) {
                buffer new_vertex_storage { (u32) (capacity * sizeof(Vertex)), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT };
                if (vertex_storage.has_storage()) vertex_storage.copy_to(new_vertex_storage);

                vertex_storage = std::move(new_vertex_storage);
            }
        }


        void update(std::span<const Vertex> vertices, std::size_t where = 0) {
            VE_ASSERT(vertex_count >= where, "Attempt to write to vertex buffer would leave uninitialized vertices within rendering range.");

            reserve(vertices.size() + where);
            vertex_storage.write(to_byte_span(vertices), where * sizeof(Vertex));

            vertex_count = std::max(vertex_count, vertices.size() + where);
        }


        VE_GET_CREF(vertex_storage);
        VE_GET_VAL(vertex_count);
    private:
        buffer vertex_storage;
        std::size_t vertex_count = 0;
    };


    template <typename Vertex, typename Index> requires (has_vertex_layout<Vertex> && meta::pack<u8, u16, u32>::template contains<Index>)
    class indexed_vertex_buffer : public vertex_buffer {
    public:
        indexed_vertex_buffer(void) = default;


        indexed_vertex_buffer(std::span<const Vertex> vertices, std::span<const Index> indices) {
            update_vertices(vertices);
            update_indices(indices);
        }


        void draw(render_context& ctx) const override {
            VkDeviceSize offset = 0;

            ctx.cmd_buffer->record(vkCmdBindVertexBuffers, 0, 1, &vertex_storage.get_handle(), &offset);
            ctx.cmd_buffer->record(vkCmdBindIndexBuffer, index_storage.get_handle(), 0, get_index_type());
            ctx.cmd_buffer->record(vkCmdDrawIndexed, (u32) index_count, 1, 0, 0, 0);
        }


        void reserve_vertices(std::size_t capacity) {
            if (vertex_storage.get_size() < capacity * sizeof(Vertex)) {
                buffer new_vertex_storage { (u32) (capacity * sizeof(Vertex)), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT };
                if (vertex_storage.has_storage()) vertex_storage.copy_to(new_vertex_storage);

                vertex_storage = std::move(new_vertex_storage);
            }
        }


        void reserve_indices(std::size_t capacity) {
            if (index_storage.get_size() < capacity * sizeof(Index)) {
                buffer new_index_storage { (u32) (capacity * sizeof(Index)), VK_BUFFER_USAGE_INDEX_BUFFER_BIT };
                if (index_storage.has_storage()) index_storage.copy_to(new_index_storage);

                index_storage = std::move(new_index_storage);
            }
        }


        void update_vertices(std::span<const Vertex> vertices, std::size_t where = 0) {
            VE_ASSERT(vertex_count >= where, "Attempt to write to vertex buffer would leave uninitialized vertices within rendering range.");

            reserve(vertices.size() + where);
            vertex_storage.write(to_byte_span(vertices), where * sizeof(Vertex));

            vertex_count = std::max(vertex_count, vertices.size() + where);
        }


        void update_indices(std::span<const Index> indices, std::size_t where = 0) {
            VE_ASSERT(index_count >= where, "Attempt to write to index buffer would leave uninitialized indices within rendering range.");

            reserve_indices(indices.size() + where);
            index_storage.write(to_byte_span(indices), where * sizeof(Index));

            index_count = std::max(index_count, indices.size() + where);
        }


        VE_GET_CREF(vertex_storage);
        VE_GET_CREF(index_storage);
        VE_GET_VAL(vertex_count);
        VE_GET_VAL(index_count);
    private:
        buffer vertex_storage;
        buffer index_storage;
        std::size_t vertex_count = 0;
        std::size_t index_count = 0;


        constexpr static VkIndexType get_index_type(void) {
            if constexpr (std::is_same_v<Index, u8>)  return VK_INDEX_TYPE_UINT8_EXT;
            if constexpr (std::is_same_v<Index, u16>) return VK_INDEX_TYPE_UINT16;
            if constexpr (std::is_same_v<Index, u32>) return VK_INDEX_TYPE_UINT32;
        }
    };
}