// //
// // Created by pc on 2023/9/4.
// //
//
// #include <Vulkan.h>
// #include "RenderTarget.h"
//
// #pragma once
//
// class Defered {
//     std::unique_ptr<RenderTarget> createRenderTarget(Image &&swapChainImage) {
//         std::vector<Image> images;
//
//
//         VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
//
//         Image depthImage(swapChainImage.getDevice(), swapChainImage.getExtent(), depthFormat,
//                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
//                          VMA_MEMORY_USAGE_GPU_ONLY);
//
//         //  Image positionImage(swapChainImage.getDevice(), swapChainImage.getExtent(), VK_FORMAT_R16G16B16A16_SFLOAT,
//         //                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
//
//         Image normalImage(swapChainImage.getDevice(), swapChainImage.getExtent(), VK_FORMAT_R16G16B16A16_SFLOAT,
//                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
//
//         Image albedoImage(swapChainImage.getDevice(), swapChainImage.getExtent(), VK_FORMAT_R8G8B8A8_SRGB,
//                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
//
//         //  Image normalImage
//
//
//
//
//
//         images.push_back(std::move(swapChainImage));
//
//
//         images.push_back(std::move(depthImage));
//
//
//         images.push_back(std::move(albedoImage));
//
//         images.push_back(std::move(normalImage));
//
//
//         return std::make_unique<RenderTarget>(std::move(images));
//     }
// };
//
//
