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

#include "GlfwWindow.h"
   
Vulcain::GlfwWindow::GlfwWindow() {
    //
    glfwInit();

    //
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
    glfwSetWindowUserPointer(_window, this);
}

Vulcain::GlfwWindow::~GlfwWindow() {
    if(_window) glfwDestroyWindow(_window);
    glfwTerminate();
}

void Vulcain::GlfwWindow::poolEventsAndDraw() {
    while(!glfwWindowShouldClose(_window)) {
        glfwPollEvents();
        _drawer->draw();
    }
}

HWND Vulcain::GlfwWindow::handle() {
    return glfwGetWin32Window(_window);
}

VkExtent2D Vulcain::GlfwWindow::framebufferSize() {
    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);
    
    return {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
}

void  Vulcain::GlfwWindow::_bindDrawer(IDrawer* drawer) {
    _drawer = drawer;
}

void Vulcain::GlfwWindow::_bindFramebufferChanges(std::atomic<bool>* framebufferChangedFlag) {
    //
    _framebufferChangedFlag = framebufferChangedFlag;

    //
    glfwSetFramebufferSizeCallback(_window, [](GLFWwindow* window, int width, int height){
        auto handler = reinterpret_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
        *handler->_framebufferChangedFlag = true;
    });
}
