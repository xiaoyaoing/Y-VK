//
// Created by pc on 2023/12/1.
//

#ifndef VULKANDEMO_RAYTRACER_H
#define VULKANDEMO_RAYTRACER_H

#include "App/Application.h"

struct RayTracerSettings{};

struct Accel
{
    VkAccelerationStructureKHR accel = VK_NULL_HANDLE;
    std::unique_ptr<Buffer> buffer;
};


class RayTracer : public Application {
public:
    RayTracer(const RayTracerSettings& settings);
    void prepare() override;
protected:
    void drawFrame(RenderGraph &renderGraph,CommandBuffer &commandBuffer) override;
    void buildBLAS();
    void buildTLAS();
    Accel createAccel(VkAccelerationStructureCreateInfoKHR & accel);
    VkDeviceAddress getAccelerationStructureDeviceAddress(uint32_t primIdx);
    PipelineLayout * layout;
    std::vector<Accel> blases;
    Accel tlas;
};



#endif //VULKANDEMO_RAYTRACER_H
