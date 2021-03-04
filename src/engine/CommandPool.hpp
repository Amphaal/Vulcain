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

#include <functional>

#include "ImageViews.hpp"

namespace Vulcain {

class CommandPool {
 public:
    CommandPool(ImageViews* views) : _views(views) {       
        _createCommandPool();
        _allocateCommandBuffers(views->count());
    }

    ~CommandPool() {
        vkDestroyCommandPool(_device()->get(), _commandPool, nullptr);
    }

    void record(std::function<void(VkCommandBuffer)> commands) {
        for(size_t i = 0; i < _commandBuffers.size(); i++) {
            //
            auto &commandBuffer = _commandBuffers[i];
            
            //
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            auto resultBegin = vkBeginCommandBuffer(commandBuffer, &beginInfo);
            assert(resultBegin == VK_SUCCESS);

            //
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = _views->renderpass()->get();
            renderPassInfo.framebuffer = _views->framebuffer(i);
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = _views->renderpass()->swapchain()->imageExtent;

            VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

                vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                    //
                    commands(commandBuffer);
                    //
                vkCmdEndRenderPass(commandBuffer);

            //
            auto resultEnd = vkEndCommandBuffer(commandBuffer);
            assert(resultEnd == VK_SUCCESS);
        }
    }

    ImageViews* views() const {
        return _views;
    }

    VkCommandBuffer commandBuffer(int index) const {
        return _commandBuffers[index];
    }

 private:
    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _commandBuffers;
    ImageViews* _views = nullptr;

    Device* _device() const {
        return _views->renderpass()->swapchain()->device();
    }

    void _createCommandPool() {
        //
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = _device()->queueIndex();
        poolInfo.flags = 0; // Optional
        auto result = vkCreateCommandPool(_device()->get(), &poolInfo, nullptr, &_commandPool);
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
        auto result = vkAllocateCommandBuffers(_device()->get(), &allocInfo, _commandBuffers.data());
        assert(result == VK_SUCCESS);
    }
};

}; // namespace Vulcain