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
#include "engine/DescriptorPool.hpp"

namespace Vulcain {

template<class T>
class UniformBuffers : private std::vector<IBuffer>, public DeviceBound, public IRegenerable {
 public:
    UniformBuffers(DescriptorPool* descrPool) : DeviceBound(descrPool), IRegenerable(descrPool), _swapchain(descrPool->swapchain()) {
        _gen();
    }

    VkBuffer buffer(uint32_t currentImage) const {
        return (*this)[currentImage].buffer;
    }

    void mapToMemory(uint32_t currentImage, const T& ubo) {
        auto &buffer = (*this)[currentImage];
        auto memory = buffer.bufferMemory;

        //
        void* data;
        vkMapMemory(*_device, memory, 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(*_device, memory);
    }

 private:
    Swapchain* _swapchain = nullptr;
   
    void _gen() final {
        VkDeviceSize bufferSize = sizeof(T);
        for(int i = 0; i < _swapchain->imagesCount(); i++) {
            this->emplace_back(
                this, 
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
        }
    }
    
    void _degen() final {
        this->clear();
    }
};

} // namespace Vulcain
