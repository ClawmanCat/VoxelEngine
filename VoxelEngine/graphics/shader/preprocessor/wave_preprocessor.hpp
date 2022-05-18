#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/then.hpp>
#include <VoxelEngine/graphics/shader/preprocessor/shader_preprocessor.hpp>

#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>


namespace ve::gfx {
    // Boost.Wave preprocessor (Standard C99 preprocessor). Custom context policy can be set through the Hook template parameter.
    // Other than macros and include paths, custom actions can be provided to initialize the wave context before it is used.
    template <typename Hook = void> class wave_preprocessor : public shader_preprocessor {
    public:
        using wave_token     = boost::wave::cpplexer::lex_token<>;
        using wave_iterator  = boost::wave::cpplexer::lex_iterator<wave_token>;
        using wave_in_policy = boost::wave::iteration_context_policies::load_file_to_string;
        using wave_hook_base = boost::wave::context_policies::default_preprocessing_hooks;


        // Default hook: ignore unknown directives rather raising an exception, since there may be other preprocessor stages.
        struct default_hook : boost::wave::context_policies::eat_whitespace<wave_token> {
            template <typename ContextT, typename ContainerT>
            bool found_unknown_directive(const ContextT& ctx, const ContainerT& line, ContainerT& pending) {
                std::copy(line.begin(), line.end(), std::back_inserter(pending));
                return true;
            }
        };


        using wave_hook      = std::conditional_t<std::is_same_v<Hook, void>, default_hook, Hook>;
        using wave_context   = boost::wave::context<typename std::string::iterator, wave_iterator, wave_in_policy, wave_hook>;
        using context_action = std::function<void(wave_context&, std::string&, arbitrary_storage&)>;


        explicit wave_preprocessor(std::string name, u16 priority = priority::NORMAL)
            : shader_preprocessor(std::move(name), priority)
        {}


        void operator()(std::string& src, arbitrary_storage& context) const override {
            auto wave_ctx = workaround_137(src, context);
            for (const auto& action : context_actions) action(*wave_ctx, src, context);


            // Apply provided preprocessor directives.
            for (const auto& [k, v] : context.template get_object<const shader_compile_settings*>("ve.compile_settings")->preprocessor_definitions) {
                std::string macro_string = k;
                if (!v.empty()) macro_string += "=";
                macro_string += v;

                wave_ctx->add_macro_definition(macro_string, true);
            }


            std::stringstream result;

            try {
                for (const auto& token : *wave_ctx) result << token.get_value();
                src = result.str();
            } catch (const boost::wave::preprocess_exception& e) {
                const auto& name  = context.template get_object<std::string>("ve.filename");
                const auto* stage = context.template get_object<const gfxapi::shader_stage*>("ve.shader_stage");

                // Boost stores the actual message in .description(), even though they provide a .what().
                throw std::runtime_error { cat(
                    "Failed to preprocess ", stage->name, " stage of shader ", name, ":\n",
                    e.description(), "\n\n",
                    "Note: error occurred here:\n",
                    get_error_location(src, wave_ctx->get_main_pos().get_line()), " <<< HERE"
                ) };
            }
        }


        std::size_t hash(void) const override {
            // Context actions cannot be effectively compared, so just hash each instance by its address.
            // This may cause some unnecessary recompilation, but at least it won't give any false negatives.
            std::size_t hash = 0;
            for (const auto& action : context_actions) ve::hash_combine(hash, &action);
            return hash;
        }


        void add_context_action(context_action action) {
            context_actions.push_back(std::move(action));
        }


        void add_include_path(fs::path path, bool sys_include = false) {
            add_context_action([p = std::move(path), sys_include] (wave_context& wave_ctx, std::string& src, arbitrary_storage& ve_ctx) {
                std::string path_string = p.string(); // Note: p.c_str() may return wchar_t on some platforms (Notably Windows).

                sys_include
                    ? wave_ctx.add_include_path(path_string.c_str())
                    : wave_ctx.add_sysinclude_path(path_string.c_str());
            });
        }


        void add_macro(std::string macro) {
            add_context_action([m = std::move(macro)] (wave_context& wave_ctx, std::string& src, arbitrary_storage& ve_ctx) {
                wave_ctx.add_macro_definition(m, true);
            });
        }
    private:
        std::vector<context_action> context_actions;


        // Fix issues related to https://github.com/boostorg/wave/issues/137
        // This should be removed when Boost 1.79 is released.
        // Note that because of this, all shaders currently have to end with a newline character.
        static unique<wave_context> workaround_137(std::string& src, arbitrary_storage& context) {
            static_assert(
                BOOST_VERSION_NUMBER_MINOR(BOOST_VERSION) < 79,
                "This issue has been fixed in Boost 1.79 and this code should be replaced once its released."
            );

            src += '\n'; // Previous stage(s) may strip final newline.

            // Boost is preventing this value from being directly returnable through some non-copyable bullshittery, even though RVO should apply.
            auto wave_ctx = make_unique<wave_context>(src.begin(), src.end(), context.get_object<std::string>("ve.filename").c_str());
            wave_ctx->set_language(boost::wave::enable_long_long(wave_ctx->get_language()));
            wave_ctx->set_language(boost::wave::enable_variadics(wave_ctx->get_language()));

            return wave_ctx;
        }


        static std::string get_error_location(std::string_view src, std::size_t error_line, std::size_t lines = 8) {
            auto error_location = split(src, "\n");
            if (error_location.empty()) return "<EMPTY FILE>";


            error_location = error_location
                | views::drop(error_line - lines)
                | views::take(lines)
                | ranges::to<std::vector>;


            return cat_range_with(error_location, "\n") | then([] (auto& str) { str.pop_back(); });
        }
    };
}