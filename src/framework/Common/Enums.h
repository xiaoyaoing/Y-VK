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

    STORAGE,
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

    TRANSFER_SRC= 0x100,
    
    TRANSFER_DST= 0x200,

    STORAGE  = 0x400,
    
    //!< Texture can be used as a subpass input
    DEFAULT = UPLOADABLE | SAMPLEABLE //!< Default texture usage
};

enum class BufferUsage : uint16_t
{
    NONE = 0x0,
    UPLOADABLE = 0x1,
    //!< Data can be uploaded into this buffer (default)
    INDEX = 0x2,
    //!< Buffer can be used as an index buffer
    VERTEX = 0x4,
    //!< Buffer can be used as a vertex buffer
    READ = 0x8,
    //!< Buffer can be used as a constant buffer
    INDIRECT = 0x10,
    //!< Buffer can be used as an indirect buffer
    STORAGE = 0x20,
    //!< Buffer can be used as a storage buffer
    TRANSFER_SRC = 0x40,
    //!< Buffer can be used as a transfer source
    TRANSFER_DST = 0x80,
    //!< Buffer can be used as a transfer destination
    UNIFORM_TEXEL = 0x100,
    //!< Buffer can be used as a uniform texel buffer
    STORAGE_TEXEL = 0x200,
    //!< Buffer can be used as a storage texel buffer
    RAY_TRACING = 0x400,
    //!< Buffer can be used as a ray tracing buffer
    DEFAULT = UPLOADABLE //!< Default buffer usage
};


enum class RenderPassType : uint8_t {
    UNDEFINED  = 0,
    GRAPHICS   = 1,
    COMPUTE    = 1 << 1,
    RAYTRACING = 1 << 2,
    ALL        = GRAPHICS | COMPUTE | RAYTRACING,
};

enum class RenderResourceType : uint8_t {
    UNDEFINED = 0,
    ETexture  = 1,
    EBuffer   = 1 << 1,
    ALL       = ETexture | EBuffer,
};

inline TextureUsage operator |(TextureUsage lhs, TextureUsage rhs)
{
    return static_cast<TextureUsage>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs));
}


inline TextureUsage operator &(TextureUsage lhs, TextureUsage rhs)
{
    return static_cast<TextureUsage>(static_cast<uint16_t>(lhs) & static_cast<uint16_t>(rhs));
}

inline bool any(TextureUsage usage)
{
    return static_cast<uint16_t>(usage) != 0;
}




inline BufferUsage operator |(BufferUsage lhs, BufferUsage rhs)
{
    return static_cast<BufferUsage>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs));
}


inline BufferUsage operator &(BufferUsage lhs, BufferUsage rhs)
{
    return static_cast<BufferUsage>(static_cast<uint16_t>(lhs) & static_cast<uint16_t>(rhs));
}

inline bool any(BufferUsage usage)
{
    return static_cast<uint16_t>(usage) != 0;
}


inline RenderPassType operator |(RenderPassType lhs, RenderPassType rhs)
{
    return static_cast<RenderPassType>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}


inline RenderPassType operator &(RenderPassType lhs, RenderPassType rhs)
{
    return static_cast<RenderPassType>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}

inline bool any(RenderPassType usage)
{
    return static_cast<uint8_t>(usage) != 0;
}

inline RenderResourceType operator |(RenderResourceType lhs, RenderResourceType rhs)
{
    return static_cast<RenderResourceType>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

inline RenderResourceType operator &(RenderResourceType lhs, RenderResourceType rhs)
{
    return static_cast<RenderResourceType>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}

inline bool any(RenderResourceType usage)
{
    return static_cast<uint8_t>(usage) != 0;
}
