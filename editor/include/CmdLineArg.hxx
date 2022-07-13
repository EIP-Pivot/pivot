#pragma once

#include <optional>
#include <string>

#include <Logger.hpp>

struct CmdLineArgs {
    const std::optional<const std::string> startupScene;
    const cpplogger::Level verbosity;
};

CmdLineArgs getCmdLineArg(const int argc, const char *argv[]);
