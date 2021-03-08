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

#include "IRegenerable.h"

#include "Debug.hpp"

#include <assert.h>

Vulcain::IRegenerable::IRegenerable(IRegenerable* chainPredecessor) {
    if (chainPredecessor) {
        _children.push(chainPredecessor);
    }
}

Vulcain::IRegenerator::IRegenerator() : IRegenerable(nullptr) { }

void Vulcain::IRegenerator::_fillPipes(std::stack<IRegenerable*>& stack, std::queue<IRegenerable*>& queue, IRegenerable* target, int level) {
    stack.push(target);
    queue.push(target);

        // log
        auto pad = [](int level) {
            return std::string(level, '--');
        };
        std::cout << pad(level) << Debug::demanglePtr(target) << std::endl;
        
    if(target->_children.empty()) return;
    auto cp = target->_children;

    while (!cp.empty()) {
        auto child = stack.top();
        
        cp.pop();
        level++;

        _fillPipes(stack, queue, child, level);
    }
}

void Vulcain::IRegenerator::regenerate() {
    std::stack<IRegenerable*> stack;
    std::queue<IRegenerable*> queue;
    _fillPipes(stack, queue, this);

    // degen LIFO style
    while (!stack.empty()) {
        stack.top()->_degen();
        stack.pop();
    }

    // gen FIFO style
    while (!queue.empty()) {
        queue.front()->_gen();
        queue.pop();
    }
}
