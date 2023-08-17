//
// Created by pc on 2023/8/12.
//
#include <windows.h>
#include <App/Application.h>
#include <imgui.h>

int main() {
    Application app("Drawing A triangle", 1024, 1024);
    app.prepare();
    app.mainloop();
}