#include "RenderPass.h"
#include <Device.h>
#include <RenderTarget.h>

void
setAttachmentLayouts(std::vector<VkSubpassDescription> subpassDescs,
                     std::vector<VkAttachmentDescription> &attachments) {

    for (auto subpass: subpassDescs) {
        for (int i = 0; i < subpass.colorAttachmentCount; i++) {
            auto reference = subpass.pColorAttachments[i];
            if (attachments[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                attachments[reference.attachment].initialLayout = reference.layout;
            }
        }

        for (int i = 0; i < subpass.inputAttachmentCount; i++) {
            auto &reference = subpass.pInputAttachments[i];
            if (attachments[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                attachments[reference.attachment].initialLayout = reference.layout;
            }
        }


        for (int i = 0; i < subpass.colorAttachmentCount; i++) {
            auto &reference = subpass.pColorAttachments[i];
            if (attachments[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                attachments[reference.attachment].initialLayout = reference.layout;
            }
        }

        if (subpass.pDepthStencilAttachment) {
            auto &reference = *subpass.pDepthStencilAttachment;
            if (attachments[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                attachments[reference.attachment].initialLayout = reference.layout;
            }
        }

        if (subpass.pResolveAttachments) {
            for (int i = 0; i < subpass.colorAttachmentCount; i++) {
                auto &reference = subpass.pResolveAttachments[i];
                if (attachments[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                    attachments[reference.attachment].initialLayout = reference.layout;
                }
            }
        }
    }


    for (auto subpass: subpassDescs) {
        for (int i = 0; i < subpass.colorAttachmentCount; i++) {
            auto reference = subpass.pColorAttachments[i];
            if (attachments[reference.attachment].finalLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                attachments[reference.attachment].finalLayout = reference.layout;
            }
        }

        for (int i = 0; i < subpass.inputAttachmentCount; i++) {
            auto &reference = subpass.pInputAttachments[i];
            if (attachments[reference.attachment].finalLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                attachments[reference.attachment].finalLayout = reference.layout;
            }
        }


        for (int i = 0; i < subpass.colorAttachmentCount; i++) {
            auto &reference = subpass.pColorAttachments[i];
            if (attachments[reference.attachment].finalLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                attachments[reference.attachment].finalLayout = reference.layout;
            }
        }

        if (subpass.pDepthStencilAttachment) {
            auto &reference = *subpass.pDepthStencilAttachment;
            if (attachments[reference.attachment].finalLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                attachments[reference.attachment].finalLayout = reference.layout;
            }
        }

        if (subpass.pResolveAttachments) {
            for (int i = 0; i < subpass.colorAttachmentCount; i++) {
                auto &reference = subpass.pResolveAttachments[i];
                if (attachments[reference.attachment].finalLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                    attachments[reference.attachment].finalLayout = reference.layout;
                }
            }
        }
    }
}

std::vector<VkSubpassDependency> getSubpassDependency(const size_t subpassCount) {
    std::vector<VkSubpassDependency> dependencies(subpassCount - 1);

    if (subpassCount > 1) {
        for (uint32_t i = 0; i < (dependencies.size()); ++i) {
            // Transition input attachments from color attachment to shader read
            dependencies[i].srcSubpass = i;
            dependencies[i].dstSubpass = i + 1;
            dependencies[i].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[i].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[i].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
            dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }
    }

    return dependencies;
}


std::vector<VkAttachmentDescription> get_attachment_descriptions(const std::vector<Attachment> &attachments,
                                                                 const std::vector<LoadStoreInfo> &load_store_infos) {
    std::vector<VkAttachmentDescription> attachment_descriptions;

    for (size_t i = 0U; i < attachments.size(); ++i) {
        VkAttachmentDescription attachment{};

        attachment.format = attachments[i].format;
        attachment.samples = attachments[i].samples;
        attachment.initialLayout = attachments[i].initial_layout;
        attachment.finalLayout = isDepthOrStencilFormat(attachment.format)
                                 ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                                 : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if (i < load_store_infos.size()) {
            attachment.loadOp = load_store_infos[i].load_op;
            attachment.storeOp = load_store_infos[i].store_op;
            attachment.stencilLoadOp = load_store_infos[i].load_op;
            attachment.stencilStoreOp = load_store_infos[i].store_op;
        }

        attachment_descriptions.push_back(std::move(attachment));
    }

    return attachment_descriptions;
}


//RenderPass::RenderPass(const ptr<Device> &device,
//                       const std::vector<VkAttachmentDescription> &attachmentDescs,
//                       const std::vector<VkSubpassDependency> &subpassDependencies,
//                       const std::vector<VkSubpassDescription> &subpassDesc) {
//    VkRenderPassCreateInfo renderPassInfo{};
//    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//    renderPassInfo.attachmentCount = attachmentDescs.size();
//    renderPassInfo.pAttachments = attachmentDescs.data();
//    renderPassInfo.subpassCount = subpassDesc.size();
//    renderPassInfo.pSubpasses = subpassDesc.data();
//    renderPassInfo.dependencyCount = subpassDependencies.size();
//    renderPassInfo.pDependencies = subpassDependencies.data();
//    VK_CHECK_RESULT(vkCreateRenderPass(device->getHandle(), &renderPassInfo, nullptr, &_pass))
//    device = device;
//}

RenderPass::~RenderPass() {
//    vkDestroyRenderPass(_device->getHandle(), _pass, nullptr);
//    _device.reset();
}

RenderPass::RenderPass(Device &device, const std::vector<Attachment> &attachments,
                       const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<SubpassInfo> &subpasses)
        : device(device) {

    std::vector<VkAttachmentDescription> attachmentDescriptions;
    //对于一个渲染pass来说 attachment说明了这个渲染过程中需要用的image资源
    for (int i = 0; i < attachments.size(); i++) {
        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = attachments[i].format;
        attachmentDescription.samples = attachments[i].samples;
        attachmentDescription.initialLayout = attachments[i].initial_layout;
        attachmentDescription.finalLayout = isDepthOrStencilFormat(attachments[i].format)
                                            ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                                            : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if (i < load_store_infos.size()) {
            attachmentDescription.loadOp = load_store_infos[i].load_op;
            attachmentDescription.storeOp = load_store_infos[i].store_op;
            attachmentDescription.stencilLoadOp = load_store_infos[i].load_op;
            attachmentDescription.stencilStoreOp = load_store_infos[i].store_op;
        }
        attachmentDescriptions.push_back(attachmentDescription);
    }

    auto subpass_count = std::max<size_t>(1, subpasses.size());


    std::vector<std::vector<VkAttachmentReference>> inputAttachments{subpass_count};
    std::vector<std::vector<VkAttachmentReference>> colorAttachments{subpass_count};
    std::vector<std::vector<VkAttachmentReference>> depthStencilAttachments{subpass_count};
    std::vector<std::vector<VkAttachmentReference>> colorResolveAttachments{subpass_count};
    std::vector<std::vector<VkAttachmentReference>> depthResolveAttachments{subpass_count};
    std::vector<VkSubpassDescription> subpassDescriptions;
    subpassDescriptions.reserve(subpass_count);
    if (subpasses.empty()) {
        VkSubpassDescription subpass_description{};
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        uint32_t default_depth_stencilAttachment{VK_ATTACHMENT_UNUSED};

        for (uint32_t k = 0U; k < (attachmentDescriptions.size()); ++k) {
            if (isDepthOrStencilFormat(attachments[k].format)) {
                if (default_depth_stencilAttachment == VK_ATTACHMENT_UNUSED) {
                    default_depth_stencilAttachment = k;
                }
                continue;
            }
            VkAttachmentReference reference{k, VK_IMAGE_LAYOUT_GENERAL};
            colorAttachments[0].push_back(reference);
        }

        subpass_description.pColorAttachments = colorAttachments[0].data();

        if (default_depth_stencilAttachment != VK_ATTACHMENT_UNUSED) {
            VkAttachmentReference reference{default_depth_stencilAttachment,
                                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

            depthStencilAttachments[0].push_back(reference);

            subpass_description.pDepthStencilAttachment = depthStencilAttachments[0].data();
        }

        subpassDescriptions.push_back(subpass_description);
    } else {
        for (int i = 0; i < subpasses.size(); i++) {
            auto &subPass = subpasses[i];
            for (auto &inputAttachment: subPass.inputAttachments) {
                auto defaultLayout = isDepthOrStencilFormat(attachmentDescriptions[inputAttachment].format)
                                     ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                                     : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                auto initialLayout =
                        attachments[inputAttachment].initial_layout == VK_IMAGE_LAYOUT_UNDEFINED ?
                        defaultLayout : attachments[inputAttachment].initial_layout;
                VkAttachmentReference ref{inputAttachment, initialLayout};
                inputAttachments[i].push_back(ref);
            }

            for (auto &outputAttachment: subPass.outputAttachments) {
                if (!isDepthOrStencilFormat(attachmentDescriptions[outputAttachment].format)) {
                    auto initialLayout = attachments[outputAttachment].initial_layout == VK_IMAGE_LAYOUT_UNDEFINED
                                         ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL :
                                         attachments[outputAttachment].initial_layout;
                    VkAttachmentReference ref{outputAttachment, initialLayout};
                    colorAttachments[i].push_back(ref);
                }
            }

            for (auto &colorResolveAttachment: subPass.colorResolveAttachments) {
                VkAttachmentReference ref{colorResolveAttachment, attachments[colorResolveAttachment].initial_layout};
                colorResolveAttachments[i].push_back(ref);
            }
            // 如果有深度测试
            if (!subPass.disableDepthStencilAttachment) {
                auto it = std::find_if(attachments.begin(), attachments.end(), [](const Attachment attachment) {
                    return isDepthOrStencilFormat(attachment.format);
                });
                if (it != attachments.end()) {
                    uint32_t iDepthStencil = std::distance(attachments.begin(), it);
                    auto initialLayout = it->initial_layout == VK_IMAGE_LAYOUT_UNDEFINED
                                         ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : it->initial_layout;
                    depthStencilAttachments[i].push_back(VkAttachmentReference{iDepthStencil, initialLayout});
                }
            }

            VkSubpassDescription subpassDescription{};

            subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            subpassDescription.inputAttachmentCount = inputAttachments[i].size();
            subpassDescription.pInputAttachments = inputAttachments[i].empty() ? nullptr : inputAttachments[i].data();

            subpassDescription.colorAttachmentCount = colorAttachments[i].size();
            subpassDescription.pColorAttachments = colorAttachments[i].empty() ? nullptr : colorAttachments[i].data();

//            subpassDescription. = colorAttachments[i].size();
            subpassDescription.pResolveAttachments = colorResolveAttachments[i].empty() ? nullptr
                                                                                        : colorResolveAttachments[i].data();

            subpassDescription.pDepthStencilAttachment = nullptr;

            if (!depthStencilAttachments[i].empty()) {
                subpassDescription.pDepthStencilAttachment = depthStencilAttachments[i].data();
            }

            subpassDescriptions.push_back(subpassDescription);
        }
    }
    setAttachmentLayouts(subpassDescriptions, attachments);
    auto subpassDependencies = getSubpassDependency(subpass_count);

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachmentDescriptions.size();
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = subpassDescriptions.size();
    renderPassInfo.pSubpasses = subpassDescriptions.data();
    renderPassInfo.dependencyCount = subpassDependencies.size();
    renderPassInfo.pDependencies = subpassDependencies.data();
    VK_CHECK_RESULT(vkCreateRenderPass(device.getHandle(), &renderPassInfo, nullptr, &_pass))


}


