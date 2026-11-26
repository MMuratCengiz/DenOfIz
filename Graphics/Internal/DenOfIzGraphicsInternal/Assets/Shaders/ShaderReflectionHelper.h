/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
