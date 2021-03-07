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
#include "CommandPool.hpp"

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace Vulcain {

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

struct BaseBuffer {
 public:
    const VkDeviceSize bufferSize;

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    BaseBuffer duplicate(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
        return BaseBuffer(this->_device, this->bufferSize, usage, properties);
    }

    BaseBuffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) : _device(device), bufferSize(size) {
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

    void copyBuffer(CommandPool* pool, BaseBuffer& dstBuffer) {
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

    ~BaseBuffer() {
        vkDestroyBuffer(_device->get(), buffer, nullptr);
        vkFreeMemory(_device->get(), bufferMemory, nullptr);
    }

 protected:
    Device* _device = nullptr;
};


#ifdef WIN32
auto i = true;
#endif

template<typename T>
class IBuffer : public BaseBuffer {
 public:
    IBuffer(Device* device, T vertices, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) : BaseBuffer(
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
    void _mapVerticesToMemory(BaseBuffer& buffer) {
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

template<class T>
class IStaticBuffer : public IBuffer<const T> {
 public:
    IStaticBuffer(CommandPool* pool, const T vertices) : IBuffer<const T>(
        pool->device(), 
        vertices, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    ) {
        // create staging buffer
        auto staging = BaseBuffer::duplicate(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        // map vertice to staging
        IBuffer<const T>::_mapVerticesToMemory(staging);

        // copy staging to GPU memory
        staging.copyBuffer(pool, *this);
    }
};

template<class T>
using StaticBuffer = IStaticBuffer<std::vector<T>>;

} // namespace Vulcain
