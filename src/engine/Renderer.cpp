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

#include "Renderer.h"

Vulcain::Renderer::Renderer(CommandPool* pool, Vulcain::GlfwWindow* window) : DeviceBound(pool), _pool(pool), _window(window) {
    _createSyncObjects();
    
    //
    _window->_bindFramebufferChanges(&_hasFramebufferResized);
    _window->_bindDrawer(this);
}

Vulcain::Renderer::~Renderer() {
    // wait for device to stop processing
    vkDeviceWaitIdle(*_device);

    //
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(*_device, _renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(*_device, _imageAvailableSemaphores[i], nullptr);
        vkDestroyFence    (*_device, _inFlightFences[i], nullptr);
    }
}

void Vulcain::Renderer::bindUniformBufferUpdater(std::function<void(uint32_t)> updater) {
    _updateUniformBuffer = updater;
}

void Vulcain::Renderer::draw() {
    // wait fences from previous draw call
    vkWaitForFences(*_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result;

    // acquire image
    result = vkAcquireNextImageKHR(
        *_device, 
        *_swapchain(), 
        UINT64_MAX, 
        _imageAvailableSemaphores[_currentFrame], 
        VK_NULL_HANDLE, 
        &imageIndex
    );

    // if swapchain is outdated
    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        _regenerateSwapChain();
        return;
    }
    // still allow submoptimal swapchain
    assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

    // update uniform buffers there if any
    if(_updateUniformBuffer) _updateUniformBuffer(imageIndex);

    // if has an image in fight has fence on index, wait for it to be processed
    if (_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(*_device, 1, &_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    _imagesInFlight[imageIndex] = _inFlightFences[_currentFrame];

    //prepare submission
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {_imageAvailableSemaphores[_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    
    submitInfo.commandBufferCount = 1;
    VkCommandBuffer buffers[] = {_pool->commandBuffer(imageIndex)};
    submitInfo.pCommandBuffers = buffers;
    
    VkSemaphore signalSemaphores[] = {_renderFinishedSemaphores[_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(*_device, 1, &_inFlightFences[_currentFrame]);

    result = vkQueueSubmit(_device->queue(), 1, &submitInfo, _inFlightFences[_currentFrame]);
    assert(result == VK_SUCCESS);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {*_swapchain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    // present queue results
    result = vkQueuePresentKHR(_device->queue(), &presentInfo);

    // check if framebuffer has been resized or swapchain image size suboptimal
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _hasFramebufferResized) {
        _hasFramebufferResized = false;
        _regenerateSwapChain();
    } else {
        assert(result == VK_SUCCESS);
    }

    // update current frame
    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Vulcain::Renderer::_createSyncObjects() {
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    _imagesInFlight.resize(_pool->views()->count(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto r1 = vkCreateSemaphore(*_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]);
        assert(r1 == VK_SUCCESS);
        
        auto r2 = vkCreateSemaphore(*_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]);
        assert(r2 == VK_SUCCESS);
        
        auto r3 = vkCreateFence    (*_device, &fenceInfo,     nullptr, &_inFlightFences[i]);
        assert(r3 == VK_SUCCESS);
    }
}

Vulcain::Swapchain* Vulcain::Renderer::_swapchain() const {
    return _pool->views()->renderpass()->swapchain();
}

void Vulcain::Renderer::_regenerateSwapChain() {
    //
    _window->waitUntilSwapchainIsLegal();

    // wait
    vkDeviceWaitIdle(*_device);

    // regenerate chain
    _swapchain()->regenerate();
}