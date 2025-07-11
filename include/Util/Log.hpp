#pragma once
#include <format>
#include <iostream>
#include <string_view>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Util {
    class Logger {
    public:
        enum class Level {
            Debug = 0,
            Info,
            Warn,
            Error,
        };

        constexpr Logger(std::string_view scope) : scope(scope) {}

        static void SetLogLevel(Level level) {
            currentLevel = level;
        }

        static Level GetLogLevel() {
            return currentLevel;
        }

        template<typename... Args>
        void Log(Level level, std::string_view fmt_str, Args&&... args) const {
            if (level < currentLevel) return;

            std::lock_guard lock(outputMutex);

            std::string prefix = std::string(levelPrefix(level));
            std::string message = std::vformat(std::string(fmt_str), std::make_format_args(args...));
            std::string timeStr = getTimestamp();
            std::string color = levelColor(level);
            std::string resetColor = "\033[0m";

            std::cerr << color
                << "[" << timeStr << "] "
                << "[" << prefix << "] "
                << "[" << scope << "] "
                << message
                << resetColor << '\n';
        }

        template<typename... Args>
        void Info(std::string_view fmt_str, Args&&... args) const {
            Log(Level::Info, fmt_str, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Warn(std::string_view fmt_str, Args&&... args) const {
            Log(Level::Warn, fmt_str, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Error(std::string_view fmt_str, Args&&... args) const {
            Log(Level::Error, fmt_str, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Debug(std::string_view fmt_str, Args&&... args) const {
            Log(Level::Debug, fmt_str, std::forward<Args>(args)...);
        }

    private:
        std::string_view scope;
        inline static std::mutex outputMutex;
        #ifdef DEBUG
        inline static Level currentLevel = Level::Debug;
        #else
        inline static Level currentLevel = Level::Info;
        #endif

        static constexpr std::string_view levelPrefix(Level level) {
            switch (level) {
            case Level::Debug: return "DEBUG";
            case Level::Info:  return "INFO";
            case Level::Warn:  return "WARN";
            case Level::Error: return "ERROR";
            default:           return "LOG";
            }
        }

        static constexpr const char* levelColor(Level level) {
            switch (level) {
            case Level::Debug: return "\033[36m";   // Cyan
            case Level::Info:  return "\033[32m";   // Green
            case Level::Warn:  return "\033[33m";   // Yellow
            case Level::Error: return "\033[31m";   // Red
            default:           return "\033[0m";    // Reset
            }
        }

        static std::string getTimestamp() {
            using namespace std::chrono;

            auto now = system_clock::now();
            auto itt = system_clock::to_time_t(now);
            auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

            std::ostringstream ss;
            ss << std::put_time(std::localtime(&itt), "%H:%M:%S")
                << '.' << std::setfill('0') << std::setw(3) << ms.count();
            return ss.str();
        }
    };
}