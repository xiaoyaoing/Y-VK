//
// Created by pc on 2023/8/27.
//

#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ext/tinygltf/tiny_gltf.h"

#include <filesystem>

#include "Buffer.h"
#include "API_VK.h"
#include "Camera.h"
//#include "Mesh.h"

namespace gltfLoading
{
    struct VertexAttribute
    {
        VkFormat format = VK_FORMAT_UNDEFINED;

        std::uint32_t stride = 0;

        std::uint32_t offset = 0;
    };


    enum FileLoadingFlags
    {
        None = 0x00000000,
        PreTransformVertices = 0x00000001,
        PreMultiplyVertexColors = 0x00000002,
        FlipY = 0x00000004,
        DontLoadImages = 0x00000008
    };

    enum RenderFlags
    {
        BindImages = 0x00000001,
        RenderOpaqueNodes = 0x00000002,
        RenderAlphaMaskedNodes = 0x00000004,
        RenderAlphaBlendedNodes = 0x00000008
    };

    struct Skin;

    struct Material
    {
        enum AlphaMode
        {
            ALPHAMODE_OPAQUE,
            ALPHAMODE_MASK,
            ALPHAMODE_BLEND
        };

        AlphaMode alphaMode = ALPHAMODE_OPAQUE;
        Device& device;
        float alphaCutoff = 1.0f;
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
        glm::vec4 baseColorFactor = glm::vec4(1.0f);

        std::unordered_map<std::string, Texture*> textures{};


        Material(Device& device);
    };


    struct Primitive
    {
        uint32_t firstIndex{};
        uint32_t indexCount{};
        uint32_t firstVertex{};
        uint32_t vertexCount{};
        Material& material;
        glm::mat4 matrix{};

        struct Dimensions
        {
            glm::vec3 min = glm::vec3(FLT_MAX);
            glm::vec3 max = glm::vec3(-FLT_MAX);
            glm::vec3 size;
            glm::vec3 center;
            float radius;
        } dimensions;

        std::unordered_map<std::string, VertexAttribute> vertexAttributes;

        std::unordered_map<std::string, std::unique_ptr<Buffer>> vertexBuffers;

        std::unique_ptr<Buffer> indexBuffer;

        bool getVertexAttribute(const std::string& name, VertexAttribute& attribute) const;

        void setVertxAttribute(const std::string& name, VertexAttribute& attribute);

        void setVertexBuffer(const std::string& name, std::unique_ptr<Buffer>& buffer);

        Buffer& getVertexBuffer(const std::string& name) const;

        void setDimensions(glm::vec3 min, glm::vec3 max);

        Primitive(uint32_t firstIndex, uint32_t indexCount, Material& material) : firstIndex(firstIndex),
            indexCount(indexCount),
            material(material)
        {
        }
    };


    struct Mesh
    {
        std::string name;
        Device& device;
        std::vector<std::unique_ptr<Primitive>> primitives;


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

    enum class VertexComponent
    {
        Position,
        Normal,
        UV,
        Color,
        Tangent,
        Joint0,
        Weight0
    };

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;

        static VkVertexInputBindingDescription vertexInputBindingDescription;
        static std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
        static VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo;

        static VkVertexInputBindingDescription inputBindingDescription(uint32_t binding);

        static VkVertexInputAttributeDescription
        inputAttributeDescription(uint32_t binding, uint32_t location, VertexComponent component);

        static std::vector<VkVertexInputAttributeDescription>
        inputAttributeDescriptions(uint32_t binding, const std::vector<VertexComponent> components);

        /** @brief Returns the default pipeline vertex input state create info structure for the requested vertex components */
        static VkPipelineVertexInputStateCreateInfo*
        getPipelineVertexInputState(const std::vector<VertexComponent> components);
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

    struct Model
    {
        Camera* camera;

        std::vector<std::unique_ptr<Primitive>> prims;

        Device& device;
        Queue& queue;
        //        Mesh *mesh;
        std::vector<Node*> nodes;
        std::vector<Node*> linearNodes;


        std::vector<Skin*> skins;

        std::vector<Texture> textures;
        Texture emptyTexture;

        std::vector<Material> materials;
        std::vector<Animation> animations;

        std::filesystem::path modelPath;


        std::unique_ptr<Buffer> vertices, indices;

        uint32_t vertexCount{0}, indexCount{0};

        bool metallicRoughnessWorkflow;

        static std::unique_ptr<Model> loadFromFile(Device& device, const std::string& path);

        using PrimitiveCallBack = std::function<void(Primitive& primitive)>;
        void IteratePrimitives(PrimitiveCallBack primitiveCallBack);

        Model(Device& device);

        Texture* getTexture(int idx);

        void loadFromFile(const std::string& path, uint32_t fileLoadingFlags = 0, float scale = 1.0f);

        void loadMaterials(tinygltf::Model& gltfModel);

        void bindBuffer(CommandBuffer& commandBuffer);

        void iterateAllNodes(std::function<void> (CommandBuffer& commandBuffer));

        void
        draw(CommandBuffer& commandBuffer, uint32_t renderFlags = 0, VkPipelineLayout pipelineLayout = VK_NULL_HANDLE,
             uint32_t bindImageSet = 1);

        void draw(Node& node, CommandBuffer& commandBuffer, uint32_t renderFlags = 0,
                  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE,
                  uint32_t bindImageSet = 1);

        void iteratePrims();

        ~Model();

        void loadImages(tinygltf::Model model, Device& device, Queue& queue);

        void
        loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model,
                 std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer, float globalscale);
    };
}
