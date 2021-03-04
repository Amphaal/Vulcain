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

#include "GlfwWindow.hpp"
#include "Instance.hpp"

namespace Vulcain {

class Surface {
 public:    
    Surface(GlfwWindow* window, Instance* instance) : _instance(instance), _window(window) {
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = _window->handle();
        createInfo.hinstance = GetModuleHandle(nullptr);

        auto result = vkCreateWin32SurfaceKHR(_instance->get(), &createInfo, nullptr, &_surface);
        assert(result == VK_SUCCESS);
    }

    ~Surface() {
        if(_instance) vkDestroySurfaceKHR(_instance->get(), _surface, nullptr);
    }

    Instance* instance() const {
        return _instance;
    }

    GlfwWindow* window() const {
        return _window;
    }

    VkSurfaceKHR get() {
        return _surface;
    }

 private:
    VkSurfaceKHR _surface;
    Instance* _instance = nullptr;
    GlfwWindow* _window = nullptr;
};

}; // namespace Vulcain
