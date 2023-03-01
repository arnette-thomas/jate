cmake_minimum_required(VERSION 3.15)

project(sandbox)

add_executable(sandbox main.cpp)

target_link_libraries(sandbox
    PRIVATE jate
)

target_compile_features(sandbox PUBLIC cxx_std_17)

# Copy jate resources next to executable
set(JATE_RESOURCES_DIR "$<TARGET_FILE_DIR:sandbox>/jate_resources")
add_custom_command(
    TARGET sandbox
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${jate_BINARY_DIR}/jate_resources"
        "${JATE_RESOURCES_DIR}"
    DEPENDS jate_resources
)
add_custom_target(sandbox_jate_resources ALL DEPENDS ${JATE_RESOURCES_DIR})