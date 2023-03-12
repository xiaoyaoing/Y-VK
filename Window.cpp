#include "Window.h"
#include <GLFW/glfw3.h>
VkSurfaceKHR Window::createSurface(VkInstance instance) {
    VkSurfaceKHR surface;
    glfwCreateWindowSurface(instance, _window, nullptr, &surface);
    return surface;
}

Window::Window(GLFWwindow *window):_window(window) {

}
//
// Created by 打工人 on 2023/3/11.
//

#include "Window.h"
