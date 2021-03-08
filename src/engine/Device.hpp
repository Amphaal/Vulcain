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

#include "common/Vulcain.h"

#include "helpers/DeviceDetails.hpp"

namespace Vulcain {

class Device {
 public:
    static inline const std::vector<const char*> REQUIRED_DEVICE_EXTENSIONS {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    Device(const PhysicalDeviceDetails* pDeviceDetails) : _pDeviceDetails(pDeviceDetails) {
        _instaciateLogicalDevice();
    }

    operator VkDevice() const { return _device; }

    ~Device() {
        vkDestroyDevice(_device, nullptr);
    }

    //
    //
    //

    int queueIndex() const {
        return _pDeviceDetails->presentationAndGraphicsQueueIndex;
    }

    VkQueue queue() const {
        return _presentationAndGraphicsQueue;
    }

    const SwapChainSupportDetails& swapchainDetails() const {
        return _pDeviceDetails->swapchainDetails;
    }

    Surface* surface() const {
        return _pDeviceDetails->surface;
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(_pDeviceDetails->pDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

 private:
    const PhysicalDeviceDetails* _pDeviceDetails = nullptr;

    VkDevice _device;
    VkQueue _presentationAndGraphicsQueue;

    float _queuePriority = 1.f;
    
    void _instaciateLogicalDevice() {
        // instanciate main queue
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = _pDeviceDetails->presentationAndGraphicsQueueIndex;
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

        // get main queue
        vkGetDeviceQueue(_device, _pDeviceDetails->presentationAndGraphicsQueueIndex, 0, &_presentationAndGraphicsQueue);
    }
};

class DeviceBound {
 public:
    explicit DeviceBound(const Device* device) : _device(device) {
        assert(_device);
    }
    explicit DeviceBound(const DeviceBound* bound) : _device(bound->_device) {
        assert(bound);
    }

 protected:
    const Device* _device = nullptr;
};

} // namespace Vulcain
