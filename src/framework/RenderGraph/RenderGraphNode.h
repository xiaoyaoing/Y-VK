#pragma once
#include <cstdint>
#include <string>
class PassNode;
class CommandBuffer;

class RenderGraphNode {
public:
    uint32_t refCount{0};

    virtual ~RenderGraphNode() = default;

    virtual bool isResource() const { return false; }

    uint32_t getRefCount() const { return refCount; }
    void     addRef() { refCount++; }

    const std::string& getName() const { return mName; }
    void               setName(const std::string& name) { mName = name; }

protected:
    std::string mName;
};