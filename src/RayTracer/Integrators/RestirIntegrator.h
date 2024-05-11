#include "Integrator.h"
class RestirIntegrator : public Integrator {
public:
    void render(RenderGraph& renderGraph) override;
    void onUpdateGUI() override;
};