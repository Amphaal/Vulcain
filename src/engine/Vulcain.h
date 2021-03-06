#pragma once

#ifdef USES_VOLK
#include <volk.h>
#else
#include <vulkan/vulkan.h>
#endif

namespace Vulcain {

static VkApplicationInfo info(const char* appName, uint32_t version = VK_MAKE_VERSION(1, 0, 0)) {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = version;
    appInfo.pEngineName = "Vulcain";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    return appInfo;
};

}