#pragma once

#include <Vulkan.h>
#include "Images/Image.h"
#include "Images/ImageView.h"

struct Attachment {
    VkFormat format{VK_FORMAT_UNDEFINED};

    VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};

    VkImageUsageFlags usage{VK_IMAGE_USAGE_SAMPLED_BIT};

    VkImageLayout initial_layout{VK_IMAGE_LAYOUT_UNDEFINED};

    Attachment() = default;

    Attachment(VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage) : format(format),
                                                                                          samples(samples),
                                                                                          usage(usage) {}
};

class RenderTarget {
    std::vector<Image> _images;
    std::vector<ImageView> _views;
    std::vector<Attachment> _attachments;
    std::vector<uint32_t> inAttachment = {};
    std::vector<uint32_t> outAttachment = {0};
    VkExtent2D _extent;
public:
    using CreateFunc = std::function<std::unique_ptr<RenderTarget>(Image &&)>;

    static CreateFunc defaultRenderTargetCreateFunction;

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
};

