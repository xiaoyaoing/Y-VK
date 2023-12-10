#include "PipelineState.h"
#include "RenderPass.h"

/* Copyright (c) 2019-2021, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


bool operator==(const VkVertexInputAttributeDescription &lhs, const VkVertexInputAttributeDescription &rhs) {
    return std::tie(lhs.binding, lhs.format, lhs.location, lhs.offset) ==
           std::tie(rhs.binding, rhs.format, rhs.location, rhs.offset);
}

bool operator==(const VkVertexInputBindingDescription &lhs, const VkVertexInputBindingDescription &rhs) {
    return std::tie(lhs.binding, lhs.inputRate, lhs.stride) == std::tie(rhs.binding, rhs.inputRate, rhs.stride);
}

bool operator==(const ColorBlendAttachmentState &lhs, const ColorBlendAttachmentState &rhs) {
    return std::tie(lhs.alphaBlendOp, lhs.blendEnable, lhs.colorBlendOp, lhs.colorWriteMask, lhs.dstAlphaBlendFactor,
                    lhs.dstColorBlendFactor, lhs.srcAlphaBlendFactor, lhs.srcColorBlendFactor) ==
           std::tie(rhs.alphaBlendOp, rhs.blendEnable, rhs.colorBlendOp, rhs.colorWriteMask, rhs.dstAlphaBlendFactor,
                    rhs.dstColorBlendFactor, rhs.srcAlphaBlendFactor, rhs.srcColorBlendFactor);
}

bool operator!=(const StencilOpState &lhs, const StencilOpState &rhs) {
    return std::tie(lhs.compareOp, lhs.depthFailOp, lhs.failOp, lhs.passOp) !=
           std::tie(rhs.compareOp, rhs.depthFailOp, rhs.failOp, rhs.passOp);
}

bool operator!=(const VertexInputState &lhs, const VertexInputState &rhs) {
    return lhs.attributes != rhs.attributes || lhs.bindings != rhs.bindings;
}

bool operator!=(const InputAssemblyState &lhs, const InputAssemblyState &rhs) {
    return std::tie(lhs.primitiveRestartEnable, lhs.topology) != std::tie(rhs.primitiveRestartEnable, rhs.topology);
}

bool operator!=(const RasterizationState &lhs, const RasterizationState &rhs) {
    return std::tie(lhs.cullMode, lhs.depthBiasEnable, lhs.depthClampEnable, lhs.frontFace, lhs.frontFace,
                    lhs.polygonMode, lhs.rasterizerDiscardEnable) !=
           std::tie(rhs.cullMode, rhs.depthBiasEnable, rhs.depthClampEnable, rhs.frontFace, rhs.frontFace,
                    rhs.polygonMode, rhs.rasterizerDiscardEnable);
}

bool operator!=(const ViewportState &lhs, const ViewportState &rhs) {
    return lhs.viewportCount != rhs.viewportCount || lhs.scissorCount != rhs.scissorCount;
}

bool operator!=(const MultisampleState &lhs, const MultisampleState &rhs) {
    return std::tie(lhs.alphaToCoverageEnable, lhs.alphaToOneEnable, lhs.minSampleShading, lhs.rasterizationSamples,
                    lhs.sampleMask, lhs.sampleShadingEnable) !=
           std::tie(rhs.alphaToCoverageEnable, rhs.alphaToOneEnable, rhs.minSampleShading, rhs.rasterizationSamples,
                    rhs.sampleMask, rhs.sampleShadingEnable);
}

bool operator!=(const DepthStencilState &lhs, const DepthStencilState &rhs) {
    return std::tie(lhs.depthBoundsTestEnable, lhs.depthCompareOp, lhs.depthTestEnable, lhs.depthWriteEnable,
                    lhs.stencilTestEnable) !=
           std::tie(rhs.depthBoundsTestEnable, rhs.depthCompareOp, rhs.depthTestEnable, rhs.depthWriteEnable,
                    rhs.stencilTestEnable) ||
           lhs.back != rhs.back || lhs.front != rhs.front;
}

bool operator!=(const ColorBlendState &lhs, const ColorBlendState &rhs) {
    return std::tie(lhs.logicOp, lhs.logicOpEnable) != std::tie(rhs.logicOp, rhs.logicOpEnable) ||
           lhs.attachments.size() != rhs.attachments.size() ||
           !std::equal(lhs.attachments.begin(), lhs.attachments.end(), rhs.attachments.begin(),
                       [](const ColorBlendAttachmentState &lhs, const ColorBlendAttachmentState &rhs) {
                           return lhs == rhs;
                       });
}

void PipelineState::reset() {
    clearDirty();

    pipelineLayout = nullptr;

    renderPass = nullptr;

    //specializationConstantState.reset();

    vertexInputState = {};

    inputAssemblyState = {};

    rasterizationState = {};

    multisampleState = {};

    depthStencilState = {};

    colorBlendState = {};

    subpassIndex = {0U};
}

PipelineState& PipelineState::setPipelineLayout(PipelineLayout& newPipelineLayout) {
    if (pipelineLayout && pipelineLayout->getHandle() != newPipelineLayout.getHandle()) {
        pipelineLayout = &newPipelineLayout;
        dirty = true;
    } else if (!pipelineLayout) {
        pipelineLayout = &newPipelineLayout;
        dirty = true;
    }
    return *this;
}

PipelineState& PipelineState::setRenderPass(const RenderPass& newRenderPass) {
    if (renderPass && renderPass->getHandle() != newRenderPass.getHandle()) {
        renderPass = &newRenderPass;
        dirty = true;
    } else if (!renderPass) {
        renderPass = &newRenderPass;
        dirty = true;
    }
    return *this;
}

PipelineState& PipelineState::setSpecializationConstant(uint32_t constantId, const std::vector<uint8_t>& data) {
    // specializationConstantState.setConstant(constantId, data);
    // if (specializationConstantState.isDirty()) {
    //     dirty = true;
    // }
    // return *this;
    return *this;
}

PipelineState& PipelineState::setVertexInputState(const VertexInputState& newVertexInputState) {
    if (vertexInputState != newVertexInputState) {
        vertexInputState = newVertexInputState;
        dirty = true;
    }
    return *this;
}

PipelineState& PipelineState::setInputAssemblyState(const InputAssemblyState& newInputAssemblyState) {
    if (inputAssemblyState != newInputAssemblyState) {
        inputAssemblyState = newInputAssemblyState;
        dirty = true;
    }
    return *this;
}

PipelineState& PipelineState::setRasterizationState(const RasterizationState& newRasterizationState) {
    if (rasterizationState != newRasterizationState) {
        rasterizationState = newRasterizationState;
        dirty = true;
    }
    return *this;
}

PipelineState& PipelineState::setViewportState(const ViewportState& newViewportState) {
    if (viewportState != newViewportState) {
        viewportState = newViewportState;
        dirty = true;
    }
    return *this;
}

PipelineState& PipelineState::setMultisampleState(const MultisampleState& newMultisampleState) {
    if (multisampleState != newMultisampleState) {
        multisampleState = newMultisampleState;
        dirty = true;
    }
    return *this;
}

PipelineState& PipelineState::setDepthStencilState(const DepthStencilState& newDepthStencilState) {
    if (depthStencilState != newDepthStencilState) {
        depthStencilState = newDepthStencilState;
        dirty = true;
    }
    return *this;
}

PipelineState& PipelineState::setColorBlendState(const ColorBlendState& newColorBlendState) {
    if (colorBlendState != newColorBlendState) {
        colorBlendState = newColorBlendState;
        dirty = true;
    }
    return *this;
}

PipelineState& PipelineState::setSubpassIndex(uint32_t newSubpassIndex) {
    if (subpassIndex != newSubpassIndex) {
        subpassIndex = newSubpassIndex;
        dirty = true;
    }
    return *this;
}

PipelineState& PipelineState::setPipelineType(PIPELINE_TYPE pipelineType_) {
    pipelineType = pipelineType_;
    return *this;
}


const PipelineLayout &PipelineState::getPipelineLayout() const {
    assert(pipelineLayout && "Graphics state Pipeline layout is not set");
    return *pipelineLayout;
}

const RenderPass *PipelineState::getRenderPass() const {
    return renderPass;
}

const SpecializationConstantState &PipelineState::getSpecializationConstantState() const {
    return specializationConstantState;
}

const VertexInputState &PipelineState::getVertexInputState() const {
    return vertexInputState;
}

const InputAssemblyState &PipelineState::getInputAssemblyState() const {
    return inputAssemblyState;
}

const RasterizationState &PipelineState::getRasterizationState() const {
    return rasterizationState;
}

const ViewportState &PipelineState::getViewportState() const {
    return viewportState;
}

const MultisampleState &PipelineState::getMultisampleState() const {
    return multisampleState;
}

const DepthStencilState &PipelineState::getDepthStencilState() const {
    return depthStencilState;
}

const ColorBlendState &PipelineState::getColorBlendState() const {
    return colorBlendState;
}

uint32_t PipelineState::getSubpassIndex() const {
    return subpassIndex;
}

PIPELINE_TYPE PipelineState::getPipelineType() const
{
    return pipelineType;
}

bool PipelineState::isDirty() const {
    return dirty;
}


PipelineState & PipelineState::clearDirty() {
    //todo
    return *this;
}

const RTPipelineSettings &  PipelineState::getrTPipelineSettings() const
{
    return rTPipelineSettings;
}

PipelineState & PipelineState::setrTPipelineSettings(const RTPipelineSettings& rTPipelineSettings_)
{
    this->rTPipelineSettings = rTPipelineSettings_;
    return *this;
}

	



