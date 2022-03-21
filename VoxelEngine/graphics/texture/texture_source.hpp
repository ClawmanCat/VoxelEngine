#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/io/file_io.hpp>


namespace ve::gfx {
    template <typename Pixel> struct texture_source {
        virtual ~texture_source(void) = default;

        // Some deriving classes contain the image inside themselves, others load it from elsewhere.
        // To prevent unnecessary image copies, return a pointer and then give the deriving class the opportunity to clean it up later.
        // TODO: This should probably be handled through some kind of RAII-wrapper instead.
        virtual const image<Pixel>* require(void) = 0;
        virtual void relinquish(const image<Pixel>* ptr) {}

        virtual std::string name(void) const = 0;
    };


    template <typename Pred, typename Pixel = RGBA8> requires std::is_invocable_r_v<image<Pixel>, Pred>
    struct generative_image_source : texture_source<Pixel> {
        std::string texture_name;
        Pred pred;


        generative_image_source(std::string texture_name, Pred pred) : texture_name(std::move(texture_name)), pred(std::move(pred)) {}


        const image<Pixel>* require(void) override {
            return new image<Pixel> { std::invoke(pred) };
        }


        void relinquish(const image<Pixel>* ptr) override {
            delete ptr;
        }


        std::string name(void) const override {
            return texture_name;
        }
    };


    // TODO: Templatize on pixel type once loading of non RGBA8-images is supported.
    struct file_image_source : texture_source<RGBA8> {
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


    template <typename Pixel> struct direct_image_source : texture_source<Pixel> {
        std::string texture_name;
        const image<Pixel>* img;


        direct_image_source(std::string texture_name, const image<Pixel>* img) : texture_name(std::move(texture_name)), img(img) {}


        const image<Pixel>* require(void) override {
            return img;
        }


        std::string name(void) const override {
            return texture_name;
        }
    };
}