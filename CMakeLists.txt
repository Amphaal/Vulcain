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

################################
## CPP Compiler Configuration ##
################################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)

#############
## Sources ##
#############

add_executable(Vulcain 
    src/main.cpp
)

target_include_directories(Vulcain PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

##################
## Dependencies ##
##################

find_package(PkgConfig REQUIRED)

pkg_check_modules(Vulkan REQUIRED IMPORTED_TARGET vulkan)
target_link_libraries(Vulcain PRIVATE PkgConfig::Vulkan)

find_package(glfw3 REQUIRED)
target_link_libraries(Vulcain PRIVATE glfw3)

find_package(glm REQUIRED)
target_link_libraries(Vulcain PRIVATE glm)

#
## 
#

add_subdirectory(shaders)
add_dependencies(Vulcain Shaders)
