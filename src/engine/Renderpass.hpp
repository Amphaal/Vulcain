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

#include "Swapchain.hpp"
#include "IRegenerable.hpp"

namespace Vulcain {

class Renderpass : public IRegenerable {
 public:
    Renderpass(Swapchain* swapchain) : IRegenerable(swapchain), _swapchain(swapchain) {
        //
        _colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        _colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        _colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        _colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        _colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        _colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        _colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        _colorAttachmentRef.attachment = 0;
        _colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        _subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        _subpass.colorAttachmentCount = 1;
        _subpass.pColorAttachments = &_colorAttachmentRef;

        _dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        _dependency.dstSubpass = 0;
        _dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        _dependency.srcAccessMask = 0;
        _dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        _dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        _renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        _renderPassInfo.attachmentCount = 1;
        _renderPassInfo.pAttachments = &_colorAttachment;
        _renderPassInfo.subpassCount = 1;
        _renderPassInfo.pSubpasses = &_subpass;
        _renderPassInfo.dependencyCount = 1;
        _renderPassInfo.pDependencies = &_dependency;

        //
        _gen();
    }

    VkRenderPass get() const {
        return _renderPass;
    }

    Swapchain* swapchain() const {
        return _swapchain;
    }

    ~Renderpass() {
        _degen();
    }
 
 private:
    VkAttachmentDescription _colorAttachment{};
    VkAttachmentReference _colorAttachmentRef{};
    VkSubpassDescription _subpass{};
    VkSubpassDependency _dependency{};
    VkRenderPassCreateInfo _renderPassInfo{};

    Swapchain* _swapchain = nullptr;
    VkRenderPass _renderPass;

    void _gen() final {
        // update imageformat from recreated swapchain
        _colorAttachment.format = _swapchain->imageFormat;

        // create
        auto result = vkCreateRenderPass(_swapchain->device()->get(), &_renderPassInfo, nullptr, &_renderPass);
        assert(result == VK_SUCCESS);
    }

    void _degen() final {
        vkDestroyRenderPass(_swapchain->device()->get(), _renderPass, nullptr);
    }
};

}; // namespace Vulcain
