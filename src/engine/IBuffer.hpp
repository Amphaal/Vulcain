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

#include "Device.hpp"

namespace Vulcain {

class IBuffer {
 public:
    const VkDeviceSize bufferSize;

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    IBuffer duplicate(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
        return IBuffer(this->_device, this->bufferSize, usage, properties);
    }

    IBuffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) : _device(device), bufferSize(size) {
        //
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        //
        auto error = vkCreateBuffer(device->get(), &bufferInfo, nullptr, &buffer);
        assert(!error);

        //
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device->get(), buffer, &memRequirements);
        
        //
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, properties);
        
        //
        error = vkAllocateMemory(device->get(), &allocInfo, nullptr, &bufferMemory);
        assert(!error);

        //
        vkBindBufferMemory(device->get(), buffer, bufferMemory, 0);
    }

    void copyBuffer(CommandPool* pool, IBuffer& dstBuffer) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = pool->get();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(pool->device()->get(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

            VkBufferCopy copyRegion{};
            copyRegion.size = bufferSize;
            vkCmdCopyBuffer(commandBuffer, buffer, dstBuffer.buffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(pool->device()->queue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(pool->device()->queue());

        vkFreeCommandBuffers(pool->device()->get(), pool->get(), 1, &commandBuffer);
    }

    ~IBuffer() {
        vkDestroyBuffer(_device->get(), buffer, nullptr);
        vkFreeMemory(_device->get(), bufferMemory, nullptr);
    }

 protected:
    Device* _device = nullptr;
};

template<typename T>
class IVerticeBuffer : public IBuffer {
 public:
    IVerticeBuffer(Device* device, T vertices, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) : IBuffer(
            device, 
            _getBufferSize(vertices), 
            usage,
            properties
        ), _vertices(vertices) { }

    auto vertexCount() const {
        return static_cast<uint32_t>(_vertices.size());
    }

    T& vertices() {
        return _vertices;
    }
 
 protected:
    void _mapVerticesToMemory(IBuffer& buffer) {
        _mapVerticesToMemory(buffer.bufferMemory);
    }

 private:
    T _vertices;

    static auto _getBufferSize(T vertices) {
        return sizeof(vertices[0]) * vertices.size();
    }

    void _mapVerticesToMemory(VkDeviceMemory memory) {
        //
        void* data;
        vkMapMemory(_device->get(), memory, 0, this->bufferSize, 0, &data);
        memcpy(data, _vertices.data(), (size_t) this->bufferSize);
        vkUnmapMemory(_device->get(), memory);
    }
};

} // namespace Vulcain
