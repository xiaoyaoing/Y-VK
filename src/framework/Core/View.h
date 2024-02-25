#pragma once
#include "Buffer.h"
#include "Scene/Compoments/Camera.h"
#include "Scene/Compoments/RenderPrimitive.h"

struct GltfMaterial;
struct Texture;
class Scene;
class View {
public:
    View(Device& device);
    void setScene(const Scene* scene);
    void setCamera(const Camera* camera);
    View & bindViewBuffer();
    View & bindViewShading();

    const Camera*                 getMCamera() const;
    void                          setMCamera(const Camera* const mCamera);
    std::vector<const Primitive*> getMVisiblePrimitives() const;
    void                          setMVisiblePrimitives(const std::vector<const Primitive*>& mVisiblePrimitives);

    std::vector<const Texture*>      GetMTextures() const;
    std::vector< GltfMaterial> GetMMaterials() const;

protected:
    const Scene* mScene{nullptr};

protected:
    const Camera*                 mCamera{nullptr};
    std::vector<const Primitive*> mVisiblePrimitives;
    std::vector<const Texture *> mTextures;
    std::vector<GltfMaterial > mMaterials;
    std::unique_ptr<Buffer> mPerViewBuffer;
    std::unique_ptr<Buffer> mLightBuffer;
};