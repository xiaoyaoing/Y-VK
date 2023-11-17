//
// Created by pc on 2023/8/27.
//
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE


#include "gltfloader.h"
#include "Descriptor/DescriptorSet.h"
#include "Buffer.h"

namespace gltfLoading
{
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

    void Model::loadFromFile(const std::string& path, uint32_t fileLoadingFlags, float scale)
    {
        this->modelPath = path;

        tinygltf::Model gltfModel;
        tinygltf::TinyGLTF gltfContext;
        //    if (fileLoadingFlags & FileLoadingFlags::DontLoadImages) {
        //        gltfContext.SetImageLoader(loadImageDataFuncEmpty, nullptr);
        //    } else {
        //    }
        std::string error, warning;

        gltfContext.SetImageLoader(loadImageDataFunc, nullptr);
        bool fileLoaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, path);

        if (!fileLoaded)
        {
            LOGE("Could not load glTF file {} {} \"", path, error)
        }

        std::vector<uint32_t> indexData;
        std::vector<Vertex> vertexData;

        if (!(fileLoadingFlags & FileLoadingFlags::DontLoadImages))
        {
            loadImages(gltfModel, device, queue);
        }
        loadMaterials(gltfModel);
        const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
        for (size_t i = 0; i < scene.nodes.size(); i++)
        {
            const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
            loadNode(nullptr, node, scene.nodes[i], gltfModel, indexData, vertexData, scale);
        }
        // if (gltfModel.animations.size() > 0) {
        //     // loadAnimations(gltfModel);
        // }
        //loadSkins(gltfModel);

        for (auto node : linearNodes)
        {
            // Assign skins
            if (node->skinIndex > -1)
            {
                node->skin = skins[node->skinIndex];
            }
            // Initial pose
            if (node->mesh)
            {
                node->update();
            }
        }


        if ((fileLoadingFlags & FileLoadingFlags::PreTransformVertices) ||
            (fileLoadingFlags & FileLoadingFlags::PreMultiplyVertexColors) ||
            (fileLoadingFlags & FileLoadingFlags::FlipY))
        {
            const bool preTransform = fileLoadingFlags & FileLoadingFlags::PreTransformVertices;
            const bool preMultiplyColor = fileLoadingFlags & FileLoadingFlags::PreMultiplyVertexColors;
            const bool flipY = fileLoadingFlags & FileLoadingFlags::FlipY;
            for (Node* node : linearNodes)
            {
                if (node->mesh)
                {
                    const glm::mat4 localMatrix = node->getMatrix();
                    for (auto& primitive : node->mesh->primitives)
                    {
                        for (uint32_t i = 0; i < primitive->vertexCount; i++)
                        {
                            Vertex& vertex = vertexData[primitive->firstVertex + i];
                            // Pre-transform vertex positions by node-hierarchy
                            if (preTransform)
                            {
                                vertex.pos = glm::vec3(localMatrix * glm::vec4(vertex.pos, 1.0f));
                                vertex.normal = glm::normalize(glm::mat3(localMatrix) * vertex.normal);
                            }
                            // Flip Y-Axis of vertex positions
                            if (flipY)
                            {
                                vertex.pos.y *= -1.0f;
                                vertex.normal.y *= -1.0f;
                            }
                            // Pre-Multiply vertex colors with material base color
                            if (preMultiplyColor)
                            {
                                // vertex.color = primitive->material.baseColorFactor * vertex.color;
                            }
                        }
                    }
                }
            }
        }

        for (auto extension : gltfModel.extensionsUsed)
        {
            if (extension == "KHR_materials_pbrSpecularGlossiness")
            {
                std::cout << "Required extension: " << extension;
                metallicRoughnessWorkflow = false;
            }
        }
        vertexCount = vertexData.size();
        indexCount = indexData.size();
        vertices = std::make_unique<Buffer>(device, DATA_SIZE(vertexData), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                            VMA_MEMORY_USAGE_CPU_TO_GPU);
        vertices->uploadData(vertexData.data(), DATA_SIZE(vertexData));
        indices = std::make_unique<Buffer>(device, DATA_SIZE(indexData), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                           VMA_MEMORY_USAGE_CPU_TO_GPU);
        indices->uploadData(indexData.data(), DATA_SIZE(indexData));
    }

    void Model::loadMaterials(tinygltf::Model& gltfModel)
    {
        //   materials.resize(gltfModel.materials.size());
        for (auto mat : gltfModel.materials)
        {
            Material material(device);

            for (auto& tex : mat.values)
            {
                if (tex.first.find("Texture") != std::string::npos)
                    material.textures.emplace(
                        tex.first, getTexture(gltfModel.textures[tex.second.TextureIndex()].source));
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

    void Model::draw(CommandBuffer& commandBuffer, uint32_t renderFlags, VkPipelineLayout pipelineLayout,
                     uint32_t bindImageSet)
    {
        VkDeviceSize offsets[1] = {0};
        VkBuffer vertexBuffer[] = {vertices->getHandle()};
        vkCmdBindVertexBuffers(commandBuffer.getHandle(), 0, 1, vertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer.getHandle(), indices->getHandle(), 0, VK_INDEX_TYPE_UINT32);
        for (auto& node : linearNodes)
            draw(*node, commandBuffer, renderFlags, pipelineLayout, bindImageSet);
    }

    void Model::draw(Node& node, CommandBuffer& commandBuffer, uint32_t renderFlags, VkPipelineLayout pipelineLayout,
                     uint32_t bindImageSet)
    {
        if (node.mesh)
        {
            for (auto& primitive : node.mesh->primitives)
            {
                bool skip = false;
                const Material& material = primitive->material;
                if (renderFlags & RenderFlags::RenderOpaqueNodes)
                {
                    skip = (material.alphaMode != Material::ALPHAMODE_OPAQUE);
                }
                if (renderFlags & RenderFlags::RenderAlphaMaskedNodes)
                {
                    skip = (material.alphaMode != Material::ALPHAMODE_MASK);
                }
                if (renderFlags & RenderFlags::RenderAlphaBlendedNodes)
                {
                    skip = (material.alphaMode != Material::ALPHAMODE_BLEND);
                }
                if (!skip)
                {
                    if (renderFlags & RenderFlags::BindImages)
                    {
                    }
                }
            }
        }
        for (auto child : node.children)
        {
            draw(*child, commandBuffer, renderFlags, pipelineLayout, bindImageSet);
        }
    }

    Model::~Model()
    {
        for (auto node : nodes)
            delete node;
    }

    void Model::loadImages(tinygltf::Model gltfModel, Device& device, Queue& queue)
    {
        auto parentDir = modelPath.parent_path().string() + "/";
        for (tinygltf::Image& image : gltfModel.images)
        {
            Texture texture = Texture::loadTexture(device, parentDir + image.uri);
            textures.push_back(std::move(texture));
        }
        // Create an empty texture to be used for empty material images
        //        createEmptyTexture(transferQueue);
        // tinygltf::Model
    }

    void Model::loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model,
                         std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer, float globalscale)
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
                loadNode(newNode, model.nodes[node.children[i]], node.children[i], model, indexBuffer, vertexBuffer,
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
                uint32_t indexStart = static_cast<uint32_t>(indexBuffer.size());
                uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
                uint32_t indexCount = 0;
                uint32_t vertexCount = 0;
                glm::vec3 posMin{};
                glm::vec3 posMax{};

                auto newPrimitive = std::make_unique<Primitive>(indexStart, indexCount,
                                                                primitive.material > -1
                                                                    ? materials[primitive.material]
                                                                    : materials.back());

                // Vertices
                {
                    const float* bufferPos = nullptr;
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
                        auto buffer = std::make_unique<Buffer>(device,DATA_SIZE(data),
                                                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                               VMA_MEMORY_USAGE_CPU_TO_GPU);
                        buffer->uploadData(data.data(),DATA_SIZE(data));

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
                        newPrimitive->indexBuffer = std::make_unique<Buffer>(device,DATA_SIZE(data),
                                                               VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                               VMA_MEMORY_USAGE_CPU_TO_GPU);
                        newPrimitive->indexBuffer->uploadData(data.data(),DATA_SIZE(data));

                    }
                    

                    // Position attribute is required
                    assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

                    const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find(
                        "POSITION")->second];
                    const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
                    bufferPos = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[
                        posAccessor.byteOffset + posView.byteOffset]));
                    posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
                    posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);


                    vertexCount = static_cast<uint32_t>(posAccessor.count);

                    for (size_t v = 0; v < posAccessor.count; v++)
                    {
                        Vertex vert{};
                        vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * 3]), 1.0f);
                        vert.normal = glm::normalize(
                            glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * 3]) : glm::vec3(0.0f)));
                        vert.uv = bufferTexCoords ? glm::make_vec2(&bufferTexCoords[v * 2]) : glm::vec3(0.0f);
                        if (bufferColors)
                        {
                            switch (numColorComponents)
                            {
                            case 3:
                            //   vert.color = glm::vec4(glm::make_vec3(&bufferColors[v * 3]), 1.0f);
                            case 4: ;
                            // vert.color = glm::make_vec4(&bufferColors[v * 4]);
                            }
                        }
                        else
                        {
                            // vert.color = glm::vec4(1.0f);
                        }
                        // vert.tangent = bufferTangents
                        //                    ? glm::vec4(glm::make_vec4(&bufferTangents[v * 4]))
                        //                    : glm::vec4(
                        //                        0.0f);
                        // vert.joint0 = hasSkin ? glm::vec4(glm::make_vec4(&bufferJoints[v * 4])) : glm::vec4(0.0f);
                        // vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * 4]) : glm::vec4(0.0f);
                        vertexBuffer.push_back(vert);
                    }
                }
                // Indices
                {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                    newPrimitive->indexCount = static_cast<uint32_t>(accessor.count);

                    switch (accessor.componentType)
                    {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                        {
                            auto buf = new uint32_t[accessor.count];
                            memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset],
                                   accessor.count * sizeof(uint32_t));
                            for (size_t index = 0; index < accessor.count; index++)
                            {
                                indexBuffer.push_back(buf[index] + vertexStart);
                            }
                            delete[] buf;
                            break;
                        }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                        {
                            auto buf = new uint16_t[accessor.count];
                            memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset],
                                   accessor.count * sizeof(uint16_t));
                            for (size_t index = 0; index < accessor.count; index++)
                            {
                                indexBuffer.push_back(buf[index] + vertexStart);
                            }
                            delete[] buf;
                            break;
                        }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                        {
                            auto buf = new uint8_t[accessor.count];
                            memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset],
                                   accessor.count * sizeof(uint8_t));
                            for (size_t index = 0; index < accessor.count; index++)
                            {
                                indexBuffer.push_back(buf[index] + vertexStart);
                            }
                            delete[] buf;
                            break;
                        }
                    default:
                        std::cerr << "Index component type " << accessor.componentType << " not supported!"
                            << std::endl;
                        return;
                    }
                }

                newPrimitive->firstVertex = vertexStart;
                newPrimitive->vertexCount = vertexCount;
                newPrimitive->setDimensions(posMin, posMax);
                newPrimitive->matrix = newNode->getMatrix();

                prims.push_back(std::move(newPrimitive));
                //    newMesh->primitives.push_back(std::move(newPrimitive));
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

    Texture* Model::getTexture(int index)
    {
        if (index < textures.size())
        {
            return &textures[index];
        }
        return nullptr;
    }

    std::unique_ptr<Model> Model::loadFromFile(Device& device, const std::string& path)
    {
        auto model = std::make_unique<Model>(device);
        model->loadFromFile(path);
        return model;
    }

    void Model::IteratePrimitives(PrimitiveCallBack primitiveCallBack)
    {
        for (auto& prim : prims)
        {
            primitiveCallBack(*prim);
        }
    }

    Model::Model(Device& device) : device(device), queue(device.getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0))
    {
    }

    void Model::bindBuffer(CommandBuffer& commandBuffer)
    {
        // commandBuffer.bindVertexBuffer(*vertices, 0);
        //commandBuffer.bindIndicesBuffer(*indices, 0);
    }

    Material::Material(Device& device) : device(device)
    {
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

    bool Primitive::getVertexAttribute(const std::string& name, VertexAttribute& attribute) const
    {
        if (vertexAttributes.contains(name))
        {
            attribute = vertexAttributes.at(name);
            return true;
        }
        return false;
    }

    void Primitive::setVertxAttribute(const std::string& name, VertexAttribute& attribute)
    {
        vertexAttributes.emplace(name, attribute);
    }

    void Primitive::setVertexBuffer(const std::string& name, std::unique_ptr<Buffer>& buffer)
    {
        vertexBuffers.emplace(name, std::move(buffer));
    }

    Buffer& Primitive::getVertexBuffer(const std::string& name) const
    {
        if (vertexBuffers.contains(name))
            return *vertexBuffers.at(name);
        LOGE("Primitive has no such buffer {}", name);
    }


    Mesh::Mesh(Device& device, glm::mat4 matrix) : device(device)
    {
    }

    void Primitive::setDimensions(glm::vec3 min, glm::vec3 max)
    {
    }
}
