#pragma once
#include "Compoments/RenderPrimitive.h"

struct GltfMaterial;
class Scene;

class RuntimeSceneManager {
public:
    static  void addPrimitives(Scene& scene, std::vector<std::unique_ptr<Primitive>>&& primitives);
    static  void addGltfMaterialsToScene(Scene& scene, std::vector<GltfMaterial>&& materials);
    static  void addSponzaRestirLight(Scene& scene);
    static  void addSponzaRestirPointLight(Scene& scene);
};
