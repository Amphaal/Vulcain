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

#include <vulkan/vulkan.h>

#include "Args.hpp"
#include "reflection/UniformBuffers.hpp"

#include <map>
#include <utility>
#include <fstream>
#include <iterator>

struct ReflectedFile {
    VkShaderStageFlagBits stage;
    UniformBuffersFiller::Container uniformBuffers;
};

using ReflectionPass = std::map<const char *, std::vector<ReflectedFile>>;

const std::map<const char*, VkShaderStageFlagBits> FIND_STAGE_FROM_EXT {
    { ".vert", VK_SHADER_STAGE_VERTEX_BIT },
    { ".frag", VK_SHADER_STAGE_FRAGMENT_BIT },
    { ".geom", VK_SHADER_STAGE_GEOMETRY_BIT }
};

class Reflector {
 public:
    Reflector(const Args* args) : _args(args) {}
 
    ReflectionPass reflect() {
        ReflectionPass pass;

        for(const auto &filePath : _args->toReflectSPRIRVFiles) {
            // add file to pass
            ReflectedFile rFile{};
            
            // fill stage
            auto filename = _getStageFlag(filePath, rFile.stage);

            // get objects
            _reflectShaderFile(filePath, rFile);

            // insert into pass
            auto pipelineFound = pass.find(filename.c_str());
            if(pipelineFound == pass.cend()) {
                std::vector<ReflectedFile> v(1);
                v[0] = std::move(rFile);
               pass.emplace(filename.c_str(), std::move(v));
            } else {
                pipelineFound->second.push_back(std::move(rFile));
            }
        }

        return pass;
    };

 private:
    const Args* _args = nullptr;

    static void _reflectShaderFile(const std::filesystem::path &filePath, ReflectedFile &rFile) {
        // get file buffer
        auto buffer = _readFile(filePath);

        // parse spirv file
        using namespace spirv_cross;
        Compiler comp(buffer.cbegin().base(), buffer.size());
        GLSLCompilerWrapper glslComp{buffer.cbegin().base(), buffer.size()};

        // get resources
        ShaderResources resources = comp.get_shader_resources();

        // fill
        UniformBuffersFiller::fillMetadata(comp, resources, glslComp, rFile.uniformBuffers);
    }

    static std::vector<uint32_t> _readFile(const std::filesystem::path &filePath) {
        // stream for file, already open
        std::ifstream stream(filePath.string().c_str(), std::ifstream::binary);

        // get size of file
        stream.ignore( std::numeric_limits<std::streamsize>::max() );
        auto fileSize = stream.gcount();
        stream.clear();   //  Since ignore will have set eof.
        stream.seekg( 0, std::ios_base::beg );

        // read it
        std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
        stream.read(reinterpret_cast<char *>(buffer.begin().base()), fileSize);

        //
        return buffer;
    }

    static std::string _getStageFlag(const std::filesystem::path& path, VkShaderStageFlagBits& bit) {
        auto filename = path.stem().string();

        for(auto [possibleExt, flag] : FIND_STAGE_FROM_EXT) {
            auto found = filename.find(possibleExt);
            if(found != std::string::npos) {
                bit = flag;
                return filename.substr(0, found);
            }
        }

        throw std::logic_error("Cannot determine associated stage for [" + path.string() + "] file");
    }
};