#pragma once

#include <string>
#include <vector>

#include <Logger.hpp>

struct CmdLineArgs {
    const std::vector<std::string> startupScenes;
    const cpplogger::Level verbosity;
};

CmdLineArgs getCmdLineArg(const int argc, const char *argv[]);
