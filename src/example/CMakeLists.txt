add_executable(Vulcain-Example
    Example.cpp
)

target_link_libraries(Vulcain-Example PRIVATE 
    Vulcain-Engine
)

set_target_properties(Vulcain-Example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)