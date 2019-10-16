cmake_minimum_required(VERSION 3.5)
# set(CMAKE_C_COMPILER "gcc")
# set(CMAKE_CXX_COMPILER "g++")

project(CPSC817_FINAL)
set(EXE_NAME "sph")

set(USE_CUDA ON)

set(ASSET_PATH "${CMAKE_SOURCE_DIR}/assets")

# Maximum number of lights that can be processed by one pass
set(LIGHTING_BATCH_SIZE 20)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXE_LINKER_FLAGS "-std=c++11")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
    set(CMAKE_CXX_FLAGS  "/W4 /O2")
else()
    set(CMAKE_CXX_FLAGS "-Wall -O3")
endif()
if (APPLE)
    set(CMAKE_MACOSX_RPATH 0)
endif()
set(CMAKE_VERBOXE_MAKEFILE TRUE)

include_directories("${CMAKE_SOURCE_DIR}/lib")
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})
find_package(glfw3 3.2 REQUIRED)
# if (NOT glfw3_FOUND)
#     add_subdirectory("${CMAKE_SOURCE_DIR}/lib/glfw-3.2.1")
#     include_directories("${CMAKE_SOURCE_DIR}/lib/glfw-3.2.1/include")
# endif()
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")
# find_package(GLM REQUIRED)
# if (NOT GLM_FOUND)
    add_subdirectory("${CMAKE_SOURCE_DIR}/lib/glm")
    include_directories("${CMAKE_SOURCE_DIR}/lib/glm")
# endif()
 find_package(OpenMP)
 if (OPENMP_FOUND)
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
     set(PNET_USE_OPENMP ON)
 else()
     message (STATUS "No OpenMP support found.")
 endif()
find_package(BLAS REQUIRED)
message(STATUS ${BLAS_LIBRARIES})
include_directories(${BLAS_INCLUDE_DIRS})
set(SOURCE_LIBRARIES
        glfw
        ${GLEW_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        ${BLAS_LIBRARIES}
        cblas
    )
if (APPLE)
    include_directories("/usr/local/include")
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVID_LIBRARY CoreVideo REQUIRED)
    set(SOURCE_LIBRARIES ${SOURCE_LIBRARIES}
            ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVID_LIBRARY})
endif()

include_directories(${CMAKE_SOURCE_DIR}/assets)
include_directories(${CMAKE_SOURCE_DIR}/src)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
file(GLOB SHADER_FILES ${SOURCE_DIR}/shaders/*.cpp)
file(GLOB SCENE_HELPER_FILES ${SOURCE_DIR}/scene_helper/*.cpp)
file(GLOB SCENE_FILES ${SOURCE_DIR}/scenes/*.cpp)
file(GLOB UTIL_FILES ${SOURCE_DIR}/utils/*.cpp)
file(GLOB ITEM_FILES ${SOURCE_DIR}/items/*.cpp
#     ${SOURCE_DIR}/items/rigid/*.cpp
     )
set(SOURCE_FILES
        ${SOURCE_DIR}/glfw.cpp
        ${SOURCE_DIR}/shader.cpp
        ${SOURCE_DIR}/error.cpp
        ${SOURCE_DIR}/camera.cpp
        ${SOURCE_DIR}/scene.cpp
        ${SOURCE_DIR}/item.cpp
        ${SOURCE_DIR}/menu.cpp
        ${SOURCE_DIR}/app.cpp
        ${SHADER_FILES}
        ${SCENE_FILES}
        ${SCENE_HELPER_FILES}
        ${ITEM_FILES}
        ${UTIL_FILES}
        ${SOURCE_DIR}/main.cpp)
configure_file(config.h.in ${SOURCE_DIR}/config.h @ONLY)

if (USE_CUDA)
    file(GLOB CUDA_SOURCE_FILES
         ${SOURCE_DIR}/scenes/*.cu)
    find_package(CUDA 5.0.0 REQUIRED)
    include_directories(${CUDA_INCLUDE_DIRS})
    add_definitions("-DUSE_CUDA=${USE_CUDA}")
    if (MSVC)
        set(CUDA_HOST_COMPILER MSVC_CL_EXE)
    endif()
    set(CUDA_NVCC_FLAGS
        -std=c++11
        -use_fast_math
        -restrict
        --ptxas-options=-O3
        -O3
#        --default-stream=per-thread
        #        --ptxas-options=-v
        #        -res-usage
        )
    cuda_add_executable(${EXE_NAME}
                        ${SOURCE_FILES}
                        ${CUDA_SOURCE_FILES})
else()
    add_executable(${EXE_NAME} ${SOURCE_FILES})
endif()

target_link_libraries(${EXE_NAME}
                      ${SOURCE_LIBRARIES}
                      ${CUDA_CUDART_LIBRARY}
                      ${CUDA_CUBLAS_LIBRARIES})

set(CMAKE_BUILD_TYPE RELEASE)
set(CMAKE_VERBOSE_MAKEFILE ON)
