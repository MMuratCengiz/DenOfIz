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
#include <algorithm>
#include <cstring>
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

    if ( m_desc.TargetIL == TargetIL::MSL )
    {
        ProduceMSL( );
        return;
    }

    for ( auto &shader : m_desc.Shaders )
    {
        CompileDesc compileDesc = { };
        compileDesc.Path        = shader.Path;
        compileDesc.Defines     = shader.Defines;
        compileDesc.EntryPoint  = shader.EntryPoint;
        compileDesc.Stage       = shader.Stage;
        compileDesc.TargetIL    = m_desc.TargetIL;

        m_compiledShaders.push_back( compiler.CompileHLSL( compileDesc ) );
    }
}

#ifdef BUILD_METAL
void PutRootParameter( std::vector<IRRootParameter1> &rootParameters, IRShaderVisibility visibility, std::vector<IRDescriptorRange1> &ranges )
{
    if ( ranges.empty( ) )
    {
        return;
    }

    IRRootParameter1 rootParameter                    = { };
    rootParameter.ParameterType                       = IRRootParameterTypeDescriptorTable;
    rootParameter.ShaderVisibility                    = visibility; // TODO test once All works
    rootParameter.ShaderVisibility                    = IRShaderVisibilityAll;
    rootParameter.DescriptorTable.NumDescriptorRanges = ranges.size( );
    rootParameter.DescriptorTable.pDescriptorRanges   = ranges.data( );
    rootParameters.push_back( rootParameter );
}

IRShaderVisibility ShaderStageToShaderVisibility( ShaderStage stage )
{
    switch ( stage )
    {
    case ShaderStage::Vertex:
        return IRShaderVisibilityVertex;
    case ShaderStage::Pixel:
        return IRShaderVisibilityPixel;
    case ShaderStage::Hull:
        return IRShaderVisibilityHull;
    case ShaderStage::Domain:
        return IRShaderVisibilityDomain;
    case ShaderStage::Geometry:
        return IRShaderVisibilityGeometry;
    case ShaderStage::Compute:
        return IRShaderVisibilityAll;
    default:
        return IRShaderVisibilityAll;
    }
}

// For metal we need to produce a root signature to compile a correct metallib
void ShaderProgram::ProduceMSL( )
{
    const ShaderCompiler           &compiler = ShaderCompilerInstance( );
    std::vector<IRDescriptorRange1> descriptorRanges;

    // In Metal, we need a separate descriptor table for samplers/textures and buffers.
    struct RegisterSpaceRange
    {
        std::vector<IRDescriptorRange1> CbvSrvUavRanges;
        std::vector<IRDescriptorRange1> SamplerRanges;
        IRShaderVisibility              ShaderVisibility;
    };
    std::vector<RegisterSpaceRange>              registerSpaceRanges;
    std::vector<std::unique_ptr<CompiledShader>> dxilShaders;

    for ( auto &shader : m_desc.Shaders )
    {
        CompileDesc compileDesc = { };
        compileDesc.Path        = shader.Path;
        compileDesc.Defines     = shader.Defines;
        compileDesc.EntryPoint  = shader.EntryPoint;
        compileDesc.Stage       = shader.Stage;
        compileDesc.TargetIL    = TargetIL::DXIL;
        auto compiledShader     = compiler.CompileHLSL( compileDesc );

        ID3D12ShaderReflection *shaderReflection = ShaderReflection( compiledShader.get( ) );
        dxilShaders.push_back( std::move( compiledShader ) );

        D3D12_SHADER_DESC shaderDesc{ };
        shaderReflection->GetDesc( &shaderDesc );

        for ( const uint32_t i : std::views::iota( 0u, shaderDesc.BoundResources ) )
        {
            D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };
            shaderReflection->GetResourceBindingDesc( i, &shaderInputBindDesc );

            ContainerUtilities::EnsureSize( registerSpaceRanges, shaderInputBindDesc.Space );
            auto &registerSpaceRange = registerSpaceRanges[ shaderInputBindDesc.Space ];

            IRShaderVisibility shaderVisibility = ShaderStageToShaderVisibility( shader.Stage );

            if ( registerSpaceRange.ShaderVisibility != 0 && registerSpaceRange.ShaderVisibility != shaderVisibility )
            {
                registerSpaceRange.ShaderVisibility = IRShaderVisibilityAll;
            }
            else
            {
                registerSpaceRange.ShaderVisibility = shaderVisibility;
            }

            IRDescriptorRange1 descriptorRange                = { };
            descriptorRange.BaseShaderRegister                = shaderInputBindDesc.BindPoint;
            descriptorRange.NumDescriptors                    = shaderInputBindDesc.BindCount;
            descriptorRange.RegisterSpace                     = shaderInputBindDesc.Space;
            descriptorRange.OffsetInDescriptorsFromTableStart = IRDescriptorRangeOffsetAppend;

            switch ( shaderInputBindDesc.Type )
            {
            case D3D_SIT_CBUFFER:
            case D3D_SIT_TBUFFER:
                descriptorRange.RangeType = IRDescriptorRangeTypeCBV;
                registerSpaceRange.CbvSrvUavRanges.push_back( descriptorRange );
                break;
            case D3D_SIT_TEXTURE:
            case D3D_SIT_STRUCTURED:
            case D3D_SIT_BYTEADDRESS:
            case D3D_SIT_RTACCELERATIONSTRUCTURE:
                descriptorRange.RangeType = IRDescriptorRangeTypeSRV;
                registerSpaceRange.CbvSrvUavRanges.push_back( descriptorRange );
                break;
            case D3D_SIT_SAMPLER:
                descriptorRange.RangeType = IRDescriptorRangeTypeSampler;
                registerSpaceRange.SamplerRanges.push_back( descriptorRange );
                break;
            case D3D_SIT_UAV_APPEND_STRUCTURED:
            case D3D_SIT_UAV_CONSUME_STRUCTURED:
            case D3D_SIT_UAV_RWSTRUCTURED:
            case D3D_SIT_UAV_RWTYPED:
            case D3D_SIT_UAV_RWBYTEADDRESS:
            case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
            case D3D_SIT_UAV_FEEDBACKTEXTURE:
                descriptorRange.RangeType = IRDescriptorRangeTypeUAV;
                registerSpaceRange.CbvSrvUavRanges.push_back( descriptorRange );
                break;
            default:
                LOG( ERROR ) << "Unknown resource type";
                break;
            }
        }
    }

    m_metalDescriptorOffsets.resize( registerSpaceRanges.size( ) );

    std::vector<IRRootParameter1> rootParameters;
    int                           registerSpace       = 0;
    int                           registerSpaceOffset = 0;
    for ( auto &registerSpaceRange : registerSpaceRanges )
    {
        MetalDescriptorOffsets &offsets = m_metalDescriptorOffsets[ registerSpace ];

        int numTables = registerSpaceOffset;
        // Only keep set offset if there are any resources in that set for debugging purposes
        if ( !registerSpaceRange.CbvSrvUavRanges.empty( ) )
        {
            offsets.CbvSrvUavOffset = numTables;
            ++numTables;
        }
        if ( !registerSpaceRange.SamplerRanges.empty( ) )
        {
            offsets.SamplerOffset = numTables;
            ++numTables;
        }

        PutRootParameter( rootParameters, registerSpaceRange.ShaderVisibility, registerSpaceRange.CbvSrvUavRanges );
        PutRootParameter( rootParameters, registerSpaceRange.ShaderVisibility, registerSpaceRange.SamplerRanges );
        ++registerSpace;

        registerSpaceOffset += numTables;
    }

    IRVersionedRootSignatureDescriptor desc;
    desc.version        = IRRootSignatureVersion_1_1;
    desc.desc_1_1.Flags = IRRootSignatureFlags( IRRootSignatureFlagCBVSRVUAVHeapDirectlyIndexed | IRRootSignatureFlagSamplerHeapDirectlyIndexed );
    // TODO we also need to handle root constants
    desc.desc_1_1.NumParameters = rootParameters.size( );
    desc.desc_1_1.pParameters   = rootParameters.data( );
    // TODO:
    desc.desc_1_1.NumStaticSamplers = 0;
    desc.desc_1_1.pStaticSamplers   = nullptr;

    IRError         *error         = nullptr;
    IRRootSignature *rootSignature = IRRootSignatureCreateFromDescriptor( &desc, &error );

    if ( error )
    {
        LOG( ERROR ) << "Error producing IRRootSignature, error code [" << IRErrorGetCode( error ) << "]";
        IRErrorDestroy( error );
    }

    int i = 0;
    for ( auto &shader : m_desc.Shaders )
    {
        CompileDesc compileDesc = { };
        compileDesc.Path        = shader.Path;
        compileDesc.Defines     = shader.Defines;
        compileDesc.EntryPoint  = shader.EntryPoint;
        compileDesc.Stage       = shader.Stage;
        compileDesc.TargetIL    = TargetIL::MSL;

        auto     &compiledShader = dxilShaders[ i++ ];
        IDxcBlob *mslBlob        = compiler.DxilToMsl( compileDesc, compiledShader->Blob, rootSignature );
        compiledShader->Blob->Release( );
        compiledShader->Blob = mslBlob;

        m_compiledShaders.push_back( std::move( compiledShader ) );
    }

    IRRootSignatureDestroy( rootSignature );
}
#endif

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

ResourceDescriptor ReflectTypeToRootSignatureType( D3D_SHADER_INPUT_TYPE type )
{
    switch ( type )
    {
    case D3D_SIT_CBUFFER:
        return ResourceDescriptor::UniformBuffer;
    case D3D_SIT_TBUFFER:
    case D3D_SIT_TEXTURE:
        return ResourceDescriptor::Texture;
    case D3D_SIT_SAMPLER:
        return ResourceDescriptor::Sampler;
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
    LOG( ERROR ) << "Unknown resource type";
    return ResourceDescriptor::Texture;
}

DescriptorBufferBindingType ReflectTypeToBufferBindingType( D3D_SHADER_INPUT_TYPE type )
{
    switch ( type )
    {
    case D3D_SIT_CBUFFER:
        return DescriptorBufferBindingType::ConstantBuffer;
    case D3D_SIT_TEXTURE:
        return DescriptorBufferBindingType::ShaderResource;
    case D3D_SIT_SAMPLER:
        return DescriptorBufferBindingType::Sampler;
    case D3D_SIT_TBUFFER:
        return DescriptorBufferBindingType::ShaderResource;
    case D3D_SIT_BYTEADDRESS:
    case D3D_SIT_STRUCTURED:
        return DescriptorBufferBindingType::ShaderResource;
    case D3D_SIT_UAV_APPEND_STRUCTURED:
    case D3D_SIT_UAV_CONSUME_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED:
    case D3D_SIT_UAV_RWTYPED:
    case D3D_SIT_UAV_RWBYTEADDRESS:
    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
    case D3D_SIT_RTACCELERATIONSTRUCTURE:
        return DescriptorBufferBindingType::UnorderedAccess;
    case D3D_SIT_UAV_FEEDBACKTEXTURE:
        break;
    }
    LOG( ERROR ) << "Unknown resource type";
    return DescriptorBufferBindingType::ConstantBuffer;
}

ShaderReflectDesc ShaderProgram::Reflect( ) const
{
    ShaderReflectDesc result{ };

    InputLayoutDesc   &inputLayout   = result.InputLayout;
    RootSignatureDesc &rootSignature = result.RootSignature;
#ifdef BUILD_METAL
    std::vector<uint32_t> descriptorTableLocations;
#endif

    for ( auto &shader : m_compiledShaders )
    {
#ifdef BUILD_METAL
        IRObject           *ir           = dynamic_cast<MetalDxcBlob_Impl *>( shader->Blob )->IrObject;
        IRShaderReflection *irReflection = IRShaderReflectionCreate( );
        IRObjectGetReflection( ir, ShaderCompiler::ConvertIrShaderStage( shader->Stage ), irReflection );
#endif
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

#ifdef BUILD_METAL
        std::vector<IRResourceLocation> resources( IRShaderReflectionGetResourceCount( irReflection ) );
        IRShaderReflectionGetResourceLocations( irReflection, resources.data( ) );

#endif

        for ( const uint32_t i : std::views::iota( 0u, shaderDesc.BoundResources ) )
        {
            D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };
            shaderReflection->GetResourceBindingDesc( i, &shaderInputBindDesc );

            ResourceBindingDesc &resourceBindingDesc = rootSignature.ResourceBindings.emplace_back( );
            resourceBindingDesc.Name                 = shaderInputBindDesc.Name;
            resourceBindingDesc.Binding              = shaderInputBindDesc.BindPoint;
            resourceBindingDesc.RegisterSpace        = shaderInputBindDesc.Space;
            resourceBindingDesc.ArraySize            = shaderInputBindDesc.BindCount;
            resourceBindingDesc.BindingType          = ReflectTypeToBufferBindingType( shaderInputBindDesc.Type );
            resourceBindingDesc.Descriptor           = ReflectTypeToRootSignatureType( shaderInputBindDesc.Type );
            // Todo !IMPROVEMENT! We should keep track of resources created and provide multiple stages if required.
            resourceBindingDesc.Stages.push_back( shader->Stage );
            FillReflectionData( shaderReflection, resourceBindingDesc.Reflection, i );
#ifdef BUILD_METAL
            switch ( resourceBindingDesc.Reflection.Type )
            {
            case ReflectionBindingType::Pointer:
            case ReflectionBindingType::Struct:
            case ReflectionBindingType::Texture:
                resourceBindingDesc.Reflection.DescriptorOffset = m_metalDescriptorOffsets[ shaderInputBindDesc.Space ].CbvSrvUavOffset;
                break;
            case ReflectionBindingType::SamplerDesc:
                resourceBindingDesc.Reflection.DescriptorOffset = m_metalDescriptorOffsets[ shaderInputBindDesc.Space ].SamplerOffset;
                break;
            }
            ContainerUtilities::EnsureSize( descriptorTableLocations, resourceBindingDesc.Reflection.DescriptorOffset );
            uint32_t &locationHint                              = descriptorTableLocations[ resourceBindingDesc.Reflection.DescriptorOffset ];
            resourceBindingDesc.Reflection.DescriptorTableIndex = locationHint++;
#endif
        }

#ifdef BUILD_METAL
        IRShaderReflectionDestroy( irReflection );
#endif
        shaderReflection->Release( );
    }

    return result;
}

ReflectionFieldType DXCVariableTypeToReflectionType( const D3D_SHADER_VARIABLE_TYPE &type )
{
    switch ( type )
    {
    case D3D_SVT_VOID:
        return ReflectionFieldType::Void;
    case D3D_SVT_BOOL:
        return ReflectionFieldType::Bool;
    case D3D_SVT_INT:
        return ReflectionFieldType::Int;
    case D3D_SVT_FLOAT:
        return ReflectionFieldType::Float;
    case D3D_SVT_STRING:
        return ReflectionFieldType::String;
    case D3D_SVT_TEXTURE:
        return ReflectionFieldType::Texture;
    case D3D_SVT_TEXTURE1D:
        return ReflectionFieldType::Texture1D;
    case D3D_SVT_TEXTURE2D:
        return ReflectionFieldType::Texture2D;
    case D3D_SVT_TEXTURE3D:
        return ReflectionFieldType::Texture3D;
    case D3D_SVT_TEXTURECUBE:
        return ReflectionFieldType::TextureCube;
    case D3D_SVT_SAMPLER:
        return ReflectionFieldType::Sampler;
    case D3D_SVT_SAMPLER1D:
        return ReflectionFieldType::Sampler1d;
    case D3D_SVT_SAMPLER2D:
        return ReflectionFieldType::Sampler2d;
    case D3D_SVT_SAMPLER3D:
        return ReflectionFieldType::Sampler3d;
    case D3D_SVT_SAMPLERCUBE:
        return ReflectionFieldType::SamplerCube;
    case D3D_SVT_PIXELFRAGMENT:
        return ReflectionFieldType::PixelFragment;
    case D3D_SVT_VERTEXFRAGMENT:
        return ReflectionFieldType::VertexFragment;
    case D3D_SVT_UINT:
        return ReflectionFieldType::Uint;
    case D3D_SVT_UINT8:
        return ReflectionFieldType::Uint8;
    case D3D_SVT_DEPTHSTENCIL:
        return ReflectionFieldType::DepthStencil;
    case D3D_SVT_BLEND:
        return ReflectionFieldType::Blend;
    case D3D_SVT_BUFFER:
        return ReflectionFieldType::Buffer;
    case D3D_SVT_CBUFFER:
        return ReflectionFieldType::CBuffer;
    case D3D_SVT_TBUFFER:
        return ReflectionFieldType::TBuffer;
    case D3D_SVT_TEXTURE1DARRAY:
        return ReflectionFieldType::Texture1DArray;
    case D3D_SVT_TEXTURE2DARRAY:
        return ReflectionFieldType::Texture2DArray;
    case D3D_SVT_RENDERTARGETVIEW:
        return ReflectionFieldType::RenderTargetView;
    case D3D_SVT_DEPTHSTENCILVIEW:
        return ReflectionFieldType::DepthStencilView;
    case D3D_SVT_TEXTURE2DMS:
        return ReflectionFieldType::Texture2Dms;
    case D3D_SVT_TEXTURE2DMSARRAY:
        return ReflectionFieldType::Texture2DmsArray;
    case D3D_SVT_TEXTURECUBEARRAY:
        return ReflectionFieldType::TextureCubeArray;
    case D3D_SVT_INTERFACE_POINTER:
        return ReflectionFieldType::InterfacePointer;
    case D3D_SVT_DOUBLE:
        return ReflectionFieldType::Double;
    case D3D_SVT_RWTEXTURE1D:
        return ReflectionFieldType::RWTexture1D;
    case D3D_SVT_RWTEXTURE1DARRAY:
        return ReflectionFieldType::RWTexture1DArray;
    case D3D_SVT_RWTEXTURE2D:
        return ReflectionFieldType::RWTexture2D;
    case D3D_SVT_RWTEXTURE2DARRAY:
        return ReflectionFieldType::RWTexture2DArray;
    case D3D_SVT_RWTEXTURE3D:
        return ReflectionFieldType::RWTexture3D;
    case D3D_SVT_RWBUFFER:
        return ReflectionFieldType::RWBuffer;
    case D3D_SVT_BYTEADDRESS_BUFFER:
        return ReflectionFieldType::ByteAddressBuffer;
    case D3D_SVT_RWBYTEADDRESS_BUFFER:
        return ReflectionFieldType::RWByteAddressBuffer;
    case D3D_SVT_STRUCTURED_BUFFER:
        return ReflectionFieldType::StructuredBuffer;
    case D3D_SVT_RWSTRUCTURED_BUFFER:
        return ReflectionFieldType::RWStructuredBuffer;
    case D3D_SVT_APPEND_STRUCTURED_BUFFER:
        return ReflectionFieldType::AppendStructuredBuffer;
    case D3D_SVT_CONSUME_STRUCTURED_BUFFER:
        return ReflectionFieldType::ConsumeStructuredBuffer;
    case D3D_SVT_MIN8FLOAT:
        return ReflectionFieldType::Min8Float;
    case D3D_SVT_MIN10FLOAT:
        return ReflectionFieldType::Min10Float;
    case D3D_SVT_MIN16FLOAT:
        return ReflectionFieldType::Min16Float;
    case D3D_SVT_MIN12INT:
        return ReflectionFieldType::Min12Int;
    case D3D_SVT_MIN16INT:
        return ReflectionFieldType::Min16Int;
    case D3D_SVT_MIN16UINT:
        return ReflectionFieldType::Min16UInt;
    case D3D_SVT_INT16:
        return ReflectionFieldType::Int16;
    case D3D_SVT_UINT16:
        return ReflectionFieldType::UInt16;
    case D3D_SVT_FLOAT16:
        return ReflectionFieldType::Float16;
    case D3D_SVT_INT64:
        return ReflectionFieldType::Int64;
    case D3D_SVT_UINT64:
        return ReflectionFieldType::UInt64;
    case D3D_SVT_PIXELSHADER:
        return ReflectionFieldType::PixelShader;
    case D3D_SVT_VERTEXSHADER:
        return ReflectionFieldType::VertexShader;
    case D3D_SVT_GEOMETRYSHADER:
        return ReflectionFieldType::GeometryShader;
    case D3D_SVT_HULLSHADER:
        return ReflectionFieldType::HullShader;
    case D3D_SVT_DOMAINSHADER:
        return ReflectionFieldType::DomainShader;
    case D3D_SVT_COMPUTESHADER:
        return ReflectionFieldType::ComputeShader;
    default:
        return ReflectionFieldType::Undefined;
    }
}

void ShaderProgram::FillReflectionData( ID3D12ShaderReflection *shaderReflection, ReflectionDesc &reflectionDesc, int resourceIndex ) const
{
    D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };

    shaderReflection->GetResourceBindingDesc( resourceIndex, &shaderInputBindDesc );

    reflectionDesc.Name = shaderInputBindDesc.Name;

    switch ( shaderInputBindDesc.Type )
    {

    case D3D_SIT_CBUFFER:
        reflectionDesc.Type = ReflectionBindingType::Struct;
        break;
    case D3D_SIT_TBUFFER:
        break;
    case D3D_SIT_TEXTURE:
        reflectionDesc.Type = ReflectionBindingType::Texture;
        break;
    case D3D_SIT_SAMPLER:
        reflectionDesc.Type = ReflectionBindingType::SamplerDesc;
        break;
    case D3D_SIT_UAV_RWTYPED:
    case D3D_SIT_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED:
    case D3D_SIT_BYTEADDRESS:
    case D3D_SIT_UAV_RWBYTEADDRESS:
    case D3D_SIT_UAV_APPEND_STRUCTURED:
    case D3D_SIT_UAV_CONSUME_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
    case D3D_SIT_RTACCELERATIONSTRUCTURE:
    case D3D_SIT_UAV_FEEDBACKTEXTURE:
        reflectionDesc.Type = ReflectionBindingType::Pointer;
        break;
    }

    ID3D12ShaderReflectionConstantBuffer *constantBuffer = shaderReflection->GetConstantBufferByIndex( resourceIndex );
    D3D12_SHADER_BUFFER_DESC              bufferDesc;
    constantBuffer->GetDesc( &bufferDesc );

    if ( reflectionDesc.Type != ReflectionBindingType::Struct )
    {
        return;
    }

    for ( const uint32_t i : std::views::iota( 0u, bufferDesc.Variables ) )
    {
        ID3D12ShaderReflectionVariable *variable = constantBuffer->GetVariableByIndex( i );
        D3D12_SHADER_VARIABLE_DESC      variableDesc;
        variable->GetDesc( &variableDesc );

        ID3D12ShaderReflectionType *reflectionType = variable->GetType( );
        D3D12_SHADER_TYPE_DESC      typeDesc;
        reflectionType->GetDesc( &typeDesc );

        ReflectionResourceField subField{ };
        subField.Name       = variableDesc.Name;
        subField.Type       = DXCVariableTypeToReflectionType( typeDesc.Type );
        subField.NumColumns = typeDesc.Columns;
        subField.NumRows    = typeDesc.Rows;
        reflectionDesc.Fields.push_back( subField );
    }
}

ID3D12ShaderReflection *ShaderProgram::ShaderReflection( CompiledShader *compiledShader ) const
{
    IDxcBlob       *reflectionBlob = compiledShader->Reflection;
    const DxcBuffer reflectionBuffer{
        .Ptr      = reflectionBlob->GetBufferPointer( ),
        .Size     = reflectionBlob->GetBufferSize( ),
        .Encoding = 0,
    };

    ID3D12ShaderReflection *shaderReflection{ };
    ShaderCompilerInstance( ).DxcUtils( )->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &shaderReflection ) );
    return shaderReflection;
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
