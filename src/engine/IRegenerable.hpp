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

#include <stack>

namespace Vulcain {

class IRegenerable {
 public:
    IRegenerable(IRegenerable* chainPredecessor = nullptr) : _chainPredecessor(chainPredecessor) {}
    
    void regenerate() {
        std::stack<IRegenerable*> stack;

        // degen FIFO style
        auto cp = this;
        while(cp) {
           cp->_degen();
           stack.push(cp);
           cp = cp->_chainPredecessor;
        }

        // gen LIFO style
        while (!stack.empty()) {
            stack.top()->_gen();
            stack.pop();
        }
    }
 
 protected:
    virtual void _gen() = 0;
    virtual void _degen() = 0;
 
 private:
    IRegenerable* _chainPredecessor = nullptr;
};

}; // namespace Vulcain
