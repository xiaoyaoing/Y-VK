#pragma once
#include <string_view>
#include <unordered_map>

class RenderGraph;
class PassBase {
public:
    virtual void render(RenderGraph& rg) = 0;
    virtual void init()                  = 0;
    virtual void updateGui() {}
    virtual void update() {}
    virtual ~PassBase() = default;
};

class RenderPtrManangr {
public:
    template<typename T>
    T* fetchPtr(const std::string_view name) {
        return static_cast<T*>(mPointersMap[name]);
    }

    template<typename T>
    void putPtr(const std::string_view name, T* ptr) {
        mPointersMap[name] = ptr;
    }

    static void init();
    static void destroy();

protected:
    std::unordered_map<std::string_view, void*> mPointersMap;
};

inline RenderPtrManangr* g_manager = nullptr;
