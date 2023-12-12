#include  "ComputePipelineTest.h"

#include <random>

#include "Common/VkCommon.h"
#	define PARTICLES_PER_ATTRACTOR 4 * 1024

Example::Example()
{
    // addDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    
}

void Example::prepare()
{
    
    
    Application::prepare();

    camera->setPerspective(60.0f, static_cast<float>(width) / static_cast<float>(height), 512.0f, 0.1f);
    camera->setRotation(glm::vec3(-26.0f, 75.0f, 0.0f));
    camera->setTranslation(glm::vec3(0.0f, 0.0f, -14.0f));

    graphics.particle = Texture::loadTexture(*device,FileUtils::getResourcePath("textures/particle_rgba.ktx"));
    graphics.gradient = Texture::loadTexture(*device,FileUtils::getResourcePath("textures/particle_gradient_rgba.ktx"));

    std::vector<glm::vec3> attractors = {
        glm::vec3(5.0f, 0.0f, 0.0f),
        glm::vec3(-5.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 5.0f),
        glm::vec3(0.0f, 0.0f, -5.0f),
        glm::vec3(0.0f, 4.0f, 0.0f),
        glm::vec3(0.0f, -8.0f, 0.0f),
    };
    num_particles = static_cast<uint32_t>(attractors.size()) * PARTICLES_PER_ATTRACTOR;

    // Initial particle positions
    std::vector<Particle> particle_buffer(num_particles);

    std::default_random_engine      rnd_engine(lock_simulation_speed ? 0 : static_cast<unsigned>(time(nullptr)));
    std::normal_distribution<float> rnd_distribution(0.0f, 1.0f);


    for (uint32_t i = 0; i < static_cast<uint32_t>(attractors.size()); i++)
    {
        for (uint32_t j = 0; j < PARTICLES_PER_ATTRACTOR; j++)
        {
            Particle &particle = particle_buffer[i * PARTICLES_PER_ATTRACTOR + j];

            // First particle in group as heavy center of gravity
            if (j == 0)
            {
                particle.pos = glm::vec4(attractors[i] * 1.5f, 90000.0f);
                particle.vel = glm::vec4(glm::vec4(0.0f));
            }
            else
            {
                // Position
                glm::vec3 position(attractors[i] + glm::vec3(rnd_distribution(rnd_engine), rnd_distribution(rnd_engine), rnd_distribution(rnd_engine)) * 0.75f);
                float     len = glm::length(glm::normalize(position - attractors[i]));
                position.y *= 2.0f - (len * len);

                // Velocity
                glm::vec3 angular  = glm::vec3(0.5f, 1.5f, 0.5f) * (((i % 2) == 0) ? 1.0f : -1.0f);
                glm::vec3 velocity = glm::cross((position - attractors[i]), angular) + glm::vec3(rnd_distribution(rnd_engine), rnd_distribution(rnd_engine), rnd_distribution(rnd_engine) * 0.025f);

                float mass   = (rnd_distribution(rnd_engine) * 0.5f + 0.5f) * 75.0f;
                particle.pos = glm::vec4(position, mass);
                particle.vel = glm::vec4(velocity, 0.0f);
            }

            // Color gradient offset
            particle.vel.w = static_cast<float>(i) * 1.0f / static_cast<uint32_t>(attractors.size());
        }
    }

    storageBuffer = std::make_unique<Buffer>(*device,sizeof(Particle) * num_particles,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VMA_MEMORY_USAGE_GPU_ONLY);

    auto stagingBuffer = std::make_unique<Buffer>(*device,sizeof(Particle) * num_particles,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VMA_MEMORY_USAGE_CPU_ONLY,particle_buffer.data());

    renderContext->copyBuffer(*stagingBuffer,*storageBuffer);
    
    computeIntegrateLayout = std::make_unique<PipelineLayout>(*device,std::vector<std::string>{"compute_nbody/particle_integrate.comp"});
    computeCalculateLayout = std::make_unique<PipelineLayout>(*device,std::vector<std::string>{"compute_nbody/particle_calculate.comp"});
    graphics.pipelineLayout  = std::make_unique<PipelineLayout>(*device,std::vector<std::string>{ "compute_nbody/particle.vert", "compute_nbody/particle.frag"});

    graphics.uniformBuffer = std::make_unique<Buffer>(*device,sizeof(graphics.ubo),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU);
    uniformBuffer = std::make_unique<Buffer>(*device,sizeof(ubo),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU);
}

void Example::drawFrame(RenderGraph& rg)
{
    graphics.ubo.projection = camera->matrices.perspective;
    graphics.ubo.view       = camera->matrices.view;
    graphics.ubo.screenDim  = glm::vec2(static_cast<float>(width), static_cast<float>(height));
    graphics.uniformBuffer->uploadData(&graphics.ubo,sizeof(graphics.ubo));

    ubo.delta_time = deltaTime;
    ubo.particle_count =    num_particles;
    uniformBuffer->uploadData(&ubo,sizeof(ubo));


    ColorBlendAttachmentState state{.blendEnable = VK_TRUE,
                                    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                                    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
                                    .colorBlendOp = VK_BLEND_OP_ADD,
                                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA,
                                    .alphaBlendOp = VK_BLEND_OP_ADD,
                                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    rg.addComputePass("Compute Pass",[&](auto & builder,auto & settings)
    {
        
        auto bufferHandle = rg.importBuffer("storageBuffer",storageBuffer.get());
        builder.writeBuffer(bufferHandle);
        rg.getBlackBoard().put("storageBuffer",bufferHandle);
    }, [&](auto & context)
    {
        auto &  commandBuffer = context.commandBuffer;
        renderContext->getPipelineState().setPipelineLayout(*computeCalculateLayout);
        renderContext->bindBuffer(0,rg.getBlackBoard().getBuffer("storageBuffer")).bindBuffer(1,*uniformBuffer);
        renderContext->dispath(commandBuffer,num_particles/128,1,1);
        renderContext->getPipelineState().setPipelineLayout(*computeIntegrateLayout);
        renderContext->dispath(commandBuffer,num_particles/128,1,1);
        renderContext->clearPassResources();
    });

    rg.addPass("Graphics Pass",
        [&](auto & builder,auto & settings  )
    {
        auto handle = rg.getBlackBoard().getHandle(SWAPCHAIN_IMAGE_NAME);
        auto depth = rg.createTexture("depth", {
                                       .extent = renderContext->getSwapChainExtent(),
                                       .useage = TextureUsage::SUBPASS_INPUT |
                                                 TextureUsage::DEPTH_ATTACHMENT

                               });        builder.writeTexture(handle);
        builder.readBuffer(rg.getBlackBoard().getHandle("storageBuffer"));
        RenderGraphPassDescriptor desc{};
        desc.textures = {handle,depth};
        desc.subpasses = {{.outputAttachments = {handle,depth}}};
        builder.declare("",desc);
    },[&](auto & context)
    {
        auto & commandBuffer = context.commandBuffer;
        renderContext->getPipelineState().setPipelineLayout(*graphics.pipelineLayout).setInputAssemblyState({.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST}).setVertexInputState(
            { .bindings =  {vkCommon::initializers::vertexInputBindingDescription(0, sizeof(Particle), VK_VERTEX_INPUT_RATE_VERTEX)},
                .attributes = {
                vkCommon::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Particle, pos)),
                vkCommon::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Particle, vel))}
            }).setDepthStencilState({.depthTestEnable =  false}).setRasterizationState({.cullMode = VK_CULL_MODE_NONE}).setColorBlendState({.attachments =  {state}})
        ;
        renderContext->bindImage(0,graphics.particle->getImage().getVkImageView(),graphics.particle->getSampler(),0,0)
        .bindImage(0,graphics.gradient->getImage().getVkImageView(),graphics.gradient->getSampler(),1,0)
        .bindBuffer(2,*graphics.uniformBuffer);
        commandBuffer.bindVertexBuffer( rg.getBlackBoard().getBuffer("storageBuffer"));
        renderContext->flushAndDraw(commandBuffer,num_particles,1,0,0);
    });
    if(enableGui)
        gui->addGuiPass(rg);
    rg.execute(renderContext->getGraphicBuffer());
    
}

int main(int argc, char* argv[])
{
    Example example;
    example.prepare();
    example.mainloop();
    return 0;
}

