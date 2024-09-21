#include "ClipmapUpdatePolicy.h"
#include "VxgiCommon.h"
#include "Common/Log.h"

struct VxgiContext::VxgiContextImpl {
    Scene*              scene;
    std::vector<BBox>   bboxes;
    std::vector<ClipmapRegion> * clipmapRegions = nullptr;
    ClipmapUpdatePolicy clipmapUpdatePolicy;
};

VxgiContext::VxgiContext() {
    mImpl = new VxgiContextImpl();
}

VxgiContext& VxgiContext::getInstance() {
    static VxgiContext instance;
    return instance;
}

std::vector<BBox>& VxgiContext::getBBoxes() {
    return getInstance().mImpl->bboxes;
}

std::vector<ClipmapRegion>& VxgiContext::getClipmapRegions() {
    if (getInstance().mImpl->clipmapRegions == nullptr) {
       LOGE("Clipmap regions are not set")
    }
    return *getInstance().mImpl->clipmapRegions;
}
void VxgiContext::setClipmapRegions(std::vector<ClipmapRegion>& regions) {
    getInstance().mImpl->clipmapRegions = &regions;
}

ClipmapUpdatePolicy& VxgiContext::getClipmapUpdatePolicy() {
    return getInstance().mImpl->clipmapUpdatePolicy;
}