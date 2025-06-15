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

#include "DenOfIzGraphics/Utilities/Common.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#include <string>
#include "IBufferResource.h"
#include "ITextureResource.h"

#include "RayTracing/RayTracingData.h"

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

    struct DZ_API ShaderStageArray
    {
        ShaderStage *Elements;
        size_t       NumElements;
    };

    /// <summary>
    /// Thread group information for compute, mesh, and task shaders
    /// </summary>
    struct DZ_API ThreadGroupInfo
    {
        uint32_t X = 0;
        uint32_t Y = 0;
        uint32_t Z = 0;
    };

    struct DZ_API ThreadGroupInfoArray
    {
        ThreadGroupInfo *Elements;
        size_t           NumElements;
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
    struct DZ_API ShaderRayTracingDesc
    {
        uint32_t MaxNumPayloadBytes   = 0;
        uint32_t MaxNumAttributeBytes = 0;
        uint32_t MaxRecursionDepth    = 1;
    };

    struct DZ_API RayTracingShaderDesc
    {
        // For metal, it needs to know before compiling what the hit group is otherwise intersection shaders do not work
        HitGroupType             HitGroupType = HitGroupType::Triangles;
        ResourceBindingSlotArray LocalBindings;
        // Local bindings are used to mark resources as local, so they are not included in the global resource list
        // The binding will be added to the corresponding ShaderDataLayoutDesc in the corresponding index of LocalRootSignatureDesc at
        // ShaderReflectDesc.LocalRootSignatures[shaderIndex], where shaderIndex is the index of the shader in the order of shaders provided to CompileDesc.
    };

    struct DZ_API BindlessSlot
    {
        ResourceBindingType Type          = ResourceBindingType::ShaderResource;
        uint32_t            Binding       = 0;
        uint32_t            RegisterSpace = 0;
        uint32_t            MaxArraySize  = 1024;
    };

    struct DZ_API BindlessSlotArray
    {
        BindlessSlot *Elements;
        size_t        NumElements;
    };

    struct DZ_API BindlessDesc
    {
        BindlessSlotArray BindlessArrays;
    };

    enum class DZ_API CodePage
    {
        ACP, // ANSI, detects UTF8 with BOM
        UTF8,
        UTF16,
        UTF32,
    };

    /// Use either Path or Data, Data takes priority if both are provided
    struct DZ_API ShaderStageDesc
    {
        ShaderStage   Stage;
        CodePage      CodePage;
        InteropString Path;
        ByteArray     Data;
        StringArray   Defines;
        InteropString EntryPoint = "main";
        /// \brief Only available for Raygen, Miss and Hit shaders(Intersection, ClosestHit, AnyHit)
        RayTracingShaderDesc RayTracing;
        BindlessDesc         Bindless;
    };

    struct DZ_API ShaderStageDescArray
    {
        ShaderStageDesc *Elements;
        size_t           NumElements;
    };

    // Needs to be used as a pointer as Blob/Reflection might be deleted multiple times otherwise
    struct DZ_API CompiledShaderStage : private NonCopyable
    {
        ShaderStage          Stage;
        ByteArray            DXIL;
        ByteArray            MSL;
        ByteArray            SPIRV;
        ByteArray            Reflection;
        InteropString        EntryPoint;
        RayTracingShaderDesc RayTracing;
        ThreadGroupInfo      ThreadGroup; // Thread group size for compute, mesh, and task shaders
    };

    struct DZ_API CompiledShaderStageArray
    {
        CompiledShaderStage **Elements;
        size_t                NumElements;
    };
} // namespace DenOfIz
