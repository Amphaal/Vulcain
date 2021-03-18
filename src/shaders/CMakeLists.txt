##
## find GLSL files
##

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "*.frag"
    "*.vert"
)

##
## Generate Reflecteur tool
##

include(ExternalProject)
ExternalProject_Add(SPIRVCppTool
    INSTALL_COMMAND ""
    CMAKE_ARGS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/generator
)
ExternalProject_Get_Property(SPIRVCppTool BINARY_DIR)
SET(REFLECTEUR_BIN ${BINARY_DIR}/${PROJECT_NAME}-Reflecteur)

##
## generate SPIRV into bytecode
##

find_program(GLSL_VALIDATOR glslangValidator REQUIRED)
SET(GENERATED_SPIRV_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated_spirv)

foreach(GLSL ${GLSL_SOURCE_FILES})
    #
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV ${GENERATED_SPIRV_DIRECTORY}/${FILE_NAME}.spv)

    #
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )

    #
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach()

include(CMakeRC)

cmrc_add_resource_library(shaderModules 
    ALIAS ${PROJECT_NAME}::ShaderModules
    WHENCE ${GENERATED_SPIRV_DIRECTORY}
    ${SPIRV_BINARY_FILES}
)

##
## create Reflecteur target
##

SET(GENERATED_SPIRV_HPP_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated_hpp)

foreach(SPIRV ${SPIRV_BINARY_FILES})
    #
    get_filename_component(EXPECTED_GENERATED_HPP ${SPIRV} NAME_WLE)
    set(EXPECTED_GENERATED_HPP ${GENERATED_SPIRV_HPP_DIRECTORY}/${EXPECTED_GENERATED_HPP}.hpp)

    #
    add_custom_command(
        OUTPUT ${EXPECTED_GENERATED_HPP}
        COMMAND ${REFLECTEUR_BIN} ${SPIRV} ${GENERATED_SPIRV_HPP_DIRECTORY}
        DEPENDS ${SPIRV}
    )

    #
    list(APPEND SPIRV_HPP_FILES ${EXPECTED_GENERATED_HPP})
endforeach()

##
## create target
##

add_library(SPIRVHeaders INTERFACE ${SPIRV_HPP_FILES})
add_library(${PROJECT_NAME}::SPIRVHeaders ALIAS SPIRVHeaders)
set_target_properties(SPIRVHeaders PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(SPIRVHeaders INTERFACE ${GENERATED_SPIRV_HPP_DIRECTORY})
add_dependencies(SPIRVHeaders SPIRVCppTool)
