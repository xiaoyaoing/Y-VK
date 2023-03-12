#include <Vulkan.h>
class Device;
class DescriptorPool;
class DescriptorLayout;
class Buffer;
class DescriptorSet{
    VkDescriptorSet  _descriptorSet;
    ptr<Device> _device;
public:
    VkDescriptorSet getHandle(){
        return _descriptorSet;
    }
    DescriptorSet(const ptr<Device>& device,
                  const ptr<DescriptorPool>& descriptorPool,
                  const ptr<DescriptorLayout>& descriptorSetLayout,
                  const uint32_t descSetCount);
    void updateBuffer(const std::vector<ptr<Buffer>>& buffers,
                      const uint32_t dstBinding,
                      const uint32_t descCount,
                      VkDescriptorType descType);

    void
    updateImage(const std::vector<VkDescriptorImageInfo> &imageInfos, const uint32_t dstBinding,
                VkDescriptorType descType);
};