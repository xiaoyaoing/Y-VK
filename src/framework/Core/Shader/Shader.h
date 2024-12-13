
#pragma once
#include <volk.h>
#include <unordered_map>
#include <string>
class Device;

//Type without binding point: Input Output PushConstant SpecializationConstant

enum class ShaderResourceType {
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
    AccelerationStructure,
    All
};

struct ShaderResource {
    VkShaderStageFlags stages;
    ShaderResourceType type;
    // ShaderResourceMode mode;
    uint32_t    set;
    uint32_t    binding;
    uint32_t    location;
    uint32_t    inputAttachmentIndex;
    uint32_t    columns;
    uint32_t    arraySize;
    uint32_t    offset;
    uint32_t    size;
    uint32_t    constantId;
    uint32_t    qualifiers;
    std::string name;
};

struct ShaderResourceQualifiers {
    enum : uint32_t {
        None        = 0,
        NonReadable = 1,
        NonWritable = 2,
    };
};

class ShaderSource {
public:
    ShaderSource(const std::string& path);

protected:
    std::vector<std::uint32_t>& spirvCode;
};

class ShaderVariant {
public:
    ShaderVariant() = default;

    ShaderVariant(std::string&& preamble, std::vector<std::string>&& processes);

    size_t get_id() const;

    /**
     * @brief Add definitions to shader variant
     * @param definitions Vector of definitions to add to the variant
     */
    void add_definitions(const std::vector<std::string>& definitions);

    /**
     * @brief Adds a define macro to the shader
     * @param def String which should go to the right of a define directive
     */
    void add_define(const std::string& def);

    /**
     * @brief Adds an undef macro to the shader
     * @param undef String which should go to the right of an undef directive
     */
    void add_undefine(const std::string& undef);

    /**
     * @brief Specifies the size of a named runtime array for automatic reflection. If already specified, overrides the size.
     * @param runtime_array_name String under which the runtime array is named in the shader
     * @param size Integer specifying the wanted size of the runtime array (in number of elements, not size in bytes), used for automatic allocation of buffers.
     * See get_declared_struct_size_runtime_array() in spirv_cross.h
     */
    void add_runtime_array_size(const std::string& runtime_array_name, size_t size);

    void set_runtime_array_sizes(const std::unordered_map<std::string, size_t>& sizes);

    const std::string& get_preamble() const;

    const std::vector<std::string>& get_processes() const;

    const std::unordered_map<std::string, size_t>& get_runtime_array_sizes() const;

    void clear();

private:
    size_t id;

    std::string preamble;

    std::vector<std::string> processes;

    std::unordered_map<std::string, size_t> runtime_array_sizes;

    void update_id();
};

enum ShaderStage : uint8_t {
    UNDEFINED = 0,
    VERTEX    = 1,
    FRAGMENT  = 2,
    COMPUTE   = 3,
};

struct ShaderKey {
    std::string   path{};
    ShaderVariant variant{};

    using EntryPoint = std::string;
    //Used for hlsl shader 
    VkShaderStageFlagBits   stage{VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM};
    EntryPoint entryPoint{"main"};
    
    ShaderKey&    operator=(const std::string& _path) {
        this->path = _path;
        return *this;
    }
    ShaderKey& operator=(const char* _path) {
        this->path = _path;
        return *this;
    }
    ShaderKey(const std::string& _path) : path(_path) {
    }
    ShaderKey(const char* _path) : path(_path) {
    }
    ShaderKey(const std::string& _path, const std::vector<std::string>& _definitions) : path(_path) {
        for (const auto& define : _definitions) {
            variant.add_define(define);
        }
    }
    ShaderKey(const char* _path, const std::vector<std::string>& _definitions) : path(_path) {
        for (const auto& define : _definitions) {
            variant.add_define(define);
        }
    }
    // std::string getDefineString() const {
    //     std::string defineString;
    //     for (const auto& define : definitions) {
    //         defineString += "#define " + define + "\n";
    //     }
    //     return defineString;
    // }
};

using ShaderPipelineKey = std::vector<ShaderKey>;

class Shader {
public:
    enum SHADER_LOAD_MODE {
        SPV,
        GLSL,
        HLSL
    };

    Shader(Device& device, const ShaderKey& key, VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM);

    ~Shader();

    VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo() const;

    const std::vector<ShaderResource>& getShaderResources() const {
        return resources;
    }

    size_t getId() const;

    VkShaderStageFlagBits getStage() const;

    Shader operator=(const Shader& other);

    // Shader(const Shader& other);

private:
    VkShaderModule shader{VK_NULL_HANDLE};

    Device& device;

    VkShaderStageFlagBits stage;

    std::vector<ShaderResource> resources;

    //Hash result of the spv code.
    size_t id;
};