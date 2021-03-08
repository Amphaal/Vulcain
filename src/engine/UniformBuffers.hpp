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
#include "DescriptorPool.hpp"

#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Vulcain {

template<class T>
class UniformBuffers : private std::vector<IBuffer>, public IRegenerable {
 public:
    UniformBuffers(DescriptorPool* descrPool) : IRegenerable(views), _device(views->renderpass()->swapchain()->device()), _views(descrPool->views()) {
        _gen();
    }

    void updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        auto swapChainExtent = _views->renderpass()->swapchain()->imageExtent;

        UBO_MVP ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        _mapToMemory(currentImage, ubo);
    }

 private:
    Device* _device = nullptr;
    ImageViews* _views = nullptr;

    void _mapToMemory(uint32_t currentImage, const UBO_MVP& ubo) {
        auto &buffer = (*this)[currentImage].bufferMemory;

        //
        void* data;
        vkMapMemory(_device->get(), buffer, 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(_device->get(), buffer);
    }
    
    void _gen() final {
        VkDeviceSize bufferSize = sizeof(T);
        for(int i = 0; i < _views->count(); i++) {
            this->emplace_back(
                _device, 
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
