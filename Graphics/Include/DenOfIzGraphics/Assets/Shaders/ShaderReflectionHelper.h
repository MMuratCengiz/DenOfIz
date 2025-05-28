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

#include <DenOfIzGraphics/Assets/Shaders/DxcEnumConverter.h>
#include "ShaderReflectDesc.h"

namespace DenOfIz
{
    class ShaderReflectionHelper
    {
    public:
        // Returns true if the bound resource is found(and an update is performed), false otherwise
        // Adds additional stages if existing stages are found
        static bool IsBindingLocalTo( const RayTracingShaderDesc &rayTracingShaderDesc, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc );
        static bool IsBindingBindless( const BindlessDesc &bindlessDesc, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc );
        static const BindlessSlot* GetBindlessSlot( const BindlessDesc &bindlessDesc, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc );
        static void FillTypeInfo( ID3D12ShaderReflectionType *reflType, InteropArray<ReflectionResourceField> &fields, uint32_t parentIndex, uint32_t level );
        static void FillReflectionData( ID3D12ShaderReflection *shaderReflection, ID3D12FunctionReflection *functionReflection, ReflectionDesc &reflectionDesc, int resourceIndex );
        static void DxcCheckResult( HRESULT hr );
        static ThreadGroupInfo   ExtractThreadGroupSize( ID3D12ShaderReflection *shaderReflection, ID3D12FunctionReflection *functionReflection );
        static PrimitiveTopology ExtractMeshOutputTopology( ID3D12ShaderReflection *shaderReflection );
    };
} // namespace DenOfIz
