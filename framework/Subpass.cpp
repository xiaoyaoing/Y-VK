//
// Created by pc on 2023/8/4.
//

#include "Subpass.h"

#include <RenderTarget.h>
#include <Scene.h>

void Subpass::updateRenderTargetAttachments(RenderTarget &renderTarget) {
    renderTarget.setInAttachment(inputAttachments);
    renderTarget.setOutAttachment(outputAttachments);
}

void Subpass::draw(CommandBuffer &commandBuffer) {

}

const std::string &Subpass::getDebugName() const {
    return debugName;
}

void Subpass::setDebugName(const std::string &debugName) {
    Subpass::debugName = debugName;
}

VkResolveModeFlagBits Subpass::getDepthStencilResolveMode() const {
    return depthStencilResolveMode;
}

void Subpass::setDepthStencilResolveMode(VkResolveModeFlagBits depthStencilResolveMode) {
    Subpass::depthStencilResolveMode = depthStencilResolveMode;
}

const std::vector<uint32_t> &Subpass::getInputAttachments() const {
    return inputAttachments;
}

void Subpass::setInputAttachments(const std::vector<uint32_t> &inputAttachments) {
    Subpass::inputAttachments = inputAttachments;
}

const std::vector<uint32_t> &Subpass::getOutputAttachments() const {
    return outputAttachments;
}

void Subpass::setOutputAttachments(const std::vector<uint32_t> &outputAttachments) {
    Subpass::outputAttachments = outputAttachments;
}

const std::vector<uint32_t> &Subpass::getColorResolveAttachments() const {
    return colorResolveAttachments;
}

void Subpass::setColorResolveAttachments(const std::vector<uint32_t> &colorResolveAttachments) {
    Subpass::colorResolveAttachments = colorResolveAttachments;
}

uint32_t Subpass::getDepthStencilResolveAttachment() const {
    return depthStencilResolveAttachment;
}

void Subpass::setDepthStencilResolveAttachment(uint32_t depthStencilResolveAttachment) {
    Subpass::depthStencilResolveAttachment = depthStencilResolveAttachment;
}

bool Subpass::getDisableDepthStencilAttachment() const {
    return disableDepthStencilAttachment;
}

void Subpass::setDisableDepthStencilAttachment(bool disableDepthStencilAttachment) {
    Subpass::disableDepthStencilAttachment = disableDepthStencilAttachment;
}

void GeomSubpass::draw(CommandBuffer &commandBuffer) {
    for (auto &mesh: scene.meshes) {
        mesh->bindOnly(commandBuffer);
        mesh->drawOnly(commandBuffer);
    }
}

GeomSubpass::GeomSubpass(Scene &scene) : scene(scene) {

}
