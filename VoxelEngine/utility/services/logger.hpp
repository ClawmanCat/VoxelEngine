#pragma once

#include <VoxelEngine/core/core.hpp>

#include <magic_enum.hpp>

#include <utility>
#include <string>
#include <vector>
#include <ostream>
#include <fstream>
#include <mutex>


namespace ve {
    class logger {
    public:
        enum level { DEBUG, INFO, WARNING, ERROR, FATAL };


        /**
         * Construct a new logger with a name, a level and a set of output streams.
         * @param name The name of the logger. This will be shown in messages which have 'info' set to true.
         * @param logged_level The minimum level a message should have to be logged by this logger.
         * @param streams A set of ostream objects to output logged messages to.
         */
        template <typename... Streams> requires (std::is_base_of_v<std::ostream, Streams> && ...)
        logger(std::string name, level logged_level, Streams&... streams) :
            name(std::move(name)),
            logged_level(logged_level),
            targets { streams... }
        {}


        /**
         * Invokes the provided action while the logger's lock is held, e.g. to print multiple messages without interleaving.
         * @param fn The function to invoke. It should take a reference to the logger as its argument.
         * @return The value returned by fn.
         */
        template <typename Fn> requires std::is_invocable_v<std::remove_reference_t<Fn>, logger&>
        decltype(auto) atomically(Fn&& fn) {
            std::lock_guard lock { mutex };
            return std::invoke(fn, *this);
        }

        /** @copydoc atomically */
        template <typename Fn> requires std::is_invocable_v<std::remove_reference_t<Fn>, const logger&>
        decltype(auto) atomically(Fn&& fn) const {
            std::lock_guard lock { mutex };
            return std::invoke(fn, *this);
        }


        /**
         * Logs the provided object, regardless of the current logger level.
         * @param value The object to log.
         * @return The logger object.
         * */
        template <typename T> requires requires (std::ostream stream, T value) { stream << value; }
        logger& operator<<(const T& value) {
            std::lock_guard lock { mutex };
            for (auto& target : targets) target.get() << value;

            return *this;
        }


        /**
         * @copydoc operator<<
         * Overload for stream manipulators of the form std::ostream& fn(std::ostream&).
         */
        logger& operator<<(fn<std::ostream&, std::ostream&> f) {
            std::lock_guard lock { mutex };
            for (auto& target : targets) target.get() << f;

            return *this;
        }


        /**
         * Logs the given message with the given level and optionally prepends some info (logger name, message level and timestamp).
         * The message is formatted through std::format before being logged, using the provided arguments.
         * @param msg The message to log.
         * @param message_level The level to log this message at. If this is less than the logger's own level, the message is not logged.
         * @param info If true, the logger name, the message log level and a timestamp are prepended to the message before logging it.
         * @param fmt_args Arguments for std::format to format the message with before it is logged.
         */
        logger& message(std::string_view msg, level message_level, bool info, auto&&... fmt_args) {
            message_impl(std::vformat(msg, std::make_format_args(fwd(fmt_args)...)), message_level, info);
            return *this;
        }


        logger& debug  (std::string_view msg, auto&&... fmt_args) { return message(msg, level::DEBUG,   true, fwd(fmt_args)...); }
        logger& info   (std::string_view msg, auto&&... fmt_args) { return message(msg, level::INFO,    true, fwd(fmt_args)...); }
        logger& warning(std::string_view msg, auto&&... fmt_args) { return message(msg, level::WARNING, true, fwd(fmt_args)...); }
        logger& error  (std::string_view msg, auto&&... fmt_args) { return message(msg, level::ERROR,   true, fwd(fmt_args)...); }
        logger& fatal  (std::string_view msg, auto&&... fmt_args) { return message(msg, level::FATAL,   true, fwd(fmt_args)...); }
    private:
        std::string name;
        level logged_level;
        std::vector<std::reference_wrapper<std::ostream>> targets;
        std::recursive_mutex mutex;


        void message_impl(std::string_view msg, level message_level, bool info);
    };


    class engine_logger : public logger {
    public:
        VE_SERVICE(engine_logger);
        engine_logger(void);
    private:
        std::ofstream filestream;
    };


    class gfxapi_logger : public logger {
    public:
        VE_SERVICE(gfxapi_logger);
        gfxapi_logger(void);
    private:
        std::ofstream filestream;
    };
}