#pragma once
#include "Buffer.h"
#include "Scene/Compoments/Camera.h"
#include "Scene/Compoments/RenderPrimitive.h"

class Scene;
class View {
public:
    View(Device& device);
    void setScene(const Scene* scene);
    void setCamera(const Camera* camera);
    void bindViewBuffer();

    const Camera*                 getMCamera() const;
    void                          setMCamera(const Camera* const mCamera);
    std::vector<const Primitive*> getMVisiblePrimitives() const;
    void                          setMVisiblePrimitives(const std::vector<const Primitive*>& mVisiblePrimitives);

protected:
    const Scene* mScene{nullptr};

protected:
    const Camera*                 mCamera{nullptr};
    std::vector<const Primitive*> mVisiblePrimitives;

    std::unique_ptr<Buffer> mPerViewBuffer;
    std::unique_ptr<Buffer> mLightBuffer;
};