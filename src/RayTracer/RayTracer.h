//
// Created by pc on 2023/12/1.
//

#ifndef VULKANDEMO_RAYTRACER_H
#define VULKANDEMO_RAYTRACER_H

#include "App/Application.h"

#include <Core/RayTracing/Accel.h>

#include "Integrators/Integrator.h"

struct RayTracerSettings{};

struct ReservoirSample {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 emission;
    float dist;
    glm::vec3 radiance;
    float p_hat;
};

struct ReSTIRReservoir {
    ReservoirSample y;
    float      w_sum;
    float      W;
    unsigned int  M;
};


class RayTracer : public Application {
public:
    RayTracer(const RayTracerSettings& settings);
    void prepare() override;
protected:
    void drawFrame(RenderGraph &renderGraph) override;
    Accel createAccel(VkAccelerationStructureCreateInfoKHR & accel);
    VkDeviceAddress getAccelerationStructureDeviceAddress(uint32_t primIdx);
    PipelineLayout * layout;
    std::vector<Accel> blases;
    Accel tlas;

    std::unique_ptr<SgImage> storageImage;
    
    std::unique_ptr<Integrator> integrator{};
    
    struct 
    {
        glm::mat4 viewInverse;
        glm::mat4 projInverse;
    } cameraUbo;
};



#endif //VULKANDEMO_RAYTRACER_H
