//
// Created by pc on 2023/8/27.
//
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE


#include "gltfloader.h"
#include "Core/Descriptor/DescriptorSet.h"
#include "Core/Buffer.h"

inline std::vector<uint8_t> convertUnderlyingDataStride(const std::vector<uint8_t>& src_data, uint32_t src_stride,
                                                        uint32_t dst_stride)
{
    auto elem_count = toUint32(src_data.size()) / src_stride;

    std::vector<uint8_t> result(elem_count * dst_stride);

    for (uint32_t idxSrc = 0, idxDst = 0;
         idxSrc < src_data.size() && idxDst < result.size();
         idxSrc += src_stride, idxDst += dst_stride)
    {
        std::copy(src_data.begin() + idxSrc, src_data.begin() + idxSrc + src_stride, result.begin() + idxDst);
    }

    return result;
}

inline VkFormat getAttributeFormat(const tinygltf::Model* model, uint32_t accessorId)
{
    assert(accessorId < model->accessors.size());
    auto& accessor = model->accessors[accessorId];

    VkFormat format;

    switch (accessor.componentType)
    {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
        {
            static const std::map<int, VkFormat> mapped_format = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_SINT},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_SINT},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_SINT},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_SINT}
            };

            format = mapped_format.at(accessor.type);

            break;
        }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        {
            static const std::map<int, VkFormat> mapped_format = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_UINT},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_UINT},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_UINT},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_UINT}
            };

            static const std::map<int, VkFormat> mapped_format_normalize = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_UNORM},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_UNORM},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_UNORM},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_UNORM}
            };

            if (accessor.normalized)
            {
                format = mapped_format_normalize.at(accessor.type);
            }
            else
            {
                format = mapped_format.at(accessor.type);
            }

            break;
        }
    case TINYGLTF_COMPONENT_TYPE_SHORT:
        {
            static const std::map<int, VkFormat> mapped_format = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_SINT},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_SINT},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_SINT},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_SINT}
            };

            format = mapped_format.at(accessor.type);

            break;
        }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        {
            static const std::map<int, VkFormat> mapped_format = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R16_UINT},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R16G16_UINT},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R16G16B16_UINT},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R16G16B16A16_UINT}
            };

            static const std::map<int, VkFormat> mapped_format_normalize = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R16_UNORM},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R16G16_UNORM},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R16G16B16_UNORM},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R16G16B16A16_UNORM}
            };

            if (accessor.normalized)
            {
                format = mapped_format_normalize.at(accessor.type);
            }
            else
            {
                format = mapped_format.at(accessor.type);
            }

            break;
        }
    case TINYGLTF_COMPONENT_TYPE_INT:
        {
            static const std::map<int, VkFormat> mapped_format = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R32_SINT},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R32G32_SINT},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R32G32B32_SINT},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_SINT}
            };

            format = mapped_format.at(accessor.type);

            break;
        }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        {
            static const std::map<int, VkFormat> mapped_format = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R32_UINT},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R32G32_UINT},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R32G32B32_UINT},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_UINT}
            };

            format = mapped_format.at(accessor.type);

            break;
        }
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
        {
            static const std::map<int, VkFormat> mapped_format = {
                {TINYGLTF_TYPE_SCALAR, VK_FORMAT_R32_SFLOAT},
                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R32G32_SFLOAT},
                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R32G32B32_SFLOAT},
                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_SFLOAT}
            };

            format = mapped_format.at(accessor.type);

            break;
        }
    default:
        {
            format = VK_FORMAT_UNDEFINED;
            break;
        }
    }

    return format;
};

struct Skin;


struct Mesh
{
    std::string name;
    Device& device;

    struct
    {
        std::unique_ptr<Buffer> buffer{nullptr};
        std::unique_ptr<DescriptorSet> descriptorSet{nullptr};
    } uniformBuffer;

    struct UniformBlock
    {
        glm::mat4 matrix;
        glm::mat4 jointMatrix[64]{};
        float jointcount{0};
    } uniformBlock;

    Mesh(Device& device, glm::mat4 matrix);

    std::vector<std::unique_ptr<Primitive>> primitives;
};


struct Node
{
    Node* parent;
    uint32_t index;
    std::vector<Node*> children;
    glm::mat4 matrix;
    std::string name;
    Mesh* mesh;
    Skin* skin;
    int32_t skinIndex = -1;
    glm::vec3 translation{};
    glm::vec3 scale{1.0f};
    glm::quat rotation{};

    glm::mat4 localMatrix();

    glm::mat4 getMatrix();

    void update();

    ~Node();
};


struct Skin
{
    std::string name;
    Node* skeletonRoot = nullptr;
    std::vector<glm::mat4> inverseBindMatrices;
    std::vector<Node*> joints;
};

struct AnimationChannel
{
    enum PathType
    {
        TRANSLATION,
        ROTATION,
        SCALE
    };

    PathType path;
    Node* node;
    uint32_t samplerIndex;
};

/*
    glTF animation sampler
*/
struct AnimationSampler
{
    enum InterpolationType
    {
        LINEAR,
        STEP,
        CUBICSPLINE
    };

    InterpolationType interpolation;
    std::vector<float> inputs;
    std::vector<glm::vec4> outputsVec4;
};

/*
glTF animation
*/
struct Animation
{
    std::string name;
    std::vector<AnimationSampler> samplers;
    std::vector<AnimationChannel> channels;
    float start = std::numeric_limits<float>::max();
    float end = std::numeric_limits<float>::min();
};


struct GLTFLoadingImpl
{
    std::vector<std::unique_ptr<Primitive>> primitives;

    Device& device;
    Queue& queue;
    //        Mesh *mesh;
    std::vector<Node*> nodes;
    std::vector<Node*> linearNodes;


    std::vector<Texture> textures;
    std::vector<Light> lights;
    std::vector<Material> materials;

    bool metallicRoughnessWorkflow;

    GLTFLoadingImpl(Device& device, const std::string& path, uint32_t fileLoadingFlags = 0, float scale = 1.0f);

    ~GLTFLoadingImpl();


    const Texture* getTexture(uint32_t idx) const;

    void loadImages(const std::filesystem::path& modelPath, const tinygltf::Model& model);

    void
    loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model,
             float globalscale);

    void loadFromFile(const std::string& path, uint32_t fileLoadingFlags = 0, float scale = 1.0f);

    void loadMaterials(tinygltf::Model& gltfModel);
};


bool loadImageDataFunc(tinygltf::Image* image, const int imageIndex, std::string* error, std::string* warning,
                       int req_width, int req_height, const unsigned char* bytes, int size, void* userData)
{
    // KTX files will be handled by our own code
    if (image->uri.find_last_of(".") != std::string::npos)
    {
        if (image->uri.substr(image->uri.find_last_of(".") + 1) == "ktx")
        {
            return true;
        }
    }

    return tinygltf::LoadImageData(image, imageIndex, error, warning, req_width, req_height, bytes, size, userData);
}

void GLTFLoadingImpl::loadFromFile(const std::string& path, uint32_t fileLoadingFlags, float scale)
{
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string error, warning;

    gltfContext.SetImageLoader(loadImageDataFunc, nullptr);
    bool fileLoaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, path);

    if (!fileLoaded)
    {
        LOGE("Could not load glTF file {} {} \"", path, error)
    }

    std::vector<uint32_t> indexData;

    if (!(fileLoadingFlags & GltfLoading::FileLoadingFlags::DontLoadImages))
    {
        loadImages(path, gltfModel);
    }
    loadMaterials(gltfModel);
    const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
    for (size_t i = 0; i < scene.nodes.size(); i++)
    {
        const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
        loadNode(nullptr, node, scene.nodes[i], gltfModel, scale);
    }


    for (auto node : linearNodes)
    {
        // Assign skins
        if (node->skinIndex > -1)
        {
            //node->skin = skins[node->skinIndex];
        }
        // Initial pose
        if (node->mesh)
        {
            node->update();
        }
    }


    for (auto extension : gltfModel.extensionsUsed)
    {
        if (extension == "KHR_materials_pbrSpecularGlossiness")
        {
            metallicRoughnessWorkflow = false;
        }
    }
}

void GLTFLoadingImpl::loadMaterials(tinygltf::Model& gltfModel)
{
    //   materials.resize(gltfModel.materials.size());
    for (auto mat : gltfModel.materials)
    {
        Material material{};

        for (auto& tex : mat.values)
        {
            if (tex.first.find("Texture") != std::string::npos)
                material.textures.emplace(
                    tex.first, *getTexture(gltfModel.textures[tex.second.TextureIndex()].source));
        }


        if (mat.values.find("roughnessFactor") != mat.values.end())
        {
            material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
        }
        if (mat.values.find("metallicFactor") != mat.values.end())
        {
            material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
        }
        if (mat.values.find("baseColorFactor") != mat.values.end())
        {
            material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
        }


        if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end())
        {
            tinygltf::Parameter param = mat.additionalValues["alphaMode"];
            if (param.string_value == "BLEND")
            {
                material.alphaMode = Material::ALPHAMODE_BLEND;
            }
            if (param.string_value == "MASK")
            {
                material.alphaMode = Material::ALPHAMODE_MASK;
            }
        }
        if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end())
        {
            material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
        }

        materials.push_back(std::move(material));
    }
}

GLTFLoadingImpl::~GLTFLoadingImpl()
{
    for (auto node : nodes)
        delete node;
}

void GLTFLoadingImpl::loadImages(const std::filesystem::path& modelPath, const tinygltf::Model& gltfModel)
{
    auto parentDir = modelPath.parent_path();
    for (const auto& image : gltfModel.images)
    {
        Texture texture = Texture::loadTexture(device, parentDir.string() + "/" + image.uri);
        textures.push_back(std::move(texture));
    }
}

void GLTFLoadingImpl::loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex,
                               const tinygltf::Model& model,
                               float globalscale)
{
    auto newNode = new Node();
    newNode->index = nodeIndex;
    newNode->parent = parent;
    newNode->name = node.name;
    newNode->skinIndex = node.skin;
    newNode->matrix = glm::mat4(1.0f);

    // Generate local node matrix
    auto translation = glm::vec3(0.0f);
    if (node.translation.size() == 3)
    {
        translation = glm::make_vec3(node.translation.data());
        newNode->translation = translation;
    }
    auto rotation = glm::mat4(1.0f);
    if (node.rotation.size() == 4)
    {
        glm::quat q = glm::make_quat(node.rotation.data());
        newNode->rotation = glm::mat4(q);
    }
    auto scale = glm::vec3(1.0f);
    if (node.scale.size() == 3)
    {
        scale = glm::make_vec3(node.scale.data());
        newNode->scale = scale;
    }
    if (node.matrix.size() == 16)
    {
        newNode->matrix = glm::make_mat4x4(node.matrix.data());
        if (globalscale != 1.0f)
        {
            //newNode->matrix = glm::scale(newNode->matrix, glm::vec3(globalscale));
        }
    };

    // Node with children
    if (node.children.size() > 0)
    {
        for (auto i = 0; i < node.children.size(); i++)
        {
            loadNode(newNode, model.nodes[node.children[i]], node.children[i], model,
                     globalscale);
        }
    }

    // Node contains mesh data
    if (node.mesh > -1)
    {
        const tinygltf::Mesh mesh = model.meshes[node.mesh];
        auto newMesh = new Mesh(device, newNode->matrix);
        newMesh->name = mesh.name;
        for (size_t j = 0; j < mesh.primitives.size(); j++)
        {
            const tinygltf::Primitive& primitive = mesh.primitives[j];
            if (primitive.indices < 0)
            {
                continue;
            }
            uint32_t indexCount = 0;
            uint32_t vertexCount = 0;
            glm::vec3 posMin{};
            glm::vec3 posMax{};

            auto newPrimitive = std::make_unique<Primitive>(0, indexCount,
                                                            primitive.material > -1
                                                                ? materials[primitive.material]
                                                                : materials.back());

            // Vertices
            {
                const float* bufferNormals = nullptr;
                const float* bufferTexCoords = nullptr;
                const float* bufferColors = nullptr;
                const float* bufferTangents = nullptr;
                uint32_t numColorComponents;
                const uint16_t* bufferJoints = nullptr;
                const float* bufferWeights = nullptr;


                for (const auto& attr : primitive.attributes)
                {
                    std::string attributeName = attr.first;
                    std::ranges::transform(attributeName.begin(), attributeName.end(), attributeName.begin(),
                                           ::tolower);


                    const tinygltf::Accessor& accessor = model.accessors[attr.second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];

                    auto startByte = accessor.byteOffset + view.byteOffset;
                    auto endByte = startByte + accessor.ByteStride(view) * accessor.count;

                    std::vector<uint8_t> data(model.buffers[view.buffer].data.begin() + startByte,
                                              model.buffers[view.buffer].data.begin() + endByte);
                    auto buffer = std::make_unique<Buffer>(device, DATA_SIZE(data),
                                                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                                                           | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
                                                           ,
                                                           //todo fix this 
                                                           VMA_MEMORY_USAGE_CPU_TO_GPU);
                    buffer->uploadData(data.data(), DATA_SIZE(data));

                    newPrimitive->vertexBuffers.emplace(attributeName, std::move(buffer));

                    VertexAttribute attribute{};
                    attribute.format = getAttributeFormat(&model, attr.second);
                    attribute.stride = accessor.ByteStride(view);

                    newPrimitive->setVertxAttribute(attributeName, attribute);
                }


                {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];

                    auto startByte = accessor.byteOffset + view.byteOffset;
                    auto endByte = startByte + accessor.ByteStride(view) * accessor.count;

                    std::vector<uint8_t> data(model.buffers[view.buffer].data.begin() + startByte,
                                              model.buffers[view.buffer].data.begin() + endByte);

                    switch (getAttributeFormat(&model, primitive.indices))
                    {
                    case VK_FORMAT_R8_UINT:
                        // Converts uint8 data into uint16 data, still represented by a uint8 vector
                        data = convertUnderlyingDataStride(data, 1, 2);
                        newPrimitive->indexType = VK_INDEX_TYPE_UINT16;
                        break;
                    case VK_FORMAT_R16_UINT:
                        newPrimitive->indexType = VK_INDEX_TYPE_UINT16;
                        break;
                    case VK_FORMAT_R32_UINT:
                        newPrimitive->indexType = VK_INDEX_TYPE_UINT32;
                        break;
                    default:
                        LOGE("gltf primitive has invalid format type");
                        break;
                    }

                    newPrimitive->indexBuffer = std::make_unique<Buffer>(device, DATA_SIZE(data),
                                                                         VK_BUFFER_USAGE_INDEX_BUFFER_BIT
                                                                         | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                                                                         | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
                                                                         ,
                                                                         VMA_MEMORY_USAGE_CPU_TO_GPU);
                    newPrimitive->indexBuffer->uploadData(data.data(), DATA_SIZE(data));
                }


                // Position attribute is required
                assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

                const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find(
                    "POSITION")->second];
                const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
                
                posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
                posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);


                vertexCount = static_cast<uint32_t>(posAccessor.count);
            }
            const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

            newPrimitive->indexCount = static_cast<uint32_t>(accessor.count);

            newPrimitive->firstVertex = 0;
            newPrimitive->vertexCount = vertexCount;
            newPrimitive->setDimensions(posMin, posMax);
            newPrimitive->matrix = newNode->getMatrix();

            auto transformBuffer = std::make_unique<Buffer>(device, sizeof(glm::mat4),
                                                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                                                            | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                                                            | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
                                                            ,
                                                            VMA_MEMORY_USAGE_CPU_TO_GPU,&newPrimitive->matrix);
            
            newPrimitive->vertexBuffers.emplace("transform", std::move(transformBuffer));

            primitives.push_back(std::move(newPrimitive));
        }
        newNode->mesh = newMesh;
    }
    if (parent)
    {
        parent->children.push_back(newNode);
    }
    else
    {
        nodes.push_back(newNode);
    }
    linearNodes.push_back(newNode);
}

const Texture* GLTFLoadingImpl::getTexture(uint32_t index) const
{
    if (index < textures.size())
    {
        return &textures[index];
    }
    return nullptr;
}


std::unique_ptr<Scene> GltfLoading::LoadSceneFromGLTFFile(Device& device, const std::string& path,
                                                          uint32_t fileLoadingFlags, float scale)
{
    auto model = std::make_unique<GLTFLoadingImpl>(device, path);
   // model->primitives[0] = std::move(model->primitives[1]);
    // model->primitives.resize(2);
    return std::make_unique<Scene>(std::move(model->primitives), std::move(model->textures),
                                   std::move(model->materials), std::move(model->lights));
}

GLTFLoadingImpl::GLTFLoadingImpl(Device& device, const std::string& path, uint32_t fileLoadingFlags, float scale) :
    device(device), queue(device.getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0))
{
    loadFromFile(path, fileLoadingFlags, scale);
}


glm::mat4 Node::localMatrix()
{
    return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) *
        matrix;
}

glm::mat4 Node::getMatrix()
{
    glm::mat4 m = localMatrix();
    Node* p = parent;
    while (p)
    {
        m = p->localMatrix() * m;
        p = p->parent;
    }
    return m;
}

void Node::update()
{
}

Node::~Node()
{
    if (mesh)
    {
        delete mesh;
    }
    for (auto& child : children)
    {
        delete child;
    }
}


Mesh::Mesh(Device& device, glm::mat4 matrix) : device(device)
{
}

void Primitive::setDimensions(glm::vec3 min, glm::vec3 max)
{
}
