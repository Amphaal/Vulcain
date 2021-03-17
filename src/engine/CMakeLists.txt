#############
## Sources ##
#############

add_library(${PROJECT_NAME}-Engine INTERFACE)

target_sources(${PROJECT_NAME}-Engine PUBLIC 
    common/IRegenerable.cpp
    Renderer.cpp
)

target_include_directories(${PROJECT_NAME}-Engine INTERFACE
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
    target_link_libraries(${PROJECT_NAME}-Engine INTERFACE PkgConfig::Vulkan)
    if (WIN32)
        target_compile_definitions(${PROJECT_NAME}-Engine INTERFACE VK_USE_PLATFORM_WIN32_KHR)
    endif()
    #define 
else()
    # optimised volk Vulkan loader
    if (WIN32)
        set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
    endif()
    add_subdirectory(deps/volk)
    target_compile_definitions(volk::volk PUBLIC USES_VOLK)
    target_link_libraries(${PROJECT_NAME}-Engine INTERFACE volk::volk)
endif()

#
# glfw
#

find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME}-Engine INTERFACE glfw3)
target_compile_definitions(${PROJECT_NAME}-Engine INTERFACE
    GLFW_INCLUDE_VULKAN
    GLFW_EXPOSE_NATIVE_WIN32
)

#
# glm
#

find_package(glm REQUIRED)
target_link_libraries(${PROJECT_NAME}-Engine INTERFACE glm)
target_compile_definitions(${PROJECT_NAME}-Engine INTERFACE
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)

#
# SPIR-V ShaderFoundry
#

target_link_libraries(${PROJECT_NAME}-Engine INTERFACE ${PROJECT_NAME}::ShaderModules)

#
# SPIR-V HPP
#

target_link_libraries(${PROJECT_NAME}-Engine INTERFACE ${PROJECT_NAME}::SPIRVHeaders)

#
# VLA
#

add_library(VulkanMemoryAllocator INTERFACE)
target_include_directories(VulkanMemoryAllocator INTERFACE 
    deps/VMA/src
)
target_link_libraries(${PROJECT_NAME}-Engine INTERFACE VulkanMemoryAllocator)