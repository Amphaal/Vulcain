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

#pragma onceZ

#include "ImageViews.hpp"

namespace Vulcain {

class CommandPool {
 public:
    CommandPool(ImageViews* views) : _views(views) {       
        _createCommandPool();
        _allocateCommandBuffers(views->count());
    }

    ~CommandPool() {
        vkDestroyCommandPool(_device(), _commandPool, nullptr);
    }

 private:
    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _commandBuffers;
    ImageViews* _views = nullptr;

    VkDevice _device() const {
        return _views->renderpass()->swapchain()->device()->get();
    }

    void _createCommandPool() {
        //
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = Vulcain::Device::REQUIRED_QUEUE_TYPE;
        poolInfo.flags = 0; // Optional
        auto result = vkCreateCommandPool(_device(), &poolInfo, nullptr, &_commandPool);
        assert(result == VK_SUCCESS);
    }

    void _allocateCommandBuffers(int howMany) {
        //
        _commandBuffers.resize(howMany);

        //
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = _commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) _commandBuffers.size();

        //
        auto result = vkAllocateCommandBuffers(_device(), &allocInfo, _commandBuffers.data());
        assert(result == VK_SUCCESS);
    }
};

}; // namespace Vulcain
