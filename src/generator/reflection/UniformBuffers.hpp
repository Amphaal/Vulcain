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

#include "IFiller.hpp"

#include <string>

struct UB_Member {
    std::string type;
    std::string name;
};

struct UB {
    std::string name;
    uint32_t binding = 0;
    uint32_t set = 0;
    std::vector<UB_Member> members;
};

class UniformBuffersFiller : public IFiller<UniformBuffersFiller, UB> {
 public:
    static void fillMetadata(spirv_cross::Compiler &comp, spirv_cross::ShaderResources &resources, GLSLCompilerWrapper &glslComp, IFiller::Container &ubs) {
        // get uniform buffers data
        ubs.resize(resources.uniform_buffers.size());

        // iterate
        for (int i = 0; i < ubs.size(); i++) {
            auto &u_source = resources.uniform_buffers[i];
            auto &ub = ubs[i];
            
            // get binding and set
            ub.binding = comp.get_decoration(u_source.id, spv::DecorationBinding);
            ub.set = comp.get_decoration(u_source.id, spv::DecorationDescriptorSet);

            // find UB name
            ub.name = u_source.name;
            
            // find members
            auto ranges = comp.get_active_buffer_ranges(u_source.id);
            auto memberTypes = comp.get_type(u_source.base_type_id).member_types;
            ub.members.resize(ranges.size());

            // fill members
            for (int y = 0; y < ub.members.size(); y++) {
                auto &range = ranges[y];
                auto &member = ub.members[range.index];
                auto type = comp.get_type(memberTypes[range.index]);

                //
                member.name = comp.get_member_name(u_source.base_type_id, range.index);
                member.type = glslComp.type_to_glsl(type);
            }
        }
    }
};
