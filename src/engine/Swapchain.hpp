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

#include "Device.hpp"
#include "IRegenerable.hpp"

namespace Vulcain {

class Swapchain : public VkSwapchainCreateInfoKHR, public IRegenerable {
 public:
    Swapchain(Device* device) : VkSwapchainCreateInfoKHR{}, IRegenerable(nullptr), _device(device) {
        //
        auto const &swapChainSupport = _device->swapchainDetails();
        const auto swapSurfaceFormat = swapChainSupport.getSwapSurfaceFormat();
        const auto presentMode = swapChainSupport.getSwapPresentMode();

        // determine image count
        auto imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        this->sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        this->surface = _device->surface()->get();
        this->minImageCount = imageCount;
        this->imageFormat = swapSurfaceFormat.format;
        this->imageColorSpace = swapSurfaceFormat.colorSpace;
        this->imageArrayLayers = 1;
        this->imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        this->preTransform = swapChainSupport.capabilities.currentTransform;
        this->compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // determine transparency behavior with other windows, here just disable any transparency
        this->presentMode = presentMode;
        this->clipped = VK_TRUE;

        // since both presentation and graphics queues are the same index...
        this->imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        this->queueFamilyIndexCount = 0; // Optional
        this->pQueueFamilyIndices = nullptr; // Optional

        //
        _gen();
    }

    VkSwapchainKHR get() const {
        return _swapChain;
    }

    Device* device() const {
        return _device;
    }

    std::vector<VkImage> images() const {
        return _swapChainImages;
    }

    VkViewport defaultViewport() const {
        VkViewport viewport{};

        //
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) this->imageExtent.width;
        viewport.height = (float) this->imageExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        //
        return viewport;
    }

    VkRect2D defaultScissor() const {
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = this->imageExtent;
        return scissor;
    }

    ~Swapchain() {
        _degen();
    }

 private:
    VkSwapchainKHR _swapChain;
    Device* _device = nullptr;
    std::vector<VkImage> _swapChainImages;

    void _gen() final {
        // update extent
        auto const &swapChainSupport = _device->swapchainDetails();
        const auto extent = swapChainSupport.getSwapExtent(_device->surface()->window());
        this->imageExtent = extent;

        // create swapchain...
        auto result = vkCreateSwapchainKHR(_device->get(), this, nullptr, &_swapChain);
        assert(result == VK_SUCCESS);

        // get images
        unsigned int imageCount = 0;
        vkGetSwapchainImagesKHR(_device->get(), _swapChain, &imageCount, nullptr);
        assert(imageCount);
        _swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(_device->get(), _swapChain, &imageCount, _swapChainImages.data());
    }

    void _degen() final {
        vkDestroySwapchainKHR(_device->get(), _swapChain, nullptr);
    }

};

}; // namespace Vulcain
