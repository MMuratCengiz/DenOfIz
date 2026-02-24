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

#include "DenOfIzGraphics/Assets/Shaders/ShaderReflectDesc.h"
#include "DenOfIzGraphicsInternal/Utilities/StringStore.h"
#include "DxcEnumConverter.h"

namespace DenOfIz
{
    class ShaderReflectionHelper
    {
    public:
        static bool                        IsBindingLocalTo( const DenOfIz_RayTracingShaderDesc &rayTracingShaderDesc, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc );
        static bool                        IsBindingBindless( const DenOfIz_BindlessDesc &bindlessDesc, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc );
        static const DenOfIz_BindlessSlot *GetBindlessSlot( const DenOfIz_BindlessDesc &bindlessDesc, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc );
        static uint64_t                    GetConstantBufferSize( ID3D12ShaderReflection *shaderReflection, ID3D12FunctionReflection *functionReflection, int resourceIndex );
        static void                        DxcCheckResult( HRESULT hr );
        static DenOfIz_ThreadGroupDesc     ExtractThreadGroupSize( ID3D12ShaderReflection *shaderReflection, ID3D12FunctionReflection *functionReflection );
        static DenOfIz_PrimitiveTopology   ExtractMeshOutputTopology( ID3D12ShaderReflection *shaderReflection );
    };
} // namespace DenOfIz
