cmake_minimum_required(VERSION 3.15)

project(jate)

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

# Try extracting VulkanSDK path from ${Vulkan_INCLUDE_DIRS}
if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
    set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
    STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
endif()
 
if(NOT Vulkan_FOUND)
    # CMake may fail to locate the libraries but could be able to 
    # provide some path in Vulkan SDK include directory variable
    # 'Vulkan_INCLUDE_DIRS', try to extract path from this.
    message(STATUS "Failed to locate Vulkan SDK, retrying again...")
    if(EXISTS "${VULKAN_PATH}")
        message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
    else()
        message("Error: Unable to locate Vulkan SDK. Please turn off auto locate option by specifying 'AUTO_LOCATE_VULKAN' as 'OFF'")
        message("and specify manually path using 'VULKAN_SDK' and 'VULKAN_VERSION' variables in the CMakeLists.txt.")
        return()
    endif()
endif()

add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)

set(JATE_RESOURCES_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/jate_resources)

### Handle shaders compilation
set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${JATE_RESOURCES_OUTPUT}/shaders)
file(GLOB SHADERS
  ${SHADER_SOURCE_DIR}/*.vert
  ${SHADER_SOURCE_DIR}/*.frag
  ${SHADER_SOURCE_DIR}/*.comp
  ${SHADER_SOURCE_DIR}/*.geom
  ${SHADER_SOURCE_DIR}/*.tesc
  ${SHADER_SOURCE_DIR}/*.tese
  ${SHADER_SOURCE_DIR}/*.mesh
  ${SHADER_SOURCE_DIR}/*.task
  ${SHADER_SOURCE_DIR}/*.rgen
  ${SHADER_SOURCE_DIR}/*.rchit
  ${SHADER_SOURCE_DIR}/*.rmiss)

add_custom_command(
  COMMAND
    ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
  OUTPUT ${SHADER_BINARY_DIR}
  COMMENT "Creating ${SHADER_BINARY_DIR}"
)

foreach(source IN LISTS SHADERS)
  get_filename_component(FILENAME ${source} NAME)
  add_custom_command(
    COMMAND
      ${glslc_executable}
      #      -MD -MF ${SHADER_BINARY_DIR}/${FILENAME}.d
      -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
      ${source}
    OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
    DEPENDS ${source} ${SHADER_BINARY_DIR}
    COMMENT "Compiling ${FILENAME}"
  )
  list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(jate_resources ALL DEPENDS ${SPV_SHADERS})

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

    # Include Vulkan header files from Vulkan SDK
    include_directories(AFTER ${VULKAN_PATH}/Include)

    # Link directory for vulkan-1
    link_directories(${VULKAN_PATH}/Bin)

endif()

add_subdirectory(extern)

SET(JATE_SOURCES
    src/application.cpp

    src/window/window.cpp

    src/rendering/vulkan/vulkan_instance.cpp
    src/rendering/vulkan/vulkan_device.cpp
    src/rendering/vulkan/vulkan_swapchain.cpp
    src/rendering/vulkan/vulkan_pipeline.cpp
    src/rendering/vulkan/vulkan_vertexbuffer.cpp
    src/rendering/vulkan/vulkan_command_manager.cpp
    src/rendering/vulkan/vulkan_renderer.cpp
)

add_library(${PROJECT_NAME}
    ${JATE_SOURCES}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE extern/spdlog/include
    PRIVATE extern/glfw/include
)

target_link_libraries(${PROJECT_NAME}
    Vulkan::Vulkan
    glfw
    glm::glm
)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER source/convert.h)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)