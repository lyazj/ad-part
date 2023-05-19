cmake_minimum_required(VERSION 3.13)
project(ad-part VERSION 0.0.1)

# 设置项目包含目录、库目录和可执行文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# ROOT 编译选项
if(NOT ROOT_DIR)
    set(ROOT_DIR ${CMAKE_SOURCE_DIR}/root/cmake)
endif()
find_package(ROOT)
separate_arguments(ROOT_CC_FLAGS)
separate_arguments(ROOT_CXX_FLAGS)

# onnxruntime 包含和库目录
if(NOT ONNX_DIR)
    set(ONNX_DIR ${CMAKE_SOURCE_DIR}/onnxruntime)
endif()
if(NOT ONNX_INC_DIR)
    set(ONNX_INC_DIR ${ONNX_DIR}/include)
endif()
if(NOT ONNX_LIB_DIR)
    set(ONNX_LIB_DIR ${ONNX_DIR}/lib)
endif()
include_directories(${ONNX_INC_DIR})
link_directories(${ONNX_LIB_DIR})

# 优化和调试选项
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
endif()
if(CMAKE_BUILD_TYPE STREQUAL DEBUG)
    add_compile_options(-Og -fno-omit-frame-pointer)
else()
    add_compile_options(-O3)
endif()
add_compile_options(-Wall -Wshadow -Wextra)
if(ENABLE_SANITIZER)
    set(SANITIZER_OPTIONS -fno-sanitize-recover=all)
    list(APPEND SANITIZER_OPTIONS -fsanitize=address)
    list(APPEND SANITIZER_OPTIONS -fsanitize=leak)
    list(APPEND SANITIZER_OPTIONS -fsanitize=undefined)
    list(APPEND SANITIZER_OPTIONS -fsanitize=float-divide-by-zero)
    add_compile_options(${SANITIZER_OPTIONS})
    add_link_options(${SANITIZER_OPTIONS})
endif()

# 以下为可执行文件构建

add_executable(onnx-test src/onnx-test.cpp)
target_link_libraries(onnx-test PUBLIC onnxruntime)

add_executable(root-test src/root-test.cpp)
target_compile_options(root-test PUBLIC ${ROOT_CXX_FLAGS})
target_include_directories(root-test PUBLIC ${ROOT_INCLUDE_DIRS})
target_link_libraries(root-test PUBLIC ${ROOT_LIBRARIES})
target_compile_definitions(root-test PUBLIC ${ROOT_DEFINITIONS})
