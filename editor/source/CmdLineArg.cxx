#include "CmdLineArg.hxx"

#include <argparse/argparse.hpp>

CmdLineArgs getCmdLineArg(const int argc, const char *argv[])
{
    int verbosity = 0;
    argparse::ArgumentParser parser("editor", "1.0", argparse::default_arguments::help);
    parser.add_argument("-s", "--scene").help("Path to the scenes that will be loaded on startup.").append();
    parser.add_argument("-a", "--asset").help("Path to the assets that will be loaded on startup.").append();
    parser.add_argument("-v", "--verbose")
        .action([&](const auto &) { ++verbosity; })
        .append()
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    auto loggerLevel = cpplogger::Level::Info;
    switch (verbosity) {
        case 0: loggerLevel = cpplogger::Level::Info; break;
        case 1: loggerLevel = cpplogger::Level::Debug; break;
        default: loggerLevel = cpplogger::Level::Trace; break;
    }

    return {
        .startupScenes = parser.get<std::vector<std::string>>("-s"),
        .startupAssets = parser.get<std::vector<std::string>>("-a"),
        .verbosity = loggerLevel,
    };
}
