//
// Created by pc on 2023/8/27.
//
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include "gltfloader.h"

#include "Common/Timer.h"
#include "Core/Descriptor/DescriptorSet.h"
#include "Core/Buffer.h"
#include "Core/RenderContext.h"

#include <ctpl_stl.h>
#include <set>

inline std::vector<uint8_t> convertUnderlyingDataStride(const std::vector<uint8_t>& src_data, uint32_t src_stride, uint32_t dst_stride) {
    auto elem_count = toUint32(src_data.size()) / src_stride;

    std::vector<uint8_t> result(elem_count * dst_stride);

    for (uint32_t idxSrc = 0, idxDst = 0;
         idxSrc < src_data.size() && idxDst < result.size();
         idxSrc += src_stride, idxDst += dst_stride) {
        std::copy(src_data.begin() + idxSrc, src_data.begin() + idxSrc + src_stride, result.begin() + idxDst);
    }

    return result;
}

static std::unordered_map<VkFormat, uint32_t> formatStrideMap = {{VK_FORMAT_R8_UINT, 1}, {VK_FORMAT_R16_UINT, 2}, {VK_FORMAT_R32_UINT, 4}};

static std::unordered_map<VkIndexType, uint32_t> indexStrideMap = {{VK_INDEX_TYPE_UINT16, 2}, {VK_INDEX_TYPE_UINT32, 4}};

inline std::vector<uint8_t> convertIndexData(const std::vector<uint8_t>& src_data, VkIndexType targetType, VkFormat format, VkIndexType type) {
    if (targetType == VK_INDEX_TYPE_NONE_KHR) {
        switch (format) {
            case VK_FORMAT_R8_UINT: {
                type = VK_INDEX_TYPE_UINT16;
                return convertUnderlyingDataStride(src_data, 1, 2);
            }
            case VK_FORMAT_R16_UINT:
                type = VK_INDEX_TYPE_UINT16;
                return src_data;
            case VK_FORMAT_R32_UINT:
                type = VK_INDEX_TYPE_UINT32;
                return src_data;
        }
    Default:
        LOGE("Unsupported index format {} ", format)
    } else {
        type = targetType;
        return convertUnderlyingDataStride(src_data, formatStrideMap.at(format), indexStrideMap.at(targetType));
    }
}

inline VkFormat getAttributeFormat(const tinygltf::Model* model, uint32_t accessorId) {
    assert(accessorId < model->accessors.size());
    auto& accessor = model->accessors[accessorId];

    VkFormat format;

    switch (accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_BYTE: {
            static const std::map<int, VkFormat> mapped_format = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_SINT},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_SINT},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_SINT},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_SINT}};

            format = mapped_format.at(accessor.type);

            break;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
            static const std::map<int, VkFormat> mapped_format = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_UINT},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_UINT},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_UINT},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_UINT}};

            static const std::map<int, VkFormat> mapped_format_normalize = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_UNORM},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_UNORM},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_UNORM},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_UNORM}};

            if (accessor.normalized) {
                format = mapped_format_normalize.at(accessor.type);
            } else {
                format = mapped_format.at(accessor.type);
            }

            break;
        }
        case TINYGLTF_COMPONENT_TYPE_SHORT: {
            static const std::map<int, VkFormat> mapped_format = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_SINT},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_SINT},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_SINT},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_SINT}};

            format = mapped_format.at(accessor.type);

            break;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
            static const std::map<int, VkFormat> mapped_format = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R16_UINT},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R16G16_UINT},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R16G16B16_UINT},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R16G16B16A16_UINT}};

            static const std::map<int, VkFormat> mapped_format_normalize = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R16_UNORM},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R16G16_UNORM},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R16G16B16_UNORM},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R16G16B16A16_UNORM}};

            if (accessor.normalized) {
                format = mapped_format_normalize.at(accessor.type);
            } else {
                format = mapped_format.at(accessor.type);
            }

            break;
        }
        case TINYGLTF_COMPONENT_TYPE_INT: {
            static const std::map<int, VkFormat> mapped_format = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R32_SINT},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R32G32_SINT},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R32G32B32_SINT},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_SINT}};

            format = mapped_format.at(accessor.type);

            break;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
            static const std::map<int, VkFormat> mapped_format = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R32_UINT},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R32G32_UINT},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R32G32B32_UINT},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_UINT}};

            format = mapped_format.at(accessor.type);

            break;
        }
        case TINYGLTF_COMPONENT_TYPE_FLOAT: {
            static const std::map<int, VkFormat> mapped_format = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R32_SFLOAT},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R32G32_SFLOAT},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R32G32B32_SFLOAT},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_SFLOAT}};

            format = mapped_format.at(accessor.type);

            break;
        }
        default: {
            format = VK_FORMAT_UNDEFINED;
            break;
        }
    }

    return format;
};

struct Skin;

struct Mesh {
    std::string name;
    Device&     device;

    struct
    {
        std::unique_ptr<Buffer>        buffer{nullptr};
        std::unique_ptr<DescriptorSet> descriptorSet{nullptr};
    } uniformBuffer;

    struct UniformBlock {
        glm::mat4 matrix;
        glm::mat4 jointMatrix[64]{};
        float     jointcount{0};
    } uniformBlock;

    Mesh(Device& device, glm::mat4 matrix);

    std::vector<std::unique_ptr<Primitive>> primitives;
};

struct Node {
    Node*              parent;
    uint32_t           index;
    std::vector<Node*> children;
    glm::mat4          matrix;
    std::string        name;
    Mesh*              mesh;
    Skin*              skin;
    int32_t            skinIndex = -1;
    glm::vec3          translation{};
    glm::vec3          scale{1.0f};
    glm::quat          rotation{};

    glm::mat4 localMatrix();

    glm::mat4 getMatrix();

    void update();

    ~Node();
};

struct Skin {
    std::string            name;
    Node*                  skeletonRoot = nullptr;
    std::vector<glm::mat4> inverseBindMatrices;
    std::vector<Node*>     joints;
};

struct AnimationChannel {
    enum PathType {
        TRANSLATION,
        ROTATION,
        SCALE
    };

    PathType path;
    Node*    node;
    uint32_t samplerIndex;
};

/*
    glTF animation sampler
*/
struct AnimationSampler {
    enum InterpolationType {
        LINEAR,
        STEP,
        CUBICSPLINE
    };

    InterpolationType      interpolation;
    std::vector<float>     inputs;
    std::vector<glm::vec4> outputsVec4;
};

/*
glTF animation
*/
struct Animation {
    std::string                   name;
    std::vector<AnimationSampler> samplers;
    std::vector<AnimationChannel> channels;
    float                         start = std::numeric_limits<float>::max();
    float                         end   = std::numeric_limits<float>::min();
};

VkBufferUsageFlags getBufferUsageFlags(const SceneLoadingConfig& config) {
    VkBufferUsageFlags vkBufferUsageFlags{};
    if (config.bufferAddressAble) vkBufferUsageFlags = vkBufferUsageFlags | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    if (config.bufferForAccel) vkBufferUsageFlags = vkBufferUsageFlags | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    if (config.bufferForTransferSrc) vkBufferUsageFlags = vkBufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (config.bufferForTransferDst) vkBufferUsageFlags = vkBufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    return vkBufferUsageFlags;
}

struct GLTFLoadingImpl {
    std::vector<std::unique_ptr<Primitive>> primitives;
    SceneLoadingConfig                      config;

    Device& device;
    //  Queue& queue;
    //        Mesh *mesh;
    // std::vector<Node*> nodes;
    //    std::vector<Node*> linearNodes;

    std::vector<std::unique_ptr<Texture>> textures;
    std::vector<SgLight>                  lights;
    // std::vector<Material>                 materials;
    std::vector<GltfMaterial> materials;

    std::vector<std::shared_ptr<Camera>> cameras;

    bool metallicRoughnessWorkflow;

    GLTFLoadingImpl(Device& device, const std::string& path, const SceneLoadingConfig& config);
    ~GLTFLoadingImpl();

    const Texture* getTexture(uint32_t idx) const;
    void           loadImages(const std::filesystem::path& modelPath, const tinygltf::Model& model);
    void           loadNode(const tinygltf::Node& node, const tinygltf::Model& model);
    void           loadFromFile(const std::string& path);
    void           loadMaterials(tinygltf::Model& gltfModel);
    void           loadCameras(const tinygltf::Model& model);

    void      process(const tinygltf::Model& model);
    void      processNode(const tinygltf::Node& node, const tinygltf::Model& model);
    glm::mat4 getTransformMatrix(const tinygltf::Node& node);

    // std::unique_ptr<Buffer> sceneVertexBuffer{nullptr};

    //Scene attributes
    std::unordered_map<std::string, VertexAttribute>         vertexAttributes;
    std::unordered_map<std::string, std::unique_ptr<Buffer>> sceneVertexBuffer;
    std::unique_ptr<Buffer>                                  sceneIndexBuffer{nullptr};
    std::unique_ptr<Buffer>                                  sceneUniformBuffer{nullptr};
    std::unique_ptr<Buffer>                                  scenePrimitiveIdBuffer{nullptr};

    VkIndexType indexType{VK_INDEX_TYPE_UINT16};
    uint32_t    mVertexCount{0}, mIndexCount{0};

    struct Accessor {
        uint32_t vertexOffset;
        uint32_t accessor;
    };

    //Offset ——> accessor
    //accessor may repeat
    std::map<std::string, std::map<uint32_t, uint32_t>> gltfVertexAccessors;
    std::map<uint32_t, uint32_t>                        gltfIndexAccessors;
    std::vector<PerPrimitiveUniform>                    primitiveUniforms;

    std::map<std::uint32_t, std::vector<const tinygltf::Primitive*>> gltfPrimitives;
};

bool loadImageDataFunc(tinygltf::Image* image, const int imageIndex, std::string* error, std::string* warning, int req_width, int req_height, const unsigned char* bytes, int size, void* userData) {
    // KTX files will be handled by our own code
    if (image->uri.find_last_of(".") != std::string::npos) {
        if (image->uri.substr(image->uri.find_last_of(".") + 1) == "ktx") {
            return true;
        }
    }

    return LoadImageData(image, imageIndex, error, warning, req_width, req_height, bytes, size, userData);
}

void GLTFLoadingImpl::loadCameras(const tinygltf::Model& model) {
    for (auto& gltfCamera : model.cameras) {
        auto camera = std::make_shared<Camera>();
        camera->setPerspective(gltfCamera.perspective.yfov, gltfCamera.perspective.aspectRatio, gltfCamera.perspective.znear, gltfCamera.perspective.zfar);
        cameras.push_back(camera);
    }
}
void GLTFLoadingImpl::process(const tinygltf::Model& model) {

    for (auto& node : model.nodes) {
        processNode(node, model);
    }
    std::unordered_map<std::string, std::unique_ptr<Buffer>> stagingBuffers;
    for (auto attribute : vertexAttributes) {
        auto stagingBuffer = std::make_unique<Buffer>(device, attribute.second.stride * mVertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        for (auto accessor : gltfVertexAccessors.at(attribute.first)) {
            auto                        accessorId   = accessor.second;
            auto                        vertexOffset = accessor.first;
            const tinygltf::Accessor&   gltfAccessor = model.accessors[accessorId];
            const tinygltf::BufferView& view         = model.bufferViews[gltfAccessor.bufferView];
            auto                        startByte    = gltfAccessor.byteOffset + view.byteOffset;
            auto                        endByte      = startByte + gltfAccessor.ByteStride(view) * gltfAccessor.count;

            std::vector<uint8_t> data(model.buffers[view.buffer].data.begin() + startByte,
                                      model.buffers[view.buffer].data.begin() + endByte);
            stagingBuffer->uploadData(data.data(), data.size(), vertexOffset * attribute.second.stride);
        }
        sceneVertexBuffer[attribute.first] = std::move(stagingBuffer);
    }

    auto                 sceneIndexStagingBuffer = std::make_unique<Buffer>(device, mIndexCount * indexStrideMap.at(indexType), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    std::vector<uint8_t> indexData;
    for (auto accessor : gltfIndexAccessors) {
        auto                        accessorId   = accessor.second;
        auto                        indexOffset  = accessor.first;
        const tinygltf::Accessor&   gltfAccessor = model.accessors[accessorId];
        const tinygltf::BufferView& view         = model.bufferViews[gltfAccessor.bufferView];
        auto                        startByte    = gltfAccessor.byteOffset + view.byteOffset;
        auto                        endByte      = startByte + gltfAccessor.ByteStride(view) * gltfAccessor.count;
        std::vector<uint8_t>        data(model.buffers[view.buffer].data.begin() + startByte,
                                  model.buffers[view.buffer].data.begin() + endByte);
        indexData = convertIndexData(data, indexType, getAttributeFormat(&model, accessorId), indexType);
        sceneIndexStagingBuffer->uploadData(indexData.data(), indexData.size(), indexOffset * indexStrideMap.at(indexType));
    }

    std::vector<uint32_t> primitiveIdxs;
    for (size_t i = 0; i < primitives.size(); i++) {
        primitiveIdxs.push_back(i);
    }

    auto uniformStagingBuffer = std::make_unique<Buffer>(device, sizeof(PerPrimitiveUniform) * primitiveUniforms.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, primitiveUniforms.data());
    auto primitiveIdxBuffer   = std::make_unique<Buffer>(device, sizeof(uint32_t) * primitiveIdxs.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, primitiveIdxs.data());

    auto commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    auto copyBuffer = [this, &commandBuffer](const std::unique_ptr<Buffer>& srcBuffer, VkBufferUsageFlags usageFlags) {
        std::unique_ptr<Buffer> dstBuffer = std::make_unique<Buffer>(device, srcBuffer->getSize(), usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        VkBufferCopy            copy{.srcOffset = 0, .size = srcBuffer->getSize()};
        vkCmdCopyBuffer(commandBuffer.getHandle(), srcBuffer->getHandle(), dstBuffer->getHandle(), 1, &copy);
        return dstBuffer;
    };

    sceneIndexBuffer   = copyBuffer(sceneIndexStagingBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    sceneUniformBuffer = copyBuffer(uniformStagingBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    for (auto& attribute : sceneVertexBuffer) {
        sceneVertexBuffer[attribute.first] = copyBuffer(attribute.second, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }

    scenePrimitiveIdBuffer = copyBuffer(primitiveIdxBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    g_context->submit(commandBuffer);
}

glm::mat4 GLTFLoadingImpl::getTransformMatrix(const tinygltf::Node& node) {
    if (!node.matrix.empty())
        return glm::make_mat4x4(node.matrix.data());
    auto translation = glm::vec3(0.0f);
    if (node.translation.size() == 3) {
        translation = glm::make_vec3(node.translation.data());
    }
    auto rotation = glm::mat4(1.0f);
    if (node.rotation.size() == 4) {
        rotation = glm::mat4_cast(glm::make_quat(node.rotation.data()));
    }
    auto scale = glm::vec3(1.0f);
    if (node.scale.size() == 3) {
        scale = glm::make_vec3(node.scale.data());
    }
    return config.sceneTransform * glm::translate(glm::mat4(1.0f), translation) * rotation * glm::scale(glm::mat4(1.0f), scale);
}

void GLTFLoadingImpl::processNode(const tinygltf::Node& node, const tinygltf::Model& model) {
    if (node.mesh > -1) {
        const tinygltf::Mesh mesh = model.meshes[node.mesh];
        for (size_t j = 0; j < mesh.primitives.size(); j++) {
            const tinygltf::Primitive& primitive = mesh.primitives[j];
            if (primitive.indices < 0) {
                continue;
            }
            if (vertexAttributes.empty()) {
                for (const auto& attr : primitive.attributes) {
                    std::string attributeName = attr.first;
                    std::ranges::transform(attributeName.begin(), attributeName.end(), attributeName.begin(), ::tolower);
                    bool attr_need_to_load = config.requiredVertexAttribute.empty() || config.requiredVertexAttribute.contains(attributeName);
                    if (!attr_need_to_load) continue;

                    const tinygltf::Accessor&   accessor = model.accessors[attr.second];
                    const tinygltf::BufferView& view     = model.bufferViews[accessor.bufferView];
                    uint32_t                    stride   = accessor.ByteStride(view);

                    VertexAttribute attribute{};
                    attribute.format                = getAttributeFormat(&model, attr.second);
                    attribute.stride                = stride;
                    vertexAttributes[attributeName] = attribute;
                }
            }
            assert(primitive.attributes.find("POSITION") != primitive.attributes.end());
            const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find(
                                                                                            "POSITION")
                                                                        ->second];
            auto                      posMin      = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
            auto                      posMax      = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);

            uint32_t primVertexCount = posAccessor.count;
            bool     loadNewVertex   = true;
            bool     loadNewIndex    = true;

            uint32_t vertexOffset{0}, indexOffset{0};
            for (const auto& attr : primitive.attributes) {
                std::string attributeName = attr.first;
                std::ranges::transform(attributeName.begin(), attributeName.end(), attributeName.begin(), ::tolower);
                auto& vertexAccessors                = gltfVertexAccessors[attributeName];
                bool  attr_need_to_load              = config.requiredVertexAttribute.empty() || config.requiredVertexAttribute.contains(attributeName);
                vertexOffset                         = mVertexCount;
                const tinygltf::Accessor&   accessor = model.accessors[attr.second];
                const tinygltf::BufferView& view     = model.bufferViews[accessor.bufferView];
                if (attr_need_to_load) {
                    vertexAccessors[mVertexCount] = attr.second;
                }
            }

            const tinygltf::Accessor& accessor               = model.accessors[primitive.indices];
            uint32_t                  curPrimitiveIndexCount = accessor.count;

            gltfIndexAccessors[mIndexCount] = primitive.indices;
            indexOffset                     = mIndexCount;

            uint32_t curIndexCount = loadNewIndex ? curPrimitiveIndexCount : gltfIndexAccessors[primitive.indices];
            auto     newPrimitive  = std::make_unique<Primitive>(vertexOffset, indexOffset, curPrimitiveIndexCount, primitive.material);
            if (loadNewVertex) mVertexCount += primVertexCount;
            if (loadNewIndex) mIndexCount += curPrimitiveIndexCount;

            newPrimitive->setDimensions(posMin, posMax);
            auto                model = getTransformMatrix(node);
            PerPrimitiveUniform primitiveUniform{model, glm::transpose(glm::inverse(model)), static_cast<uint32_t>(primitive.material)};
            primitiveUniforms.push_back(primitiveUniform);
            primitives.push_back(std::move(newPrimitive));
            gltfPrimitives[primitive.indices].emplace_back(&primitive);
        }
    }
}

void GLTFLoadingImpl::loadFromFile(const std::string& path) {

    Timer timer;
    timer.start();

    tinygltf::Model    gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string        error, warning;
    gltfContext.SetImageLoader(loadImageDataFunc, nullptr);
    bool fileLoaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, path);

    if (!fileLoaded) {
        LOGE("Could not load glTF file {} {} \"", path, error)
    }

    std::vector<uint32_t> indexData;

    loadImages(path, gltfModel);
    loadMaterials(gltfModel);
    loadCameras(gltfModel);
    const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];

    if (config.bufferRate == BufferRate::PER_PRIMITIVE) {
        for (size_t i = 0; i < scene.nodes.size(); i++) {
            const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
            loadNode(node, gltfModel);
        }
    } else {
        process(gltfModel);
    }

    if (cameras.empty()) {
        auto camera = std::make_shared<Camera>();
        camera->setPerspective(45.0f, 1.0f, 0.1f, 100.0f);
        cameras.push_back(camera);
    }

    for (auto extension : gltfModel.extensionsUsed) {
        if (extension == "KHR_materials_pbrSpecularGlossiness") {
            metallicRoughnessWorkflow = false;
        }
    }

    LOGI("{} loaded in {} s", path, timer.elapsed<Timer::Seconds>());
}

void GLTFLoadingImpl::loadMaterials(tinygltf::Model& gltfModel) {

    materials.reserve(gltfModel.materials.size());

    for (const auto& mat : gltfModel.materials) {
        GltfMaterial material;
        material.alphaCutoff              = static_cast<float>(mat.alphaCutoff);
        material.alphaMode                = mat.alphaMode == "MASK" ? 1 : (mat.alphaMode == "BLEND" ? 2 : 0);
        material.doubleSided              = mat.doubleSided ? 1 : 0;
        material.emissiveFactor           = glm::vec3(mat.emissiveFactor[0], mat.emissiveFactor[1], mat.emissiveFactor[2]);
        material.emissiveTexture          = mat.emissiveTexture.index;
        material.normalTexture            = mat.normalTexture.index;
        material.normalTextureScale       = static_cast<float>(mat.normalTexture.scale);
        material.occlusionTexture         = mat.occlusionTexture.index;
        material.occlusionTextureStrength = static_cast<float>(mat.occlusionTexture.strength);

        auto& pbr                            = mat.pbrMetallicRoughness;
        material.pbrBaseColorFactor          = glm::vec4(pbr.baseColorFactor[0], pbr.baseColorFactor[1], pbr.baseColorFactor[2], pbr.baseColorFactor[3]);
        material.pbrBaseColorTexture         = pbr.baseColorTexture.index;
        material.pbrMetallicFactor           = static_cast<float>(pbr.metallicFactor);
        material.pbrMetallicRoughnessTexture = pbr.metallicRoughnessTexture.index;
        material.pbrRoughnessFactor          = static_cast<float>(pbr.roughnessFactor);
        materials.emplace_back(material);
    }
    //   materials.resize(gltfModel.materials.size());
}

GLTFLoadingImpl::~GLTFLoadingImpl() {
    // for (auto node : nodes)
    //     delete node;
}

void GLTFLoadingImpl::loadImages(const std::filesystem::path& modelPath, const tinygltf::Model& gltfModel) {
    auto parentDir = modelPath.parent_path();

    auto thread_count = std::thread::hardware_concurrency();

    thread_count = thread_count == 0 ? 1 : thread_count;
    ctpl::thread_pool thread_pool(thread_count);

    for (const auto& image : gltfModel.images) {
        auto fut = thread_pool.push(
            [this, parentDir, image](size_t) {
                if (image.uri.empty() && image.image.size() > 0)
                    return Texture::loadTextureFromMemory(device, image.image, VkExtent3D{static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height), 1});
                else if (!image.uri.empty())
                    return Texture::loadTextureFromFile(device, parentDir.string() + "/" + image.uri);
                else
                    LOGE("Image uri is empty and image data is empty!");
            });
        textures.emplace_back(fut.get());
    }
}

template<class T, class Y>
struct TypeCast {
    Y operator()(T value) const noexcept {
        return static_cast<Y>(value);
    }
};

void GLTFLoadingImpl::loadNode(const tinygltf::Node& node, const tinygltf::Model& model) {
    // Node with children
    if (node.children.size() > 0) {
        for (auto i = 0; i < node.children.size(); i++) {
            loadNode(model.nodes[node.children[i]], model);
        }
    }

    if (node.camera > -1 && !cameras.empty()) {
        auto& camera = cameras[node.camera];

        if (!node.translation.empty())
            camera->setTranslation(glm::make_vec3(node.translation.data()));

        //todo set rotation
        // glm::quat rotation;
        // std::ranges::transform(node.rotation.begin(),node.rotation.end(), glm::value_ptr(rotation), TypeCast<double, float>{});
    }

    // Node contains mesh data
    if (node.mesh > -1) {
        const tinygltf::Mesh mesh = model.meshes[node.mesh];
        for (size_t j = 0; j < mesh.primitives.size(); j++) {
            const tinygltf::Primitive& primitive = mesh.primitives[j];
            if (primitive.indices < 0) {
                continue;
            }
            uint32_t  vertexCount = 0;
            glm::vec3 posMin{};
            glm::vec3 posMax{};

            const tinygltf::Accessor& accessor   = model.accessors[primitive.indices];
            uint32_t                  indexCount = accessor.count;

            auto newPrimitive = std::make_unique<Primitive>(0, 0, indexCount, primitive.material);
            // Vertices
            {
                for (const auto& attr : primitive.attributes) {
                    std::string attributeName = attr.first;
                    std::ranges::transform(attributeName.begin(), attributeName.end(), attributeName.begin(), ::tolower);

                    if (config.requiredVertexAttribute.empty() || config.requiredVertexAttribute.contains(attributeName)) {
                        const tinygltf::Accessor&   accessor = model.accessors[attr.second];
                        const tinygltf::BufferView& view     = model.bufferViews[accessor.bufferView];

                        auto startByte = accessor.byteOffset + view.byteOffset;
                        auto endByte   = startByte + accessor.ByteStride(view) * accessor.count;

                        std::vector<uint8_t> data(model.buffers[view.buffer].data.begin() + startByte,
                                                  model.buffers[view.buffer].data.begin() + endByte);

                        VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | getBufferUsageFlags(config);

                        auto buffer = std::make_unique<Buffer>(device, DATA_SIZE(data), bufferUsageFlags, VMA_MEMORY_USAGE_CPU_TO_GPU);
                        buffer->uploadData(data.data(), DATA_SIZE(data));

                        newPrimitive->setVertexBuffer(attributeName, buffer);

                        VertexAttribute attribute{};
                        attribute.format = getAttributeFormat(&model, attr.second);
                        attribute.stride = accessor.ByteStride(view);

                        newPrimitive->setVertxAttribute(attributeName, attribute);
                    }
                }

                if (config.requiredVertexAttribute.empty() || config.requiredVertexAttribute.contains("indices")) {
                    const tinygltf::Accessor&   accessor = model.accessors[primitive.indices];
                    const tinygltf::BufferView& view     = model.bufferViews[accessor.bufferView];

                    auto startByte = accessor.byteOffset + view.byteOffset;
                    auto endByte   = startByte + accessor.ByteStride(view) * accessor.count;

                    std::vector<uint8_t> data(model.buffers[view.buffer].data.begin() + startByte,
                                              model.buffers[view.buffer].data.begin() + endByte);

                    data = convertIndexData(data, config.indexType, getAttributeFormat(&model, primitive.indices), newPrimitive->getIndexType());

                    VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | getBufferUsageFlags(config);

                    auto indexBuffer = std::make_unique<Buffer>(device, DATA_SIZE(data), bufferUsageFlags, VMA_MEMORY_USAGE_CPU_TO_GPU);
                    indexBuffer->uploadData(data.data(), DATA_SIZE(data));
                    newPrimitive->setIndexBuffer(indexBuffer);
                }

                // Position attribute is required
                assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

                const tinygltf::Accessor&   posAccessor = model.accessors[primitive.attributes.find(
                                                                                                "POSITION")
                                                                            ->second];
                const tinygltf::BufferView& posView     = model.bufferViews[posAccessor.bufferView];

                posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
                posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);

                vertexCount = static_cast<uint32_t>(posAccessor.count);
            }

            newPrimitive->setDimensions(posMin, posMax);

            auto matrix = getTransformMatrix(node);

            if (config.bufferAddressAble) {
                auto transformBuffer = std::make_unique<Buffer>(device, sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_CPU_TO_GPU, &matrix);
                newPrimitive->setVertexBuffer("transform", transformBuffer);
            }

            // newPrimitive->vertexBuffers.emplace("transform", std::move(transformBuffer));

            PerPrimitiveUniform primitiveUniform{matrix};
            auto                primitiveUniformBuffer = std::make_unique<Buffer>(device, sizeof(PerPrimitiveUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_CPU_TO_GPU, &primitiveUniform);
            newPrimitive->setUniformBuffer(primitiveUniformBuffer);

            primitives.push_back(std::move(newPrimitive));
        }
    }
}
const Texture* GLTFLoadingImpl::getTexture(uint32_t index) const {
    if (index < textures.size()) {
        return textures[index].get();
    }
    return nullptr;
}

std::unique_ptr<Scene> GltfLoading::LoadSceneFromGLTFFile(Device& device, const std::string& path, const SceneLoadingConfig& config) {
    auto model                = std::make_unique<GLTFLoadingImpl>(device, path, config);
    auto scene                = std::make_unique<Scene>();
    scene->primitives         = std::move(model->primitives);
    scene->textures           = std::move(model->textures);
    scene->materials          = std::move(model->materials);
    scene->lights             = std::move(model->lights);
    scene->cameras            = std::move(model->cameras);
    scene->vertexAttributes   = std::move(model->vertexAttributes);
    scene->sceneVertexBuffer  = std::move(model->sceneVertexBuffer);
    scene->sceneIndexBuffer   = std::move(model->sceneIndexBuffer);
    scene->sceneUniformBuffer = std::move(model->sceneUniformBuffer);
    scene->indexType          = model->indexType;
    scene->primitiveIdBuffer  = std::move(model->scenePrimitiveIdBuffer);

    return scene;
}

GLTFLoadingImpl::GLTFLoadingImpl(Device& device, const std::string& path, const SceneLoadingConfig& config) : device(device), config(config) {
    loadFromFile(path);
}

glm::mat4 Node::localMatrix() {
    return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) *
           matrix;
}

glm::mat4 Node::getMatrix() {
    glm::mat4 m = localMatrix();
    Node*     p = parent;
    while (p) {
        m = p->localMatrix() * m;
        p = p->parent;
    }
    return m;
}

void Node::update() {
}

Node::~Node() {
    if (mesh) {
        delete mesh;
    }
    for (auto& child : children) {
        delete child;
    }
}

Mesh::Mesh(Device& device, glm::mat4 matrix) : device(device) {
}

void Primitive::setDimensions(glm::vec3 min, glm::vec3 max) {
    dimensions = BBox(min, max);
}

const BBox& Primitive::getDimensions() const {
    return dimensions;
}
