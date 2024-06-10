# The function below is a modified version of dawn by google, licensed below

# Copyright 2022 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

function(build_dxcompiler dxc_directory)
    set(HLSL_OPTIONAL_PROJS_IN_DEFAULT OFF)
    set(HLSL_ENABLE_ANALYZE OFF)
    set(HLSL_OFFICIAL_BUILD OFF)
    set(HLSL_ENABLE_FIXED_VER OFF)
    set(HLSL_BUILD_DXILCONV OFF)
    set(HLSL_INCLUDE_TESTS OFF)
    set(ENABLE_SPIRV_CODEGEN OFF)
    set(SPIRV_BUILD_TESTS OFF)
    set(LLVM_BUILD_RUNTIME ON)
    set(LLVM_BUILD_EXAMPLES OFF)
    set(LLVM_BUILD_TESTS OFF)
    set(LLVM_INCLUDE_TESTS OFF)
    set(LLVM_INCLUDE_DOCS OFF)
    set(LLVM_INCLUDE_EXAMPLES OFF)
    set(LLVM_OPTIMIZED_TABLEGEN OFF)
    set(LLVM_APPEND_VC_REV OFF)
    set(LLVM_ENABLE_RTTI ON)
    set(LLVM_ENABLE_EH ON)
#    set(LLVM_INCLUDE_TOOLS OFF)
    set(LLVM_BUILD_TOOLS OFF)
    set(LLVM_INCLUDE_UTILS OFF)
    set(CLANG_CL OFF)
    set(CLANG_FORMAT_EXE "" CACHE STRING "" FORCE)
    set(LLVM_TARGETS_TO_BUILD "None" CACHE STRING "" FORCE)
    set(LLVM_DEFAULT_TARGET_TRIPLE "dxil-ms-dx" CACHE STRING "" FORCE)
    set(CLANG_ENABLE_STATIC_ANALYZER OFF CACHE BOOL "" FORCE)
    set(CLANG_ENABLE_ARCMT OFF CACHE BOOL "" FORCE)
    set(CLANG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CLANG_INCLUDE_TESTS OFF CACHE BOOL "" FORCE)
    set(LLVM_ENABLE_TERMINFO OFF CACHE BOOL "" FORCE)
    set(LIBCLANG_BUILD_STATIC ON CACHE BOOL "" FORCE)
    set(LLVM_DISTRIBUTION_COMPONENTS "dxcompiler;dxc-headers" CACHE STRING "" FORCE)
    set(BUILD_TESTING OFF)

    add_subdirectory(${dxc_directory} EXCLUDE_FROM_ALL)
    set_target_properties(dxcompiler PROPERTIES EXCLUDE_FROM_ALL FALSE)
endfunction()