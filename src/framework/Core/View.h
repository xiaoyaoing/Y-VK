#pragma once
#include "Buffer.h"
// #include "RenderContext.h"
#include "Scene/Compoments/Camera.h"
#include "Scene/Compoments/RenderPrimitive.h"
#include "Scene/Compoments/SgLight.h"

class CommandBuffer;
struct GltfMaterial;
class Scene;
class Texture;
class ImageView;
class Sampler;

class View {
public:
    View(Device& device);
    void  setScene(const Scene* scene);
    void  setCamera(const Camera* camera);
    View& bindViewBuffer();
    View& bindViewBuffer(const SgLight & light);
    View& bindViewShading();
    int   bindTexture(const ImageView& imageView, const Sampler& sampler);
    View& bindViewGeom(CommandBuffer& commandBuffer);

    void drawPrimitives(CommandBuffer& commandBuffer);

    using PrimitiveSelectFunc = std::function<bool(const Primitive& primitive)>;
    void drawPrimitives(CommandBuffer& commandBuffer, const PrimitiveSelectFunc& selectFunc);

    void drawPrimitivesUseSeparateBuffers(CommandBuffer& commandBuffer);
    void setLightDirty(bool dirty) { lightDirty = dirty; }
    const Camera*                 getCamera() const;
    std::vector<const Primitive*> getMVisiblePrimitives() const;
    void                          setMVisiblePrimitives(const std::vector<const Primitive*>& mVisiblePrimitives);

    std::vector<GltfMaterial>   GetMMaterials() const;

protected:
    struct PerViewUnifom {
        glm::mat4 view_proj;
        glm::mat4 inv_view_proj;
        glm::mat4 proj;
        glm::mat4 view;
        glm::mat4 inv_proj;
        glm::mat4 inv_view;

        glm::vec3 camera_pos;
        uint32_t  light_count;

        glm::ivec2 resolution;
        glm::ivec2 inv_resolution;

        bool operator!=(const PerViewUnifom& other) const {
            return memcmp(this, &other, sizeof(PerViewUnifom)) != 0;
        }
    };

protected:

    void updateLight();
    
    const Camera*                 mCamera{nullptr};
    std::vector<const Primitive*> mVisiblePrimitives;
    std::vector<const ImageView *> mImageViews;
    std::vector<const Sampler *>  mSamplers;
    std::vector<GltfMaterial>     mMaterials;
    const Scene*                  mScene{nullptr};
    std::unique_ptr<Buffer>       mPerViewBuffer;
    PerViewUnifom                 mPerViewUniform;
    std::unique_ptr<Buffer>       mLightBuffer;

    bool lightDirty{false};
};