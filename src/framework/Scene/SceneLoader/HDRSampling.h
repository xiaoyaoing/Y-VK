#pragma once
#include "Core/Texture.h"
#include "Raytracing/commons.h"

class HDRSampling {
public:
    // HdrSampling() = default;

    void setup(const VkDevice& device);
    void loadEnvironment(const std::string& hrdImage);


    void  destroy();
    float getIntegral() { return m_integral; }
    float getAverage() { return m_average; }

    // Resources
    Texture * m_texHdr;
    std::unique_ptr<Buffer> m_accelImpSmpl;
    std::vector<EnvAccel> createEnvironmentAccel(const float* pixels, VkExtent2D size);
private:
    // VkDevice                 m_device{VK_NULL_HANDLE};
    // uint32_t                 m_familyIndex{0};

    float m_integral{1.f};
    float m_average{1.f};


    float                 buildAliasmap(const std::vector<float>& data, std::vector<EnvAccel>& accel);
};
