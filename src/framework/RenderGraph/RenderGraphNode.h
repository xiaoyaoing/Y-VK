#pragma once
#include <cstdint>

class PassNode;
class CommandBuffer;

class RenderGraphNode {
public:
    uint32_t refCount{0};

    virtual ~RenderGraphNode() = default;

    virtual bool isResource() const { return false; }

    uint32_t getRefCount() const { return refCount; }
    void     addRef() { refCount++; }

    const char* getName() const { return mName; }
    void        setName(const char* name) { mName = name; }

protected:
    const char* mName;
};