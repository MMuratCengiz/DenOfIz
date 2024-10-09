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

#include <string>
#include "IBufferResource.h"
#include "ITextureResource.h"

#ifdef _WIN32
#include <DenOfIzCore/Common_Windows.h> // Include this before to make sure NOMINMAX is defined
#include <wrl/client.h>
#else
#define __EMULATE_UUID
#include "WinAdapter.h"
#endif

#include "dxcapi.h"

namespace DenOfIz
{
    enum class ShaderStage
    {
        Geometry,
        Hull,
        Domain,
        Vertex,
        Pixel,
        Compute,
        AllGraphics,
        All,
        Raygen,
        AnyHit,
        ClosestHit,
        Miss,
        Intersection,
        Callable,
        Task,
        Mesh,
    };

    // Needs to be used as a pointer as Blob/Reflection might be deleted multiple times otherwise
    struct CompiledShader : private NonCopyable
    {
        ShaderStage Stage;
        IDxcBlob   *Blob;
        IDxcBlob   *Reflection;
        std::string EntryPoint;

        ~CompiledShader( )
        {
            if ( Blob )
            {
                Blob->Release( );
                Blob = nullptr;
            }

            if ( Reflection )
            {
                Reflection->Release( );
                Reflection = nullptr;
            }
        }
    };
} // namespace DenOfIz
