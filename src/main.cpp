// Vulcain
// Toy project for Vulkan oriented graphics
// Copyright (C) 2021 Guillaume Vara <guillaume.vara@gmail.com>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GPL.

#include "engine/Vulcain.h"

#include "engine/Renderer.h"
#include "engine/Pipeline.hpp"
#include "engine/DevicePicker.hpp"

#include "engine/StaticBuffer.hpp"
#include "engine/UniformBuffers.hpp"
#include "engine/DescriptorPool.hpp"

using namespace Vulcain;

int main() {
    #ifdef USES_VOLK
    auto result = volkInitialize();
    assert(result == VK_SUCCESS);
    #endif
    
    GlfwWindow window;

    auto appInfo = info("Hello Triangle");
    InstanceCreateInfo createInfo(&appInfo);
    Instance instance(&createInfo);
    Surface surface(&window, &instance);
    auto device = DevicePicker::getBestDevice(&surface);
    
    ShaderFoundry foundry(&device);

    Swapchain swapchain(&device);
    Renderpass renderpass(&swapchain);
    auto basicPipeline = Pipeline { &renderpass, foundry.modulesFromShaderName("basic") };
    
    ImageViews views(&renderpass);
    DescriptorPool descrPool(&views);
    UniformBuffers<UBO_MVP> ubo(&descrPool);

    CommandPool cmdPool(&views);

    StaticBuffer<Vertex> vertexes(&cmdPool, {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    });

    StaticIndexBuffer indexes(&cmdPool, {
        0, 1, 2,
        2, 3, 0
    });

    cmdPool.record([&basicPipeline, &vertexes, &indexes](VkCommandBuffer cmdBuf) {
        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, basicPipeline.get());
        
        VkBuffer vertexBuffers[] = {vertexes.buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmdBuf, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(cmdBuf, indexes.buffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(cmdBuf, indexes.vertexCount(), 1, 0, 0, 0);
    });

    Renderer renderer(&cmdPool, &window);
    renderer.bindUniformBufferUpdater([&ubo](uint32_t currentImage) {
        ubo.updateUniformBuffer(currentImage);
    });

    window.pollEventsAndDraw();

    return 0;
}