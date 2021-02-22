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

#include "Instance.hpp"

#include <map>

namespace Vulcain {

class Device {
 public:
    Device(Vulcain::Instance* instance) : _instance(instance) {
        _pickBestPhysicalDevice();
        _instaciateLogicalDevice();

        // get graphics queue
        vkGetDeviceQueue(_device, REQUIRED_QUEUE_TYPE, 0, &_graphicsQueue);
    }

    ~Device() {
        vkDestroyDevice(_device, nullptr);
    }

 private:
    Vulcain::Instance* _instance = nullptr;

    VkPhysicalDevice _pickedPDevice = VK_NULL_HANDLE;
    VkDevice _device;
    VkQueue _graphicsQueue;

    std::multimap<int, VkPhysicalDevice> _pDevicesCandidates;
    float _queuePriority = 1.f;

    static auto const REQUIRED_QUEUE_TYPE = VK_QUEUE_GRAPHICS_BIT;

    void _pickBestPhysicalDevice() {
    // get physical devices
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_instance->get(), &deviceCount, nullptr);
        assert(deviceCount);
        std::vector<VkPhysicalDevice> availablePDevices(deviceCount);
        vkEnumeratePhysicalDevices(_instance->get(), &deviceCount, availablePDevices.data());

        // find score for each physical device
        for (const auto& device : availablePDevices) {
            auto score = _rateDeviceSuitability(device);
            if(!score) continue;
            _pDevicesCandidates.insert(std::make_pair(score, device));
        }
        assert(_pDevicesCandidates.size());

        // pick the one with most points
        _pickedPDevice = _pDevicesCandidates.rbegin()->second;
    }

    int _rateDeviceSuitability(const VkPhysicalDevice &pDevice) {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(pDevice, &deviceProperties);
        vkGetPhysicalDeviceFeatures(pDevice, &deviceFeatures);

        int score = 0;

        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        // Application can't function without geometry shaders
        if (!deviceFeatures.geometryShader) {
            return 0;
        }

        // get queues
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, queueFamilies.data());

        // make sure this physical device can do graphics
        bool hasGraphicsQueue = false;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & REQUIRED_QUEUE_TYPE) {
                hasGraphicsQueue = true; 
                break;
            }
        }
        if(!hasGraphicsQueue) return false;

        //
        return score;
    }

    void _instaciateLogicalDevice() {
        // define properties
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = REQUIRED_QUEUE_TYPE;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &_queuePriority;

        //
        VkPhysicalDeviceFeatures deviceFeatures{};

        //
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        //
        if(auto createInfo = _instance->createInfo(); createInfo->enabledLayerCount) {
            deviceCreateInfo.enabledLayerCount = createInfo->enabledLayerCount;
            deviceCreateInfo.ppEnabledLayerNames = createInfo->ppEnabledLayerNames;
        }

        // create device
        auto result = vkCreateDevice(_pickedPDevice, &deviceCreateInfo, nullptr, &_device);
        assert(result == VK_SUCCESS);
    }
};

}; // namespace Vulcain
