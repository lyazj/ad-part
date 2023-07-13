cmake_minimum_required(VERSION 3.13)
project(ad-part VERSION 0.0.1)

# 设置项目包含目录、库目录和可执行文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# ROOT 编译选项
if(NOT ROOT_DIR)
    set(ROOT_DIR ${PROJECT_SOURCE_DIR}/root/cmake)
endif()
find_package(ROOT)
set(ROOT_CC_FLAG_LIST ${ROOT_CC_FLAGS})
set(ROOT_CXX_FLAG_LIST ${ROOT_CXX_FLAGS})
separate_arguments(ROOT_CC_FLAG_LIST)
separate_arguments(ROOT_CXX_FLAG_LIST)
add_compile_options(${ROOT_CXX_FLAG_LIST})
add_compile_definitions(${ROOT_DEFINITIONS})
include_directories(${ROOT_INCLUDE_DIRS})

# onnxruntime 编译选项
if(NOT AD_NOONNX)
    if(NOT ONNX_DIR)
        set(ONNX_DIR ${PROJECT_SOURCE_DIR}/onnxruntime)
    endif()
    if(NOT ONNX_INC_DIR)
        set(ONNX_INC_DIR ${ONNX_DIR}/include)
    endif()
    if(NOT ONNX_LIB_DIR)
        set(ONNX_LIB_DIR ${ONNX_DIR}/lib)
    endif()
    include_directories(${ONNX_INC_DIR})
    link_directories(${ONNX_LIB_DIR})
endif()

# Delphes 编译选项
if(NOT DELPHES_DIR)
    set(DELPHES_DIR ${PROJECT_SOURCE_DIR}/MG5_aMC/Delphes)
endif()
set(DELPHES_INC_DIR ${DELPHES_DIR} ${DELPHES_DIR}/external)
set(DELPHES_LIB_DIR ${DELPHES_DIR})
include_directories(${DELPHES_INC_DIR})
link_directories(${DELPHES_LIB_DIR})
add_compile_definitions(DELPHES_DIR="${DELPHES_DIR}")
add_compile_definitions(DELPHES_EXTERNAL_DIR="${DELPHES_DIR}/external")
add_compile_definitions(LIBDELPHES_SO="${DELPHES_LIB_DIR}/libDelphes.so")

# zlib 编译选项
if(ZLIB_DIR)
    if(NOT ZLIB_INC_DIR)
        set(ZLIB_INC_DIR ${ZLIB_DIR}/include)
    endif()
    if(NOT ZLIB_LIB_DIR)
        set(ZLIB_LIB_DIR ${ZLIB_DIR}/lib)
    endif()
endif()
if(ZLIB_INC_DIR)
    include_directories(${ZLIB_INC_DIR})
endif()
if(ZLIB_LIB_DIR)
    link_directories(${ZLIB_LIB_DIR})
endif()

# PDG 数据生成和编译
if(NOT AD_NOPDG)
    if(NOT PDG_PATH)
        set(PDG_PATH ${PROJECT_SOURCE_DIR}/run/pdginfo)
    endif()
    execute_process(COMMAND ${PROJECT_SOURCE_DIR}/bin/pdg-gen ${PDG_PATH} RESULT_VARIABLE RETCODE)
    if(NOT RETCODE EQUAL 0)
        message(FATAL_ERROR "error generating pdginfo")
    endif()
    add_compile_definitions(PDG_PATH="${PDG_PATH}")
endif()

# 优化、调试和通用编译选项
option(ENABLE_SANITIZER "compile and link with sanitizing flags" OFF)
option(ENABLE_GPROF "compile and link with gprof flags" OFF)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
endif()
if(CMAKE_BUILD_TYPE STREQUAL DEBUG)
    add_compile_options(-O0 -fno-omit-frame-pointer)
    if(ENABLE_SANITIZER)
        set(SANITIZER_OPTIONS -fno-sanitize-recover=all)
        list(APPEND SANITIZER_OPTIONS -fsanitize=address)
        list(APPEND SANITIZER_OPTIONS -fsanitize=leak)
        list(APPEND SANITIZER_OPTIONS -fsanitize=undefined)
        list(APPEND SANITIZER_OPTIONS -fsanitize=float-divide-by-zero)
        add_compile_options(${SANITIZER_OPTIONS})
        add_link_options(${SANITIZER_OPTIONS})
    endif()
else()
    add_compile_options(-O3)
endif()
if(ENABLE_GPROF)
    set(GPROF_OPTIONS -pg)
    add_compile_options(${GPROF_OPTIONS})
    add_link_options(${GPROF_OPTIONS})
endif()
add_compile_options(-Wall -Wshadow -Wextra)
add_compile_definitions(_GNU_SOURCE)

# 编译环境检查
set(CODE_TO_COMPILE "#include <string>\\nusing std::operator\"\"s;\\n")
set(TEMP_OUTPUT "/tmp/trail_$$.o")
set(TRAIL_FLAGS "-xc++ ${ROOT_CXX_FLAGS} - -o ${TEMP_OUTPUT} -c")
message("compiling trail code...")
execute_process(COMMAND sh -c
"printf '${CODE_TO_COMPILE}' | ${CMAKE_CXX_COMPILER} ${TRAIL_FLAGS} && rm ${TEMP_OUTPUT}"
RESULT_VARIABLE RETCODE)
if(NOT RETCODE EQUAL 0)
    message(FATAL_ERROR "string literals not supported, expect C++14 or later")
endif()
message("passed test for string literal operator support")

# 以下为静态库文件构建

execute_process(COMMAND sh -c
"ls ${PROJECT_SOURCE_DIR}/src/ad*.cpp | xargs echo -n"
OUTPUT_VARIABLE AD_SOURCES RESULT_VARIABLE AD_STATUS
)
if(NOT AD_STATUS EQUAL 0)
    message(FATAL_ERROR "error listing libadhep sources")
endif()
separate_arguments(AD_SOURCES)
if(AD_NOONNX)
    list(REMOVE_ITEM AD_SOURCES ${PROJECT_SOURCE_DIR}/src/adtensor.cpp)
    list(REMOVE_ITEM AD_SOURCES ${PROJECT_SOURCE_DIR}/src/adrunner.cpp)
endif()
if(AD_NOPDG)
    list(REMOVE_ITEM AD_SOURCES ${PROJECT_SOURCE_DIR}/src/adpdg.cpp)
endif()
add_library(adhep ${AD_SOURCES})

# 以下为可执行文件构建

if(NOT AD_NOTEST)

    add_executable(sanitize-test src/sanitizer-test.cpp)
    
    if(NOT AD_NOONNX)
        add_executable(onnx-test src/onnx-test.cpp)
        target_link_libraries(onnx-test PUBLIC onnxruntime)
    
        add_executable(onnx-test-gpu src/onnx-test-gpu.cpp)
        target_link_libraries(onnx-test-gpu PUBLIC onnxruntime)
    endif()
    
    add_executable(root-test src/root-test.cpp)
    target_link_libraries(root-test PUBLIC ROOT::MathCore)
    
    add_executable(delphes-test src/delphes-test.cpp)
    target_link_libraries(delphes-test PUBLIC adhep Delphes ROOT::Tree)

    add_executable(ecf-test src/ecf-test.cpp src/adecf.cpp)
    target_link_libraries(ecf-test ROOT::Physics)
    target_compile_definitions(ecf-test PUBLIC ECF_DEBUG)

endif()

if(NOT AD_NOEXEC)

    add_executable(flatten src/flatten.cpp)
    target_link_libraries(flatten PUBLIC adhep Delphes ROOT::Tree ROOT::Physics z)

    add_executable(flatten-without-label src/flatten-without-label.cpp)
    target_link_libraries(flatten-without-label PUBLIC adhep Delphes ROOT::Tree ROOT::Physics z)

    add_executable(ntuple src/ntuple.cpp)
    target_link_libraries(ntuple PUBLIC adhep ROOT::Tree z)
    
    if(NOT AD_NOONNX)
        add_executable(part src/part.cpp)
        target_link_libraries(part PUBLIC adhep onnxruntime z)

        add_executable(part-gpu src/part-gpu.cpp)
        target_link_libraries(part-gpu PUBLIC adhep onnxruntime z)
    endif()

endif()
