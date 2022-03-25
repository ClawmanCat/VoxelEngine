#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/priority.hpp>
#include <VoxelEngine/utility/arbitrary_storage.hpp>

#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>


namespace ve::gfx::preprocessors {
    namespace utility {
        struct directive_token { std::string_view key, args; };

        inline std::optional<directive_token> find_directive(std::string_view line) {
            enum { NO_TOKEN, STARTING_KEY, PARSING_KEY, STARTING_ARGS, PARSING_ARGS } state = NO_TOKEN;

            std::string_view::const_iterator token_begin, token_end, args_begin;
            token_begin = token_end = args_begin = line.end();

            for (auto it = line.begin(); it != line.end(); ++it) {
                if (state == NO_TOKEN && *it == '#') {
                    state = STARTING_KEY;
                    continue;
                }

                if (state == STARTING_KEY && std::isalpha(*it)) {
                    state = PARSING_KEY;
                    token_begin = it;
                    continue;
                }

                if (state == PARSING_KEY && std::isspace((*it))) {
                    state = STARTING_ARGS;
                    token_end = std::prev(it);
                    continue;
                }

                if (state == STARTING_ARGS && std::isalnum(*it)) {
                    state = PARSING_ARGS;
                    args_begin = it;
                    break; // Args continue until end of line.
                }
            }

            if (token_begin == line.end()) return std::nullopt;
            else return directive_token {
                .key  = std::string_view { token_begin, token_end },
                .args = std::string_view { args_begin, line.end() }
            };
        }
    }




    class shader_preprocessor {
    public:
        explicit shader_preprocessor(std::string name, u16 priority = priority::NORMAL) : priority(priority), name(std::move(name)) {}
        virtual ~shader_preprocessor(void) = default;

        // Performs some transform on the shader source code.
        // Context can be used to pass data between preprocessor stages.
        virtual void operator()(std::string& src, arbitrary_storage& context) const = 0;

        VE_GET_VAL(priority);
        VE_GET_CREF(name);
    private:
        u16 priority;
        std::string name;
    };




    // Preprocessor to add custom preprocessor directives.
    class directive_preprocessor : public shader_preprocessor {
    public:
        struct directive_state_machine {
            enum { STARTED, STOPPED } state = STOPPED;

            virtual ~directive_state_machine(void) = default;

            // Returns true if the current line should start the machine.
            virtual bool start(std::string_view) = 0;
            // Returns true if the current line should stop the machine.
            virtual bool stop(std::string_view) = 0;
            // Invoked for every line in the source file while the machine is started.
            // Note: progress() is invoked before stop(), so both the start and end lines are also fed to this function.
            virtual bool progress(std::string&) = 0;
        };


        explicit directive_preprocessor(std::string name, u16 priority = priority::NORMAL)
            : shader_preprocessor(std::move(name), priority)
        {}


        void operator()(std::string& src, arbitrary_storage& context) const override {
            std::string result;
            result.reserve(src.size());

            for (const auto& line : src | views::split('\n')) {
                std::string transformed = line | ranges::to<std::string>;

                using dsm = directive_state_machine;
                for (auto& directive : directives | views::indirect) {
                    if (directive.state == dsm::STOPPED && directive.start(transformed)) {
                        directive.state = dsm::STARTED;
                    }

                    if (directive.state == dsm::STARTED) {
                        directive.progress(transformed);
                    }

                    if (directive.state == dsm::STARTED && directive.stop(transformed)) {
                        directive.state = dsm::STOPPED;
                    }
                }

                result.append(transformed);
                result.push_back('\n');
            }

            src = std::move(result);
        }


        void add_directive(std::string key, std::function<std::string(std::string)> action) {
            struct machine : directive_state_machine {
                std::string key;
                std::function<std::string(std::string)> action;

                machine(std::string key, std::function<std::string(std::string)> action)
                    : key(std::move(key)), action(std::move(action))
                {}

                bool start(std::string_view line) override {
                    auto directive = utility::find_directive(line);
                    return directive && directive->key == key;
                };

                bool stop(std::string_view line) override {
                    return true;
                }

                bool progress(std::string& line) override {
                    action(line);
                    return true;
                }
            };

            add_directive(machine { std::move(key), std::move(action) });
        }


        template <typename Machine> requires std::is_base_of_v<directive_state_machine, Machine>
        void add_directive(Machine m) {
            directives.emplace_back(make_unique<Machine>(std::move(m)));
        }


        VE_GET_CREF(directives);
    private:
        mutable std::vector<unique<directive_state_machine>> directives;
    };




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


            std::stringstream result;

            try {
                for (const auto& token : *wave_ctx) result << token.get_value();
                src = result.str();
            } catch (const boost::wave::preprocess_exception& e) {
                const auto& name  = context.template get_object<std::string>("ve.filename");
                const auto* stage = context.template get_object<const gfxapi::shader_stage*>("ve.shader_stage");


                // Boost stores the actual message in .description(), even though they provide a .what().
                std::string msg { cat(
                    "Failed to preprocess ", stage->name, " stage of shader ", name, ":\n",
                    e.description(), "\n\n",
                    "Note: error occurred here:\n",
                    get_error_location(result.str()), " <<< HERE"
                ) };

                throw std::runtime_error { msg };
            }
        }


        void add_context_action(context_action action) {
            context_actions.push_back(std::move(action));
        }

        void add_include_path(fs::path path) {
            add_context_action([p = std::move(path)] (wave_context& wave_ctx, std::string& src, arbitrary_storage& ve_ctx) {
                std::string path_string = p.string(); // Note: p.c_str() may return wchar_t on some platforms (Notably Windows).
                wave_ctx.add_include_path(path_string.c_str());
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
        unique<wave_context> workaround_137(std::string& src, arbitrary_storage& context) const {
            static_assert(
                BOOST_VERSION_NUMBER_MINOR(BOOST_VERSION) < 79,
                "This issue has been fixed in Boost 1.79 and this code should be replaced once its released."
            );

            src += '\n'; // Previous stage may strip final newline.

            // Boost is preventing this value from being directly returnable through some non-copyable bullshittery,
            // even though RVO should apply.
            auto wave_ctx = make_unique<wave_context>(src.begin(), src.end(), context.get_object<std::string>("ve.filename").c_str());
            wave_ctx->set_language(boost::wave::enable_long_long(wave_ctx->get_language()));
            wave_ctx->set_language(boost::wave::enable_variadics(wave_ctx->get_language()));

            return wave_ctx;
        }


        static std::string get_error_location(std::string_view src, std::size_t lines = 10) {
            auto error_location = split(src, "\n");

            if (error_location.empty()) error_location.push_back("<EMPTY FILE>");
            if (error_location.size() > lines) error_location.erase(error_location.begin(), error_location.end() - lines);

            return cat_range_with(error_location, "\n");
        }
    };
}