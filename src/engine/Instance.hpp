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

#include <volk.h>

#include "InstanceCreateInfo.hpp"

namespace Vulcain {

class Instance {
 public:
    Instance(const InstanceCreateInfo* createInfos) : _createInfos(createInfos) {
        assert(createInfos);

        //
        auto result = vkCreateInstance(_createInfos, nullptr, &_instance);
        assert(result == VK_SUCCESS); 
        volkLoadInstance(_instance);

        //
        _mayCreateDebugMessenger();
    }

    ~Instance() {
        //
        if(_debugMessenger) {
            auto DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
            assert(DestroyDebugUtilsMessengerEXT);
            DestroyDebugUtilsMessengerEXT(_instance, *_debugMessenger, nullptr);
        }

        //
        vkDestroyInstance(_instance, nullptr);
    }

    VkInstance& get() {
        return _instance;
    }

    const InstanceCreateInfo* createInfo() const {
        return _createInfos;
    }

 private:
    VkDebugUtilsMessengerEXT* _debugMessenger = nullptr;
    void _mayCreateDebugMessenger() {
        //
        auto debugUtil = _createInfos->debugUtil();
        if(!debugUtil) return;

        //
        _debugMessenger = new VkDebugUtilsMessengerEXT;
        auto CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
        auto result = CreateDebugUtilsMessengerEXT(_instance, debugUtil, nullptr, _debugMessenger);
        assert(result == VK_SUCCESS);
    };


    const InstanceCreateInfo* _createInfos = nullptr;
    VkInstance _instance;
};

}; // namespace Vulcain

