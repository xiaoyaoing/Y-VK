////
//// Created by pc on 2023/8/23.
////
//
//#include "TtfImage.h"
//
//#include <imgui.h>
//
//sg::TtfImage::TtfImage(const std::string &path) {
//    assert(FileUtils::getFileExt(path) == "ttf");
//    ImGuiIO &io = ImGui::GetIO();
//    io.Fonts->AddFontFromFileTTF(path.c_str(), 16.f);
//
//    unsigned char *fontData;
//    int texWidth, texHeight;
//    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
//
//    data = std::vector<uint8_t>(fontData, fontData + texHeight * texWidth * 4);
//    format = VK_FORMAT_R8G8B8A8_UNORM;
//    extent3D = {toUint32(texWidth), toUint32(texHeight), 1};
//}
