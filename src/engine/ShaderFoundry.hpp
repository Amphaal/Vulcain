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

#include <iostream>
#include <map>
#include <filesystem>

#include "Device.hpp"

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(shadersResources);

namespace Vulcain {

class ShaderFoundry {
 public:
    using CreateInfoByStage = std::map<VkShaderStageFlagBits, VkPipelineShaderStageCreateInfo>;
    using ModulesByPipeline = std::map<std::string, CreateInfoByStage>;
    using Modules = std::vector<VkPipelineShaderStageCreateInfo>;

    ShaderFoundry(Device* device) : _device(device) {
        _createShaderModules();
    }

    // get the pipeline structs necessary from each module of a shader
    Modules modulesFromShaderName(const std::string& shaderName) {
        Modules out;

        auto found = _pipelines.find(shaderName);
        assert(found != _pipelines.end());

        for(auto& [flag, instr] : found->second) {
            out.push_back(instr);
        }

        return out;
    }

    ~ShaderFoundry() {
        for(auto module : _modules) {
            vkDestroyShaderModule(_device->get(), module, nullptr);
        }
    }   

 private:
    static inline std::map<std::string, VkShaderStageFlagBits> STAGE_FROM_EXT {
        { ".vert", VK_SHADER_STAGE_VERTEX_BIT },
        { ".frag", VK_SHADER_STAGE_FRAGMENT_BIT }
    };

    void _createShaderModules() {
        //
        auto fs = cmrc::shadersResources::get_filesystem();

        //
        for(const auto &i : fs.iterate_directory("/")) {
            //
            if(!i.is_file()) continue;

            //
            auto stem = std::filesystem::path(i.filename()).stem();
            auto p = std::filesystem::path(stem);
            auto name = p.stem().string();
            auto stage = p.extension().string();

            //
            auto find_stageFlag = STAGE_FROM_EXT.find(stage);
            assert(find_stageFlag != STAGE_FROM_EXT.end());
            auto stageFlag = find_stageFlag->second;

            // ensure pipeline exists
            if(_pipelines.find(name) == _pipelines.end()) {
                _pipelines.emplace(name, CreateInfoByStage{});
            }

            //
            auto pipeline = _pipelines.find(name);
            auto &stages = pipeline->second;

            //
            auto module = _createShaderModule(fs, i.filename());

            //
            VkPipelineShaderStageCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            createInfo.stage = stageFlag;
            createInfo.module = module;
            createInfo.pName = "main";

            //
            stages.emplace(stageFlag, createInfo);
        }

        //
        assert(_pipelines.size() != 0);
    }

    VkShaderModule _createShaderModule(cmrc::embedded_filesystem& fs, const std::string& path) {
        //
        auto shaderBinary = fs.open(path);

        //        
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderBinary.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBinary.begin());

        //
        VkShaderModule shaderModule;
        auto result = vkCreateShaderModule(_device->get(), &createInfo, nullptr, &shaderModule);
        assert(result == VK_SUCCESS);
        
        //
        _modules.push_back(shaderModule);

        //
        return shaderModule;
    }

    std::vector<VkShaderModule> _modules;
    ModulesByPipeline _pipelines;
    Device* _device = nullptr;
};

} // namespace Vulcain
