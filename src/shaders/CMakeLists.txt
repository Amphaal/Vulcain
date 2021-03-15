find_program(GLSL_VALIDATOR glslangValidator REQUIRED)

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "*.frag"
    "*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV ${CMAKE_CURRENT_SOURCE_DIR}/generated/${FILE_NAME}.spv)
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
    WHENCE generated
    ${SPIRV_BINARY_FILES}
)