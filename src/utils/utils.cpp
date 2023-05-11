#include "utils.hpp"

void setup_logger() {
    try {
        // Customize msg format for all loggers
        spdlog::set_pattern("[%D %H:%M:%S] [%^%L%$] [thread %t] %v");
        // Set global log level to info
        spdlog::set_level(spdlog::level::debug);

        // Flush all *registered* loggers using a worker thread every 3 seconds.
        // note: registered loggers *must* be thread safe for this to work correctly!
        spdlog::flush_every(std::chrono::seconds(3));
    }
        // Exceptions will only be thrown upon failed logger or sink construction (not during logging).
    catch (const spdlog::spdlog_ex &ex) {
        std::printf("Log initialization failed: %s\n", ex.what());
        exit(1);
    }
}

void cleanup_logger() {
    // Apply some function on all registered loggers
    spdlog::apply_all([&](std::shared_ptr<spdlog::logger> l) { l->debug("App exit"); });
    // Release all spdlog resources, and drop all loggers in the registry
    spdlog::shutdown();
}
