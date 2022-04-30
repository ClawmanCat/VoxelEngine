#include <VoxelEngine/graphics/shader/preprocessor/binding_generator.hpp>
#include <VoxelEngine/utility/string.hpp>


namespace ve::gfx {
    void autobinding_preprocessor::operator()(std::string& src, arbitrary_storage& context) const {
        enum { NO_TOKEN = 1, UBO = 3, SSBO = 4, CONSTANT = 8 } current_token = NO_TOKEN;
        std::size_t directive_start = 0;
        std::size_t current_char = 0;


        auto replace_before_current = [&] (std::size_t where, std::size_t length, std::string_view content) {
            src.erase(where, length);
            current_char -= length;
            src.insert(where, content);
            current_char += content.length();
        };

        auto& data = context.template get_or_create_object<autobinding_data>("ve.autobinding.data");


        while (current_char < src.length()) {
            auto remaining = std::string_view { src.begin() + current_char, src.end() };
            eat_whitespace(remaining, current_char);


            // Begin new token.
            if (current_token == NO_TOKEN) {
                directive_start = current_char;

                if (remaining.starts_with("UBO"))      current_token = UBO;
                if (remaining.starts_with("SSBO"))     current_token = SSBO;
                if (remaining.starts_with("CONSTANT")) current_token = CONSTANT;

                current_char += (std::size_t) current_token;
            }


            // Read name and replace directive with appropriate GLSL.
            else {
                std::string current_name = std::string { eat_word(remaining, current_char) };

                auto& binding_map = [&] () -> auto& {
                    switch (current_token) {
                        case UBO:      return data.ubo_bindings;
                        case SSBO:     return data.ssbo_bindings;
                        case CONSTANT: return data.constant_bindings;
                        default:       VE_UNREACHABLE;
                    }
                }();

                std::size_t binding;
                if (auto it = binding_map.find(current_name); it != binding_map.end()) {
                    binding = it->second;
                } else {
                    binding = (current_token == CONSTANT) ? data.constant_count++ : data.uniform_count++;
                    binding_map.emplace(std::move(current_name), binding);
                }

                std::string replacement_string;
                switch (current_token) {
                    case UBO:      replacement_string = cat("layout(std140, binding = ", binding, ") uniform "); break;
                    case SSBO:     replacement_string = cat("layout(std430, binding = ", binding, ") buffer  "); break;
                    case CONSTANT: replacement_string = cat("layout(constant_id = ", binding, ") const "); break;
                    default:       VE_UNREACHABLE;
                }

                replace_before_current(directive_start, (std::size_t) current_token, replacement_string);
                current_token = NO_TOKEN;
            }
        }
    }


    void autobinding_preprocessor::eat_whitespace(std::string_view& sv, std::size_t& counter) {
        std::size_t i = 0;
        while (i < sv.length() && std::isspace(sv[i])) ++i;

        counter += i;
        sv.remove_prefix(i);
    }


    std::string_view autobinding_preprocessor::eat_word(std::string_view& sv, std::size_t& counter) {
        std::size_t i = 0;
        while (i < sv.length() && !std::isspace(sv[i])) ++i;

        counter += i;
        std::string_view result = sv.substr(0, i);
        sv.remove_prefix(i);

        return result;
    }
}