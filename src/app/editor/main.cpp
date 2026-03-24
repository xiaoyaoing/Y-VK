#include "EditorApplication.h"

#include <iostream>
#include <string_view>

namespace {

void printUsage(const char* exeName) {
    std::cout << "Usage: " << exeName << " [--help]\n";
    std::cout << "Launches the editor and lets you switch renderers from the GUI.\n";
}

bool wantsHelp(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            return true;
        }
    }
    return false;
}

} // namespace

int main(int argc, char** argv) {
    if (wantsHelp(argc, argv)) {
        printUsage(argv[0]);
        return 0;
    }

    EditorApplication editor;
    editor.prepare();
    editor.mainloop();
    return 0;
}
