#include "Vulkan.h"
#include "Instance.h"
#include <GLFW/glfw3.h>

#pragma once

class Window {
public:
    inline GLFWwindow *getHandle() {
        return _window;;
    }

    VkSurfaceKHR createSurface(Instance &instance);

    Window(GLFWwindow *window);

protected:
    GLFWwindow *_window;
};
