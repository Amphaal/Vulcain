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

#include "engine/Device.hpp"

#include <map>

namespace Vulcain {

class DevicePicker {
 public:
    static Device getBestDevice(Surface* surface) {
        _mayRatePhysicalDevice(surface);
        return { &_getPreferedPhysicalDevice() };
    };

 private:
    static inline std::multimap<int, PhysicalDeviceDetails> _pDevicesCandidates;

     static void _mayRatePhysicalDevice(Surface* surface) {
        // no need to re-rate
        if(_pDevicesCandidates.size()) return;
        
        // find score for each physical device
        for (auto device : surface->instance()->getPhysicalDevices()) {
            PhysicalDeviceDetails pDetails {device, surface};
            
            auto score = _rateDeviceSuitability(pDetails, surface);
            if(!score) continue;

            _pDevicesCandidates.insert(
                std::make_pair(score, pDetails)
            );
        }

        // make sure there are candidates
        assert(_pDevicesCandidates.size());
    }

    static const PhysicalDeviceDetails& _getPreferedPhysicalDevice() {
        return _pDevicesCandidates.rbegin()->second;
    }

    //
    //
    //

    static int _rateDeviceSuitability(PhysicalDeviceDetails &details, Surface* surface) {
        int score = 0;

        // check properties
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(details.pDevice, &deviceProperties);
        vkGetPhysicalDeviceFeatures(details.pDevice, &deviceFeatures);

        // fill max sampling rate handled
        details.handledMaxSampling = getMaxUsableSampleCount(deviceProperties);

        // Application can't function without geometry shaders
        if (!deviceFeatures.geometryShader) return 0;

        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        // check queues
        if(!_hasPotententQueue(details, surface)) return 0;

        // ensure device supports swapchain
        if(!_supportsSwapchain(details, surface)) return 0;

        //
        return score;
    }

    static VkSampleCountFlags getMaxUsableSampleCount(VkPhysicalDeviceProperties& physicalDeviceProperties) {
        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & 
        physicalDeviceProperties.limits.framebufferDepthSampleCounts;

        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    static bool _supportsSwapchain(PhysicalDeviceDetails &details, Surface* surface) {
        // get available extensions on device
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(details.pDevice, nullptr, &extensionCount, nullptr);
        VkExtensionProperties availableExtensions[extensionCount];
        vkEnumerateDeviceExtensionProperties(details.pDevice, nullptr, &extensionCount, availableExtensions);

        // check all required are available
        for (const auto& required : Device::REQUIRED_DEVICE_EXTENSIONS) {
            bool requiredIsAvailable = false;
            for(const auto &available : availableExtensions) {
                if(strcmp(required, available.extensionName) == 0) {
                    requiredIsAvailable = true;
                    break;
                }
            }
            if(!requiredIsAvailable) return false;
        }

        //
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(details.pDevice, *surface, &details.swapchainDetails.capabilities);

        //
        {
            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(details.pDevice, *surface, &formatCount, nullptr);
            if (formatCount != 0) {
                details.swapchainDetails.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(
                    details.pDevice, 
                    *surface, 
                    &formatCount, 
                    details.swapchainDetails.formats.data()
                );
            }
            if(!formatCount) return false;
        }

        //
        {
            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(details.pDevice, *surface, &presentModeCount, nullptr);
            if (presentModeCount != 0) {
                details.swapchainDetails.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(
                    details.pDevice, 
                    *surface, 
                    &presentModeCount, 
                    details.swapchainDetails.presentModes.data()
                );
            }
            if(!presentModeCount) return false;
        }

        //
        return true;
    }

    // returns potent queue index
    static bool _hasPotententQueue(PhysicalDeviceDetails &details, Surface* surface) {
        // get queues
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(details.pDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(details.pDevice, &queueFamilyCount, queueFamilies.data());

        // make sure this physical device can do graphics
        for (size_t i = 0; i < queueFamilies.size(); i++) {
            // check if graphics queue is handled
            auto requiredQueueHandled = queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
            if (!requiredQueueHandled) continue;
            
            // check if queue can do presentation
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(details.pDevice, i, *surface, &presentSupport);
            if (!presentSupport) continue;

            // set this potent queue
            details.presentationAndGraphicsQueueIndex = i;
            return true;
        }

        // none acceptable found !
        return false;
    }
};

} // namespace Vulcain
