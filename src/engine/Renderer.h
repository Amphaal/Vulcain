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

#include "common/IDrawer.h"
#include "CommandPool.hpp"

namespace Vulcain {

class Renderer : public IDrawer, public DeviceBound {
 public:
   using BeforeWaitingCurrentImageCallback = std::function<void(uint32_t)>;

    Renderer(const CommandPool* pool, GlfwWindow* window, Vulcain::Swapchain* swapchain);
    ~Renderer();

    void draw() final;

    void onBeforeWaitingCurrentImage(BeforeWaitingCurrentImageCallback cb);

 private:
    static const int MAX_FRAMES_IN_FLIGHT = 2;
    size_t _currentFrame = 0;

    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;
    std::vector<VkFence> _imagesInFlight;

    std::atomic<bool> _hasFramebufferResized;

    BeforeWaitingCurrentImageCallback _onBeforeWaitingCurrentImage;

    const CommandPool* _cmdPool = nullptr;

    // non-const
    Swapchain* _swapchain = nullptr;
    GlfwWindow* _window = nullptr;

    void _createSyncObjects();



    void _regenerateSwapChain();
};

} // namespace Vulcain
