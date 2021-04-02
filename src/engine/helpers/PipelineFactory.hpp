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

#include "ShaderFoundry.hpp"

#include "engine/DescriptorPools.hpp"
#include "engine/Renderpass.hpp"
#include "engine/Pipeline.hpp"

namespace Vulcain {

class PipelineFactory {
 public:
    PipelineFactory(const Renderpass* renderpass, DescriptorPools* descrPools) : 
        _foundry(renderpass->swapchain()->device()), 
        _renderpass(renderpass), 
        _descrPool(descrPools) {}
    
    Pipeline create(const char* moduleName) {
        return Pipeline(
            _renderpass, 
            _descrPool, 
            _foundry.modulesFromShaderName(moduleName) 
        );
    }
    
 private:
    ShaderFoundry _foundry;
    DescriptorPools* _descrPool = nullptr;
    const Renderpass* _renderpass = nullptr;
};

} // namespace Vulcain
