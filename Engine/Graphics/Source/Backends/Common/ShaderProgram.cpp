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

#include <DenOfIzGraphics/Backends/Common/ShaderProgram.h>
#include <directx/d3d12shader.h>
#include <ranges>

using namespace DenOfIz;

ShaderProgram::ShaderProgram( const ShaderProgramDesc &desc ) : m_desc( desc )
{
    Compile( );
}

void ShaderProgram::Compile( )
{
    const ShaderCompiler &compiler = ShaderCompilerInstance( );

    for ( auto &shader : m_desc.Shaders )
    {
        CompileOptions options = { };
        options.Defines        = shader.Defines;
        options.EntryPoint     = shader.EntryPoint;
        options.Stage          = shader.Stage;
        options.TargetIL       = m_desc.TargetIL;

        m_compiledShaders.push_back( compiler.CompileHLSL( shader.Path, options ) );
    }
}

const ShaderCompiler &ShaderProgram::ShaderCompilerInstance( ) const
{
    static ShaderCompiler compiler;
    return compiler;
}

const std::vector<CompiledShader *> ShaderProgram::GetCompiledShaders( ) const
{
    std::vector<CompiledShader *> compiledShaders;
    for ( auto &shader : m_compiledShaders )
    {
        compiledShaders.push_back( shader.get( ) );
    }
    return compiledShaders;
}

Format MaskToFormat( uint32_t mask )
{
    switch ( mask )
    {
    case 1:
        return Format::R32Float;
    case 3:
        return Format::R32G32Float;
    case 7:
        return Format::R32G32B32Float;
    case 15:
        return Format::R32G32B32A32Float;
    default:
        return Format::Undefined;
    }
}

ShaderReflectDesc ShaderProgram::Reflect( ) const
{
    ShaderReflectDesc result{ };

    InputLayoutDesc   &inputLayout   = result.InputLayout;
    RootSignatureDesc &rootSignature = result.RootSignature;

    for ( auto &shader : m_compiledShaders )
    {
        IDxcBlob       *reflectionBlob = shader->Reflection;
        const DxcBuffer reflectionBuffer{
            .Ptr      = reflectionBlob->GetBufferPointer( ),
            .Size     = reflectionBlob->GetBufferSize( ),
            .Encoding = 0,
        };

        ID3D12ShaderReflection *shaderReflection{ };

        ShaderCompilerInstance( ).DxcUtils( )->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &shaderReflection ) );
        D3D12_SHADER_DESC shaderDesc{ };
        shaderReflection->GetDesc( &shaderDesc );

        if ( shader->Stage == ShaderStage::Vertex )
        {
            InitInputLayout( shaderReflection, inputLayout, shaderDesc );
        }

        ProcessRootSignature( shaderReflection, rootSignature, shaderDesc );
    }

    return result;
}

void ShaderProgram::InitInputLayout( ID3D12ShaderReflection *shaderReflection, InputLayoutDesc &inputLayoutDesc, const D3D12_SHADER_DESC &shaderDesc ) const
{
    InputGroupDesc &inputGroupDesc = inputLayoutDesc.InputGroups.emplace_back( );
    for ( const uint32_t parameterIndex : std::views::iota( 0u, shaderDesc.InputParameters ) )
    {
        D3D12_SIGNATURE_PARAMETER_DESC signatureParameterDesc{ };
        shaderReflection->GetInputParameterDesc( parameterIndex, &signatureParameterDesc );
        InputLayoutElementDesc &inputElementDesc = inputGroupDesc.Elements.emplace_back( );

        inputElementDesc.Semantic      = SemanticFromString( signatureParameterDesc.SemanticName );
        inputElementDesc.SemanticIndex = signatureParameterDesc.SemanticIndex;
        inputElementDesc.Format        = MaskToFormat( signatureParameterDesc.Mask );
    }
}

ResourceDescriptor SetRootSignatureType( D3D_SHADER_INPUT_TYPE type )
{
    switch ( type )
    {
    case D3D_SIT_CBUFFER:
        return ResourceDescriptor::UniformBuffer;
    case D3D_SIT_TEXTURE:
        return ResourceDescriptor::Texture;
    case D3D_SIT_SAMPLER:
        return ResourceDescriptor::Sampler;
    case D3D_SIT_TBUFFER:
        return ResourceDescriptor::Texture;
    case D3D_SIT_BYTEADDRESS:
    case D3D_SIT_STRUCTURED:
        return ResourceDescriptor::Buffer;
    case D3D_SIT_UAV_APPEND_STRUCTURED:
    case D3D_SIT_UAV_CONSUME_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED:
    case D3D_SIT_UAV_RWTYPED:
    case D3D_SIT_UAV_RWBYTEADDRESS:
    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
    case D3D_SIT_RTACCELERATIONSTRUCTURE:
        return ResourceDescriptor::RWBuffer;
    case D3D_SIT_UAV_FEEDBACKTEXTURE:
        break;
    }
}

void ShaderProgram::ProcessRootSignature( ID3D12ShaderReflection *shaderReflection, RootSignatureDesc &rootSignatureDesc, const D3D12_SHADER_DESC &shaderDesc ) const
{
    for ( const uint32_t i : std::views::iota( 0u, shaderDesc.BoundResources ) )
    {
        D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };
        shaderReflection->GetResourceBindingDesc( i, &shaderInputBindDesc );

        ResourceDescriptor   resourceDescriptor  = SetRootSignatureType( shaderInputBindDesc.Type );

        ResourceBindingDesc &resourceBindingDesc = rootSignatureDesc.ResourceBindings.emplace_back( );
        resourceBindingDesc.Name                 = shaderInputBindDesc.Name;
        resourceBindingDesc.Binding              = shaderInputBindDesc.BindPoint;
        resourceBindingDesc.RegisterSpace        = shaderInputBindDesc.Space;
        resourceBindingDesc.BindingType          = DescriptorBufferBindingType::ConstantBuffer;
        resourceBindingDesc.ArraySize            = shaderInputBindDesc.BindCount;
        resourceBindingDesc.Descriptor           = resourceDescriptor;
    }
}
