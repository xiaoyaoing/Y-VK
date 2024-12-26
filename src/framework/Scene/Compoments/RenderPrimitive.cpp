//
// Created by pc on 2023/11/29.
//

#include "RenderPrimitive.h"
void Primitive::setDimensions(glm::vec3 min, glm::vec3 max) {
    dimensions = BBox(min, max);
}
void Primitive::setDimensions(const BBox& box) {
    dimensions = box;
}

const BBox& Primitive::getDimensions() const {
    return dimensions;
}
