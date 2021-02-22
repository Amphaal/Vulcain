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

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Vulcain {

class WindowHandler {
 public:    
    WindowHandler() {
        //
        glfwInit();
        _inited = true;

        //
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        _window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
    }

    ~WindowHandler() {
        if(_window) glfwDestroyWindow(_window);
        if(_inited) glfwTerminate();
    }

    void waitForWindowEvents() {
        while(!glfwWindowShouldClose(_window)) {
            glfwWaitEvents();
        }
    }

    HWND handle() {
        return glfwGetWin32Window(_window);
    }

 private:
    GLFWwindow* _window = nullptr;
    bool _inited = false;
};

}; // namespace Vulcain
