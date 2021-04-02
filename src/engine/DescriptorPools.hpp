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

#include "Swapchain.hpp"

namespace Vulcain {

class DescriptorPools : public DeviceBound, public IRegenerable {
 public:
    DescriptorPools(Swapchain* swapchain) : DeviceBound(swapchain), IRegenerable(swapchain), _swapchain(swapchain) {}

    // might create specialized pool if needed
    VkDescriptorPool pool(VkDescriptorType type) {
        //
        if(!_descriptorPools.contains(type)) {
            _createDescrPool(type);
        }
        
        //
        return _descriptorPools[type];
    }

    ~DescriptorPools() {
        _degen();
    }

    const Swapchain* swapchain() const {
        return _swapchain;
    }

 private:
    const Swapchain* _swapchain = nullptr;
    std::map<VkDescriptorType, VkDescriptorPool> _descriptorPools;

    void _createDescrPool(VkDescriptorType type) {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = type;
        poolSize.descriptorCount = static_cast<uint32_t>(_swapchain->imagesCount());

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = poolSize.descriptorCount;

        auto result = vkCreateDescriptorPool(*_device, &poolInfo, nullptr, &_descriptorPools[type]);
        assert(result == VK_SUCCESS);
    }

    void _gen() final {
        for(auto [type, pool] : _descriptorPools) {
            _createDescrPool(type);
        }
    }

    void _degen() final {
        for(auto [type, pool] : _descriptorPools) {
            vkDestroyDescriptorPool(*_device, pool, nullptr);
        }
    }
};

} // namespace Vulcain
