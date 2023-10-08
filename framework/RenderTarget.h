#pragma once

#include <Vulkan.h>
#include "Images/Image.h"
#include "Images/ImageView.h"
#include "RenderGraph/RenderGraphTexture.h"

struct HwTexture;

struct Attachment {
    VkFormat format{VK_FORMAT_UNDEFINED};

    VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};

    VkImageUsageFlags usage{VK_IMAGE_USAGE_SAMPLED_BIT};

    VkImageLayout initial_layout{VK_IMAGE_LAYOUT_UNDEFINED};

    VkAttachmentLoadOp loadOp{VK_ATTACHMENT_LOAD_OP_CLEAR};

    VkAttachmentStoreOp storeOp{VK_ATTACHMENT_STORE_OP_STORE};

    Attachment(VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage);


    Attachment() = default;

    Attachment(VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkAttachmentLoadOp loadOp,
               VkAttachmentStoreOp storeOp) : format(format),
                                              samples(samples),
                                              usage(usage),
                                              loadOp(loadOp),
                                              storeOp(storeOp) {
    }
};

struct FrameBufferAttchment {
    // std::vector
};

// class RenderTarget
// {
//     std::vector<Image> _images;
//     std::vector<ImageView> _views;
//     std::vector<Attachment> _attachments;
// };

//struct VulkanAttachment {
//    const HwTexture *texture;
//    uint8_t level = 0;
//    uint16_t layer = 0;
//
//    ImageView &getImageView() const {
//        return *texture->view;
//    }
//
//    Image &getImage() const {
//        return *texture->image;
//    }
//};

//通过vulkanAttachments初始化attachments数组
//
class RenderTarget {
    //   std::vector<ImageView *> views;

    std::vector<sg::SgImage *> hwTextures;

    std::vector<Image> _images;
    std::vector<ImageView> _views;
    std::vector<Attachment> _attachments;
    std::vector<uint32_t> inAttachment = {};
    std::vector<uint32_t> outAttachment = {0};
    VkExtent2D _extent;

    // std::vector<VulkanAttachment> attachments;

public:
    RenderTarget(const std::vector<sg::SgImage *> hwTextures);


    using CreateFunc = std::function<std::unique_ptr<RenderTarget>(Image &&)>;

    static CreateFunc defaultRenderTargetCreateFunction;

//    explicit RenderTarget(const std::vector<VulkanAttachment> &attachments)
//            : attachments(attachments) {
//    }


    RenderTarget(std::vector<Image> &&images);

    RenderTarget(std::vector<ImageView> &&imageViews);

    const std::vector<uint32_t> &getInAttachment() const;

    void setInAttachment(const std::vector<uint32_t> &inAttachment);

    const std::vector<uint32_t> &getOutAttachment() const;

    void setOutAttachment(const std::vector<uint32_t> &outAttachment);

    const std::vector<Image> &getImages() const;

    const std::vector<ImageView> &getViews() const;

    void setLayout(uint32_t &i, VkImageLayout layout);

    VkExtent2D getExtent() const;

    const std::vector<Attachment> &getAttachments() const;

    void setAttachments(const std::vector<Attachment> &attachments);

    const std::vector<sg::SgImage *> &getHwTextures() const;
};
