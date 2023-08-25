//
// Created by pc on 2023/8/23.
//

#include <Scene/SgImage.h>

#pragma once
namespace sg {
    class TtfImage : public SgImage {
    public:
        TtfImage(const std::string &path);
    };
}
