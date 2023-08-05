//
// Created by 打工人 on 2023/3/28.
//

#include "RenderTarget.h"

const std::vector<int> &RenderTarget::getInAttachment() const {
    return inAttachment;
}

void RenderTarget::setInAttachment(const std::vector<int> &inAttachment) {
    RenderTarget::inAttachment = inAttachment;
}

const std::vector<int> &RenderTarget::getOutAttachment() const {
    return outAttachment;
}

void RenderTarget::setOutAttachment(const std::vector<int> &outAttachment) {
    RenderTarget::outAttachment = outAttachment;
}

RenderTarget::RenderTarget(std::vector<Image> &&images): _images(std::move(images)){
    _extent = VkExtent2D{_images.back().getExtent().width,_images.back().getExtent().height};
    for( auto & image:_images){
        _views.emplace_back(image,VK_IMAGE_VIEW_TYPE_2D);
        _attachments.emplace_back(Attachment{image.getFormat(), image.getSampleCount(), image.getUseFlags()});
    }
}

RenderTarget::RenderTarget(std::vector<ImageView> &&imageViews):_views(std::move(imageViews)) {

}

const std::vector<Image> &RenderTarget::getImages() const {
    return _images;
}

const std::vector<ImageView> &RenderTarget::getViews() const {
    return _views;
}
