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

#include "ImageViews.hpp"

namespace Vulcain {

class DescriptorPool : public DeviceBound, public IRegenerable {
 public:
    DescriptorPool(ImageViews *views) : DeviceBound(views), IRegenerable(views), _views(views) {
        _gen();
    }

    ~DescriptorPool() {
        _degen();
    }

    ImageViews* views() const {
        return _views;
    }

 private:
    ImageViews* _views = nullptr;
    VkDescriptorPool _descriptorPool;
    uint32_t _count = 0;

    void _gen() final {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(_views->count());

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = poolSize.descriptorCount;

        auto result = vkCreateDescriptorPool(*_device, &poolInfo, nullptr, &_descriptorPool);
        assert(result == VK_SUCCESS);
    }

    void _degen() final {
        vkDestroyDescriptorPool(*_device, _descriptorPool, nullptr);
    }
};

} // namespace Vulcain
