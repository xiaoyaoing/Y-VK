#include "Vulkan.h"
class PipelineBase{
public:
    inline VkPipeline getHandle(){return pipeline;}

    explicit PipelineBase(const VkPipeline pipeline) : pipeline(pipeline) {}
    void cleanup(){}
protected:
    VkPipeline  pipeline;
};