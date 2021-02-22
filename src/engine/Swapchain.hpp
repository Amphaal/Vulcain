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

namespace Vulcain {

class Swapchain {
 public:
    Swapchain(Device* device) : _device(device) {
        //
        auto const &swapChainSupport = _device->swapchainDetails();
        auto imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        const auto swapSurfaceFormat = swapChainSupport.getSwapSurfaceFormat();
        const auto extent = swapChainSupport.getSwapExtent(device->surface()->window());

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = _device->surface()->get();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = swapSurfaceFormat.format;
        createInfo.imageColorSpace = swapSurfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        //TODO finish with https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain

        //
        auto result = vkCreateSwapchainKHR(_device->get(), &createInfo, nullptr, &_swapChain);
        assert(result == VK_SUCCESS);
    }
    
    ~Swapchain() {
        vkDestroySwapchainKHR(_device->get(), _swapChain, nullptr);
    }

 private:
    VkSwapchainKHR _swapChain;
    Device* _device = nullptr;
};

}; // namespace Vulcain
