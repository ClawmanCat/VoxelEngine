#include <VoxelEngine/graphics/shader/preprocessor/dsm_preprocessor.hpp>


namespace ve::gfx {
    std::optional<utility::directive_token> utility::find_directive(std::string_view line) {
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


    directive_preprocessor::directive_preprocessor(std::string name, u16 priority)
        : shader_preprocessor(std::move(name), priority)
    {}


    void directive_preprocessor::operator()(std::string& src, arbitrary_storage& context) const {
        std::string result;
        result.reserve(src.size());


        for (auto& directive : directives | views::indirect) {
            directive.original_source = &src;
            directive.parsed_source   = &result;
        }


        for (const auto& line : src | views::split('\n')) {
            std::string transformed = line | ranges::to<std::string>;

            using dsm = directive_state_machine;
            for (auto& directive : directives | views::indirect) {
                if (directive.state == dsm::STOPPED && directive.start(transformed)) {
                    directive.state = dsm::STARTED;
                }

                if (directive.state == dsm::STARTED) {
                    directive.progress(transformed, context);
                }

                if (directive.state == dsm::STARTED && directive.stop(transformed)) {
                    directive.state = dsm::STOPPED;
                }
            }

            result.append(transformed);
            result.push_back('\n');
        }


        for (auto& directive : directives | views::indirect) directive.reset();
        src = std::move(result);
    }


    std::size_t directive_preprocessor::hash(void) const {
        std::size_t result = 0;
        for (const auto& dsm : directives) hash_combine(result, dsm);
        return result;
    }


    void directive_preprocessor::add_directive(std::string key, std::function<std::string(std::string)> action) {
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

            bool stop(std::string_view line) override { return true;  }
            void progress(std::string& line, arbitrary_storage& c) override { action(line); }
        };

        add_directive(machine { std::move(key), std::move(action) });
    }
}