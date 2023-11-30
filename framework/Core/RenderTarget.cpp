//
// Created by 打工人 on 2023/3/28.
//

#include "RenderTarget.h"
#include "Core/Images/Image.h"
#include "Core/Images/ImageView.h"

const std::vector<uint32_t>& RenderTarget::getInAttachment() const
{
    return inAttachment;
}

void RenderTarget::setInAttachment(const std::vector<uint32_t>& inAttachment)
{
    RenderTarget::inAttachment = inAttachment;
}

const std::vector<uint32_t>& RenderTarget::getOutAttachment() const
{
    return outAttachment;
}

void RenderTarget::setOutAttachment(const std::vector<uint32_t>& outAttachment)
{
    RenderTarget::outAttachment = outAttachment;
}

RenderTarget::RenderTarget(const std::vector<SgImage*> hwTextures) : mHwTextures(hwTextures)
{
    _extent = VkExtent2D{hwTextures.back()->getExtent().width, hwTextures.back()->getExtent().height};

    for (auto& hwTexture : hwTextures)
    {
        // ImageView view(image, VK_IMAGE_VIEW_TYPE_2D);
        // _views.emplace_back(image, VK_IMAGE_VIEW_TYPE_2D);
        //todo handle
        mAttachments.emplace_back(hwTexture->getFormat(), hwTexture->getVkImage().getSampleCount(),
                                  hwTexture->getVkImage().getUseFlags());
    }
}

RenderTarget::RenderTarget(const std::vector<SgImage*>& hwTextures, const std::vector<Attachment>& attachments) :
    mHwTextures(hwTextures), mAttachments(attachments), _extent(hwTextures.back()->getExtent2D())
{
}

RenderTarget::RenderTarget(std::vector<Image>&& images) : _images(std::move(images))
{
    _extent = VkExtent2D{_images.back().getExtent().width, _images.back().getExtent().height};
    for (auto& image : _images)
    {
        ImageView view(image, VK_IMAGE_VIEW_TYPE_2D);
        _views.emplace_back(image, VK_IMAGE_VIEW_TYPE_2D);
        mAttachments.emplace_back(image.getFormat(), image.getSampleCount(), image.getUseFlags());
    }
}

RenderTarget::RenderTarget(std::vector<ImageView>&& imageViews) : _views(std::move(imageViews))
{
}

const std::vector<Image>& RenderTarget::getImages() const
{
    return _images;
}

const std::vector<ImageView>& RenderTarget::getViews() const
{
    return _views;
}

void RenderTarget::setLayout(uint32_t& i, VkImageLayout layout)
{
}

VkExtent2D RenderTarget::getExtent() const
{
    return _extent;
}

// RenderTarget::CreateFunc RenderTarget::defaultRenderTargetCreateFunction = [](
//     Image&& swapChainImage) -> std::unique_ptr<RenderTarget>
// {
//     VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
//     Image depthImage(swapChainImage.getDevice(), swapChainImage.getExtent(), depthFormat,
//                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
//                      VMA_MEMORY_USAGE_GPU_ONLY);
//     std::vector<Image> images;
//     images.push_back(std::move(swapChainImage));
//     images.push_back(std::move(depthImage));
//     return std::make_unique<RenderTarget>(std::move(images));
// };

const std::vector<Attachment>& RenderTarget::getAttachments() const
{
    return mAttachments;
}

void RenderTarget::setAttachments(const std::vector<Attachment>& attachments)
{
    mAttachments = attachments;
}

const std::vector<SgImage*>& RenderTarget::getHwTextures() const
{
    return mHwTextures;
}

const ImageView& RenderTarget::getImageView(uint32_t index) const
{
    return mHwTextures[index]->getVkImageView();
}

const Image& RenderTarget::getImage(uint32_t index) const
{
    return mHwTextures[index]->getVkImage();
}

//
// Attachment::Attachment(VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage) :
//     format{format},
//     samples{samples},
//     usage{usage}
// {
// }

std::vector<VkClearValue> RenderTarget::getDefaultClearValues() const
{
    std::vector<VkClearValue> clearValues;
    for (int i = 0; i < mHwTextures.size(); i++)
    {
        if (isDepthOrStencilFormat(mHwTextures[i]->getFormat()))
            clearValues.emplace_back(VkClearValue{.depthStencil = {1.f}});
        else
            clearValues.emplace_back(VkClearValue{.color = {0.f, 0.f, 0.f, 0.f}});
    }
    return clearValues;
}
