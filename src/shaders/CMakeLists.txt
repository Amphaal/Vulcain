find_program(GLSL_VALIDATOR glslangValidator REQUIRED)

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "*.frag"
    "*.vert"
)

SET(GENERATED_SPIRV_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated_spirv)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV ${GENERATED_SPIRV_DIRECTORY}/${FILE_NAME}.spv)
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

include(CMakeRC)

cmrc_add_resource_library(shaderModules 
    ALIAS Vulcain::ShaderModules
    WHENCE ${GENERATED_SPIRV_DIRECTORY}
    ${SPIRV_BINARY_FILES}
)