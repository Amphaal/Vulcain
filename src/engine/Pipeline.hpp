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

#include "helpers/PipelineBuilder.hpp"
#include "helpers/ShaderFoundry.hpp"
#include "Renderpass.hpp"
#include "DescriptorPool.hpp"

#include "buffers/UniformBuffers.hpp"
#include "buffers/UBO.hpp"

namespace Vulcain {

class Pipeline : public DeviceBound, public IRegenerable {
 public:
    Pipeline(Renderpass* renderpass, DescriptorPool* descrPool, const ShaderFoundry::Modules& modules) : 
        DeviceBound(renderpass), IRegenerable(descrPool), _swapchain(renderpass->swapchain()), _descrPool(descrPool), _uniformBuffers(descrPool) {
        _createDescriptorSetLayouts();
        _gen();
        _createPipelineLayout();
        _createPipeline(_swapchain, renderpass, modules);
    }

    operator VkPipeline() const { return _pipeline; }

    ~Pipeline() {
        vkDestroyPipeline(*_device, _pipeline, nullptr);
        vkDestroyPipelineLayout(*_device, _layout, nullptr);
        vkDestroyDescriptorSetLayout(*_device, _descriptorSetLayout, nullptr);
    }

    void updateUniformBuffer(uint32_t currentImage) {
        auto generated = UBO_MVP::generate(_swapchain);
        _uniformBuffers.mapToMemory(currentImage, generated);
    }

    VkPipelineLayout layout() const {
        return _layout;
    }

    const VkDescriptorSet* descriptorSet(uint32_t currentImage) const {
        return &_descriptorSets[currentImage];
    }
 
 private:
    VkPipeline _pipeline;
    VkPipelineLayout _layout;
    VkDescriptorSetLayout _descriptorSetLayout;

    DescriptorPool* _descrPool = nullptr;
    Swapchain* _swapchain = nullptr;
    std::vector<VkDescriptorSet> _descriptorSets;

    UniformBuffers<UBO_MVP> _uniformBuffers;

    void _degen() final {}
    void _gen() final {
        _createDescriptorSets();
    }

    void _createPipeline(Swapchain* swapchain, Renderpass* renderpass, const ShaderFoundry::Modules& modules) {
        //
        PipelineBuilder builder;
        
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
        pipelineInfo.pDynamicState = &builder.dynamicState; // Optional
        pipelineInfo.layout = _layout;
        pipelineInfo.renderPass = *renderpass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        //
        auto result = vkCreateGraphicsPipelines(*_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline);
        assert(result == VK_SUCCESS);
    }

    void _createDescriptorSetLayouts() {
        //
        auto uboLayoutBinding = UBO_MVP::createDescriptorSetLayout();

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        auto result = vkCreateDescriptorSetLayout(*_device, &layoutInfo, nullptr, &_descriptorSetLayout);
        assert(result == VK_SUCCESS);
    }

    void _createDescriptorSets() {
        auto imgsCount = _swapchain->imagesCount(); 
        //
        std::vector<VkDescriptorSetLayout> layouts(imgsCount, _descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = *_descrPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(imgsCount);
        allocInfo.pSetLayouts = layouts.data();

        _descriptorSets.resize(imgsCount);
        auto result = vkAllocateDescriptorSets(*_device, &allocInfo, _descriptorSets.data());
        assert(result == VK_SUCCESS);

        for (size_t i = 0; i < imgsCount; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = _uniformBuffers.buffer(i);
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UBO_MVP);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = _descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(*_device, 1, &descriptorWrite, 0, nullptr);
        }
    }

    void _createPipelineLayout() {
        //
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1; // Optional
        pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        auto result = vkCreatePipelineLayout(*_device, &pipelineLayoutInfo, nullptr, &_layout);
        assert(result == VK_SUCCESS);
    }
};

} // namespace Vulcain
