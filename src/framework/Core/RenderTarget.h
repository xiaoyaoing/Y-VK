#pragma once

#include "Core/Vulkan.h"
#include "Core/Images/Image.h"

#include "Core/Images/ImageView.h"

#include "RenderGraph/RenderGraphTexture.h"

struct HwTexture;

struct Attachment {
    VkFormat format{VK_FORMAT_UNDEFINED};

    VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};

    VkImageUsageFlags usage{VK_IMAGE_USAGE_SAMPLED_BIT};

    VulkanLayout initial_layout{VulkanLayout::UNDEFINED};
    VulkanLayout final_layout{VulkanLayout::UNDEFINED};

    VkAttachmentLoadOp loadOp{VK_ATTACHMENT_LOAD_OP_LOAD};

    VkAttachmentStoreOp storeOp{VK_ATTACHMENT_STORE_OP_STORE};
};

class RenderTarget {
    std::vector<SgImage*> mHwTextures;

    std::vector<Image>      _images;
    std::vector<ImageView>  _views;
    std::vector<Attachment> mAttachments;
    std::vector<uint32_t>   inAttachment  = {};
    std::vector<uint32_t>   outAttachment = {0};
    VkExtent2D              _extent;

public:
    RenderTarget(const std::vector<SgImage*> hwTextures);

    RenderTarget(const std::vector<SgImage*>& hwTextures, const std::vector<Attachment>& attachments, VkExtent2D extent);

    std::vector<VkClearValue> getDefaultClearValues() const;

    RenderTarget(std::vector<Image>&& images);

    RenderTarget(std::vector<ImageView>&& imageViews);

    const std::vector<uint32_t>& getInAttachment() const;

    void setInAttachment(const std::vector<uint32_t>& inAttachment);

    const std::vector<uint32_t>& getOutAttachment() const;

    void setOutAttachment(const std::vector<uint32_t>& outAttachment);

    const std::vector<Image>& getImages() const;

    const std::vector<ImageView>& getViews() const;

    void setLayout(uint32_t& i, VkImageLayout layout);

    VkExtent2D getExtent() const;

    const std::vector<Attachment>& getAttachments() const;

    void setAttachments(const std::vector<Attachment>& attachments);

    const std::vector<SgImage*>& getHwTextures() const;

    const ImageView& getImageView(uint32_t index) const;

    Image& getImage(uint32_t index) const;
};