#include "RenderPassBase.h"
void RenderPtrManangr::destroy() {
    delete g_manager;
}

View* RenderPtrManangr::getView() {
    return fetchPtr<View>("view");
}
void RenderPtrManangr::init() {
    g_manager = new RenderPtrManangr();
}