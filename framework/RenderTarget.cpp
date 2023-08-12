//
// Created by 打工人 on 2023/3/28.
//

#include "RenderTarget.h"

const std::vector<uint32_t> &RenderTarget::getInAttachment() const {
    return inAttachment;
}

void RenderTarget::setInAttachment(const std::vector<uint32_t> &inAttachment) {
    RenderTarget::inAttachment = inAttachment;
}

const std::vector<uint32_t> &RenderTarget::getOutAttachment() const {
    return outAttachment;
}

void RenderTarget::setOutAttachment(const std::vector<uint32_t> &outAttachment) {
    RenderTarget::outAttachment = outAttachment;
}

RenderTarget::RenderTarget(std::vector<Image> &&images) : _images(std::move(images)) {
    _extent = VkExtent2D{_images.back().getExtent().width, _images.back().getExtent().height};
    for (auto &image: _images) {
        _views.emplace_back(image, VK_IMAGE_VIEW_TYPE_2D);
        _attachments.emplace_back(image.getFormat(), image.getSampleCount(), image.getUseFlags());
    }
}

RenderTarget::RenderTarget(std::vector<ImageView> &&imageViews) : _views(std::move(imageViews)) {

}

const std::vector<Image> &RenderTarget::getImages() const {
    return _images;
}

const std::vector<ImageView> &RenderTarget::getViews() const {
    return _views;
}

void RenderTarget::setLayout(uint32_t &i, VkImageLayout layout) {

}

VkExtent2D RenderTarget::getExtent() const {
    return _extent;
}

RenderTarget::CreateFunc RenderTarget::defaultRenderTargetCreateFunction = [](
        Image &&swapChainImage) -> std::unique_ptr<RenderTarget> {
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    Image depthImage(swapChainImage.getDevice(), swapChainImage.getExtent(), depthFormat,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                     VMA_MEMORY_USAGE_GPU_ONLY);
    std::vector<Image> images;
    images.push_back(std::move(swapChainImage));
    images.push_back(std::move(depthImage));
    return std::make_unique<RenderTarget>(std::move(images));

};

const std::vector<Attachment> &RenderTarget::getAttachments() const {
    return _attachments;
}

void RenderTarget::setAttachments(const std::vector<Attachment> &attachments) {
    _attachments = attachments;
}
