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

#include "CommandPool.hpp"

namespace Vulcain {

class Renderer {
 public:
    Renderer(CommandPool* pool) : _pool(pool) {
        _createSyncObjects();
    }

    void draw() {
        vkWaitForFences(_device()->get(), 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(
            _device()->get(), 
            _swapchain()->get(), 
            UINT64_MAX, 
            _imageAvailableSemaphores[_currentFrame], 
            VK_NULL_HANDLE, 
            &imageIndex
        );

        if (_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(_device()->get(), 1, &_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        _imagesInFlight[imageIndex] = _inFlightFences[_currentFrame];

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

        vkResetFences(_device()->get(), 1, &_inFlightFences[_currentFrame]);

        auto result = vkQueueSubmit(_device()->queue(), 1, &submitInfo, VK_NULL_HANDLE);
        assert(result == VK_SUCCESS);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {_swapchain()->get()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        vkQueuePresentKHR(_device()->queue(), &presentInfo);

        //
        _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    ~Renderer() {
        // wait for device to stop processing
        vkDeviceWaitIdle(_device()->get());

        //
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(_device()->get(), _renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(_device()->get(), _imageAvailableSemaphores[i], nullptr);
            vkDestroyFence    (_device()->get(), _inFlightFences[i], nullptr);
        }
    }

 private:
    static const int MAX_FRAMES_IN_FLIGHT = 2;
    size_t _currentFrame = 0;

    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;
    std::vector<VkFence> _imagesInFlight;

    CommandPool* _pool = nullptr;

    void _createSyncObjects() {
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
            auto r1 = vkCreateSemaphore(_device()->get(), &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]);
            assert(r1 == VK_SUCCESS);
            
            auto r2 = vkCreateSemaphore(_device()->get(), &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]);
            assert(r2 == VK_SUCCESS);
            
            auto r3 = vkCreateFence    (_device()->get(), &fenceInfo,     nullptr, &_inFlightFences[i]);
            assert(r3 == VK_SUCCESS);
        }
    }

    Device* _device() const {
        return _pool->views()->renderpass()->swapchain()->device();
    }

    Swapchain* _swapchain() const {
        return _pool->views()->renderpass()->swapchain();
    }
};

}; // namespace Vulcain
