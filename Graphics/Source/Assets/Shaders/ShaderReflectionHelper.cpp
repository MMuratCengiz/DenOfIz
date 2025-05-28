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

#include <DenOfIzGraphics/Assets/Shaders/ShaderReflectionHelper.h>

using namespace DenOfIz;

bool ShaderReflectionHelper::IsBindingLocalTo( const RayTracingShaderDesc &rayTracingShaderDesc, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc )
{
    const auto &bindings = rayTracingShaderDesc.LocalBindings;
    for ( int i = 0; i < bindings.NumElements( ); ++i )
    {
        if ( auto &element = bindings.GetElement( i ); element.Binding == shaderInputBindDesc.BindPoint && element.RegisterSpace == shaderInputBindDesc.Space &&
                                                       element.Type == DxcEnumConverter::ReflectTypeToBufferBindingType( shaderInputBindDesc.Type ) )
        {
            return true;
        }
    }
    return false;
}

bool ShaderReflectionHelper::IsBindingBindless( const BindlessDesc &bindlessDesc, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc )
{
    const auto &bindings = bindlessDesc.BindlessArrays;
    for ( int i = 0; i < bindings.NumElements( ); ++i )
    {
        if ( auto &element = bindings.GetElement( i ); element.Binding == shaderInputBindDesc.BindPoint && element.RegisterSpace == shaderInputBindDesc.Space &&
                                                       element.Type == DxcEnumConverter::ReflectTypeToBufferBindingType( shaderInputBindDesc.Type ) )
        {
            return true;
        }
    }
    return false;
}

void ShaderReflectionHelper::FillTypeInfo( ID3D12ShaderReflectionType *reflType, InteropArray<ReflectionResourceField> &fields, const uint32_t parentIndex, const uint32_t level )
{
    D3D12_SHADER_TYPE_DESC typeDesc;
    DxcCheckResult( reflType->GetDesc( &typeDesc ) );

    if ( typeDesc.Members > 0 )
    {
        for ( UINT i = 0; i < typeDesc.Members; i++ )
        {
            ID3D12ShaderReflectionType *memberType = reflType->GetMemberTypeByIndex( i );
            D3D12_SHADER_TYPE_DESC      memberTypeDesc;
            DxcCheckResult( memberType->GetDesc( &memberTypeDesc ) );

            const uint32_t           currentIndex = fields.NumElements( );
            ReflectionResourceField &memberField  = fields.EmplaceElement( );
            memberField.Name                      = reflType->GetMemberTypeName( i );
            memberField.Type                      = DxcEnumConverter::VariableTypeToReflectionType( memberTypeDesc.Type );
            memberField.NumColumns                = memberTypeDesc.Columns;
            memberField.NumRows                   = memberTypeDesc.Rows;
            memberField.Elements                  = memberTypeDesc.Elements;
            memberField.Offset                    = memberTypeDesc.Offset;
            memberField.Level                     = level;
            memberField.ParentIndex               = parentIndex;

            if ( memberTypeDesc.Members > 0 )
            {
                FillTypeInfo( memberType, fields, currentIndex, level + 1 );
            }
        }
    }
}

void ShaderReflectionHelper::FillReflectionData( ID3D12ShaderReflection *shaderReflection, ID3D12FunctionReflection *functionReflection, ReflectionDesc &reflectionDesc,
                                                 const int resourceIndex )
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
        LOG( FATAL ) << "No shader reflection object found, make sure no compilation errors occurred.";
        return;
    }

    reflectionDesc.Name = shaderInputBindDesc.Name;
    reflectionDesc.Type = DxcEnumConverter::ReflectTypeToBufferReflectionBindingType( shaderInputBindDesc.Type );

    DZ_RETURN_IF( reflectionDesc.Type != ReflectionBindingType::Struct );

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
    reflectionDesc.NumBytes = ( bufferDesc.Size + 15 ) & ~( 16 - 1 ); // Align 16

    for ( const uint32_t i : std::views::iota( 0u, bufferDesc.Variables ) )
    {
        ID3D12ShaderReflectionVariable *variable = constantBuffer->GetVariableByIndex( i );
        D3D12_SHADER_VARIABLE_DESC      variableDesc;
        DxcCheckResult( variable->GetDesc( &variableDesc ) );

        ID3D12ShaderReflectionType *reflectionType = variable->GetType( );
        D3D12_SHADER_TYPE_DESC      typeDesc;
        DxcCheckResult( reflectionType->GetDesc( &typeDesc ) );

        const uint32_t           currentIndex = reflectionDesc.Fields.NumElements( );
        ReflectionResourceField &field        = reflectionDesc.Fields.EmplaceElement( );
        field.Name                            = variableDesc.Name;
        field.Type                            = DxcEnumConverter::VariableTypeToReflectionType( typeDesc.Type );
        field.NumColumns                      = typeDesc.Columns;
        field.NumRows                         = typeDesc.Rows;
        field.Elements                        = typeDesc.Elements;
        field.Offset                          = variableDesc.StartOffset;
        field.Level                           = 0;
        field.ParentIndex                     = UINT32_MAX;

        if ( typeDesc.Members > 0 )
        {
            FillTypeInfo( reflectionType, reflectionDesc.Fields, currentIndex, 1 );
        }
    }
}

void ShaderReflectionHelper::DxcCheckResult( const HRESULT hr )
{
    if ( FAILED( hr ) )
    {
        LOG( ERROR ) << "DXC Error: " << hr;
    }
}

ThreadGroupInfo ShaderReflectionHelper::ExtractThreadGroupSize( ID3D12ShaderReflection *shaderReflection, ID3D12FunctionReflection *functionReflection )
{
    ThreadGroupInfo threadGroup = { 0, 0, 0 };
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

PrimitiveTopology ShaderReflectionHelper::ExtractMeshOutputTopology( ID3D12ShaderReflection *shaderReflection )
{
    if ( !shaderReflection )
    {
        return PrimitiveTopology::Triangle;
    }

    D3D12_SHADER_DESC shaderDesc;
    if ( SUCCEEDED( shaderReflection->GetDesc( &shaderDesc ) ) && D3D12_SHVER_GET_TYPE( shaderDesc.Version ) == D3D12_SHVER_MESH_SHADER )
    {
        switch ( shaderDesc.InputPrimitive )
        {
        case D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
            return PrimitiveTopology::Point;
        case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
        case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
            return PrimitiveTopology::Line;
        case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
        case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
            return PrimitiveTopology::Triangle;
        default:
            break;
        }
    }

    return PrimitiveTopology::Triangle;
}
