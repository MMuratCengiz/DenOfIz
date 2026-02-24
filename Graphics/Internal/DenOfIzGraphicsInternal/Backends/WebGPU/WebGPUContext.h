/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <webgpu/webgpu.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/emscripten.h>
#endif
#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphicsInternal/Backends/Common/InternalAutoSync.h"

#ifdef __EMSCRIPTEN__
#define DZ_EMSCRIPTEN_VERSION_ENCODE( major, minor, tiny ) ( ( major ) * 10000 + ( minor ) * 100 + ( tiny ) )
#define DZ_EMSCRIPTEN_VERSION DZ_EMSCRIPTEN_VERSION_ENCODE( __EMSCRIPTEN_major__, __EMSCRIPTEN_minor__, __EMSCRIPTEN_tiny__ )
#define DZ_EMSCRIPTEN_VERSION_AT_LEAST( major, minor, tiny ) ( DZ_EMSCRIPTEN_VERSION >= DZ_EMSCRIPTEN_VERSION_ENCODE( major, minor, tiny ) )
#else
#define DZ_EMSCRIPTEN_VERSION_AT_LEAST( major, minor, tiny ) 0
#endif

#if defined( BUILD_WEBGPU_NATIVE ) || ( defined( __EMSCRIPTEN__ ) && DZ_EMSCRIPTEN_VERSION_AT_LEAST( 4, 0, 10 ) )
#define DZ_WEBGPU_USE_DAWN_API 1
#else
#define DZ_WEBGPU_USE_DAWN_API 0
#endif

namespace DenOfIz
{
    struct WebGPUContext
    {
        WGPUInstance Instance = nullptr;
        WGPUAdapter  Adapter  = nullptr;
        WGPUDevice   Device   = nullptr;
        WGPUQueue    Queue    = nullptr;

        InternalAutoSync *AutoSync;

        DenOfIz_PhysicalDevice SelectedDevice;
    };

} // namespace DenOfIz

#if DZ_WEBGPU_USE_DAWN_API
#define DZ_WEBGPU_STRING( label ) { label, strlen( label ) }
#define DZ_WEBGPU_NULL_STRING { nullptr, 0 }
#else
#define DZ_WEBGPU_STRING( label ) label
#define DZ_WEBGPU_NULL_STRING nullptr
#endif
