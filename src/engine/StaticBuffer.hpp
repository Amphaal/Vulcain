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

#include "IBuffer.hpp"

namespace Vulcain {

template<class T, VkBufferUsageFlagBits q>
class IStaticBuffer : public IVerticeBuffer<const T> {
 public:
    IStaticBuffer(CommandPool* pool, const T vertices) : IVerticeBuffer<const T>(
        pool->device(), 
        vertices, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | q, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    ) {
        // create staging buffer
        auto staging = IBuffer::duplicate(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        // map vertice to staging
        IVerticeBuffer<const T>::_mapVerticesToMemory(staging);

        // copy staging to GPU memory
        staging.copyBuffer(pool, *this);
    }
};

template<class T>
using StaticBuffer = IStaticBuffer<std::vector<T>, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT>;

using StaticIndexBuffer = IStaticBuffer<std::vector<uint16_t>, VK_BUFFER_USAGE_INDEX_BUFFER_BIT>;

} // namespace Vulcain
