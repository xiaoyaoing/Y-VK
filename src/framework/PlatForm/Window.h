#include "Core/Vulkan.h"
#include "Core/Device/Instance.h"
#include "GLFW/glfw3.h"

#pragma once

class Application;

class Window {
public:
    struct Extent {
        uint32_t width;
        uint32_t height;
    };

    struct WindowProp {
        std::string title = "";
        Extent extent = {1280, 720};
    };

    inline GLFWwindow *getHandle() {
        return handle;
    }

    VkSurfaceKHR createSurface(Instance &instance);

    Window(const WindowProp &prop, Application *app);

    VkExtent2D getExtent() const {
        return {prop.extent.width, prop.extent.height};
    }

protected:
    WindowProp prop{};
    GLFWwindow *handle;
};
