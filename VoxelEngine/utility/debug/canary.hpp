#pragma once

#include <VoxelEngine/core/core.hpp>

#include <string_view>


namespace ve::debug {
    /**
     * Utility class that logs itself being constructed/destructed/copied/moved to a provided target.
     * @tparam GetTarget A function returning a target to log to. Target must support streaming operations.
     */
    template <auto GetTarget> class canary {
    public:
        canary(void)            { log_operation("canary(void)"); }
        ~canary(void)           { log_operation("~canary(void)"); }
        canary(const canary& o) { log_operation("canary(const canary&)", &o); }
        canary(canary&& o)      { log_operation("canary(canary&&)", &o); }

        canary& operator=(const canary& o) { log_operation("canary& operator=(const canary&)", &o); return *this; }
        canary& operator=(canary&& o)      { log_operation("canary& operator=(canary&&)", &o); return *this; }


        [[nodiscard]] u64 get_id(void) const { return id; }
    private:
        static inline u64 next_id = 0;
        u64 id = next_id++;


        void log_operation(std::string_view operation, const canary* from = nullptr) {
            auto as_int = [] (const canary* ptr) { return ptr->id; };

            if (from) GetTarget() << operation << " #" << as_int(this) << " (From object #" << as_int(from) << ")\n";
            else GetTarget() << operation << " #" << as_int(this) << '\n';
        }
    };
}