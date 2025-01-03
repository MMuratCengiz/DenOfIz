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
#include <DenOfIzGraphics/Utilities/Common_Windows.h> // Include this before to make sure NOMINMAX is defined
#include <wrl/client.h>
#else
#define __EMULATE_UUID
#include "WinAdapter.h"
#endif

#include "RayTracing/RayTracingData.h"
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
        Mesh
    };

    enum class TargetIL
    {
        DXIL,
        MSL,
        SPIRV
    };

    enum class RayTracingStage
    {
        Raygen,
        HitGroup,
        Miss,
        Callable
    };

    /// <summary>
    /// Raytracing description for the ShaderProgram
    /// </summary>
    struct DZ_API ProgramRayTracingDesc
    {
        uint32_t MaxNumPayloadBytes   = 0;
        uint32_t MaxNumAttributeBytes = 0;
        uint32_t MaxRecursionDepth    = 1;
    };

    struct DZ_API RayTracingShaderDesc
    {
        InteropArray<ResourceBindingSlot> LocalBindings;
        // Local bindings are used to mark resources as local, so they are not included in the global resource list
        // The binding will be added to the corresponding ShaderDataLayoutDesc in the corresponding index of LocalRootSignatureDesc at
        // ShaderReflectDesc.LocalRootSignatures[shaderIndex], where shaderIndex is the index of the shader in the order of shaders provided to CompileDesc.
        void MarkCbvAsLocal( uint32_t binding, uint32_t registerSpace );
        void MarkSrvAsLocal( uint32_t binding, uint32_t registerSpace );
        void MarkUavAsLocal( uint32_t binding, uint32_t registerSpace );
        void MarkSamplerAsLocal( uint32_t binding, uint32_t registerSpace );
    };

    // Needs to be used as a pointer as Blob/Reflection might be deleted multiple times otherwise
    struct DZ_API CompiledShader : private NonCopyable
    {
        InteropString        Path;
        ShaderStage          Stage;
        IDxcBlob            *Blob;
        IDxcBlob            *Reflection;
        InteropString        EntryPoint;
        RayTracingShaderDesc RayTracing;
    };
    template class DZ_API InteropArray<CompiledShader *>;
} // namespace DenOfIz
