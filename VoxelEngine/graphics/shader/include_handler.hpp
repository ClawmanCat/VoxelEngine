#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/string.hpp>
#include <VoxelEngine/utility/io/file_io.hpp>

#include <shaderc/shaderc.hpp>


namespace ve::gfx::detail {
    struct include_handler : public shaderc::CompileOptions::IncluderInterface {
        fs::path directory;


        explicit include_handler(fs::path directory) :
            shaderc::CompileOptions::IncluderInterface(),
            directory(std::move(directory))
        {}


        shaderc_include_result* GetInclude(const char* requested, shaderc_include_type type, const char* requestor, std::size_t depth) override {
            if (type == shaderc_include_type_relative) {
                // TODO: Support relative paths.
                return make_error_result("Relative include paths are not supported.");
            }

            fs::path requested_path = directory / requested;

            try {
                auto file_data = io::read_text(requested_path);
                return make_success_result(requested_path, file_data);
            } catch (const io::io_error& e) {
                return make_error_result("Failed to read file: "s + e.what());
            }
        }


        void ReleaseInclude(shaderc_include_result* data) override {
            if (data->source_name_length) delete[] data->source_name;
            if (data->content_length) delete[] data->content;
            delete[] data;
        }


        // shaderc, being a C-API, does not allow us to provide any kind of managed data.
        // Provide a heap allocated message buffer, and remember to delete it when calling ReleaseInclude.
        static const char* make_heap_buffer(std::string_view message) {
            char* msg_storage = new char[message.length() + 1];
            memcpy(msg_storage, message.data(), message.length());
            msg_storage[message.length()] = '\0';

            return msg_storage;
        }


        static shaderc_include_result* make_error_result(std::string_view message) {
            const char* msg_ptr = make_heap_buffer(message);
            return new shaderc_include_result { "", 0, msg_ptr, message.length(), nullptr };
        }


        static shaderc_include_result* make_success_result(const fs::path& path, const io::text_file& file) {
            auto path_string = path.string();
            const char* path_ptr = make_heap_buffer(path_string);

            auto file_string = cat_range_with(file, "\n");
            const char* file_ptr = make_heap_buffer(file_string);

            return new shaderc_include_result { path_ptr, path_string.length(), file_ptr, file_string.length(), nullptr };
        }
    };
}