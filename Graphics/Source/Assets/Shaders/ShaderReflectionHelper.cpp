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

#include "DenOfIzGraphicsInternal/Assets/Shaders/ShaderReflectionHelper.h"

#include <ranges>

#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/StringStore.h"

using namespace DenOfIz;

bool ShaderReflectionHelper::IsBindingLocalTo( const DenOfIz_RayTracingShaderDesc &rayTracingShaderDesc, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc )
{
    const auto &bindings = rayTracingShaderDesc.LocalBindings;
    for ( size_t i = 0; i < bindings.NumElements; ++i )
    {
        const DenOfIz_ResourceBindingSlot &element = bindings.Elements[ i ];
        if ( element.Binding == shaderInputBindDesc.BindPoint && element.RegisterSpace == shaderInputBindDesc.Space &&
             element.Type == DxcEnumConverter::ReflectTypeToBufferBindingType( shaderInputBindDesc.Type ) )
        {
            return true;
        }
    }
    return false;
}

bool ShaderReflectionHelper::IsBindingBindless( const DenOfIz_BindlessDesc &bindlessDesc, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc )
{
    const auto &bindings = bindlessDesc.BindlessArrays;
    for ( uint32_t i = 0; i < bindings.NumElements; ++i )
    {
        const DenOfIz_BindlessSlot       &element     = bindings.Elements[ i ];
        const DenOfIz_ResourceBindingType elementType = DenOfIz_ResourceBindingType_FromDescriptor( element.Descriptor );
        if ( element.Binding == shaderInputBindDesc.BindPoint && element.RegisterSpace == shaderInputBindDesc.Space &&
             elementType == DxcEnumConverter::ReflectTypeToBufferBindingType( shaderInputBindDesc.Type ) )
        {
            return true;
        }
    }
    return false;
}

const DenOfIz_BindlessSlot *ShaderReflectionHelper::GetBindlessSlot( const DenOfIz_BindlessDesc &bindlessDesc, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc )
{
    const auto &bindings = bindlessDesc.BindlessArrays;
    for ( uint32_t i = 0; i < bindings.NumElements; ++i )
    {
        const DenOfIz_BindlessSlot       &element     = bindings.Elements[ i ];
        const DenOfIz_ResourceBindingType elementType = DenOfIz_ResourceBindingType_FromDescriptor( element.Descriptor );
        if ( element.Binding == shaderInputBindDesc.BindPoint && element.RegisterSpace == shaderInputBindDesc.Space &&
             elementType == DxcEnumConverter::ReflectTypeToBufferBindingType( shaderInputBindDesc.Type ) )
        {
            return &element;
        }
    }
    return nullptr;
}

uint64_t ShaderReflectionHelper::GetConstantBufferSize( ID3D12ShaderReflection *shaderReflection, ID3D12FunctionReflection *functionReflection, const int resourceIndex )
{
    D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };
    if ( shaderReflection )
    {
        DxcCheckResult( shaderReflection->GetResourceBindingDesc( resourceIndex, &shaderInputBindDesc ) );
    }
    else if ( functionReflection )
    {
        DxcCheckResult( functionReflection->GetResourceBindingDesc( resourceIndex, &shaderInputBindDesc ) );
    }
    else
    {
        spdlog::critical( "No shader reflection object found, make sure no compilation errors occurred." );
        return 0;
    }

    if ( shaderInputBindDesc.Type != D3D_SIT_CBUFFER )
    {
        return 0;
    }

    ID3D12ShaderReflectionConstantBuffer *constantBuffer = nullptr;
    if ( shaderReflection )
    {
        constantBuffer = shaderReflection->GetConstantBufferByIndex( resourceIndex );
    }
    else if ( functionReflection )
    {
        constantBuffer = functionReflection->GetConstantBufferByIndex( resourceIndex );
    }

    D3D12_SHADER_BUFFER_DESC bufferDesc;
    DxcCheckResult( constantBuffer->GetDesc( &bufferDesc ) );
    return ( bufferDesc.Size + 15 ) & ~( 16 - 1 ); // Align to 16 bytes
}

void ShaderReflectionHelper::DxcCheckResult( const HRESULT hr )
{
    if ( FAILED( hr ) )
    {
        spdlog::error( "DXC Error: {}", hr );
    }
}

DenOfIz_ThreadGroupDesc ShaderReflectionHelper::ExtractThreadGroupSize( ID3D12ShaderReflection *shaderReflection, ID3D12FunctionReflection *functionReflection )
{
    DenOfIz_ThreadGroupDesc threadGroup = { 0, 0, 0 };
    if ( shaderReflection )
    {
        D3D12_SHADER_DESC shaderDesc;
        if ( SUCCEEDED( shaderReflection->GetDesc( &shaderDesc ) ) )
        {
            shaderReflection->GetThreadGroupSize( &threadGroup.X, &threadGroup.Y, &threadGroup.Z );
        }
    }
    else if ( functionReflection )
    {
        D3D12_FUNCTION_DESC functionDesc;
        if ( SUCCEEDED( functionReflection->GetDesc( &functionDesc ) ) )
        {
            // Not common,
        }
    }

    return threadGroup;
}

DenOfIz_PrimitiveTopology ShaderReflectionHelper::ExtractMeshOutputTopology( ID3D12ShaderReflection *shaderReflection )
{
    if ( !shaderReflection )
    {
        return DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
    }

    D3D12_SHADER_DESC shaderDesc;
    if ( SUCCEEDED( shaderReflection->GetDesc( &shaderDesc ) ) && D3D12_SHVER_GET_TYPE( shaderDesc.Version ) == D3D12_SHVER_MESH_SHADER )
    {
        switch ( shaderDesc.InputPrimitive )
        {
        case D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
            return DENOFIZ_PRIMITIVE_TOPOLOGY_POINT;
        case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
        case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
            return DENOFIZ_PRIMITIVE_TOPOLOGY_LINE;
        case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
        case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
            return DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
        default:
            break;
        }
    }

    return DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
}
