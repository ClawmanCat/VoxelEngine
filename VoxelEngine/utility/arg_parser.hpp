#pragma once

#include <VoxelEngine/core/core.hpp>

#include <boost/algorithm/string.hpp>

#include <variant>


namespace ve {
    class arg_parser {
    public:
        struct none {};
        using argument_t = std::variant<std::string, i64, double, bool, none>;
        
        
        arg_parser(void) = default;
        arg_parser(std::size_t argc, const char** argv) { feed(argc, argv); }
        
        
        void feed(std::size_t argc, const char** argv) {
            for (std::size_t i = 0; i < argc; ++i) {
                const char* arg_ptr = argv[i];
                while(arg_ptr[0] == '-') ++arg_ptr;
                
                std::string arg { arg_ptr };
                
                
                if (auto split = arg.find('='); split != std::string::npos) {
                    auto key = arg.substr(0, split);
                    auto val = arg.substr(split + 1, std::string::npos);
                    
                    arguments.emplace(std::pair { std::move(key), parse_argument(val) });
                } else {
                    arguments.emplace(std::pair { std::move(arg), none { } });
                }
            }
        }
        
        
        template <typename T> std::optional<T> get(std::string_view name) {
            auto it = arguments.find(name);
            if (it == arguments.end()) return std::nullopt;
            
            if (std::holds_alternative<T>(it->second)) return std::get<T>(it->second);
            else return std::nullopt;
        }
        
        
        bool has(std::string_view name) const {
            return arguments.contains(name);
        }
    
    
        template <typename T> bool has(std::string_view name) const {
            auto it = arguments.find(name);
            return it != arguments.end() && std::holds_alternative<T>(it->second);
        }
    private:
        hash_map<std::string, argument_t> arguments;
        
        
        argument_t parse_argument(const std::string& value) {
            if (value.length() == 0) return argument_t { none { } };
            
            auto lower = boost::algorithm::to_lower_copy(value);
            if (lower == "true")  return argument_t { true };
            if (lower == "false") return argument_t { false };
            
            try {
                return argument_t { std::stoll(value) };
            } catch (...) {}
            
            try {
                return argument_t { std::stod(value) };
            } catch (...) {}
            
            return argument_t { value };
        }
    };
}