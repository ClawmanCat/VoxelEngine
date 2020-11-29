#pragma once

#include <VoxelEngine/core/core.hpp>

#include <iostream>
#include <ostream>


namespace ve {
    class canary {
    public:
        canary(std::ostream& target = std::cout) : target(&target), id(next_id++) {
            (*target) << "Canary" << id << ": [Constructor]\n";
        }
        
        ~canary(void) {
            (*target) << "Canary" << id << ": [Destructor]\n";
        }
        
        
        canary(const canary& o) : target(o.target), id(next_id++) {
            (*target) << "Canary" << id << ": [Copy Constructor]\n";
        }
    
        canary(canary&& o) : target(o.target), id(next_id++) {
            (*target) << "Canary" << id << ": [Move Constructor]\n";
        }
        
        
        canary& operator=(const canary& o) {
            (*target) << "Canary" << id << ": [Copy Assignment]\n";
            return *this;
        }
    
        canary& operator=(canary&& o) {
            (*target) << "Canary" << id << ": [Move Assignment]\n";
            return *this;
        }
        
    private:
        static inline u64 next_id = 0;
        
        std::ostream* target;
        u64 id;
    };
}