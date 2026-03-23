#include "Distrib.hpp"

std::unique_ptr<Buffer> Distribution1D::toGpuBuffer(Device& device) const {
    std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>(device, sizeof(float) * 2 * (1+Count()), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    auto mapped = buffer->map();
    float count = Count();
    memcpy(mapped, &count, sizeof(float));
    memcpy(static_cast<char*>(mapped) + sizeof(float), &funcInt, sizeof(float));
    memcpy(static_cast<char*>(mapped) + 2 * sizeof(float),cdf.data(), sizeof(float) * Count());
    memcpy(static_cast<char*>(mapped) + 2 * sizeof(float) + sizeof(float) * Count(), func.data(), sizeof(float) * Count());
    buffer->unmap();
    return buffer;
}
Distribution2D::Distribution2D(const float* data, int nu, int nv) {
    pConditionalV.reserve(nv);
    for (int i = 0; i < nv; i++) {
        pConditionalV.emplace_back(std::make_unique<Distribution1D>(data + nu * i, nu));
    }
    std::vector<float> marginalFunc;
    marginalFunc.reserve(nv);
    for (int v = 0; v < nv; ++v)
        marginalFunc.push_back(pConditionalV[v]->funcInt);
    pMarginal.reset(new Distribution1D(&marginalFunc[0], nv));
}

void Distribution2D::warp(glm::vec2& uv, int& row, int& column) const {
    pMarginal->warp(uv.x, row);
    pConditionalV[row]->warp(uv.y, column);
}