#include "Vulkan.h"

#pragma once

class Device;


//Type without binding point: Input Output PushConstant SpecializationConstant

enum class ShaderResourceType
{
    Input,
    InputAttachment,
    Output,
    Image,
    ImageSampler,
    ImageStorage,
    Sampler,
    BufferUniform,
    BufferStorage,
    PushConstant,
    SpecializationConstant,
    All
};

struct ShaderResource
{
    VkShaderStageFlags stages;
    ShaderResourceType type;
    // ShaderResourceMode mode;
    uint32_t set;
    uint32_t binding;
    uint32_t location;
    uint32_t inputAttachmentIndex;
    //   uint32_t vecSize;
    uint32_t columns;
    uint32_t arraySize;
    uint32_t offset;
    uint32_t size;
    uint32_t constantId;
    uint32_t qualifiers;
    std::string name;
};


class Shader
{
public:
    enum SHADER_LOAD_MODE
    {
        SPV,
        ORIGIN_SHADER
    };

    Shader(Device& device, std::string path);


    //  Shader(Shader & other) = delete;

    ~Shader();

    VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo() const;


    const std::vector<ShaderResource>& getShaderResources() const
    {
        return resources;
    }

    size_t getId() const;

    VkShaderStageFlagBits getStage() const;

    const std::string& getEntryPoint() const;

private:
    bool initFromSpv();

    bool initFromOriginShader();

    VkShaderModule shader{VK_NULL_HANDLE};

    Device& device;

    VkShaderStageFlagBits stage;

    std::vector<ShaderResource> resources;

    //Hash result of the spv code.
    size_t id;
};
