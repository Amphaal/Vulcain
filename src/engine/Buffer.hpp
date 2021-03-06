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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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

template<typename T>
class IBuffer {
 public:
    IBuffer(Device* device, T vertices) : _device(device), _vertices(vertices) {
        //
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(_vertices[0]) * _vertices.size();
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        _bufferSize = bufferInfo.size;

        //
        auto error = vkCreateBuffer(_device->get(), &bufferInfo, nullptr, &_vertexBuffer);
        assert(!error);

        //
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(_device->get(), _vertexBuffer, &memRequirements);
        
        //
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        auto memTypeIndex = _device->findMemoryType(
            memRequirements.memoryTypeBits, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
        allocInfo.memoryTypeIndex = memTypeIndex;
        
        //
        error = vkAllocateMemory(_device->get(), &allocInfo, nullptr, &_vertexBufferMemory);
        assert(!error);

        //
        vkBindBufferMemory(_device->get(), _vertexBuffer, _vertexBufferMemory, 0);
    }
 
    VkBuffer get() const {
        return _vertexBuffer;
    }

    auto size() const {
        return static_cast<uint32_t>(_vertices.size());
    }

    T& vertices() {
        return _vertices;
    }

    ~IBuffer() {
        vkDestroyBuffer(_device->get(), _vertexBuffer, nullptr);
        vkFreeMemory(_device->get(), _vertexBufferMemory, nullptr);
    }
 
 protected:
    void _mapMemory() {        
        void* data;
        vkMapMemory(_device->get(), _vertexBufferMemory, 0, _bufferSize, 0, &data);
        memcpy(data, _vertices.data(), (size_t) _bufferSize);
        vkUnmapMemory(_device->get(), _vertexBufferMemory);
    }

 private:
    T _vertices;
    Device* _device = nullptr;

    VkDeviceSize _bufferSize;
    VkBuffer _vertexBuffer;
    VkDeviceMemory _vertexBufferMemory;
};

template<class T>
class IStaticBuffer : public IBuffer<const T> {
 public:
    IStaticBuffer(Device* device, const T vertices) : IBuffer<const T>(device, vertices) {
        IBuffer<const T>::_mapMemory();
    }
};

template<class T>
using StaticBuffer = IStaticBuffer<std::vector<T>>;

} // namespace Vulcain
