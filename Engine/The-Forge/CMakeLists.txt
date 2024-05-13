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

## Contains modified version of the github project from: https://github.com/wutipong/the-forge-template

include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

set(GAINPUT_SOURCES
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/gainput.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputAllocator.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputInputDeltaState.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputInputDevice.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputInputManager.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputInputMap.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputInputState.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputMapFilters.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/GainputHID.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/GainputHIDWhitelist.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserPS4Controller.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserPS4Controller.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserPS4Controller.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserPS5Controller.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserPS5Controller.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserPS5Controller.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserSwitchController.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserSwitchController.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/keyboard/GainputInputDeviceKeyboard.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/mouse/GainputInputDeviceMouse.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/pad/GainputInputDevicePad.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/touch/GainputInputDeviceTouch.cpp"
        $<$<PLATFORM_ID:Windows>:
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/hidapi/windows/hid.c"
        >
        $<$<PLATFORM_ID:Linux>:
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/hidapi/linux/hid.c"
        >
        $<$<PLATFORM_ID:Darwin>:
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/hidapi/mac/hid.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/apple/GainputGCKit.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/apple/GainputInputDeviceBuiltInIos.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/apple/GainputInputDeviceKeyboardMac.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/apple/GainputInputDeviceMouseMac.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/apple/GainputInputDeviceMouseMacRaw.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/apple/GainputInputDevicePadAppleGCKit.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/apple/GainputInputDevicePadMac.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/apple/GainputInputDeviceTouchIos.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/apple/GainputIos.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/apple/GainputMac.mm"
        >
)

add_library(gainput ${LIB_TYPE} ${GAINPUT_SOURCES})

if (MSVC)
    target_compile_definitions(gainput PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif ()

target_include_directories(gainput PUBLIC
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/include/gainput/apple/"
)

if (NOT APPLE)
    target_compile_features(gainput PRIVATE cxx_std_14)
    set_property(TARGET gainput PROPERTY CXX_STANDARD 14)
    set_property(TARGET gainput PROPERTY CXX_STANDARD_REQUIRED ON)
endif ()

add_library(TheForge ${LIB_TYPE}
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/CameraController.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/Fonts/FontSystem.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/Fonts/stbtt.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/InputSystem.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/Profiler/GpuProfiler.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/Profiler/ProfilerBase.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/RemoteControl.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/Screenshot.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/imgui/imgui_demo.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/imgui/imgui_draw.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/imgui/imgui_tables.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/imgui/imgui_widgets.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/ThirdParty/OpenSource/imgui/imgui.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/UI/UI.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/Scripting/LuaManager.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/Scripting/LuaManagerImpl.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/Scripting/LuaSystem.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lapi.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lauxlib.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lbaselib.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lbitlib.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lcode.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lcorolib.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lctype.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/ldblib.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/ldebug.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/ldo.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/ldump.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lfunc.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lgc.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/linit.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/liolib.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/llex.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lmathlib.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lmem.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/loadlib.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lobject.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lopcodes.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/loslib.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lparser.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lstate.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lstring.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lstrlib.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/ltable.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/ltablib.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/ltm.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lundump.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lutf8lib.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lvm.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lzio.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/CommonShaderReflection.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/Direct3D11/Direct3D11.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/Direct3D11/Direct3D11ShaderReflection.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/Direct3D12/Direct3D12.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/Direct3D12/Direct3D12Hooks.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/Direct3D12/Direct3D12Raytracing.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/Direct3D12/Direct3D12ShaderReflection.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/GraphicsConfig.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/PickRenderingAPI.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_cfg.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_cpp.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_cross_parsed_ir.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_cross_util.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_cross.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_glsl.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_hlsl.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_msl.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_parser.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_reflect.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/SpirvTools.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/Vulkan/Vulkan.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/Vulkan/VulkanRaytracing.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/Vulkan/VulkanShaderReflection.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/CPUConfig.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/WindowSystem/WindowSystem.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Resources/ResourceLoader/ResourceLoader.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Tools/Network/Network.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Tools/ReloadServer/ReloadClient.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/FileSystem/FileSystem.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/FileSystem/SystemRun.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/Log/Log.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/Math/Algorithms.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/Math/StbDs.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/MemoryTracking/MemoryTracking.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/bstrlib/bstrlib.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/lz4/lz4.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/lz4/lz4.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/lz4/lz4hc.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/lz4/lz4hc.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/bits.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/bitstream.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/compiler.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/cpu.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/debug.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/debug.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/entropy_common.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/error_private.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/error_private.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/fse_decompress.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/fse.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/huf.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/mem.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/pool.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/pool.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/portability_macros.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/threading.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/threading.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/xxhash.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/xxhash.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/zstd_common.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/zstd_deps.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/zstd_internal.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/zstd_trace.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/huf_decompress_amd64.S"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/huf_decompress.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_ddict.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_ddict.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_decompress_block.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_decompress_block.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_decompress_internal.h"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_decompress.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/Timer.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Renderer/VisibilityBuffer/VisibilityBuffer.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Examples_3/Visibility_Buffer/src/SanMiguel.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Examples_3/Visibility_Buffer/src/SanMiguelSDF.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Examples_3/Visibility_Buffer/src/Visibility_Buffer.cpp"

        $<$<PLATFORM_ID:Windows>:
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/ThirdParty/OpenSource/cpu_features/src/impl_x86_windows.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Windows/WindowsBase.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Windows/WindowsFileSystem.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Windows/WindowsLog.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Windows/WindowsStackTraceDump.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Windows/WindowsThread.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Windows/WindowsTime.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Windows/WindowsToolsFileSystem.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Windows/WindowsWindow.cpp"
        >

        $<$<PLATFORM_ID:Linux>:
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/ThirdParty/OpenSource/cpu_features/src/impl_x86_linux_or_android.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Linux/LinuxBase.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Linux/LinuxFileSystem.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Linux/LinuxLog.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Linux/LinuxThread.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Linux/LinuxTime.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Linux/LinuxToolsFileSystem.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Linux/LinuxWindow.cpp"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/FileSystem/UnixFileSystem.c"
        >

        $<$<PLATFORM_ID:Darwin>:
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/Metal/MetalRaytracing.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/Metal/MetalRenderer.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Graphics/Metal/MetalShaderReflection.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/ThirdParty/OpenSource/cpu_features/src/impl_x86_macos.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/ThirdParty/OpenSource/cpu_features/src/impl_aarch64_iOS.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Darwin/CocoaFileSystem.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Darwin/CocoaToolsFileSystem.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Darwin/DarwinLog.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Darwin/DarwinThread.c"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Darwin/macOSAppDelegate.m"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Darwin/macOSBase.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Darwin/macOSWindow.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Utilities/FileSystem/UnixFileSystem.c"
        >
        $<$<PLATFORM_ID:IOS>:
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Darwin/iOSAppDelegate.m"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Darwin/iOSBase.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Darwin/iOSToolsFileSystem.mm"
        "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Darwin/iOSWindow.mm"
        >
)

if (MSVC)
    target_compile_definitions(TheForge PRIVATE -D_UNICODE -DUNICODE)
    target_compile_definitions(TheForge PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif ()

if (LINUX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(PkgConfig REQUIRED)
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)

    pkg_check_modules(GTK REQUIRED gtk+-3.0)
    pkg_check_modules(UDEV REQUIRED libudev)
    include_directories(${GTK_INCLUDE_DIRS})

    target_include_directories(TheForge PRIVATE
            ${GTK_INCLUDE_DIRS}
            ${X11_INCLUDE_DIRS}
    )

    target_link_libraries(TheForge PRIVATE
            ${GTK_LIBRARIES}
            ${X11_LIBRARIES}
            ${X11_Xrandr_LIB}
            Threads::Threads
            ${CMAKE_DL_LIBS}
    )

    target_link_libraries(gainput PRIVATE ${UDEV_LIBRARIES})
endif ()

if (APPLE)
    target_link_libraries(gainput
            PRIVATE "-framework Cocoa"
            PRIVATE "-framework QuartzCore"
            PRIVATE "-framework Foundation"
            PRIVATE "-framework CoreData"
            PRIVATE "-framework CoreHaptics"
            PRIVATE "-framework AppKit"
            PRIVATE "-framework Metal"
            PRIVATE "-framework CoreVideo"
            PRIVATE "-framework MetalKit"
            PRIVATE "-framework MetalPerformanceShaders"
            PRIVATE "-framework IOKit"
            PRIVATE "-framework GameKit"
            PRIVATE "-framework GameController"
    )

    target_link_libraries(TheForge
            PRIVATE "-framework Cocoa"
            PRIVATE "-framework QuartzCore"
            PRIVATE "-framework Foundation"
            PRIVATE "-framework CoreData"
            PRIVATE "-framework CoreHaptics"
            PRIVATE "-framework AppKit"
            PRIVATE "-framework Metal"
            PRIVATE "-framework CoreVideo"
            PRIVATE "-framework MetalKit"
            PRIVATE "-framework MetalPerformanceShaders"
            PRIVATE "-framework IOKit"
            PRIVATE "-framework GameKit"
            PRIVATE "-framework GameController"
    )

    if (IOS)
        target_link_libraries(TheForge
            PRIVATE "-framework UIKit"
        )
    endif ()
    set_target_properties(TheForge
            PROPERTIES
            XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_WEAK YES)
endif ()

target_link_libraries(TheForge PUBLIC gainput)