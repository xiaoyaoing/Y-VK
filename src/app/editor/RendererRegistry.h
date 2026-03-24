#pragma once

#include "Renderer.h"

#include <functional>
#include <memory>
#include <string_view>
#include <vector>

class EditorApplication;

struct RendererDescriptor {
    std::string_view name;
    std::string_view description;
    std::function<void(EditorApplication&)> configureEditor;
    std::function<std::unique_ptr<EditorRenderer>()> create;
};

const std::vector<RendererDescriptor>& getRendererRegistry();
