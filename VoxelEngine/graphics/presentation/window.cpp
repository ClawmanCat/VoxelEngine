#include <VoxelEngine/graphics/presentation/window.hpp>
#include <VoxelEngine/graphics/texture/utility/utility.hpp>
#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/utility/priority.hpp>
#include <VoxelEngine/utility/raii.hpp>
#include <VoxelEngine/utility/io/file_io.hpp>
#include <VoxelEngine/utility/io/paths.hpp>


namespace ve::gfx {
    void window::init(const window::arguments& args) {
        VE_PROFILE_FN();

        if (args.graphics_window) gfxapi::prepare_api_state(args.api_settings);


        handle = SDL_CreateWindow(
            args.title.c_str(),
            args.position.x, args.position.y,
            (i32) args.size.x, (i32) args.size.y,
            args.flags | (args.graphics_window ? gfxapi::window_helpers::get_window_flags() : 0)
        );


        // Set window icon if specified, otherwise use the engine icon.
        set_icon(args.icon ? *args.icon : io::load_image(io::paths::PATH_ASSETS / "meta" / "logo.png"));


        if (args.start_maximized) maximize();


        if (args.graphics_window) {
            gfxapi::get_or_create_context(handle, args.api_settings);
            canvas = make_shared<gfxapi::canvas>(this, args.present_mode);
        }


        // Clicking the close button simply triggers the event, but does not actually call SDL_DestroyWindow.
        // This is only an issue with multiple windows, as closing the final window triggers an SDL_QuitEvent anyway.
        if (args.exit_button_closes_window) input_manager::instance().add_raw_handler(
            [this] (const window_closed_event& event) { if (event.window == this) close(); },
            priority::LOWEST
        );


        window_registry::instance().add_window(this);
    }


    window::~window(void) {
        close();
    }


    void window::begin_frame(void) {
        VE_PROFILE_FN();
        if (canvas) canvas->begin_frame();
    }


    void window::end_frame(void) {
        VE_PROFILE_FN();
        if (canvas) canvas->end_frame();
    }


    void window::close(void) {
        if (handle) {
            canvas = nullptr;
            SDL_DestroyWindow(handle);

            window_registry::instance().remove_window(this);
        }

        handle = nullptr;
    }


    bool window::is_closed(void) const {
        return !handle;
    }


    void window::set_window_mode(window_mode mode) {
        switch (mode) {
            case window_mode::BORDERED:
                SDL_SetWindowFullscreen(handle, 0);
                SDL_SetWindowBordered(handle, SDL_TRUE);
                break;
            case window_mode::BORDERLESS:
                SDL_SetWindowFullscreen(handle, 0);
                SDL_SetWindowBordered(handle, SDL_FALSE);
                break;
            case window_mode::FULLSCREEN:
                SDL_SetWindowFullscreen(handle, SDL_WINDOW_FULLSCREEN);
                break;
        }
    }


    window::window_mode window::get_window_mode(void) const {
        auto flags = SDL_GetWindowFlags(handle);

        return flags & SDL_WINDOW_FULLSCREEN
            ? window_mode::FULLSCREEN
            : (flags & SDL_WINDOW_BORDERLESS ? window_mode::BORDERLESS : window_mode::BORDERED);
    }


    vec2ui window::get_window_size(void) const {
        vec2i result;
        SDL_GetWindowSize(handle, &result.x, &result.y);
        return (vec2ui) result;
    }


    void window::set_window_size(const vec2ui& size) {
        SDL_SetWindowSize(handle, (i32) size.x, (i32) size.y);
    }


    vec2ui window::get_canvas_size(void) const {
        return gfxapi::window_helpers::get_canvas_size(handle);
    }


    void window::set_canvas_size(const vec2ui& size) {
        vec2ui border = get_window_size() - get_canvas_size();
        set_window_size(size + border);
    }


    window::window_location window::get_location(void) const {
        window_location result;

        SDL_GetWindowPosition(handle, &result.x, &result.y);
        result.display = SDL_GetWindowDisplayIndex(handle);

        return result;
    }


    vec2i window::get_position(void) const {
        vec2i result;
        SDL_GetWindowPosition(handle, &result.x, &result.y);
        return result;
    }


    void window::set_position(const vec2i& position) {
        SDL_SetWindowPosition(handle, position.x, position.y);
    }


    present_mode_t window::get_present_mode(void) const {
        return canvas->get_mode();
    }


    void window::set_present_mode(present_mode_t mode) {
        canvas->set_present_mode(mode);
    }


    void window::set_icon(const image_rgba8& icon) {
        // There is no explicit requirement that icons be 64x64 anywhere in SDLs documentation,
        // but if they're not, SDL_SetWindowIcon just doesn't do anything.
        auto resized_icon = resize_image(icon, vec2ui { 64, 64 }, image_samplers::bilinear<RGBA8>{});


        SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
            (void*) resized_icon.data.data(),
            (int) resized_icon.size.x,
            (int) resized_icon.size.y,
            8 * 4,
            (int) (4 * resized_icon.size.x),
            0x00'00'00'FF, // R
            0x00'00'FF'00, // G
            0x00'FF'00'00, // B
            0xFF'00'00'00  // A
        );


        if (!surface) {
            // Surface is null, no need to free.
            throw std::runtime_error { cat("Failed to create SDL surface for icon image: ", SDL_GetError()) };
        }


        SDL_SetWindowIcon(handle, surface);
        SDL_FreeSurface(surface);
    }
}