#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/preprocessor/shader_preprocessor.hpp>


namespace ve::gfx {
    namespace utility {
        // Helper function for finding custom directives in source files.
        struct directive_token { std::string_view key, args; };
        extern std::optional<directive_token> find_directive(std::string_view line);
    }


    // Preprocessor to add simple custom preprocessor directives through
    class directive_preprocessor : public shader_preprocessor {
    public:
        class directive_state_machine {
        public:
            virtual ~directive_state_machine(void) = default;

            // Returns true if the current line should start the machine.
            virtual bool start(std::string_view) = 0;
            // Returns true if the current line should stop the machine.
            virtual bool stop(std::string_view) = 0;
            // Invoked for every line in the source file while the machine is started.
            // Note: progress() is invoked before stop(), so both the start and end lines are also fed to this function.
            virtual void progress(std::string&, arbitrary_storage&) = 0;
            // Invoked after a source file is fully parsed.
            virtual void reset(void) {}
            // Can be used to compare state machines to reduce recompilations. Default is to hash the object address.
            virtual std::size_t hash(void) const { return hash_of(this); }

        protected:
            // Returns the state the source file was in before any state machines in the current preprocessor were invoked.
            const std::string& get_original_source(void) { return *original_source;  }
            // Returns the currently preprocessed state (by the current preprocessor) of the source file.
            // Unprocessed content is missing from the end of the string.
            std::string& get_current_source(void)  { return *parsed_source; }

        private:
            friend class directive_preprocessor;

            enum { STARTED, STOPPED } state = STOPPED;

            const std::string* original_source;
            std::string* parsed_source;
        };


        template <typename Start, typename Stop, typename Progress>
        struct lambda_state_machine : directive_state_machine {
            Start    on_start;
            Stop     on_stop;
            Progress on_progress;

            lambda_state_machine(Start on_start, Stop on_stop, Progress on_progress, std::size_t passes = 1)
                : on_start(std::move(on_start)), on_stop(std::move(on_stop)), on_progress(std::move(on_progress))
            {}

            virtual bool start(std::string_view s) override { return std::invoke(on_start, s); }
            virtual bool stop (std::string_view s) override { return std::invoke(on_stop, s);  }
            virtual void progress(std::string& s, arbitrary_storage& c) override { std::invoke(on_progress, s, c); }
        };


        explicit directive_preprocessor(std::string name, u16 priority = priority::NORMAL);
        void operator()(std::string& src, arbitrary_storage& context) const override;
        std::size_t hash(void) const override;


        void add_directive(std::string key, std::function<std::string(std::string)> action);


        template <typename Machine> requires std::is_base_of_v<directive_state_machine, Machine>
        void add_directive(Machine m) {
            directives.emplace_back(make_unique<Machine>(std::move(m)));
        }


        VE_GET_CREF(directives);
    private:
        mutable std::vector<unique<directive_state_machine>> directives;
    };
}