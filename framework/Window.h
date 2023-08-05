#pragma once
#include "Vulkan.h"
#include "Instance.h"
#include <GLFW/glfw3.h>
//struct GLFWwindow;
class Window {
public:
    inline GLFWwindow * getHandle() {
        return _window; ;
    }
    VkSurfaceKHR createSurface(Instance  & instance);
    Window(GLFWwindow * window);
protected:
    GLFWwindow * _window;
};
