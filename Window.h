#pragma once
#include <Vulkan.h>
//#include <glfw/glfw3.h>
struct GLFWwindow;
class Window {
public:
    inline GLFWwindow * getHandle() {
        return _window; ;
    }
    VkSurfaceKHR createSurface(VkInstance instance);
    Window(GLFWwindow * window);
protected:
    GLFWwindow * _window;
};
