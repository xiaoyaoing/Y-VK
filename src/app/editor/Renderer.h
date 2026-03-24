#pragma once

#include "RenderGraph/BlackBoard.h"
#include "Scene/SceneLoader/SceneLoadingConfig.h"

#include <memory>
#include <string>
#include <string_view>

class EditorApplication;
class RenderGraph;

class EditorRenderer {
public:
    virtual ~EditorRenderer() = default;

    virtual std::string_view getName() const = 0;
    virtual void initialize(EditorApplication& editor) = 0;
    virtual void onActivated(EditorApplication& editor) = 0;
    virtual void onSceneLoaded(EditorApplication& editor) = 0;
    virtual void render(EditorApplication& editor, RenderGraph& renderGraph) = 0;
    virtual void updateGui(EditorApplication& editor) = 0;

    virtual std::string getLdrImageToSave() const { return RENDER_VIEW_PORT_IMAGE_NAME; }
    virtual std::string getHdrImageToSave() const { return getLdrImageToSave(); }
};
