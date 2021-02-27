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

#include "Renderpass.hpp"

namespace Vulcain {

class ImageViews {
 public:
    ImageViews(Swapchain* swapchain, Renderpass* renderpass) : _device(swapchain->device()) {
        //
        auto swapChainImages = swapchain->images();
        
        _views.resize(swapChainImages.size());
        _fbs.resize(swapChainImages.size());

        //
        for(size_t i = 0; i < swapChainImages.size(); i++) {
            _pushImageView(swapchain, swapChainImages[i], &_views[i]);
            _pushFramebuffer(swapchain, renderpass, _views[i], &_fbs[i]);
        }
    }

    ~ImageViews() {
        //
        for (auto framebuffer : _fbs) {
            vkDestroyFramebuffer(_device->get(), framebuffer, nullptr);
        }

        //
        for (auto imageView : _views) {
            vkDestroyImageView(_device->get(), imageView, nullptr);
        }
    }

 private:
    void _pushImageView(Swapchain* swapchain, VkImage swapChainImage, VkImageView* into) {
        //
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImage;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchain->imageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        //
        auto result = vkCreateImageView(_device->get(), &createInfo, nullptr, into);
        assert(result == VK_SUCCESS);
    }

    void _pushFramebuffer(Swapchain* swapchain, Renderpass* renderpass, VkImageView targetView, VkFramebuffer* into) {
        //
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderpass->get();
        framebufferInfo.attachmentCount = 1;
        VkImageView attachments[] = {
            targetView
        };
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchain->imageExtent.width;
        framebufferInfo.height = swapchain->imageExtent.height;
        framebufferInfo.layers = 1;

        //
        auto result = vkCreateFramebuffer(_device->get(), &framebufferInfo, nullptr, into);
        assert(result == VK_SUCCESS);
    }

    std::vector<VkImageView> _views;
    std::vector<VkFramebuffer> _fbs;
    Device* _device = nullptr;
};

}; // namespace Vulcain
