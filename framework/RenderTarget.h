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
    std::vector<int> inAttachment = {};
    std::vector<int> outAttachment = {0};
    VkExtent2D _extent;
public:
    RenderTarget(std::vector<Image> && images);
    RenderTarget(std::vector<ImageView> &&  imageViews);
    const std::vector<int> &getInAttachment() const;

    void setInAttachment(const std::vector<int> &inAttachment);

    const std::vector<int> &getOutAttachment() const;

    void setOutAttachment(const std::vector<int> &outAttachment);

    const std::vector<Image> &getImages() const;

    const std::vector<ImageView> &getViews() const;
};

