#include "Scene.h"

void Scene::IteratePrimitives(PrimitiveCallBack primitiveCallBack) const {
    for (const auto& prim : primitives) {
        primitiveCallBack(*prim);
    }
}

Scene::Scene(std::vector<std::unique_ptr<Primitive>>&& primitives, std::vector<std::unique_ptr<Texture>>&& textures, std::vector<Material>&& materials, std::vector<SgLight>&& lights, std::vector<std::shared_ptr<Camera>>&& cameras) : materials(std::move(materials)),
                                                                                                                                                                                                                                         lights(std::move(lights)),
                                                                                                                                                                                                                                         primitives(std::move(primitives)),
                                                                                                                                                                                                                                         textures(std::move(textures)),
                                                                                                                                                                                                                                         cameras(std::move(cameras)) {
}

void Scene::addLight(const SgLight& light) {
    lights.emplace_back(light);
}

const std::vector<SgLight>& Scene::getLights() const {
    return lights;
}

const std::vector<std::unique_ptr<Primitive>>& Scene::getPrimitives() const {
    return primitives;
}

const std::vector<std::unique_ptr<Texture>>& Scene::getTextures() const {
    return textures;
}

const std::vector<Material>& Scene::getMaterials() const {
    return materials;
}

std::vector<std::shared_ptr<Camera>>& Scene::getCameras() {
    return cameras;
}

bool Primitive::getVertexAttribute(const std::string& name, VertexAttribute& attribute) const {
    if (vertexAttributes.contains(name)) {
        attribute = vertexAttributes.at(name);
        return true;
    }
    return false;
}

void Primitive::setVertxAttribute(const std::string& name, VertexAttribute& attribute) {
    vertexAttributes.emplace(name, attribute);
}

void Primitive::setVertexBuffer(const std::string& name, std::unique_ptr<Buffer>& buffer) {
    vertexBuffers.emplace(name, std::move(buffer));
}

Buffer& Primitive::getVertexBuffer(const std::string& name) const {
    if (vertexBuffers.contains(name))
        return *vertexBuffers.at(name);
    LOGE("Primitive has no such buffer {}", name);
}