#include "App/Application.h"

class Example : public  Application
{

protected:
    void drawFrame(RenderGraph& renderGraph, CommandBuffer& commandBuffer) override;
};