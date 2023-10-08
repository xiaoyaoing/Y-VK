#include "SpirvShaderReflection.h"

//first just handle pipeline layout 相关 
template <spv::Decoration T>
inline void readResourceDecoration(const spirv_cross::Compiler& /*compiler*/,
                                   const spirv_cross::Resource& /*resource*/,
                                   ShaderResource& /*shaderResource*/)
{
    LOGE("Not implemented! Read resources decoration of type.");
}

template <>
inline void readResourceDecoration<spv::DecorationBinding>(const spirv_cross::Compiler& compiler,
                                                           const spirv_cross::Resource& resource,
                                                           ShaderResource& shaderResource)
{
    shaderResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
}


template <>
inline void readResourceDecoration<spv::DecorationLocation>(const spirv_cross::Compiler& compiler,
                                                            const spirv_cross::Resource& resource,
                                                            ShaderResource& shaderResource)
{
    shaderResource.location = compiler.get_decoration(resource.id, spv::DecorationLocation);
}


template <>
inline void readResourceDecoration<spv::DecorationDescriptorSet>(const spirv_cross::Compiler& compiler,
                                                                 const spirv_cross::Resource& resource,
                                                                 ShaderResource& shaderResource)
{
    shaderResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
}


inline void read_resource_vecSize(const spirv_cross::Compiler& compiler,
                                  const spirv_cross::Resource& resource,
                                  ShaderResource& shaderResource
)
{
    const auto& sprivType = compiler.get_type_from_variable(resource.id);

    // shaderResource.vecSize = sprivType.vecsize;
    shaderResource.columns = sprivType.columns;
}

inline void read_resource_arraySize(const spirv_cross::Compiler& compiler,
                                    const spirv_cross::Resource& resource,
                                    ShaderResource& shaderResource)
{
    const auto& sprivType = compiler.get_type_from_variable(resource.id);

    shaderResource.arraySize = sprivType.array.size() ? sprivType.array[0] : 1;
}

inline void read_resource_size(const spirv_cross::Compiler& compiler,
                               const spirv_cross::Resource& resource,
                               ShaderResource& shaderResource)
{
    const auto& sprivType = compiler.get_type_from_variable(resource.id);

    shaderResource.size = toUint32(compiler.get_declared_struct_size(sprivType));
}


inline void readShaderResourceInput(const spirv_cross::CompilerReflection& compiler,
                                    VkShaderStageFlagBits stage,
                                    std::vector<ShaderResource>& resources)
{
    auto inputResources = compiler.get_shader_resources().stage_inputs;
    for (auto& spirvResource : inputResources)
    {
        ShaderResource resource{};
        resource.type = ShaderResourceType::Input;
        resource.stages = stage;
        resource.name = spirvResource.name;
        // readResourceDecoration<spv::DecorationBinding>(compiler,spirvResource,resource);
        read_resource_arraySize(compiler, spirvResource, resource);
        readResourceDecoration<spv::DecorationLocation>(compiler, spirvResource, resource);
        resources.emplace_back(std::move(resource));
    }
}

inline void readShaderResourceUniformBuffer(const spirv_cross::CompilerReflection& compiler,
                                            VkShaderStageFlagBits stage,
                                            std::vector<ShaderResource>& resources)
{
    auto inputResources = compiler.get_shader_resources().uniform_buffers;
    for (auto& spirvResource : inputResources)
    {
        ShaderResource resource{};
        resource.type = ShaderResourceType::BufferUniform;
        resource.stages = stage;
        resource.name = spirvResource.name;

        read_resource_arraySize(compiler, spirvResource, resource);
        read_resource_size(compiler, spirvResource, resource);
        readResourceDecoration<spv::DecorationLocation>(compiler, spirvResource, resource);
        readResourceDecoration<spv::DecorationBinding>(compiler, spirvResource, resource);
        resources.emplace_back(std::move(resource));
    }
}

inline void readShaderResourceImageSampler(const spirv_cross::CompilerReflection& compiler,
                                           VkShaderStageFlagBits stage,
                                           std::vector<ShaderResource>& resources)
{
    auto inputResources = compiler.get_shader_resources().sampled_images;
    for (auto& spirvResource : inputResources)
    {
        ShaderResource resource{};
        resource.type = ShaderResourceType::ImageSampler;
        resource.stages = stage;
        resource.name = spirvResource.name;

        readResourceDecoration<spv::DecorationBinding>(compiler, spirvResource, resource);
        read_resource_arraySize(compiler, spirvResource, resource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, spirvResource, resource);
        //   read_resource_decoration<spv::DecorationDescriptorSet>(compiler, resource, shaderResource, variant);

        resources.emplace_back(std::move(resource));
    }
}


inline void readShaderResourcePushConstant(const spirv_cross::CompilerReflection& compiler,
                                           VkShaderStageFlagBits stage,
                                           std::vector<ShaderResource>& resources)
{
    auto inputResources = compiler.get_shader_resources().uniform_buffers;
    for (auto& spirvResource : inputResources)
    {
        ShaderResource resource{};
        resource.type = ShaderResourceType::ImageSampler;
        resource.stages = stage;
        resource.name = spirvResource.name;

        readResourceDecoration<spv::DecorationBinding>(compiler, spirvResource, resource);
        read_resource_arraySize(compiler, spirvResource, resource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, spirvResource, resource);
        //   read_resource_decoration<spv::DecorationDescriptorSet>(compiler, resource, shaderResource, variant);

        resources.emplace_back(std::move(resource));
    }
}


bool SpirvShaderReflection::reflectShaderResources(const std::vector<uint32_t> sprivSource,
                                                   VkShaderStageFlagBits stage,
                                                   std::vector<ShaderResource>& shaderResources)
{
    spirv_cross::CompilerReflection compiler(sprivSource);

    // readShaderResourceInput(compiler, stage, shaderResources);
    //todo handle more 
    readShaderResourceUniformBuffer(compiler, stage, shaderResources);
    readShaderResourceImageSampler(compiler, stage, shaderResources);

    return true;
}
