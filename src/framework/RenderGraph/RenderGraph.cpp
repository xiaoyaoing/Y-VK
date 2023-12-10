#include "RenderGraph.h"
#include "Core/CommandBuffer.h"
#include <stack>

#include "Core/Pipeline.h"
#include "Core/Texture.h"

// void RenderGraph::Builder::read(VirtualResource* resource, PassNode* node)
// {
// }
//
// void RenderGraph::Builder::write(VirtualResource* resource, PassNode* node)
// {
// }

void RenderGraph::Builder::declare(const char *name, const RenderGraphPassDescriptor &desc) {
    auto rNode = static_cast<RenderPassNode *>(node);
    rNode->declareRenderPass(name, desc);

    for(const auto & subpass : desc.subpasses)
    {
        for(auto inputAtt : subpass.inputAttachments)
        {
            readTexture(inputAtt);
        }
        for(auto outputAtt : subpass.outputAttachments)
        {
            writeTexture(outputAtt);
        }
    }
}

RenderGraph::RenderGraph(Device &device) : device(device) {
    mBlackBoard = std::make_unique<Blackboard>(*this);
}

RenderGraphHandle RenderGraph::Builder::readTexture(RenderGraphHandle input, RenderGraphTexture::Usage usage) {
    auto texture = renderGraph.getTexture(input);
    if(usage == RenderGraphTexture::Usage::NONE)
        usage = texture->isDepthStencilTexture()?RenderGraphTexture::Usage::DEPTH_READ_ONLY:RenderGraphTexture::Usage::READ_ONLY;
     renderGraph.edges.emplace_back(Edge{.pass = node, .resource = texture, .usage = static_cast<uint16_t>(usage), .read = true});
    return input;
}

RenderGraphHandle RenderGraph::Builder::writeTexture(RenderGraphHandle output, RenderGraphTexture::Usage usage) {
    auto texture = renderGraph.getTexture(output);
    if(usage == RenderGraphTexture::Usage::NONE)
        usage = texture->isDepthStencilTexture()?RenderGraphTexture::Usage::DEPTH_ATTACHMENT:RenderGraphTexture::Usage::COLOR_ATTACHMENT;
    renderGraph.edges.emplace_back(Edge{.pass = node, .resource = texture, .usage = static_cast<uint16_t>(usage), .read = false});
    return output;
}

RenderGraphHandle RenderGraph::Builder::readBuffer(RenderGraphHandle input, RenderGraphBuffer::Usage usage)
{
    auto buffer = renderGraph.getBuffer(input);
    renderGraph.edges.emplace_back(Edge{.pass = node, .resource = buffer, .usage = static_cast<uint16_t>(usage), .read = true});
    return input;
}

RenderGraphHandle RenderGraph::Builder::writeBuffer(RenderGraphHandle output, RenderGraphBuffer::Usage usage)
{
    auto buffer = renderGraph.getBuffer(output);
    renderGraph.edges.emplace_back(Edge{.pass = node, .resource = buffer, .usage = static_cast<uint16_t>(usage), .read = false});
    return output;
}

RenderGraphHandle RenderGraph::createBuffer(const char* name, const RenderGraphBuffer::Descriptor& desc)
{
    auto buffer = new RenderGraphBuffer(name, desc);
    return addBuffer(buffer);
}

RenderGraphHandle RenderGraph::importBuffer(const char* name, Buffer* hwBuffer)
{
    auto buffer = new RenderGraphBuffer(name, hwBuffer);
    return addBuffer(buffer);
}

RenderGraphTexture* RenderGraph::getTexture(RenderGraphHandle handle) const
{
    auto resource = getResource(handle);
    CHECK_RESULT(resource->getType() == RENDER_GRAPH_RESOURCE_TYPE::TEXTURE);
    return static_cast<RenderGraphTexture*>(resource);
}

RenderGraphBuffer* RenderGraph::getBuffer(RenderGraphHandle handle) const
{
    auto resource = getResource(handle);
    CHECK_RESULT(resource->getType() == RENDER_GRAPH_RESOURCE_TYPE::BUFFER);
    return static_cast<RenderGraphBuffer*>(resource);
}

RenderGraphHandle RenderGraph::addBuffer(RenderGraphBuffer* buffer)
{
    const RenderGraphHandle handle(mResources.size());
    buffer->handle = handle;
    mResources.push_back(buffer);
    return handle;  
}


RenderGraphHandle RenderGraph::createTexture(const char *name, const RenderGraphTexture::Descriptor &desc) {
    auto texture = new RenderGraphTexture(name, desc);
    return addTexture(texture);
}


std::vector<RenderGraphNode *> RenderGraph::getInComingNodes(RenderGraphNode *node) const {
    if (auto resource = dynamic_cast<ResourceNode *>(node)) {
        std::vector<Edge> inComingEdges{};
        std::ranges::copy_if(edges.begin(), edges.end(),
                             std::back_insert_iterator(inComingEdges),
                             [resource](const Edge &edge) { return edge.inComing(resource); });
        std::vector<RenderGraphNode *> results{};
        std::ranges::transform(inComingEdges.begin(), inComingEdges.end(), std::back_insert_iterator(results),
                               [](const Edge &edge) {
                                   return edge.pass;
                               });
        return results;
    }
    if (auto pass = dynamic_cast<PassNode *>(node)) {
        std::vector<Edge> inComingEdges{};
        std::ranges::copy_if(edges.begin(), edges.end(),
                             std::back_insert_iterator(inComingEdges),
                             [pass](const Edge &edge) { return edge.inComing(pass); });
        std::vector<RenderGraphNode *> results{};
        std::ranges::transform(inComingEdges.begin(), inComingEdges.end(), std::back_insert_iterator(results),
                               [](const Edge &edge) {
                                   return edge.resource;
                               });
        return results;
    }
    LOGE("Node Invalid")
}

std::vector<RenderGraphNode *> RenderGraph::getOutComingNodes(RenderGraphNode *node) const {
    if (auto resource = dynamic_cast<ResourceNode *>(node)) {
        std::vector<Edge> outComingEdges{};
        std::ranges::copy_if(edges.begin(), edges.end(),
                             std::back_insert_iterator(outComingEdges),
                             [resource](const Edge &edge) { return edge.outComing(resource); });
        std::vector<RenderGraphNode *> results{};
        std::ranges::transform(outComingEdges.begin(), outComingEdges.end(), std::back_insert_iterator(results),
                               [](const Edge &edge) {
                                   return edge.pass;
                               });
        return results;
    }
    if (auto pass = dynamic_cast<PassNode *>(node)) {
        std::vector<Edge> outComingEdges{};
        std::ranges::copy_if(edges.begin(), edges.end(),
                             std::back_insert_iterator(outComingEdges),
                             [pass](const Edge &edge) { return edge.outComing(pass); });
        std::vector<RenderGraphNode *> results{};
        std::ranges::transform(outComingEdges.begin(), outComingEdges.end(), std::back_insert_iterator(results),
                               [](const Edge &edge) {
                                   return edge.resource;
                               });
        return results;
    }
    LOGE("Node Invalid")
}

std::vector<const RenderGraph::Edge *> RenderGraph::getEdges(RenderGraphNode *node) const {
    std::vector<const Edge *> results{};

    if (auto resource = dynamic_cast<ResourceNode *>(node)) {
        for (const auto &edge: edges) {
            if (edge.resource == resource)
                results.push_back(&edge);
        }
    }
    if (auto pass = dynamic_cast<PassNode *>(node)) {
        for (const auto &edge: edges) {
            if (edge.pass == pass)
                results.push_back(&edge);
        }
    }
    return results;
}

RenderGraphHandle RenderGraph::addTexture(RenderGraphTexture *texture) {
    const RenderGraphHandle handle(mResources.size());
    texture->handle = handle;
    mResources.push_back(texture);
    return handle;
}

bool RenderGraph::isWrite(RenderGraphHandle handle, const RenderPassNode *passNode) const {
    for (const auto &edge: edges) {
        if (edge.pass == passNode && edge.resource == getResource(handle)) {
            if (!edge.read)
                return true;
        }
    }
    return false;
}

bool RenderGraph::isRead(RenderGraphHandle handle, const RenderPassNode *passNode) const {
    for (const auto &edge: edges) {
        if (edge.pass == passNode && edge.resource == getResource(handle)) {
            if (edge.read)
                return true;
        }
    }
    return false;
}


void RenderGraph::setUp() {
}


ResourceNode *RenderGraph::getResource(RenderGraphHandle handle) const {
    return mResources[handle.index];
}

void RenderGraph::addPass(const char* name, const GraphicSetup& setup, GraphicsExecute&& execute)
{

    GraphicRenderGraphPass * pass = new GraphicRenderGraphPass(std::move(execute));
    auto node = new RenderPassNode(*this, name, pass);
    mPassNodes.emplace_back(node);
    Builder builder(node, *this);
    setup(builder,pass->getData());

}

void RenderGraph::addComputePass(const char* name, const ComputeSetUp& setup, ComputeExecute&& execute)
{
    ComputeRenderGraphPass * pass = new ComputeRenderGraphPass(std::move(execute));
    auto node = new ComputePassNode(*this, name, pass);
    mPassNodes.emplace_back(node);
    Builder builder(node, *this);
    setup(builder,pass->getData());
}

void RenderGraph::addRaytracingPass(const char* name, const RayTracingSetup& setup, RaytracingExecute&& execute)
{
    RaytracingRenderGraphPass * pass = new RaytracingRenderGraphPass(std::move(execute));
    auto node = new RayTracingPassNode(*this, name, pass);
    mPassNodes.emplace_back(node);
    Builder builder(node, *this);
    setup(builder,pass->getData());
}

void RenderGraph::addImageCopyPass(RenderGraphHandle src,RenderGraphHandle dst)
{
    if(src == dst)
        return;
    auto node = new ImageCopyPassNode(src,dst);
    mPassNodes.emplace_back(node);
    Builder builder(node, *this);
    builder.readTexture(src,RenderGraphTexture::Usage::TRANSFER_SRC);
    builder.writeTexture(dst,RenderGraphTexture::Usage::TRANSFER_DST);
}

void RenderGraph::compile() {
    //first cull Graph

    for (auto edge: edges) {
        if (edge.read)
            edge.resource->addRef();
        else
            edge.pass->addRef();
    }

    std::stack<RenderGraphNode *> stack;
    for (const auto &node: mResources)
        if (node->getRefCount() == 0)
            stack.push(node);
    for (const auto &node: mPassNodes)
        if (node->getRefCount() == 0)
            stack.push(node);
    while (!stack.empty()) {
        const auto node = stack.top();
        stack.pop();
        for (auto inComingNode: getInComingNodes(node)) {
            if (--inComingNode->refCount == 0)
                stack.push(inComingNode);
        }
    }

    mActivePassNodesEnd = std::stable_partition(this->mPassNodes.begin(), mPassNodes.end(),
                                                [](const auto &passNode) { return passNode->getRefCount() != 0; });

    auto first = mPassNodes.begin();
    const auto last = mActivePassNodesEnd;
    while (first != last) {
        PassNode *const passNode = *first;
        first++;
        auto inResources = getInComingNodes(passNode);
        auto outResources = getOutComingNodes(passNode);


        for (const auto inResource: inResources) {
            const auto resource = dynamic_cast<ResourceNode *>(inResource);
            assert(resource);
            resource->first = resource->first ? resource->first : passNode;
            resource->last = passNode;
            //   passNode->addTextureUsage(static_cast<const RenderGraphTexture*>(inResource), );
        }
        for (const auto outResource: outResources) {
            const auto resource = dynamic_cast<ResourceNode *>(outResource);
            assert(resource);
            resource->first = resource->first ? resource->first : passNode;
            resource->last = passNode;
            //passNode->addTextureUsage(static_cast<const RenderGraphTexture*>(inResource), texture->usage);
        }

        for (const auto edge: getEdges(passNode)) {
            passNode->addResourceUsage(edge->resource, edge->usage);
        }
    }

    for (const auto &resource : mResources) {
        if (resource ->getRefCount() != 0) {
            resource ->first->devirtualize.push_back(resource );
            resource ->last->destroy.push_back(resource );
        }
    }

    for (const auto &edge: edges) {
        edge.resource->resourceUsage = edge.resource->resourceUsage | edge.usage;
    }
}

Device &RenderGraph::getDevice() const {
    return device;
}

void RenderGraph::execute(CommandBuffer &commandBuffer) {
    //todo handle compile

    compile();


    auto first = mPassNodes.begin();
    while (first != mActivePassNodesEnd) {
        const auto pass = *first;
        first++;

        for (const auto &resource : pass->devirtualize) {
            resource ->devirtualize();
            //getBlackBoard().put(resource->getName(), resource->handle);
        }
        pass->resolveTextureUsages(*this, commandBuffer);
        pass->execute(*this, commandBuffer);

        for (const auto &texture: pass->destroy)
        {
            //getBlackBoard().remove(texture->getName());
            texture->destroy();
        }
    }
}

Blackboard &RenderGraph::getBlackBoard() const {
    return *mBlackBoard;
}


RenderGraphHandle RenderGraph::importTexture(const char *name, SgImage *hwTexture,bool addRef) {
    auto texture = new RenderGraphTexture(name, hwTexture);
    if(addRef) texture->addRef();
    return addTexture(texture);
}

// RenderGraphHandle RenderGraph::Builder::readTexture(RenderGraphHandle input,
//                                                     RenderGraphTexture::Usage usage)
// {
//     node->addTextureUsage(input, usage);
//     return input;
// }
//
// RenderGraphHandle RenderGraph::Builder::writeTexture(RenderGraphHandle output,
//                                                      RenderGraphTexture::Usage usage)
// {
//     node->addTextureUsage(output, usage);
//     return output;
// }
