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

#include <atomic>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Vulcain {

class GlfwWindow {
 public:    
    GlfwWindow() {
        //
        glfwInit();

        //
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        _window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
        glfwSetWindowUserPointer(_window, this);
    }

    ~GlfwWindow() {
        if(_window) glfwDestroyWindow(_window);
        glfwTerminate();
    }

    void waitForWindowEvents() {
        while(!glfwWindowShouldClose(_window)) {
            glfwWaitEvents();
        }
    }

    HWND handle() {
        return glfwGetWin32Window(_window);
    }

    void bindFramebufferChanges(std::atomic<bool>* framebufferChangedFlag) {
        //
        _framebufferChangedFlag = framebufferChangedFlag;

        //
        glfwSetFramebufferSizeCallback(_window, [](GLFWwindow* window, int width, int height){
            auto handler = reinterpret_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            *handler->_framebufferChangedFlag = true;
        });
    }

    VkExtent2D framebufferSize() {
        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);
        
        return {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
    }

 private:
    GLFWwindow* _window = nullptr;

    std::atomic<bool>* _framebufferChangedFlag = nullptr;
};

}; // namespace Vulcain
