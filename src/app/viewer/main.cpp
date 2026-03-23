#include "AppRegistry.h"
#include "App/Application.h"

#include <iostream>
#include <memory>
#include <string_view>

namespace {

void printUsage(const char* exeName) {
    std::cout << "Usage: " << exeName << " [--app <name>] [--list]\n";
    std::cout << "Default app: raytracer\n";
}

void printAppList() {
    std::cout << "Available render modules:\n";
    for (const auto& app : getAppRegistry()) {
        std::cout << "  " << app.name << " - " << app.description << '\n';
    }
}

std::string_view parseAppName(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--list") {
            printAppList();
            std::exit(0);
        }
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            printAppList();
            std::exit(0);
        }
        if (arg == "--app" && i + 1 < argc) {
            return argv[i + 1];
        }
        if (!arg.starts_with("--")) {
            return arg;
        }
    }
    return "raytracer";
}

} // namespace

int main(int argc, char** argv) {
    const std::string_view appName = parseAppName(argc, argv);
    auto                   app     = createAppByName(appName);

    if (!app) {
        std::cerr << "Unknown app: " << appName << '\n';
        printUsage(argv[0]);
        printAppList();
        return 1;
    }

    std::cout << "Launching app: " << appName << '\n';
    app->prepare();
    app->mainloop();
    return 0;
}
