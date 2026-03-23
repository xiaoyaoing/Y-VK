#pragma once
#include "App/Application.h"

class TinyNanite : public Application {
    public:
    void drawFrame(RenderGraph& rg) override;
    void prepare() override;
    void onUpdateGUI() override;
protected:
    std::unique_ptr<Primitive> mPrimitive;
};