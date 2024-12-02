#pragma once
#include "Core/RenderPass.h"
#include "Core/Buffer.h"
#include "Core/Pipeline.h"
#include "NaniteBuilder.h"
#include <random>

// class VisualizeClusterPass : public RenderPass {
// public:
//     VisualizeClusterPass();
//     virtual ~VisualizeClusterPass() = default;
//
//     void setup(const std::vector<Cluster>& clusters);
//     virtual void execute(CommandBuffer* commandBuffer) override;
//
// private:
//     struct ClusterVertex {
//         glm::vec3 position;
//         glm::vec3 normal;
//         glm::vec2 uv;
//         glm::vec4 color;  // 每个cluster一个随机颜色
//     };
//
//     struct PushConstants {
//         glm::mat4 viewProj;
//         int32_t highlightClusterId;  // -1表示不高亮任何cluster
//         float wireframeWidth;        // 线框宽度
//         float padding[2];
//     };
//
//     // 生成随机颜色
//     glm::vec4 generateRandomColor() {
//         static std::random_device rd;
//         static std::mt19937 gen(rd());
//         static std::uniform_real_distribution<float> dis(0.2f, 0.8f);
//         return glm::vec4(dis(gen), dis(gen), dis(gen), 1.0f);
//     }
//
//     std::unique_ptr<Buffer> m_vertexBuffer;
//     std::unique_ptr<Buffer> m_indexBuffer;
//     std::unique_ptr<Pipeline> m_solidPipeline;    // 实体渲染
//     std::unique_ptr<Pipeline> m_wirePipeline;     // 线框渲染
//     std::unique_ptr<Shader> m_vertexShader;
//     std::unique_ptr<Shader> m_fragmentShader;
//     std::unique_ptr<Shader> m_wireframeVS;
//     std::unique_ptr<Shader> m_wireframeFS;
//
//     uint32_t m_totalIndices{0};
//     int32_t m_currentHighlightCluster{-1};
// };
