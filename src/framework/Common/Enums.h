#pragma once

#include <stdint.h>

enum class VulkanLayout : uint8_t
{
    // The initial layout after the creation of the VkImage. We use this to denote the state before
    // any transition.
    UNDEFINED,
    // Fragment/vertex shader accessible layout for reading and writing.
    READ_WRITE,
    // Fragment/vertex shader accessible layout for reading only.
    READ_ONLY,
    // For the source of a copy operation.
    TRANSFER_SRC,
    // For the destination of a copy operation.
    TRANSFER_DST,
    // For using a depth texture as an attachment.
    DEPTH_ATTACHMENT,

    DEPTH_READ_ONLY,
    // For using a depth texture both as an attachment and as a sampler.
    DEPTH_SAMPLER,
    // For swapchain images that will be presented.
    PRESENT,
    // For color attachments, but also used when the image is a sampler.
    // TODO: explore separate layout policies for attachment+sampling and just attachment.
    COLOR_ATTACHMENT,
    // For color attachment MSAA resolves.
    COLOR_ATTACHMENT_RESOLVE,
};


enum class TextureUsage : uint16_t
{
    NONE = 0x0,
    COLOR_ATTACHMENT = 0x1,
    //!< Texture can be used as a color attachment
    DEPTH_ATTACHMENT = 0x2,
    //!< Texture can be used as a depth attachment
    STENCIL_ATTACHMENT = 0x4,
    //!< Texture can be used as a stencil attachment
    UPLOADABLE = 0x8,
    //!< Data can be uploaded into this texture (default)
    SAMPLEABLE = 0x10,
    //!< Texture can be sampled (default)
    SUBPASS_INPUT = 0x20,
    
    READ_ONLY = 0x40,
    
    DEPTH_READ_ONLY = 0x80,
    //!< Texture can be used as a subpass input
    DEFAULT = UPLOADABLE | SAMPLEABLE //!< Default texture usage
};

inline TextureUsage operator |(TextureUsage lhs, TextureUsage rhs)
{
    return static_cast<TextureUsage>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}


inline TextureUsage operator &(TextureUsage lhs, TextureUsage rhs)
{
    return static_cast<TextureUsage>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}

inline bool any(TextureUsage usage)
{
    return static_cast<uint8_t>(usage) != 0;
}
