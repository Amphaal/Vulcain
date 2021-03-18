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

#include "engine/Device.hpp"

namespace Vulcain {

class IBuffer : public DeviceBound {
 public:
    const VkDeviceSize bufferSize;

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    IBuffer duplicate(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
        return IBuffer(this, this->bufferSize, usage, properties);
    }

    IBuffer(const DeviceBound* deviceBound, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) : DeviceBound(deviceBound), bufferSize(size) {
        //
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        //
        auto error = vkCreateBuffer(*_device, &bufferInfo, nullptr, &buffer);
        assert(!error);

        //
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(*_device, buffer, &memRequirements);
        
        //
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = _device->findMemoryType(memRequirements.memoryTypeBits, properties);
        
        //
        error = vkAllocateMemory(*_device, &allocInfo, nullptr, &bufferMemory);
        assert(!error);

        //
        vkBindBufferMemory(*_device, buffer, bufferMemory, 0);
    }

    void copyBuffer(const CommandPool* pool, IBuffer& dstBuffer) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = *pool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(*_device, &allocInfo, &commandBuffer);

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

        vkQueueSubmit(_device->queue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(_device->queue());

        vkFreeCommandBuffers(*_device, *pool, 1, &commandBuffer);
    }

    ~IBuffer() {
        vkDestroyBuffer(*_device, buffer, nullptr);
        vkFreeMemory(*_device, bufferMemory, nullptr);
    }
};

template<typename T>
class IVerticeBuffer : public IBuffer {
 public:
    IVerticeBuffer(const DeviceBound* deviceBound, T vertices, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) : IBuffer(
            deviceBound, 
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
        vkMapMemory(*_device, memory, 0, this->bufferSize, 0, &data);
        memcpy(data, _vertices.data(), (size_t) this->bufferSize);
        vkUnmapMemory(*_device, memory);
    }
};

} // namespace Vulcain
