# policies first
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0048 NEW)
endif()

cmake_minimum_required(VERSION 3.19.3 FATAL_ERROR)

project(Vulcain-Reflecteur
    VERSION 0.0.1
    LANGUAGES CXX
)

################################
## CPP Compiler Configuration ##
################################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#############
## Sources ##
#############

add_executable(${PROJECT_NAME}
    main.cpp
)

##################
## Dependencies ##
##################

#
# SPIR-V Cross (Reflexion)
#

#TODO once llvm 12 is live, statically link
find_package(spirv_cross_c_shared REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-c-shared)

#######################
## Deps : magic_enum ##
#######################

add_subdirectory(deps/magic_enum)

#link
target_link_libraries(${PROJECT_NAME} PRIVATE magic_enum::magic_enum)
