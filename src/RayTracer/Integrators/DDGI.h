#pragma once
#include "RenderGraph/RenderGraph.h"

#include <variant>

class IBLRenderer{};
class RayTracerRenderer{};
class DeferredRenderer{};
class DDGIRenderer{};

using Renderer = std::variant<IBLRenderer, RayTracerRenderer, DeferredRenderer, DDGIRenderer>;

class DDGI {
    void render(RenderGraph& graph);
    DDGI();
protected:
    class Impl;
    Impl* impl;
};