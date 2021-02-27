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

#pragma once

#include "PipelineBuilder.hpp"
#include "ShaderFoundry.hpp"
#include "Renderpass.hpp"

namespace Vulcain {

class Pipeline {
 public:
    Pipeline(Renderpass* renderpass, const ShaderFoundry::Modules& modules) : _device(renderpass->swapchain()->device()) {
        _createLayout();
        _createPipeline(renderpass->swapchain(), renderpass, modules);
    }

    ~Pipeline() {
        vkDestroyPipeline(_device->get(), _pipeline, nullptr);
        vkDestroyPipelineLayout(_device->get(), _layout, nullptr);
    }
 
 private:
    VkPipeline _pipeline;
    VkPipelineLayout _layout;
    Device* _device = nullptr;

    void _createPipeline(Swapchain* swapchain, Renderpass* renderpass, const ShaderFoundry::Modules& modules) {
        //
        PipelineBuilder builder(swapchain);
        
        //
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = modules.size();
        pipelineInfo.pStages = modules.data();
        pipelineInfo.pVertexInputState = &builder.vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &builder.inputAssembly;
        pipelineInfo.pViewportState = &builder.viewportState;
        pipelineInfo.pRasterizationState = &builder.rasterizer;
        pipelineInfo.pMultisampleState = &builder.multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &builder.colorBlending;
        pipelineInfo.pDynamicState = nullptr; // Optional
        pipelineInfo.layout = _layout;
        pipelineInfo.renderPass = renderpass->get();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        //
        auto result = vkCreateGraphicsPipelines(_device->get(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline);
        assert(result == VK_SUCCESS);
    }
    
    void _createLayout() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        auto result = vkCreatePipelineLayout(_device->get(), &pipelineLayoutInfo, nullptr, &_layout);
        assert(result == VK_SUCCESS);
    }
};

}; // namespace Vulcain
