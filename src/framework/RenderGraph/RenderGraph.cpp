#include "RenderGraph.h"
#include "Core/CommandBuffer.h"
#include <stack>

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
}

RenderGraph::RenderGraph(Device &device) : device(device) {
    mBlackBoard = std::make_unique<Blackboard>(*this);
}

RenderGraphHandle RenderGraph::Builder::readTexture(RenderGraphHandle input, RenderGraphTexture::Usage usage) {
    auto texture = renderGraph.getResource(input);
    if(usage == RenderGraphTexture::Usage::NONE)
        usage = texture->isDepthStencilTexture()?RenderGraphTexture::Usage::DEPTH_READ_ONLY:RenderGraphTexture::Usage::READ_ONLY;
     renderGraph.edges.emplace_back(Edge{.pass = node, .texture = texture, .usage = usage, .read = true});
    return input;
}

RenderGraphHandle RenderGraph::Builder::writeTexture(RenderGraphHandle output, RenderGraphTexture::Usage usage) {
    auto texture = renderGraph.getResource(output);
    if(usage == RenderGraphTexture::Usage::NONE)
        usage = texture->isDepthStencilTexture()?RenderGraphTexture::Usage::DEPTH_ATTACHMENT:RenderGraphTexture::Usage::COLOR_ATTACHMENT;
    renderGraph.edges.emplace_back(Edge{.pass = node, .texture = texture, .usage = usage, .read = false});
    return output;
}


RenderGraphHandle RenderGraph::createTexture(const char *name, const RenderGraphTexture::Descriptor &desc) {
    auto texture = new RenderGraphTexture(name, desc);
    return addTexture(texture);
}

RenderGraphHandle RenderGraph::addResource(VirtualResource *resource) {
    const RenderGraphHandle handle(mVirtualResources.size());

    auto &slot = mResourceSlots.emplace_back();
    slot.rid = mVirtualResources.size();
    mVirtualResources.push_back(resource);

    return handle;
    //return RenderGraphHandle(RenderGraphHandle());
}

std::vector<RenderGraphNode *> RenderGraph::getInComingNodes(RenderGraphNode *node) const {
    if (auto texture = dynamic_cast<RenderGraphTexture *>(node)) {
        std::vector<Edge> inComingEdges{};
        std::ranges::copy_if(edges.begin(), edges.end(),
                             std::back_insert_iterator(inComingEdges),
                             [node](const Edge &edge) { return !edge.read && edge.texture == node; });
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
                             [node](const Edge &edge) { return edge.read && edge.pass == node; });
        std::vector<RenderGraphNode *> results{};
        std::ranges::transform(inComingEdges.begin(), inComingEdges.end(), std::back_insert_iterator(results),
                               [](const Edge &edge) {
                                   return edge.texture;
                               });
        return results;
    }
    LOGE("Node Invalid")
}

std::vector<RenderGraphNode *> RenderGraph::getOutComingNodes(RenderGraphNode *node) const {
    if (auto texture = dynamic_cast<RenderGraphTexture *>(node)) {
        std::vector<Edge> outComingEdges{};
        std::ranges::copy_if(edges.begin(), edges.end(),
                             std::back_insert_iterator(outComingEdges),
                             [node](const Edge &edge) { return edge.read && edge.texture == node; });
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
                             [node](const Edge &edge) { return !edge.read && edge.pass == node; });
        std::vector<RenderGraphNode *> results{};
        std::ranges::transform(outComingEdges.begin(), outComingEdges.end(), std::back_insert_iterator(results),
                               [](const Edge &edge) {
                                   return edge.texture;
                               });
        return results;
    }
    LOGE("Node Invalid")
}

std::vector<const RenderGraph::Edge *> RenderGraph::getEdges(RenderGraphNode *node) const {
    std::vector<const Edge *> results{};

    if (auto texture = dynamic_cast<RenderGraphTexture *>(node)) {
        for (const auto &edge: edges) {
            if (edge.texture == texture)
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
    const RenderGraphHandle handle(mTextures.size());

    // auto& slot = mResourceSlots.emplace_back();
    // slot.rid = mTextures.size();

    mTextures.push_back(texture);
    return handle;
}

bool RenderGraph::isWrite(RenderGraphHandle handle, const RenderPassNode *passNode) const {
    for (const auto &edge: edges) {
        if (edge.pass == passNode && edge.texture == getResource(handle)) {
            if (!edge.read)
                return true;
        }
    }
    return false;
}

bool RenderGraph::isRead(RenderGraphHandle handle, const RenderPassNode *passNode) const {
    for (const auto &edge: edges) {
        if (edge.pass == passNode && edge.texture == getResource(handle)) {
            if (edge.read)
                return true;
        }
    }
    return false;
}


void RenderGraph::setUp() {
}


RenderGraphTexture *RenderGraph::getResource(RenderGraphHandle handle) const {
    return mTextures[handle.index];
}

void RenderGraph::compile() {
    //first cull Graph

    for (auto edge: edges) {
        if (edge.read)
            edge.texture->refCount++;
        else
            edge.pass->refCount++;
    }

    std::stack<RenderGraphNode *> stack;
    for (const auto &node: mTextures)
        if (node->refCount == 0)
            stack.push(node);
    for (const auto &node: mPassNodes)
        if (node->refCount == 0)
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
                                                [](const auto &passNode) { return passNode->refCount != 0; });

    auto first = mPassNodes.begin();
    const auto last = mActivePassNodesEnd;
    while (first != last) {
        PassNode *const passNode = *first;
        first++;
        auto inTextures = getInComingNodes(passNode);
        auto outTextures = getOutComingNodes(passNode);


        for (const auto inTexture: inTextures) {
            const auto texture = dynamic_cast<RenderGraphTexture *>(inTexture);
            assert(texture);
            texture->first = texture->first ? texture->first : passNode;
            texture->last = passNode;
            //   passNode->addTextureUsage(static_cast<const RenderGraphTexture*>(inTexture), );
        }
        for (const auto outTexture: outTextures) {
            const auto texture = dynamic_cast<RenderGraphTexture *>(outTexture);
            assert(texture);
            texture->first = texture->first ? texture->first : passNode;
            texture->last = passNode;
            //passNode->addTextureUsage(static_cast<const RenderGraphTexture*>(inTexture), texture->usage);
        }

        for (const auto edge: getEdges(passNode)) {
            passNode->addTextureUsage(edge->texture, edge->usage);
        }
    }

    for (const auto &texture: mTextures) {
        if (texture->refCount != 0) {
            texture->first->devirtualize.push_back(texture);
            texture->last->destroy.push_back(texture);
        }
    }

    for (const auto &edge: edges) {
        edge.texture->usage = edge.texture->usage | edge.usage;
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

        for (const auto &texture: pass->devirtualize) {
            texture->devirtualize();
            //texture->resolveTextureUsage(commandBuffer);
        }
        pass->resolveTextureUsages(*this, commandBuffer);
        pass->execute(*this, commandBuffer);

        for (const auto &texture: pass->destroy)
            texture->destroy();
    }
}

Blackboard &RenderGraph::getBlackBoard() const {
    return *mBlackBoard;
}


RenderGraphHandle RenderGraph::importTexture(const char *name, SgImage *hwTexture) {
    auto texture = new RenderGraphTexture(name, hwTexture);
    // VirtualResource* resource = new ImportedResource<RenderGraphTexture>(name, texture);
    // return RenderGraphHandle(addResource(resource));
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
