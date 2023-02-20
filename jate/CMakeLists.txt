cmake_minimum_required(VERSION 3.15)

project(jate)

find_package(Vulkan REQUIRED)

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

    src/vulkan/vulkan_instance.cpp
    src/vulkan/vulkan_device.cpp
    src/vulkan/vulkan_swapchain.cpp
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
)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER source/convert.h)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)