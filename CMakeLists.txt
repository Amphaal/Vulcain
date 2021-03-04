# use ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message("== Using CCache ! ==")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif()

# minimal required
cmake_minimum_required(VERSION 3.19.3 FATAL_ERROR)

# project definition
project(Vulcain
    VERSION 0.0.1
    DESCRIPTION "Toy project for Vulkan oriented graphics"
    HOMEPAGE_URL "https://github.com/Amphaal/Vulcain"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
)

################################
## CPP Compiler Configuration ##
################################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

#############
## Sources ##
#############

add_executable(Vulcain 
    src/engine/Renderer.cpp
    src/main.cpp
)

target_include_directories(Vulcain PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
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
        target_link_libraries(Vulcain PRIVATE PkgConfig::Vulkan)
    else()
        # optimised volk Vulkan loader
        if (WIN32)
            set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
        endif()
        add_subdirectory(deps/volk)
        target_link_libraries(Vulcain PRIVATE volk)
        add_compile_definitions(USES_VOLK)
    endif()

#
# glfw
#

    find_package(glfw3 REQUIRED)
    target_link_libraries(Vulcain PRIVATE glfw3)

#
# glm
#

    find_package(glm REQUIRED)
    target_link_libraries(Vulcain PRIVATE glm)

#
# SPIR-V ShaderFoundry
#

    add_subdirectory(shaders)
    target_link_libraries(Vulcain PRIVATE shadersResources)

#
# VLA
#

    add_library(VulkanMemoryAllocator INTERFACE)
    target_include_directories(VulkanMemoryAllocator INTERFACE 
        deps/VMA/src
    )
    target_link_libraries(Vulcain PRIVATE VulkanMemoryAllocator)