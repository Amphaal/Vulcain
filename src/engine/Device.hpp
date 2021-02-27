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

#include "Vulcain.h"

#include "Surface.hpp"

namespace Vulcain {

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    VkSurfaceFormatKHR getSwapSurfaceFormat() const {
        for (const auto& availableFormat : formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return formats[0];
    }

    VkPresentModeKHR getSwapPresentMode() const {
        for (const auto& availablePresentMode : presentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D getSwapExtent(WindowHandler* windowHandler) const {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            auto actualExtent = windowHandler->framebufferSize();

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }
};

struct PhysicalDeviceDetails {
    const VkPhysicalDevice pDevice;
    Vulcain::Surface* surface = nullptr;
    SwapChainSupportDetails swapchainDetails;
};

class Device {
 public:
    static auto const REQUIRED_QUEUE_TYPE = VK_QUEUE_GRAPHICS_BIT;
    static inline const std::vector<const char*> REQUIRED_DEVICE_EXTENSIONS {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    Device(const PhysicalDeviceDetails* pDeviceDetails) : _pDeviceDetails(pDeviceDetails) {
        _instaciateLogicalDevice();

        // get graphics queue
        vkGetDeviceQueue(_device, REQUIRED_QUEUE_TYPE, 0, &_graphicsQueue);
    }

    ~Device() {
        vkDestroyDevice(_device, nullptr);
    }

    //
    //
    //

    VkDevice get() {
        return _device;
    }

    const SwapChainSupportDetails& swapchainDetails() const {
        return _pDeviceDetails->swapchainDetails;
    }

    Vulcain::Surface* surface() {
        return _pDeviceDetails->surface;
    }

 private:
    const Vulcain::PhysicalDeviceDetails* _pDeviceDetails = nullptr;

    VkDevice _device;
    VkQueue _graphicsQueue;

    float _queuePriority = 1.f;
    
    void _instaciateLogicalDevice() {
        // instanciate graphics queue
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = REQUIRED_QUEUE_TYPE;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &_queuePriority;

        //
        VkPhysicalDeviceFeatures deviceFeatures{};

        // create infos
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            //
            deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
            deviceCreateInfo.queueCreateInfoCount = 1;

            //
            deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

            //
            if(auto createInfo = surface()->instance()->createInfo(); createInfo->enabledLayerCount) {
                deviceCreateInfo.enabledLayerCount = createInfo->enabledLayerCount;
                deviceCreateInfo.ppEnabledLayerNames = createInfo->ppEnabledLayerNames;
            }
            
            //
            deviceCreateInfo.enabledExtensionCount = REQUIRED_DEVICE_EXTENSIONS.size();
            deviceCreateInfo.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS.data();

        // create device
        auto result = vkCreateDevice(
            _pDeviceDetails->pDevice, 
            &deviceCreateInfo, 
            nullptr, 
            &_device
        );
        assert(result == VK_SUCCESS);
        
        //
        #ifdef USES_VOLK
        volkLoadDevice(_device);
        #endif
    }
};

}; // namespace Vulcain
