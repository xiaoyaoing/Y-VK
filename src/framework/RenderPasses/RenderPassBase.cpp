#include "RenderPassBase.h"
void RenderPtrManangr::destroy() {
    delete g_manager;
}

void RenderPtrManangr::init() {
    g_manager = new RenderPtrManangr();
}