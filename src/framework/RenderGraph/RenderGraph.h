#pragma once

#include "PassNode.h"
#include "VirtualResource.h"
#include "Core/RenderTarget.h"
#include "RenderGraphId.h"
#include "RenderGraphPass.h"
#include "RenderGraphTexture.h"
#include "Core/RenderPass.h"
#include "BlackBoard.h"
#include "RenderGraphBuffer.h"
/**render Graph **/
/*
 * 1.add pass 需要指明pass的输入 注册RenderGraphTexture 这一步会添加passNode 这里node会根据传入的RenderGraphPass::Descriptor创建RenderTarget
 * 2.指明pass node在execute时需要执行的东西 renderpass的创建根据node
 * 
 *
 *
 *
 */

// class RenderGraphPassBase;
// class RenderGraph;

class CommandBuffer;

class RenderGraph {
public:
    RenderGraph(Device& device);

    RenderGraph(RenderGraph& rhs) = delete;

    RenderGraph(RenderGraph&& rhs) = delete;

    class Builder {
    public:
        Builder& readTexture(RenderGraphHandle         input,
                             RenderGraphTexture::Usage usage =
                                 RenderGraphTexture::Usage::NONE);

        Builder& writeTexture(RenderGraphHandle         output,
                              RenderGraphTexture::Usage usage =
                                  RenderGraphTexture::Usage::NONE);

        Builder& readTextures(const std::vector<RenderGraphHandle>& inputs,
                              RenderGraphTexture::Usage             usage =
                                  RenderGraphTexture::Usage::NONE);

        Builder& writeTextures(const std::vector<RenderGraphHandle>& output,
                               RenderGraphTexture::Usage             usage =
                                   RenderGraphTexture::Usage::NONE);

        RenderGraphHandle readBuffer(RenderGraphHandle        input,
                                     RenderGraphBuffer::Usage usage =
                                         RenderGraphBuffer::Usage::NONE);

        RenderGraphHandle writeBuffer(RenderGraphHandle        output,
                                      RenderGraphBuffer::Usage usage =
                                          RenderGraphBuffer::Usage::NONE);

        // void addSubpass(const RenderGraphSubpassInfo&);

        void declare(const RenderGraphPassDescriptor& desc);

        Builder(PassNode* node, RenderGraph& renderGraph)
            : node(node),
              renderGraph(renderGraph) {
        }

    protected:
        PassNode*    node;
        RenderGraph& renderGraph;
    };

    RenderGraphHandle createTexture(const char* name, const RenderGraphTexture::Descriptor& desc = {});
    RenderGraphHandle importTexture(const char* name, SgImage* hwTexture, bool addRef = true);

    RenderGraphHandle createBuffer(const char* name, const RenderGraphBuffer::Descriptor& desc = {});
    RenderGraphHandle importBuffer(const char* name, Buffer* hwBuffer);

    ResourceNode*       getResource(RenderGraphHandle handle) const;
    RenderGraphTexture* getTexture(RenderGraphHandle handle) const;
    RenderGraphBuffer*  getBuffer(RenderGraphHandle handle) const;

    bool isWrite(RenderGraphHandle handle, const RenderPassNode* passNode) const;
    bool isRead(RenderGraphHandle handle, const RenderPassNode* passNode) const;

    void setUp();

    void execute(CommandBuffer& commandBuffer);

    Blackboard& getBlackBoard() const;

    using GraphicSetup    = std::function<void(Builder& builder, GraphicPassSettings&)>;
    using ComputeSetUp    = std::function<void(Builder& builder, ComputePassSettings&)>;
    using RayTracingSetup = std::function<void(Builder& builder, RaytracingPassSettings&)>;

    void addPass(const char* name, const GraphicSetup& setup, GraphicsExecute&& execute);

    // rg.addComputePass("",
    // [&](RenderGraph::Builder& builder, ComputePassSettings& settings){
    //
    // },
    // [&](RenderPassContext& context) {
    //
    // });

    void addComputePass(const char* name, const ComputeSetUp& setup, ComputeExecute&& execute);

    // rg.addRayTracingPass("",
    // [&](RenderGraph::Builder& builder, RayTracingPassSettings& settings){
    //
    // },
    // [&](RenderPassContext& context) {
    //
    // });
    void addRaytracingPass(const char* name, const RayTracingSetup& setup, RaytracingExecute&& execute);
    void addImageCopyPass(RenderGraphHandle src, RenderGraphHandle dst);

    // void /(RenderGraphHandle textureId);

    void compile();
    void clearPass();

    Device& getDevice() const;

    std::vector<const char*> getResourceNames(RENDER_GRAPH_RESOURCE_TYPE type) const;
    std::vector<const char*> getPasseNames(RENDER_GRAPH_PASS_TYPE type) const;

    bool needToCutResource(ResourceNode* resourceNode) const;

    ~RenderGraph() {
        for (const auto& passNode : mPassNodes)
            delete passNode;
    }

    bool getCutUnUsedResources() const;
    void setCutUnUsedResources(const bool cut_un_used_resources);

private:
    RenderGraphHandle addTexture(RenderGraphTexture* texture);
    RenderGraphHandle addBuffer(RenderGraphBuffer* buffer);

    std::vector<RenderGraphNode*> getInComingNodes(RenderGraphNode* node) const;
    std::vector<RenderGraphNode*> getOutComingNodes(RenderGraphNode* node) const;

    PassNode* addPassImpl(const char* name, RenderGraphPassBase* base) {
        auto node = new RenderPassNode(*this, name, base);
        mPassNodes.emplace_back(node);
        return node;
    }

    // Using union instead of directly use two type of struct may be better;
    // Using texture type and buffer type to avoid call get type function at runtime
    // Pointer cast is not safe and ugly.

    // union
    // {
    //     RenderGraphTexture *texture{nullptr};
    //     RenderGraphBuffer *buffer{nullptr};
    // };
    // union
    // {
    //     RenderGraphTexture::Usage textureUsage;
    //     RenderGraphBuffer::Usage bufferUsage;
    // };

    // struct TextureEdge {
    //     PassNode *pass{nullptr};
    //     RenderGraphTexture *texture{nullptr};
    //     RenderGraphTexture::Usage usage{};
    //     bool read{true};
    // };

    struct Edge {
        PassNode*     pass{nullptr};
        ResourceNode* resource{nullptr};
        uint16_t      usage{};
        bool          read{true};

        //For Given Node ,whether this edge is incoming or outgoing
        bool inComing(const ResourceNode* resourceNode) const { return resource == resourceNode && !read; }
        bool outComing(const ResourceNode* resourceNode) const { return resource == resourceNode && read; }
        bool inComing(const PassNode* passNode) const { return pass == passNode && read; }
        bool outComing(const PassNode* passNode) const { return pass == passNode && !read; }
    };

    std::vector<const Edge*> getEdges(RenderGraphNode* node) const;

    std::unique_ptr<Blackboard> mBlackBoard{};
    std::vector<PassNode*>      mPassNodes{};
    std::vector<ResourceNode*>  mResources{};

    std::vector<PassNode*>::iterator mActivePassNodesEnd;

    Device& device;

    std::vector<Edge> edges;

    //when an algothrim is not completed,some resource may be cutted,which is not desired for debug process
    bool cutUnUsedResources{true};

    // std::vector<std::unique_ptr<Vi>>
};