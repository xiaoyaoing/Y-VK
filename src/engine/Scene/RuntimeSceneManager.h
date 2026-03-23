#pragma once
#include "Compoments/RenderPrimitive.h"

struct GltfMaterial;
class Scene;

class RuntimeSceneManager {
public:
    static  void addPrimitives(Scene& scene, std::vector<std::unique_ptr<Primitive>>&& primitives);
    static  void addPrimitive(Scene& scene, std::unique_ptr<Primitive>&& primitive);
    static  void addGltfMaterialsToScene(Scene& scene, std::vector<GltfMaterial>&& materials);
    static  void addSponzaRestirLight(Scene& scene);
    static  void addSponzaRestirPointLight(Scene& scene);
    static  void addPlane(Scene& scene);
};
