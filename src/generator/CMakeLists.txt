#############
## Sources ##
#############

add_executable(Vulcain-Reflecteur
    Reflecteur.cpp
)

set_target_properties(Vulcain-Reflecteur PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

##################
## Dependencies ##
##################

#
# SPIR-V Cross (Reflexion)
#

#TODO once llvm 12 is live, statically link
find_package(spirv_cross_c_shared REQUIRED)
target_link_libraries(Vulcain-Reflecteur PRIVATE spirv-cross-c-shared)