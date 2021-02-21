#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <vector>

#include <stdio.h>
#include <string.h>

#include <vulkan/vulkan.h>

namespace Vulcain {

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
    VkDebugUtilsMessageTypeFlagsEXT messageType, 
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
    void* pUserData
    ) {
    //
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

class AvailableExtensions {
 public:
    AvailableExtensions() {
        // get available extensions
        vkEnumerateInstanceExtensionProperties(nullptr, &_extensionCount, nullptr);
        assert(_extensionCount);
        _extensions.resize(_extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &_extensionCount, _extensions.data());
    }

    void assertAllAndInsert(const char **extsToUse, int extsToUseCount, std::vector<const char*> &toPopulate) const {
        for(auto requiredExtCount = 0; requiredExtCount < extsToUseCount; requiredExtCount++) {
            //
            const auto & requiredExt = extsToUse[requiredExtCount];
            
            //
            auto requiredIsAvailable = _extensionsContains(requiredExt);

            //
            assert(requiredIsAvailable);

            //
            toPopulate.push_back(requiredExt);
        }
    }

    void assertAll(const std::vector<const char*> &extsToUse) {
        for(auto const & requiredExt : extsToUse) {            
            //
            auto requiredIsAvailable = _extensionsContains(requiredExt);

            //
            assert(requiredIsAvailable);
        }
    }

 private:
    std::vector<VkExtensionProperties> _extensions;
    uint32_t _extensionCount = 0;

    bool _extensionsContains(const char *const &requiredExt) const{
        for(const auto &available : _extensions) {
            if(strcmp(requiredExt, available.extensionName) == 0) {
                return true;
            }
        }
        return false;
    }
};

class CreateInfo : public VkInstanceCreateInfo {
 public:
    CreateInfo(const VkApplicationInfo * appInfo) : VkInstanceCreateInfo{} {
        //
        assert(appInfo);

        // bind data
        this->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        this->pApplicationInfo = appInfo;

        /* order is important !*/
        if(getenv("VK_LAYER_PATH")) { // if not layer path is hardcoded, assume it is prod env and skipping
            _bindValidationLayers();
            _addDebugCallback(Vulcain::debugCallback);
        }

        //
        _bindRequiredExtensions();
    }

    const VkDebugUtilsMessengerCreateInfoEXT* debugUtil() const {
        return _debugInfo;
    }

    ~CreateInfo() {
        if(_debugInfo) delete _debugInfo;
    }

 private:
    static inline std::vector<const char*> WANTED_LAYERS {
        "VK_LAYER_KHRONOS_validation"
    };
    void _bindValidationLayers() {
        //
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        VkLayerProperties availableLayers[layerCount];
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

        // check
        for (const char* layerName : WANTED_LAYERS) {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            assert(layerFound);
        }

        //
        _required_exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        //
        this->ppEnabledLayerNames = WANTED_LAYERS.data();
        this->enabledLayerCount = WANTED_LAYERS.size();
    }   
    
    std::vector<const char*> _required_exts;
    void _bindRequiredExtensions() {
        // check layout required ext
        AvailableExtensions available;
        available.assertAll(_required_exts);

        // check GLFW required ext
        {
            // get the extensions required by glfw
            uint32_t glfwExtensionCount = 0;
            auto extsToUse = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            assert(glfwExtensionCount); // no extensions required, has glfw been launched ?

            // check if all of them are available,
            available.assertAllAndInsert(extsToUse, glfwExtensionCount, _required_exts);
        }

        //
        this->enabledExtensionCount = _required_exts.size();
        this->ppEnabledExtensionNames = _required_exts.data();
    }

    VkDebugUtilsMessengerCreateInfoEXT* _debugInfo = nullptr;
    void _addDebugCallback(PFN_vkDebugUtilsMessengerCallbackEXT cb) {
        assert(!_debugInfo);

        _debugInfo = new VkDebugUtilsMessengerCreateInfoEXT{};
        _debugInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        _debugInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        _debugInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        _debugInfo->pfnUserCallback = cb;

        this->pNext = _debugInfo;
    }
};

class Instance {
 public:
    Instance(const CreateInfo* createInfos) : _createInfos(createInfos) {
        assert(createInfos);
        //
        auto result = vkCreateInstance(_createInfos, nullptr, &_instance);
        assert(result == VK_SUCCESS); 

        //
        _mayCreateDebugMessenger();
    }

    ~Instance() {
        //
        if(_debugMessenger) {
            auto DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
            assert(DestroyDebugUtilsMessengerEXT);
            DestroyDebugUtilsMessengerEXT(_instance, *_debugMessenger, nullptr);
        }

        //
        vkDestroyInstance(_instance, nullptr);
    }

    VkInstance* get() {
        return &_instance;
    }

 private:
    VkDebugUtilsMessengerEXT* _debugMessenger = nullptr;
    void _mayCreateDebugMessenger() {
        //
        auto debugUtil = _createInfos->debugUtil();
        if(!debugUtil) return;

        //
        _debugMessenger = new VkDebugUtilsMessengerEXT;
        auto CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
        auto result = CreateDebugUtilsMessengerEXT(_instance, debugUtil, nullptr, _debugMessenger);
        assert(result == VK_SUCCESS);
    };


    const CreateInfo* _createInfos = nullptr;
    VkInstance _instance;
};

}; // namespace Vulcain

class WindowHandler {
 public:    
    WindowHandler() {
        //
        glfwInit();
        _inited = true;

        //
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        _window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
    }

    ~WindowHandler() {
        if(_window) glfwDestroyWindow(_window);
        if(_inited) glfwTerminate();
    }

    void waitForWindowEvents() {
        while(!glfwWindowShouldClose(_window)) {
            glfwWaitEvents();
        }
    }

 private:
    GLFWwindow* _window = nullptr;
    bool _inited = false;
};


int main() {
    WindowHandler handler;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    Vulcain::CreateInfo createInfos(&appInfo);
    Vulcain::Instance instance(&createInfos);

    return 0;
}