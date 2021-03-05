#pragma once

#include <VoxelEngine/core/core.hpp>

#include <GL/glew.h>

#include <string>
#include <cstddef>
#include <vector>


namespace ve::graphics {
    // Defines the in- and outputs of a shader program.
    struct shader_layout {
        struct shader_layout_attribute {
            std::string name;
            GLenum type;
            std::size_t count;
            
            ve_eq_comparable(shader_layout_attribute);
        };
        
        
        std::vector<shader_layout_attribute> inputs;
        std::vector<shader_layout_attribute> outputs;
        
        
        bool operator==(const shader_layout& other) const {
            for (auto& ctr : { &shader_layout::inputs, &shader_layout::outputs }) {
                if ((this->*ctr).size() != (other.*ctr).size()) return false;
                
                for (std::size_t i = 0; i < (this->*ctr).size(); ++i) {
                    if ((this->*ctr)[i] != (other.*ctr)[i]) return false;
                }
            }
            
            return true;
        }
        
        bool operator!=(const shader_layout& other) const = default;
    };
}