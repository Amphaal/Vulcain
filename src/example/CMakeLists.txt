add_executable(${PROJECT_NAME}-Example
    main.cpp
)

target_link_libraries(${PROJECT_NAME}-Example PRIVATE 
    ${PROJECT_NAME}-Engine
)

set_target_properties(${PROJECT_NAME}-Example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)