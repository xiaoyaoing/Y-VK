#include "SpirvShaderReflection.h"

//first just handle pipeline layout 相关 
template <spv::Decoration T>
inline void readResourceDecoration(const spirv_cross::Compiler& /*compiler*/,
                                   const spirv_cross::Resource& /*resource*/,
                                   ShaderResource& /*shaderResource*/)
{
    LOGE("Not implemented! Read resources decoration of type.");
}

static VkShaderStageFlagBits getShaderStage(spv::ExecutionModel executionModel) {
    switch (executionModel) {
    case spv::ExecutionModelVertex:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case spv::ExecutionModelFragment:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case spv::ExecutionModelGLCompute:
        return VK_SHADER_STAGE_COMPUTE_BIT;
    case spv::ExecutionModelTaskNV:
        return VK_SHADER_STAGE_TASK_BIT_NV;
    case spv::ExecutionModelMeshNV:
        return VK_SHADER_STAGE_MESH_BIT_NV;
    case spv::ExecutionModelRayGenerationKHR:
        return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    case spv::ExecutionModelIntersectionKHR:
        return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    case spv::ExecutionModelAnyHitKHR:
        return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    case spv::ExecutionModelClosestHitKHR:
        return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    case spv::ExecutionModelMissKHR:
        return VK_SHADER_STAGE_MISS_BIT_KHR;
    default:
        assert(!"Unsupported execution model");
        return VkShaderStageFlagBits(0);
    }
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
inline void readResourceDecoration<spv::DecorationNonWritable>(const spirv_cross::Compiler &compiler,
                                                                 const spirv_cross::Resource &resource,
                                                                 ShaderResource &             shader_resource)
{
    shader_resource.qualifiers |= ShaderResourceQualifiers::NonWritable;
}

template <>
inline void readResourceDecoration<spv::DecorationNonReadable>(const spirv_cross::Compiler &compiler,
                                                                 const spirv_cross::Resource &resource,
                                                                 ShaderResource &             shader_resource)
{
    shader_resource.qualifiers |= ShaderResourceQualifiers::NonReadable;
}


template <>
inline void readResourceDecoration<spv::DecorationDescriptorSet>(const spirv_cross::Compiler& compiler,
                                                                 const spirv_cross::Resource& resource,
                                                                 ShaderResource& shaderResource)
{
    shaderResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
}


template <>
inline void readResourceDecoration<spv::DecorationInputAttachmentIndex>(const spirv_cross::Compiler& compiler,
                                                                        const spirv_cross::Resource& resource,
                                                                        ShaderResource& shaderResource)
{
    shaderResource.inputAttachmentIndex = compiler.get_decoration(resource.id, spv::DecorationInputAttachmentIndex);
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

inline void readResourceArraySize(const spirv_cross::Compiler& compiler,
                                    const spirv_cross::Resource& resource,
                                    ShaderResource& shaderResource)
{
    const auto& sprivType = compiler.get_type_from_variable(resource.id);

    shaderResource.arraySize = sprivType.array.size() ? sprivType.array[0] : 1;
}

inline void readResourceSize(const spirv_cross::Compiler& compiler,
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
        readResourceArraySize(compiler, spirvResource, resource);
        readResourceDecoration<spv::DecorationBinding>(compiler,spirvResource,resource);
        readResourceDecoration<spv::DecorationLocation>(compiler, spirvResource, resource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, spirvResource, resource);
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

        readResourceArraySize(compiler, spirvResource, resource);
        readResourceSize(compiler, spirvResource, resource);
        readResourceDecoration<spv::DecorationLocation>(compiler, spirvResource, resource);
        readResourceDecoration<spv::DecorationBinding>(compiler, spirvResource, resource);
        resources.emplace_back(std::move(resource));
    }
}

inline void readShaderResource(spirv_cross::CompilerReflection & compiler,VkShaderStageFlagBits stage,std::vector<ShaderResource>& resources,const spirv_cross::SmallVector<spirv_cross::Resource> & inputResources
    ,ShaderResourceType type)
{
    for (auto& spirvResource : inputResources)
    {
        ShaderResource resource{};
        //resource.type =
        resource.type = type;
        resource.stages = stage;
        resource.name = spirvResource.name;

        readResourceArraySize(compiler, spirvResource, resource);
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
        readResourceArraySize(compiler, spirvResource, resource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, spirvResource, resource);
        //   readResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, shaderResource);

        resources.emplace_back(std::move(resource));
    }
}


inline void readShaderResourceInputAttachment(const spirv_cross::CompilerReflection& compiler,
                                              VkShaderStageFlagBits stage,
                                              std::vector<ShaderResource>& resources)
{
    auto subpass_resources = compiler.get_shader_resources().subpass_inputs;

    for (auto& resource : subpass_resources)
    {
        ShaderResource shader_resource{};
        shader_resource.type = ShaderResourceType::InputAttachment;
        shader_resource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_resource.name = resource.name;

        readResourceArraySize(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationInputAttachmentIndex>(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationBinding>(compiler, resource, shader_resource);

        resources.push_back(shader_resource);
    }
}


inline void readShaderResourceInputResources(const spirv_cross::CompilerReflection& compiler,
                                             VkShaderStageFlagBits stage,
                                             std::vector<ShaderResource>& resources)
{
    auto subpass_resources = compiler.get_shader_resources().stage_inputs;

    for (auto& resource : subpass_resources)
    {
        ShaderResource shader_resource{};
        shader_resource.type = ShaderResourceType::Input;
        shader_resource.stages = VK_SHADER_STAGE_VERTEX_BIT;
        shader_resource.name = resource.name;

        readResourceArraySize(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationBinding>(compiler, resource, shader_resource);

        resources.push_back(shader_resource);
    }
}


inline void readShaderResourcePushConstants(const spirv_cross::CompilerReflection& compiler,
                                            VkShaderStageFlagBits stage,
                                            std::vector<ShaderResource>& resources)
{
    auto shader_resources = compiler.get_shader_resources();

    for (auto& resource : shader_resources.push_constant_buffers)
    {
        const auto& spivr_type = compiler.get_type_from_variable(resource.id);

        std::uint32_t offset = std::numeric_limits<std::uint32_t>::max();

        for (auto i = 0U; i < spivr_type.member_types.size(); ++i)
        {
            auto mem_offset = compiler.get_member_decoration(spivr_type.self, i, spv::DecorationOffset);

            offset = std::min(offset, mem_offset);
        }
        ShaderResource shader_resource{};
        shader_resource.type = ShaderResourceType::PushConstant;
        shader_resource.stages = stage;
        shader_resource.name = resource.name;
        shader_resource.offset = offset;

        readResourceSize(compiler, resource, shader_resource);

        shader_resource.size -= shader_resource.offset;

        resources.push_back(shader_resource);
    }
}


inline void readShaderResourceAccelerationStructure(const spirv_cross::CompilerReflection& compiler,
                                            VkShaderStageFlagBits stage,
                                            std::vector<ShaderResource>& resources){
    const auto &  acceleration_resources = compiler.get_shader_resources().acceleration_structures;

    for (auto& resource : acceleration_resources)
    {
        ShaderResource shader_resource{};
        shader_resource.type = ShaderResourceType::AccelerationStructure;
        shader_resource.stages = stage;
        shader_resource.name = resource.name;

        readResourceArraySize(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationBinding>(compiler, resource, shader_resource);
    
        resources.push_back(shader_resource);
    }
}

inline void readShaderResourceOutput(const spirv_cross::CompilerReflection& compiler,
                                            VkShaderStageFlagBits stage,
                                            std::vector<ShaderResource>& resources)
{
    const auto & outputResources = compiler.get_shader_resources().stage_outputs;
    for (auto & spirvResource : outputResources)
    {
        ShaderResource resource{};
        resource.type = ShaderResourceType::Output;
        resource.stages = stage;
        resource.name = spirvResource.name;
        readResourceArraySize(compiler, spirvResource, resource);
        readResourceDecoration<spv::DecorationLocation>(compiler, spirvResource, resource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, spirvResource, resource);
        readResourceDecoration<spv::DecorationBinding>(compiler, spirvResource, resource);
        resources.emplace_back(std::move(resource));
    }
}
inline void readShaderResourceImage(const spirv_cross::CompilerReflection& compiler,
                                            VkShaderStageFlagBits stage,
                                            std::vector<ShaderResource>& resources)
{
    auto image_resources = compiler.get_shader_resources().separate_images;

    for (auto &resource : image_resources)
    {
        ShaderResource shader_resource{};
        shader_resource.type   = ShaderResourceType::Image;
        shader_resource.stages = stage;
        shader_resource.name   = resource.name;

        readResourceArraySize(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationBinding>(compiler, resource, shader_resource);

        resources.push_back(shader_resource);
    }
}



inline void readShaderResourceImageStorge(const spirv_cross::CompilerReflection& compiler,
                                            VkShaderStageFlagBits stage,
                                            std::vector<ShaderResource>& resources)
{
    auto storage_resources = compiler.get_shader_resources().storage_images;

    for (auto &resource : storage_resources)
    {
        ShaderResource shader_resource{};
        shader_resource.type   = ShaderResourceType::ImageStorage;
        shader_resource.stages = stage;
        shader_resource.name   = resource.name;

        readResourceArraySize(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationNonReadable>(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationNonWritable>(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationBinding>(compiler, resource, shader_resource);

        resources.push_back(shader_resource);
    }  
}
inline void readShaderResourceStorgeBuffer(const spirv_cross::CompilerReflection& compiler,
                                            VkShaderStageFlagBits stage,
                                            std::vector<ShaderResource>& resources)
{
    auto storage_resources = compiler.get_shader_resources().storage_buffers;

    for (auto &resource : storage_resources)
    {
        ShaderResource shader_resource;
        shader_resource.type   = ShaderResourceType::BufferStorage;
        shader_resource.stages = stage;
        shader_resource.name   = resource.name;

        readResourceSize(compiler, resource, shader_resource);
        readResourceArraySize(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationNonReadable>(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationNonWritable>(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationBinding>(compiler, resource, shader_resource);

        resources.push_back(shader_resource);
    } 
}

inline void readShaderResourceSampler(const spirv_cross::CompilerReflection& compiler,
                                            VkShaderStageFlagBits stage,
                                            std::vector<ShaderResource>& resources)
{
    auto sampler_resources = compiler.get_shader_resources().separate_samplers;

    for (auto &resource : sampler_resources)
    {
        ShaderResource shader_resource{};
        shader_resource.type   = ShaderResourceType::Sampler;
        shader_resource.stages = stage;
        shader_resource.name   = resource.name;

        readResourceArraySize(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, shader_resource);
        readResourceDecoration<spv::DecorationBinding>(compiler, resource, shader_resource);

        resources.push_back(shader_resource);
    }
}
bool SpirvShaderReflection::reflectShaderResources(const std::vector<uint32_t> sprivSource,
                                                   VkShaderStageFlagBits & stage,
                                                   std::vector<ShaderResource>& shaderResources)
{
    spirv_cross::CompilerReflection compiler(sprivSource);

   // stage = getShaderStage(compiler.get_execution_model());
    // readShaderResourceInput(compiler, stage, shaderResources);

    readShaderResourceInput(compiler, stage, shaderResources);
    readShaderResourceInputAttachment(compiler, stage, shaderResources);
    readShaderResourceOutput(compiler, stage, shaderResources);
    
    readShaderResourceImage(compiler, stage, shaderResources);
    readShaderResourceImageSampler(compiler, stage, shaderResources);
    readShaderResourceImageStorge(compiler, stage, shaderResources);
    readShaderResourceSampler(compiler, stage, shaderResources);
    
    readShaderResourceUniformBuffer(compiler, stage, shaderResources);
    readShaderResourceStorgeBuffer(compiler, stage, shaderResources);
    
    readShaderResourcePushConstants(compiler, stage, shaderResources);
    
    //For Ray Tracing
    readShaderResourceAccelerationStructure(compiler, stage, shaderResources);
    return true;
}
