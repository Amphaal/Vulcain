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

#include "Surface.hpp"

#include <map>

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
    SwapChainSupportDetails swapchainDetails;
};

class Device {
 public:
    Device(Vulcain::Surface* surface) : _surface(surface) {
        _getBestPhysicalDevice();
        _instaciateLogicalDevice();

        // get graphics queue
        vkGetDeviceQueue(_device, REQUIRED_QUEUE_TYPE, 0, &_graphicsQueue);
    }

    VkDevice& get() {
        return _device;
    }

    const SwapChainSupportDetails& swapchainDetails() const {
        return _getPreferedPhysicalDevice().swapchainDetails;
    }

    Vulcain::Surface* surface() {
        return _surface;
    }

    ~Device() {
        vkDestroyDevice(_device, nullptr);
    }

 private:
    Vulcain::Surface* _surface = nullptr;

    VkDevice _device;
    VkQueue _graphicsQueue;

    std::multimap<int, PhysicalDeviceDetails> _pDevicesCandidates;
    float _queuePriority = 1.f;

    static auto const REQUIRED_QUEUE_TYPE = VK_QUEUE_GRAPHICS_BIT;

    const std::vector<const char*> REQUIRED_DEVICE_EXTENSIONS {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    Instance* _instance() {
        return _surface->instance();
    }

    void _getBestPhysicalDevice() {
        // get physical devices
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_instance()->get(), &deviceCount, nullptr);
        assert(deviceCount);
        std::vector<VkPhysicalDevice> availablePDevices(deviceCount);
        vkEnumeratePhysicalDevices(_instance()->get(), &deviceCount, availablePDevices.data());

        // find score for each physical device
        for (const auto& device : availablePDevices) {
            PhysicalDeviceDetails pDetails {device};
            auto score = _rateDeviceSuitability(pDetails);
            if(!score) continue;
            _pDevicesCandidates.insert(std::make_pair(score, pDetails));
        }
        assert(_pDevicesCandidates.size());
    }

    int _rateDeviceSuitability(PhysicalDeviceDetails &details) const {
        int score = 0;

        // check properties
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(details.pDevice, &deviceProperties);
        vkGetPhysicalDeviceFeatures(details.pDevice, &deviceFeatures);

        // Application can't function without geometry shaders
        if (!deviceFeatures.geometryShader) return 0;

        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        // check queues
        if(!_hasPotententQueue(details.pDevice)) return 0;

        // ensure device supports swapchain
        if(!_supportsSwapchain(details)) return 0;

        //
        return score;
    }

    bool _supportsSwapchain(PhysicalDeviceDetails &details) const {
        // get available extensions on device
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(details.pDevice, nullptr, &extensionCount, nullptr);
        VkExtensionProperties availableExtensions[extensionCount];
        vkEnumerateDeviceExtensionProperties(details.pDevice, nullptr, &extensionCount, availableExtensions);

        // check all required are available
        for (const auto& required : REQUIRED_DEVICE_EXTENSIONS) {
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
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(details.pDevice, _surface->get(), &details.swapchainDetails.capabilities);

        //
        {
            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(details.pDevice, _surface->get(), &formatCount, nullptr);
            if (formatCount != 0) {
                details.swapchainDetails.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(
                    details.pDevice, 
                    _surface->get(), 
                    &formatCount, 
                    details.swapchainDetails.formats.data()
                );
            }
            if(!formatCount) return false;
        }

        //
        {
            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(details.pDevice, _surface->get(), &presentModeCount, nullptr);
            if (presentModeCount != 0) {
                details.swapchainDetails.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(
                    details.pDevice, 
                    _surface->get(), 
                    &presentModeCount, 
                    details.swapchainDetails.presentModes.data()
                );
            }
            if(!presentModeCount) return false;
        }

        //
        return true;
    }

    // TODO(amphaal) handle multiple queues ? (https://vulkan-tutorial.com/code/05_window_surface.cpp)
    bool _hasPotententQueue(const VkPhysicalDevice &pDevice) const {
        bool hasPotentQueue = false;
        
        // get queues
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, queueFamilies.data());


        // make sure this physical device can do graphics
        for (const auto& queueFamily : queueFamilies) {
            // check if required queue is handled
            auto requiredQueueHandled = queueFamily.queueFlags & REQUIRED_QUEUE_TYPE;
            if (!requiredQueueHandled) continue;
            
            // check if surface can do presentation
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, REQUIRED_QUEUE_TYPE, _surface->get(), &presentSupport);
            if (!presentSupport) continue;

            //
            hasPotentQueue = true;
            break;
        }

        return hasPotentQueue;
    }

    const PhysicalDeviceDetails& _getPreferedPhysicalDevice() const {
        return _pDevicesCandidates.rbegin()->second;
    }
    
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
            if(auto createInfo = _instance()->createInfo(); createInfo->enabledLayerCount) {
                deviceCreateInfo.enabledLayerCount = createInfo->enabledLayerCount;
                deviceCreateInfo.ppEnabledLayerNames = createInfo->ppEnabledLayerNames;
            }
            
            //
            deviceCreateInfo.enabledExtensionCount = REQUIRED_DEVICE_EXTENSIONS.size();
            deviceCreateInfo.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS.data();

        // create device
        auto result = vkCreateDevice(
            _getPreferedPhysicalDevice().pDevice, 
            &deviceCreateInfo, 
            nullptr, 
            &_device
        );
        assert(result == VK_SUCCESS);
    }
};

}; // namespace Vulcain
