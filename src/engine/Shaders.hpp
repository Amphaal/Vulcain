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

#include <iostream>
#include <map>
#include <filesystem>

#include "Swapchain.hpp"

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(shadersResources);

namespace Vulcain {

class Shaders {
 public:
    using CreateInfoByStage = std::map<VkShaderStageFlagBits, VkPipelineShaderStageCreateInfo>;
    using ModulesByPipeline = std::map<std::string, CreateInfoByStage>;

    Shaders(Swapchain* swapchain) : _swapchain(swapchain) {
        _createShaderModules();
        // _createPipelines();
    }

    ~Shaders() {
        for(auto module : _modules) {
            vkDestroyShaderModule(_swapchain->device()->get(), module, nullptr);
        }
    }   

 private:
    static inline std::map<std::string, VkShaderStageFlagBits> STAGE_FROM_EXT {
        { ".vert", VK_SHADER_STAGE_VERTEX_BIT },
        { ".frag", VK_SHADER_STAGE_FRAGMENT_BIT }
    };
    
    void _createPipelines() {
        //
        for(auto& [shaderName, stages] : _pipelines) {
            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = 0;
            vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
            vertexInputInfo.vertexAttributeDescriptionCount = 0;
            vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float) _swapchain->imageExtent.width;
            viewport.height = (float) _swapchain->imageExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = _swapchain->imageExtent;

            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.pViewports = &viewport;
            viewportState.scissorCount = 1;
            viewportState.pScissors = &scissor;

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;
            rasterizer.depthBiasConstantFactor = 0.0f; // Optional
            rasterizer.depthBiasClamp = 0.0f; // Optional
            rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            multisampling.minSampleShading = 1.0f; // Optional
            multisampling.pSampleMask = nullptr; // Optional
            multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
            multisampling.alphaToOneEnable = VK_FALSE; // Optional

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo colorBlending{};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;
            colorBlending.blendConstants[0] = 0.0f; // Optional
            colorBlending.blendConstants[1] = 0.0f; // Optional
            colorBlending.blendConstants[2] = 0.0f; // Optional
            colorBlending.blendConstants[3] = 0.0f; // Optional

            VkDynamicState dynamicStates[] = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_LINE_WIDTH
            };

            VkPipelineDynamicStateCreateInfo dynamicState{};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = 2;
            dynamicState.pDynamicStates = dynamicStates;

            VkPipelineLayout pipelineLayout;
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 0; // Optional
            pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
            pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
            pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

            auto result = vkCreatePipelineLayout(_swapchain->device()->get(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
            assert(result == VK_SUCCESS);
            vkDestroyPipelineLayout(_swapchain->device()->get(), pipelineLayout, nullptr);

            // TODO end pipeline creation
        }
    }

    void _createShaderModules() {
        //
        auto fs = cmrc::shadersResources::get_filesystem();

        //
        for(const auto &i : fs.iterate_directory("/")) {
            //
            if(!i.is_file()) continue;

            //
            auto stem = std::filesystem::path(i.filename()).stem();
            auto p = std::filesystem::path(stem);
            auto name = p.stem().string();
            auto stage = p.extension().string();

            //
            auto find_stageFlag = STAGE_FROM_EXT.find(stage);
            assert(find_stageFlag != STAGE_FROM_EXT.end());
            auto stageFlag = find_stageFlag->second;

            // ensure pipeline exists
            if(_pipelines.find(name) == _pipelines.end()) {
                _pipelines.emplace(name, CreateInfoByStage{});
            }

            //
            auto pipeline = _pipelines.find(name);
            auto &stages = pipeline->second;

            //
            auto module = _createShaderModule(fs, i.filename());

            //
            VkPipelineShaderStageCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            createInfo.stage = stageFlag;
            createInfo.module = module;
            createInfo.pName = pipeline->first.c_str();

            //
            stages.emplace(stageFlag, createInfo);
        }
    }

    VkShaderModule _createShaderModule(cmrc::embedded_filesystem& fs, const std::string& path) {
        //
        auto shaderBinary = fs.open(path);

        //        
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderBinary.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBinary.begin());

        //
        VkShaderModule shaderModule;
        auto result = vkCreateShaderModule(_swapchain->device()->get(), &createInfo, nullptr, &shaderModule);
        assert(result == VK_SUCCESS);
        
        //
        _modules.push_back(shaderModule);

        //
        return shaderModule;
    }

    std::vector<VkShaderModule> _modules;
    ModulesByPipeline _pipelines;
    Swapchain* _swapchain = nullptr;
};

}; // namespace Vulcain
