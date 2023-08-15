//
// Created by pc on 2023/8/12.
//
#include <windows.h>
#include <App/Application.h>


int main() {
//    AllocConsole();
//
//    FILE *consoleStream;
//    if (freopen_s(&consoleStream, "CONOUT$", "w", stdout) != 0) {
//        // Handle error
//        return 1;
//    }

    Application app("Drawing A triangle", 1024, 1024);
    app.prepare();
    app.mainloop();
}