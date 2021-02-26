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

#include "Debug.hpp"

#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <vector>

namespace Vulcain {

class AvailableExtensions {
 public:
    AvailableExtensions() {
        // get available extensions
        vkEnumerateInstanceExtensionProperties(nullptr, &_extensionCount, nullptr);
        assert(_extensionCount);
        _extensions.resize(_extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &_extensionCount, _extensions.data());
    }

    void assertAllAndInsert(const char **extsToUse, int extsToUseCount, std::vector<const char*> &toPopulate) const {
        for(auto requiredExtCount = 0; requiredExtCount < extsToUseCount; requiredExtCount++) {
            //
            const auto & requiredExt = extsToUse[requiredExtCount];
            
            //
            auto requiredIsAvailable = _extensionsContains(requiredExt);

            //
            assert(requiredIsAvailable);

            //
            toPopulate.push_back(requiredExt);
        }
    }

    void assertAll(const std::vector<const char*> &extsToUse) const {
        for(auto const & requiredExt : extsToUse) {            
            //
            auto requiredIsAvailable = _extensionsContains(requiredExt);

            //
            assert(requiredIsAvailable);
        }
    }

 private:
    std::vector<VkExtensionProperties> _extensions;
    uint32_t _extensionCount = 0;

    bool _extensionsContains(const char *const &requiredExt) const {
        for(const auto &available : _extensions) {
            if(strcmp(requiredExt, available.extensionName) == 0) {
                return true;
            }
        }
        return false;
    }
};

class InstanceCreateInfo : public VkInstanceCreateInfo {
 public:
    InstanceCreateInfo(const VkApplicationInfo * appInfo) : VkInstanceCreateInfo{} {
        //
        assert(appInfo);

        // bind data
        this->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        this->pApplicationInfo = appInfo;

        /* order is important !*/
        if(getenv("VK_LAYER_PATH")) { // if not layer path is hardcoded, assume it is prod env and skipping
            _bindValidationLayers();
            _addDebugCallback(Vulcain::debugCallback);
        }

        //
        _bindRequiredExtensions();
    }

    const VkDebugUtilsMessengerCreateInfoEXT* debugUtil() const {
        return _debugInfo;
    }

    ~InstanceCreateInfo() {
        if(_debugInfo) delete _debugInfo;
    }

 private:
    static inline std::vector<const char*> WANTED_LAYERS {
        "VK_LAYER_KHRONOS_validation"
    };
    void _bindValidationLayers() {
        //
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        VkLayerProperties availableLayers[layerCount];
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

        // check
        for (const char* layerName : WANTED_LAYERS) {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            assert(layerFound);
        }

        //
        _required_exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        //
        this->ppEnabledLayerNames = WANTED_LAYERS.data();
        this->enabledLayerCount = WANTED_LAYERS.size();
    }   
    
    std::vector<const char*> _required_exts;
    void _bindRequiredExtensions() {
        // check layout required ext
        AvailableExtensions available;
        available.assertAll(_required_exts);

        // check GLFW required ext
        {
            // get the extensions required by glfw
            uint32_t glfwExtensionCount = 0;
            auto extsToUse = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            assert(glfwExtensionCount); // no extensions required, has glfw been launched ?

            // check if all of them are available,
            available.assertAllAndInsert(extsToUse, glfwExtensionCount, _required_exts);
        }

        //
        this->enabledExtensionCount = _required_exts.size();
        this->ppEnabledExtensionNames = _required_exts.data();
    }

    VkDebugUtilsMessengerCreateInfoEXT* _debugInfo = nullptr;
    void _addDebugCallback(PFN_vkDebugUtilsMessengerCallbackEXT cb) {
        assert(!_debugInfo);

        _debugInfo = new VkDebugUtilsMessengerCreateInfoEXT{};
        _debugInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        _debugInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        _debugInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        _debugInfo->pfnUserCallback = cb;

        this->pNext = _debugInfo;
    }
};

}; // namespace Vulcain
