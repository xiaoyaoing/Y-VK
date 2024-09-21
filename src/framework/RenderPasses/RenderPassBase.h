#pragma once
#include <string_view>
#include <unordered_map>
#include <Common/Log.h>
#include "Core/PipelineLayout.h"
#include "Core/Images/Sampler.h"
#include "Scene/SgImage.h"

class View;

class RenderGraph;
class PassBase {
public:
    virtual void render(RenderGraph& rg) = 0;
    virtual void init() {}
    virtual void updateGui() {}
    virtual void update() {}
    virtual ~PassBase() = default;
};

class RenderPtrManangr {
public:

    // RenderPtrManangr(const RenderPtrManangr&) = delete;
    // RenderPtrManangr& operator=(const RenderPtrManangr&) = delete;
    // RenderPtrManangr(RenderPtrManangr&&) = delete;
    
    template<typename T>
    T* fetchPtr(const std::string_view name) {
        void * ptr = mPointersMap[name];
        if (ptr == nullptr) {
            LOGE("Failed to fetch pointer with name: {0}", name);
        }
        return static_cast<T*>(ptr);
    }

    template<typename T>
    void putPtr(const std::string_view name, T* ptr) {
        mPointersMap[name] = ptr;
    }

    View * getView();

    static void Initalize();
    static void destroy();

protected:
    std::unordered_map<std::string_view, void*> mPointersMap;
};

inline RenderPtrManangr* g_manager = nullptr;
