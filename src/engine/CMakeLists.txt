#############
## Sources ##
#############

add_library(Vulcain-Engine INTERFACE)

target_sources(Vulcain-Engine PUBLIC 
    common/IRegenerable.cpp
    Renderer.cpp
)

target_include_directories(Vulcain-Engine INTERFACE
    ${CMAKE_SOURCE_DIR}/src
)

##################
## Dependencies ##
##################

#
# Vulkan Loader !
#

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    #standard Vulkan loader
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(Vulkan REQUIRED IMPORTED_TARGET vulkan)
    target_link_libraries(Vulcain-Engine INTERFACE PkgConfig::Vulkan)
    if (WIN32)
        target_compile_definitions(Vulcain-Engine INTERFACE VK_USE_PLATFORM_WIN32_KHR)
    endif()
    #define 
else()
    # optimised volk Vulkan loader
    if (WIN32)
        set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
    endif()
    add_subdirectory(deps/volk)
    target_compile_definitions(volk::volk PUBLIC USES_VOLK)
    target_link_libraries(Vulcain-Engine INTERFACE volk::volk)
endif()

#
# glfw
#

find_package(glfw3 REQUIRED)
target_link_libraries(Vulcain-Engine INTERFACE glfw3)
target_compile_definitions(Vulcain-Engine INTERFACE
    GLFW_INCLUDE_VULKAN
    GLFW_EXPOSE_NATIVE_WIN32
)

#
# glm
#

find_package(glm REQUIRED)
target_link_libraries(Vulcain-Engine INTERFACE glm)
target_compile_definitions(Vulcain-Engine INTERFACE
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)

#
# SPIR-V ShaderFoundry
#

target_link_libraries(Vulcain-Engine INTERFACE Vulcain::ShaderModules)

#
# VLA
#

add_library(VulkanMemoryAllocator INTERFACE)
target_include_directories(VulkanMemoryAllocator INTERFACE 
    deps/VMA/src
)
target_link_libraries(Vulcain-Engine INTERFACE VulkanMemoryAllocator)