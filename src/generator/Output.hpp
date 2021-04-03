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

#include "Reflector.hpp"
#include "Args.hpp"

#include <magic_enum.hpp>

class Output {
 public:
    static std::vector<std::filesystem::path> generate(ReflectionPass &pass, const Args &args) {
        std::vector<std::filesystem::path> out;
        out.reserve(pass.size());

        for(auto const &[pipelineName, rFiles] : pass) {
            //
            auto outputPath = _generateCppFilePath(pipelineName, args.destinationDirectory);

            //
            for(auto const & rFile : rFiles) {
                if(!rFile.uniformBuffers.size()) continue;

                //
                std::ofstream stream(outputPath.string().c_str(), std::ofstream::trunc);
                _fillStreamFromReflectedFile(stream, rFile);
            }

            // add to results
            out.emplace_back(outputPath);
        }

        return out;
    }
 
 private:
    static std::filesystem::path _generateCppFilePath(const char* pipelineName, const std::filesystem::path &outputDirectoryPath) {
        auto temp = outputDirectoryPath / pipelineName;
        return temp.replace_extension(".hpp");
    }

    static void _fillStreamFromReflectedFile(std::ofstream& outStream, const ReflectedFile &rFile) {
        //
        outStream << "// This file is autogenerated" << "\n\n";

        //
        outStream << "#include \"generator/include/IDescriptorSetGenerator.h\"" << '\n';
        outStream << '\n';

        //
        _fillStreamFromReflectedUBs(outStream, rFile.uniformBuffers, rFile.stage);
    }

    static void _fillStreamFromReflectedUBs(std::ofstream& outStream, const UniformBuffersFiller::Container &container, VkShaderStageFlagBits stage) {
        //
        for(auto const &ub: container) {
            //
            outStream << "struct " << ub.name << " {" << '\n';

                //
                for(auto const &member : ub.members) {
                    outStream << '\t' << "glm::" << member.type << ' ' << member.name << ';' << '\n';
                }

                //
                outStream << '\n';
                outStream << '\t' << "static VkDescriptorSetLayoutBinding binding() {" << '\n';
                outStream << "\t\t" << "VkDescriptorSetLayoutBinding lBinding{};" << '\n';
                outStream << "\t\t" << "lBinding.binding = " << std::to_string(ub.binding) << ";" << '\n';
                outStream << "\t\t" << "lBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;" << '\n';
                outStream << "\t\t" << "lBinding.descriptorCount = 1;" << '\n';
                outStream << "\t\t" << "lBinding.stageFlags = " << magic_enum::enum_name(stage) << ";" << '\n';
                outStream << "\t\t" << "lBinding.pImmutableSamplers = nullptr; // Optional" << '\n';
                outStream << "\t\t" << "return lBinding;" << '\n';
                outStream << '\t' << '}' << '\n';

            //
            outStream << "};" << '\n';
        }
    }
};



