cmake_minimum_required(VERSION 3.20)

project(DenOfIzBindings LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(LLVM CONFIG QUIET)
find_package(spdlog CONFIG REQUIRED)
find_program(DOTNET_EXECUTABLE NAMES dotnet)

if (NOT LLVM_FOUND)
    if (DEFINED ENV{LLVM_DIR})
        set(LLVM_ROOT_DIR "$ENV{LLVM_DIR}")
        if (NOT EXISTS "${LLVM_ROOT_DIR}/include/clang-c/Index.h")
            message(FATAL_ERROR "LLVM_DIR='${LLVM_ROOT_DIR}' is invalid. Expected: ${LLVM_ROOT_DIR}/include/clang-c/Index.h")
        endif ()
        set(BINDINGS_LLVM_INCLUDE_DIRS "${LLVM_ROOT_DIR}/include")
        set(LIBCLANG_LIB "${LLVM_ROOT_DIR}/lib/libclang.lib")
    else ()
        message(STATUS "LLVM not found. Bindings disabled.")
        return()
    endif ()
else ()
    set(BINDINGS_LLVM_INCLUDE_DIRS ${LLVM_INCLUDE_DIRS})
    set(BINDINGS_LLVM_DEFINITIONS ${LLVM_DEFINITIONS})
    find_library(LIBCLANG_LIB NAMES clang libclang HINTS ${LLVM_LIBRARY_DIR} ${LLVM_LIBRARY_DIRS} PATH_SUFFIXES lib)
endif ()

if (NOT LIBCLANG_LIB)
    message(STATUS "libclang not found. Bindings disabled.")
    return()
endif ()

set(BINDING_CORE_SOURCES
        Source/Core/DZIndex.cpp
        Source/Core/Dzir.cpp
        Source/Core/DzirTypeVisitor.cpp
        Source/Core/DzirVisitor.cpp
        Source/Core/NativeConvention.cpp
        Source/Core/ProjectContext.cpp
        Source/Emitters/CSharpEmitter.cpp
        Source/Emitters/EmitterStream.cpp
)

set(DENOFIZ_GENERATED_CSHARP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CSharp/Generated")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/BindingGeneration.cmake")
