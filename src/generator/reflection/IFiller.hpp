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

#include <vector>

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

class GLSLCompilerWrapper : public spirv_cross::CompilerGLSL {
 public:
    GLSLCompilerWrapper(const uint32_t *ir_, size_t word_count) : spirv_cross::CompilerGLSL(ir_, word_count) {}

    std::string type_to_glsl(const spirv_cross::SPIRType &type, uint32_t id = 0) {
        return spirv_cross::CompilerGLSL::type_to_glsl(type, id);
    }
};

// using https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern 
template<class R, class T>
class IFiller {
 public:
    using Container = std::vector<T>; 
    static void fillMetadata(spirv_cross::Compiler &comp, spirv_cross::ShaderResources &resources, GLSLCompilerWrapper &glslComp, Container &toBeFilled) {
        R::fillMetadata(comp, resources, glslComp, toBeFilled);
    }
};
