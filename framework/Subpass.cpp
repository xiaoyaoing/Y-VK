//
// Created by pc on 2023/8/4.
//

#include "Subpass.h"
#include <Mesh.h>

void Subpass::updateRenderTargetAttachments(RenderTarget &renderTarget) {

}

void Subpass::draw(CommandBuffer &commandBuffer) {

}

void geomSubpass::draw(CommandBuffer &commandBuffer) {
    for (auto mesh: meshes) {
        mesh->bindOnly(commandBuffer);
        mesh->drawOnly(commandBuffer);
    }

}
