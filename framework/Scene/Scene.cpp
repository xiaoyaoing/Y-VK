#include "Scene.h"

void Scene::IteratePrimitives(PrimitiveCallBack primitiveCallBack) const
{
    for (const auto& prim : primitives)
    {
        primitiveCallBack(*prim);
    }
}

Scene::Scene(std::vector<std::unique_ptr<Primitive>>&& primitives, std::vector<Texture>&& textures,
             std::vector<Material> materials, std::vector<Light> lights): materials(std::move(materials)),
                                                                          lights(std::move(lights)),
                                                                          primitives(std::move(primitives)),
                                                                          textures(std::move(textures))
{
}


bool Primitive::getVertexAttribute(const std::string& name, VertexAttribute& attribute) const
{
    if (vertexAttributes.contains(name))
    {
        attribute = vertexAttributes.at(name);
        return true;
    }
    return false;
}

void Primitive::setVertxAttribute(const std::string& name, VertexAttribute& attribute)
{
    vertexAttributes.emplace(name, attribute);
}

void Primitive::setVertexBuffer(const std::string& name, std::unique_ptr<Buffer>& buffer)
{
    vertexBuffers.emplace(name, std::move(buffer));
}

Buffer& Primitive::getVertexBuffer(const std::string& name) const
{
    if (vertexBuffers.contains(name))
        return *vertexBuffers.at(name);
    LOGE("Primitive has no such buffer {}", name);
}
