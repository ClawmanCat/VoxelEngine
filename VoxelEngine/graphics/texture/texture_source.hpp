#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/io/file_io.hpp>


namespace ve::gfx {
    struct texture_source {
        virtual ~texture_source(void) = default;

        // Some deriving classes contain the image inside themselves, others load it from elsewhere.
        // To prevent unnecessary image copies, return a pointer and then give the deriving class the opportunity to clean it up later.
        // TODO: This should probably be handled through RAII instead.
        virtual const image_rgba8* require(void) = 0;
        virtual void relinquish(const image_rgba8* ptr) {}

        virtual std::string name(void) const = 0;
    };


    template <typename Pred> requires std::is_invocable_r_v<image_rgba8, Pred>
    struct generative_image_source : texture_source {
        std::string texture_name;
        Pred pred;


        generative_image_source(std::string texture_name, Pred pred) : texture_name(std::move(texture_name)), pred(std::move(pred)) {}


        const image_rgba8* require(void) override {
            return new image_rgba8 { std::invoke(pred) };
        }


        void relinquish(const image_rgba8* ptr) override {
            delete ptr;
        }


        std::string name(void) const override {
            return texture_name;
        }
    };


    struct file_image_source : texture_source {
        const fs::path* path;
        const image_rgba8* fallback;


        file_image_source(const fs::path* path, const image_rgba8* fallback) : path(path), fallback(fallback) {}


        const image_rgba8* require(void) override {
            try {
                return new image_rgba8 { io::load_image(*path) };
            } catch (const io::io_error& e) {
                VE_LOG_ERROR(cat("Failed to load texture ", path->string(), " (", e.what(), "). Using fallback texture."));
                return fallback;
            }
        }


        void relinquish(const image_rgba8* ptr) override {
            if (ptr != fallback) delete ptr;
        }


        std::string name(void) const override {
            return fs::weakly_canonical(*path).string();
        }
    };


    struct direct_image_source : texture_source {
        std::string texture_name;
        const image_rgba8* image;


        direct_image_source(std::string texture_name, const image_rgba8* image) : texture_name(std::move(texture_name)), image(image) {}


        const image_rgba8* require(void) override {
            return image;
        }


        std::string name(void) const override {
            return texture_name;
        }
    };
}