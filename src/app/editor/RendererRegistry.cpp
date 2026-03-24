#include "RendererRegistry.h"

#include "EditorApplication.h"
#include "render_modules/pbr/PbrRenderer.h"
#include "render_modules/raytracing/renderer/RayTracingEditorRenderer.h"
#include "render_modules/vxgi/VxgiRenderer.h"

namespace {

const std::vector<RendererDescriptor> kRendererRegistry = {
    {"pbr", "PBR and IBL renderer",
     [](EditorApplication& editor) {
         editor.requireDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
     },
     [] { return std::make_unique<PbrRenderer>(); }},
    {"vxgi", "Voxel cone tracing renderer",
     [](EditorApplication& editor) {
         editor.requireDeviceExtension(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
         editor.requireInstanceExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
     },
     [] { return std::make_unique<VxgiRenderer>(); }},
    {"pt", "Path tracing renderer",
     [](EditorApplication& editor) {
         editor.requireDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
     },
     [] { return std::make_unique<PathTracingEditorRenderer>(); }},
    {"ddgi", "DDGI ray tracing renderer",
     [](EditorApplication& editor) {
         editor.requireDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
     },
     [] { return std::make_unique<DDGIEditorRenderer>(); }},
    {"restir", "ReSTIR DI ray tracing renderer",
     [](EditorApplication& editor) {
         editor.requireDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
         editor.requireDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
     },
     [] { return std::make_unique<RestirDIEditorRenderer>(); }},
};

} // namespace

const std::vector<RendererDescriptor>& getRendererRegistry() {
    return kRendererRegistry;
}
