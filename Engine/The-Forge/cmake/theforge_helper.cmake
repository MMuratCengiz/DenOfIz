# MIT License
# 
# Copyright (c) 2023 Wutipong Wongsakuldej
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

### Contains modified version of the github project from: https://github.com/wutipong/the-forge-template

include("${PROJECT_SOURCE_DIR}/cmake/common.cmake")

function(INCLUDE_THEFORGE target)
    target_link_libraries(${target} PUBLIC TheForge)

    if (WIN32)
        target_link_libraries(${target} PUBLIC
                "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/ThirdParty/OpenSource/winpixeventruntime/bin/WinPixEventRuntime.lib"
                "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/ags/ags_lib/lib/amd_ags_x64.lib"
                "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/nvapi/amd64/nvapi64.lib"
                "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/lib/x64/dxcompiler.lib"
                XInput
        )

        configure_file("${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/ThirdParty/OpenSource/winpixeventruntime/bin/WinPixEventRuntime.dll" "${PROJECT_BINARY_DIR}/WinPixEventRuntime.dll" COPYONLY)
        configure_file("${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/ags/ags_lib/lib/amd_ags_x64.dll" "${PROJECT_BINARY_DIR}/amd_ags_x64.dll" COPYONLY)
        configure_file("${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/bin/x64/dxcompiler.dll" "${PROJECT_BINARY_DIR}/dxcompiler.dll" COPYONLY)
        configure_file("${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/bin/x64/dxil.dll" "${PROJECT_BINARY_DIR}/dxil.dll" COPYONLY)
    elseif (LINUX)
        configure_file("${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/VulkanSDK/bin/Linux/VkLayer_khronos_validation.json" "${PROJECT_BINARY_DIR}/VkLayer_khronos_validation.json" COPYONLY)
        configure_file("${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/VulkanSDK/bin/Linux/libVkLayer_khronos_validation.so" "${PROJECT_BINARY_DIR}/libVkLayer_khronos_validation.so" COPYONLY)
    endif ()

    if (NOT APPLE)
        target_compile_features(${target} PRIVATE cxx_std_20)
        set_property(TARGET ${target} PROPERTY CXX_STANDARD 20)
        set_property(TARGET ${target} PROPERTY CXX_STANDARD_REQUIRED ON)
    endif ()

    if (MSVC)
        # set_property(TARGET main PROPERTY WIN32_EXECUTABLE ON)
        target_compile_options(${target} PRIVATE /Zc:__cplusplus)
    endif ()

    copy_to_binary(${CMAKE_CURRENT_SOURCE_DIR} "Assets" *)

    if (APPLE)
        target_compile_options(${target} PRIVATE
                $<$<COMPILE_LANGUAGE:CXX>:-x objective-c++>
                $<$<COMPILE_LANGUAGE:C>:-x objective-c>)
    endif()
    
    target_include_directories(${target} PUBLIC
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/Interfaces"
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source"
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/Interfaces"
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/inc"
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Interfaces"
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Resources/ResourceLoader/Interfaces"
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities"
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/Interfaces"
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/Nothings/"
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Tools/ForgeShadingLanguage/includes"
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics"
            ${VULKAN_HEADERS_INCLUDE_DIRS}
    )

    target_link_directories(${target} PUBLIC
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/ags/ags_lib/lib"
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/lib/x64"
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/nvapi/amd64"
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/ThirdParty/OpenSource/winpixeventruntime/bin"
    )

    set(BINARY_DIR "${PROJECT_BINARY_DIR}")
    if (APPLE)
        set(BINARY_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/Resources")
    endif ()

    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/Assets/)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/Assets/GPUCfg)
    if (WIN32)
        configure_file(${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Windows/pc_gpu.data ${BINARY_DIR}/Assets/GPUCfg/gpu.data COPYONLY)
    elseif (APPLE)
        configure_file(${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Darwin/apple_gpu.data ${BINARY_DIR}/Assets/GPUCfg/gpu.data COPYONLY)
    elseif (LINUX)
        configure_file(${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Linux/steamdeck_gpu.data ${BINARY_DIR}/Assets/GPUCfg/gpu.data COPYONLY)
    elseif (ANDROID)
        configure_file(${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Android/android_gpu.data ${BINARY_DIR}/Assets/GPUCfg/gpu.data COPYONLY)
    endif ()
endfunction()


if (WIN32)
    set(Python3_ROOT_DIR "${CMAKE_SOURCE_DIR}/_External/The-Forge/Tools/python-3.6.0-embed-amd64")
endif ()

# To try locally:
# PS C:\Workspace\DenOfIz\_External\The-Forge> python C:/Workspace/DenOfIz/_External/The-Forge/Common_3/Tools/ForgeShadingLanguage/fsl.py -dShaders -bCompiledShaders -l DIRECT3D12 --compile --verbose "C:/Workspace/DenOfIz/Engine/The-Forge/Assets/Shaders/FSL/ShaderList.fsl"
function(compile_shaders)
    set(oneValueArgs TARGET_PROJECT SHADER_TARGET SHADER_LIST)
    cmake_parse_arguments(COMPILE_SHADERS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/Assets/Shaders)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/Assets/CompiledShaders)
    # file(TOUCH ${PROJECT_BINARY_DIR}/CompiledShaders/reload-server.txt)

    set(target_lang "VULKAN")
    if (WIN32)
        set(target_lang "DIRECT3D12")
    elseif (APPLE)
        set(target_lang "MACOS")
    endif ()

    set(BINARY_DIR "${PROJECT_BINARY_DIR}")
    if (APPLE)
        set(BINARY_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/Resources")
    endif ()

    set(Py3_EXECUTABLE "${PROJECT_SOURCE_DIR}/_External/The-Forge/Tools/python-3.6.0-embed-amd64/python.exe")
    message("${Py3_EXECUTABLE} "
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Tools/ForgeShadingLanguage/fsl.py"
            " -dShaders" " -bCompiledShaders" " -l ${target_lang}" " --compile" " --verbose"
            " ${COMPILE_SHADERS_SHADER_LIST}")
    execute_process(COMMAND "${Py3_EXECUTABLE}"
            "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Tools/ForgeShadingLanguage/fsl.py"
            "-dShaders" "-bCompiledShaders" "-l ${target_lang}" --compile --verbose
            "${COMPILE_SHADERS_SHADER_LIST}"
            WORKING_DIRECTORY "${BINARY_DIR}/Assets"
    )
endfunction()