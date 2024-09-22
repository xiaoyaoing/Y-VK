#include "App/Application.h"

class VXGI : public  Application
{
public:
    VXGI();
    void prepare() override;
protected:

    struct Particle
    {
        glm::vec4 pos;        // xyz = position, w = mass
        glm::vec4 vel;
    };


    struct ComputeUBO
    {                              // Compute shader uniform block object
        float   delta_time;        //		Frame delta time
        int32_t particle_count;
    } ubo;
    
    void drawFrame(RenderGraph& renderGraph) override;
    uint32_t num_particles;
    bool lock_simulation_speed{false};
    std::unique_ptr<Buffer> storageBuffer{nullptr};
    std::unique_ptr<Buffer> uniformBuffer{nullptr};
    std::unique_ptr<PipelineLayout> computeCalculateLayout{nullptr};
    std::unique_ptr<PipelineLayout> computeIntegrateLayout{nullptr};
    
    struct 
    {
        std::unique_ptr<Texture> particle;
        std::unique_ptr<Texture> gradient;
        std::unique_ptr<Buffer> uniformBuffer;
        struct
        {
            glm::mat4 projection;
            glm::mat4 view;
            glm::vec2 screenDim;
        } ubo;
        std::unique_ptr<PipelineLayout> pipelineLayout;
    } graphics;
};