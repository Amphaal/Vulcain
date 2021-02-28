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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

int main() {
    #ifdef USES_VOLK
    auto result = volkInitialize();
    assert(result == VK_SUCCESS);
    #endif
    
    Vulcain::GlfwWindow window;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Vulcain";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    Vulcain::InstanceCreateInfo createInfo(&appInfo);
    Vulcain::Instance instance(&createInfo);
    Vulcain::Surface surface(&window, &instance);
    auto device = Vulcain::DevicePicker::getBestDevice(&surface);
    
    Vulcain::ShaderFoundry foundry(&device);

    Vulcain::Swapchain swapchain(&device);
    Vulcain::Renderpass renderpass(&swapchain);
    Vulcain::ImageViews views(&renderpass);
    Vulcain::CommandPool pool(&views);
    
    auto basicPipeline = Vulcain::Pipeline { &renderpass, foundry.modulesFromShaderName("basic") };

    pool.record([&basicPipeline](VkCommandBuffer cmdBuf) {
        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, basicPipeline.get());
        vkCmdDraw(cmdBuf, 3, 1, 0, 0);
    });

    Vulcain::Renderer renderer(&pool, &window);
    window.poolEventsAndDraw();

    return 0;
}