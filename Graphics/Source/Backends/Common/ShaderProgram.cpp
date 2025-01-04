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
#include <DenOfIzGraphics/Utilities/ContainerUtilities.h>
#include <directx/d3d12shader.h>
#include <ranges>
#include <set>
#include <utility>

using namespace DenOfIz;

#define DXC_CHECK_RESULT( result )                                                                                                                                                 \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        if ( FAILED( result ) )                                                                                                                                                    \
        {                                                                                                                                                                          \
            LOG( ERROR ) << "DXC Error: " << result;                                                                                                                               \
        }                                                                                                                                                                          \
    }                                                                                                                                                                              \
    while ( false )

ShaderProgram::ShaderProgram( ShaderProgramDesc desc ) : m_desc( std::move( desc ) )
{
    Compile( );
}
/**
 * \brief Compiles the shaders targeting MSL/DXIL/SPIR-V. MSL is double compiled, first time to DXIL and reflect and provide a root signature to the second compilation.
 */
void ShaderProgram::Compile( )
{
    const ShaderCompiler &compiler = ShaderCompilerInstance( );

    if ( m_desc.TargetIL == TargetIL::MSL )
    {
#ifdef BUILD_METAL
        ProduceMSL( );
        return;
#endif
    }

    for ( int i = 0; i < m_desc.Shaders.NumElements( ); ++i )
    {
        const auto &shader = m_desc.Shaders.GetElement( i );

        // Validate Shader
        if ( shader.Path.IsEmpty( ) )
        {
            LOG( ERROR ) << "Shader path is empty";
            continue;
        }

        CompileDesc compileDesc   = { };
        compileDesc.Path          = shader.Path;
        compileDesc.Defines       = shader.Defines;
        compileDesc.EntryPoint    = shader.EntryPoint;
        compileDesc.Stage         = shader.Stage;
        compileDesc.TargetIL      = m_desc.TargetIL;
        compileDesc.RayTracing    = shader.RayTracing;
        compileDesc.EnableCaching = m_desc.EnableCaching;

        m_compiledShaders.push_back( compiler.CompileHLSL( compileDesc ) );
        m_shaderDescs.push_back( shader );
    }
}

#ifdef BUILD_METAL
void PutRootParameterDescriptorTable( std::vector<IRRootParameter1> &rootParameters, IRShaderVisibility visibility, std::vector<IRDescriptorRange1> &ranges )
{
    if ( ranges.empty( ) )
    {
        return;
    }

    IRRootParameter1 &rootParameter                   = rootParameters.emplace_back( );
    rootParameter.ParameterType                       = IRRootParameterTypeDescriptorTable;
    rootParameter.ShaderVisibility                    = visibility; // TODO test once All works
    rootParameter.ShaderVisibility                    = IRShaderVisibilityAll;
    rootParameter.DescriptorTable.NumDescriptorRanges = ranges.size( );
    rootParameter.DescriptorTable.pDescriptorRanges   = ranges.data( );
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

IRRootParameterType BindingTypeToIRRootParameterType( const ResourceBindingType &type )
{
    switch ( type )
    {
    case ResourceBindingType::ConstantBuffer:
        return IRRootParameterTypeCBV;
    case ResourceBindingType::ShaderResource:
        return IRRootParameterTypeSRV;
    case ResourceBindingType::UnorderedAccess:
        return IRRootParameterTypeUAV;
    default:
        break;
    }

    return IRRootParameterTypeCBV;
}

IRRootParameterType IRDescriptorRangeTypeToIRRootParameterType( const IRDescriptorRangeType &type )
{
    switch ( type )
    {
    case IRDescriptorRangeTypeCBV:
        return IRRootParameterTypeCBV;
    case IRDescriptorRangeTypeSRV:
        return IRRootParameterTypeSRV;
    case IRDescriptorRangeTypeUAV:
        return IRRootParameterTypeUAV;
    default:
        break;
    }

    return IRRootParameterTypeCBV;
}

IRDescriptorRangeType ShaderTypeToIRDescriptorType( const D3D_SHADER_INPUT_TYPE &type )
{
    IRDescriptorRangeType descriptorRangeType = IRDescriptorRangeTypeCBV;
    switch ( type )
    {
    case D3D_SIT_CBUFFER:
    case D3D_SIT_TBUFFER:
        descriptorRangeType = IRDescriptorRangeTypeCBV;
        break;
    case D3D_SIT_TEXTURE:
    case D3D_SIT_STRUCTURED:
    case D3D_SIT_BYTEADDRESS:
    case D3D_SIT_RTACCELERATIONSTRUCTURE:
        descriptorRangeType = IRDescriptorRangeTypeSRV;
        break;
    case D3D_SIT_SAMPLER:
        descriptorRangeType = IRDescriptorRangeTypeSampler;
        break;
    case D3D_SIT_UAV_APPEND_STRUCTURED:
    case D3D_SIT_UAV_CONSUME_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED:
    case D3D_SIT_UAV_RWTYPED:
    case D3D_SIT_UAV_RWBYTEADDRESS:
    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
    case D3D_SIT_UAV_FEEDBACKTEXTURE:
        descriptorRangeType = IRDescriptorRangeTypeUAV;
        break;
    default:
        LOG( ERROR ) << "Unknown resource type";
        break;
    }
    return descriptorRangeType;
}

// Todo perhaps share this with main reflection code
void ShaderProgram::IterateBoundResources( CompiledShader *shader, ReflectionState &state, ReflectionCallback &callback ) const
{
    ID3D12ShaderReflection  *shaderReflection  = nullptr;
    ID3D12LibraryReflection *libraryReflection = nullptr;

    IDxcBlob       *reflectionBlob = shader->Reflection;
    const DxcBuffer reflectionBuffer{
        .Ptr      = reflectionBlob->GetBufferPointer( ),
        .Size     = reflectionBlob->GetBufferSize( ),
        .Encoding = 0,
    };

    switch ( shader->Stage )
    {
    case ShaderStage::AnyHit:
    case ShaderStage::Intersection:
    case ShaderStage::ClosestHit:
    case ShaderStage::Raygen:
    case ShaderStage::Miss:
        {
            DXC_CHECK_RESULT( ShaderCompilerInstance( ).DxcUtils( )->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &libraryReflection ) ) );
            state.LibraryReflection = libraryReflection;
            D3D12_LIBRARY_DESC libraryDesc{ };
            DXC_CHECK_RESULT( libraryReflection->GetDesc( &libraryDesc ) );

            for ( const uint32_t i : std::views::iota( 0u, libraryDesc.FunctionCount ) )
            {
                ID3D12FunctionReflection *functionReflection = libraryReflection->GetFunctionByIndex( i );
                D3D12_FUNCTION_DESC       functionDesc{ };
                DXC_CHECK_RESULT( functionReflection->GetDesc( &functionDesc ) );
                state.FunctionReflection = functionReflection;
                for ( const uint32_t j : std::views::iota( 0u, functionDesc.BoundResources ) )
                {
                    D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };
                    DXC_CHECK_RESULT( functionReflection->GetResourceBindingDesc( j, &shaderInputBindDesc ) );

                    callback( shaderInputBindDesc, j );
                }
            }
        }
        break;
    default:
        {
            DXC_CHECK_RESULT( ShaderCompilerInstance( ).DxcUtils( )->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &shaderReflection ) ) );
            state.ShaderReflection = shaderReflection;

            D3D12_SHADER_DESC shaderDesc{ };
            DXC_CHECK_RESULT( shaderReflection->GetDesc( &shaderDesc ) );

            for ( const uint32_t i : std::views::iota( 0u, shaderDesc.BoundResources ) )
            {
                D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };
                DXC_CHECK_RESULT( shaderReflection->GetResourceBindingDesc( i, &shaderInputBindDesc ) );

                callback( shaderInputBindDesc, i );
            }
        }
        break;
    }
}

bool IsResourceAlreadyProcessed( std::vector<D3D12_SHADER_INPUT_BIND_DESC> &processedInputs, D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc )
{
    for ( auto &processedInput : processedInputs )
    {
        if ( processedInput.BindPoint == shaderInputBindDesc.BindPoint && processedInput.Space == shaderInputBindDesc.Space && processedInput.Type == shaderInputBindDesc.Type )
        {
            return true;
        }
    }
    return false;
}

IRDescriptorRange1 CreateDescriptorRange( D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc )
{
    IRDescriptorRange1 descriptorRange                = { };
    descriptorRange.BaseShaderRegister                = shaderInputBindDesc.BindPoint;
    descriptorRange.NumDescriptors                    = shaderInputBindDesc.BindCount;
    descriptorRange.RegisterSpace                     = shaderInputBindDesc.Space;
    descriptorRange.OffsetInDescriptorsFromTableStart = IRDescriptorRangeOffsetAppend;
    descriptorRange.RangeType                         = ShaderTypeToIRDescriptorType( shaderInputBindDesc.Type );
    return descriptorRange;
}

IRRootSignature *ShaderProgram::CreateRootSignature( std::vector<RegisterSpaceRange> &registerSpaceRanges, std::vector<MetalDescriptorOffsets> &metalDescriptorOffsets,
                                                     bool isLocal ) const
{
    std::vector<IRRootParameter1> rootParameters;
    int                           registerSpace = 0;
    int                           numEntries    = 0;
    for ( auto &registerSpaceRange : registerSpaceRanges )
    {
        int                     rootConstantIndex = 0;
        MetalDescriptorOffsets &offsets           = metalDescriptorOffsets[ registerSpace ];
        for ( auto &rootConstant : registerSpaceRange.RootConstants )
        {
            IRRootParameter1 &rootParameter        = rootParameters.emplace_back( );
            rootParameter.ParameterType            = IRRootParameterType32BitConstants;
            rootParameter.ShaderVisibility         = IRShaderVisibilityAll;
            rootParameter.Constants.Num32BitValues = rootConstant.Num32BitValues;
            rootParameter.Constants.RegisterSpace  = rootConstant.RegisterSpace;
            rootParameter.Constants.ShaderRegister = rootConstant.ShaderRegister;
            uint32_t hash                          = Utilities::HashInts( rootParameter.ParameterType, rootConstant.RegisterSpace, rootConstant.ShaderRegister );
            offsets.UniqueTLABIndex[ hash ]        = numEntries++;
            ++rootConstantIndex;
        }
    }

    for ( auto &registerSpaceRange : registerSpaceRanges )
    {
        MetalDescriptorOffsets &offsets = metalDescriptorOffsets[ registerSpace ];
        if ( !registerSpaceRange.CbvSrvUavRanges.empty( ) )
        {
            offsets.CbvSrvUavOffset = numEntries++;
        }
        PutRootParameterDescriptorTable( rootParameters, registerSpaceRange.ShaderVisibility, registerSpaceRange.CbvSrvUavRanges );

        if ( !registerSpaceRange.SamplerRanges.empty( ) )
        {
            offsets.SamplerOffset = numEntries++;
            PutRootParameterDescriptorTable( rootParameters, registerSpaceRange.ShaderVisibility, registerSpaceRange.SamplerRanges );
        }

        int rootArgumentIndex = 0;
        for ( auto &rootArgument : registerSpaceRange.RootArguments )
        {
            IRRootParameter1 &rootParameter         = rootParameters.emplace_back( );
            rootParameter.ParameterType             = registerSpaceRange.RootArgumentTypes[ rootArgumentIndex ];
            rootParameter.ShaderVisibility          = IRShaderVisibilityAll;
            rootParameter.Descriptor.RegisterSpace  = rootArgument.RegisterSpace;
            rootParameter.Descriptor.ShaderRegister = rootArgument.ShaderRegister;
            uint32_t hash                           = Utilities::HashInts( rootParameter.ParameterType, rootArgument.RegisterSpace, rootArgument.ShaderRegister );
            offsets.UniqueTLABIndex[ hash ]         = numEntries++;
            LOG( INFO ) << "Root argument offset: " << offsets.UniqueTLABIndex[ hash ] << " for register space: " << registerSpace;
            ++rootArgumentIndex;
        }

        ++registerSpace;
    }

#ifndef NDEBUG
    if ( isLocal )
    {
        DumpIRRootParameters( rootParameters, "Metal Local Root Signature" );
    }
    else
    {
        DumpIRRootParameters( rootParameters, "Metal Global Root Signature" );
    }
#endif

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

    return rootSignature;
}

void ShaderProgram::DumpIRRootParameters( const std::vector<IRRootParameter1> &rootParameters, const char *prefix = "" ) const
{
    LOG( INFO ) << "\n=== " << prefix << " IR Root Parameters ===";
    LOG( INFO ) << "Total Parameters: " << rootParameters.size( );

    for ( size_t i = 0; i < rootParameters.size( ); ++i )
    {
        const auto &param = rootParameters[ i ];
        LOG( INFO ) << "\nParameter[" << i << "]:";
        LOG( INFO ) << "  Type: " << [ & ]( )
        {
            switch ( param.ParameterType )
            {
            case IRRootParameterTypeDescriptorTable:
                return "Descriptor Table";
            case IRRootParameterType32BitConstants:
                return "32 Bit Constants";
            case IRRootParameterTypeCBV:
                return "CBV";
            case IRRootParameterTypeSRV:
                return "SRV";
            case IRRootParameterTypeUAV:
                return "UAV";
            default:
                return "Unknown";
            }
        }( );

        LOG( INFO ) << "  Shader Visibility: " << [ & ]( )
        {
            switch ( param.ShaderVisibility )
            {
            case IRShaderVisibilityAll:
                return "All";
            case IRShaderVisibilityVertex:
                return "Vertex";
            case IRShaderVisibilityPixel:
                return "Pixel";
            case IRShaderVisibilityGeometry:
                return "Geometry";
            case IRShaderVisibilityHull:
                return "Hull";
            case IRShaderVisibilityDomain:
                return "Domain";
            default:
                return "Unknown";
            }
        }( );

        // Log specific data based on parameter type
        switch ( param.ParameterType )
        {
        case IRRootParameterTypeDescriptorTable:
            {
                LOG( INFO ) << "  Descriptor Table:";
                LOG( INFO ) << "    NumDescriptorRanges: " << param.DescriptorTable.NumDescriptorRanges;

                for ( uint32_t j = 0; j < param.DescriptorTable.NumDescriptorRanges; j++ )
                {
                    const auto &range = param.DescriptorTable.pDescriptorRanges[ j ];
                    LOG( INFO ) << "    Range[" << j << "]:";
                    LOG( INFO ) << "      RangeType: " << [ & ]( )
                    {
                        switch ( range.RangeType )
                        {
                        case IRDescriptorRangeTypeSRV:
                            return "SRV";
                        case IRDescriptorRangeTypeUAV:
                            return "UAV";
                        case IRDescriptorRangeTypeCBV:
                            return "CBV";
                        case IRDescriptorRangeTypeSampler:
                            return "Sampler";
                        default:
                            return "Unknown";
                        }
                    }( );
                    LOG( INFO ) << "      NumDescriptors: " << range.NumDescriptors;
                    LOG( INFO ) << "      BaseShaderRegister: " << range.BaseShaderRegister;
                    LOG( INFO ) << "      RegisterSpace: " << range.RegisterSpace;
                    LOG( INFO ) << "      Offset: " << range.OffsetInDescriptorsFromTableStart;
                }
                break;
            }
        case IRRootParameterType32BitConstants:
            {
                LOG( INFO ) << "  32-Bit Constants:";
                LOG( INFO ) << "    ShaderRegister: " << param.Constants.ShaderRegister;
                LOG( INFO ) << "    RegisterSpace: " << param.Constants.RegisterSpace;
                LOG( INFO ) << "    Num32BitValues: " << param.Constants.Num32BitValues;
                break;
            }
        case IRRootParameterTypeCBV:
        case IRRootParameterTypeSRV:
        case IRRootParameterTypeUAV:
            {
                LOG( INFO ) << "  Descriptor:";
                LOG( INFO ) << "    ShaderRegister: " << param.Descriptor.ShaderRegister;
                LOG( INFO ) << "    RegisterSpace: " << param.Descriptor.RegisterSpace;
                break;
            }
        }
    }
}

// For metal, we need to produce a root signature to compile a correct metal lib
// We also keep track of how the root parameter layout looks like so
void ShaderProgram::ProduceMSL( )
{
    const ShaderCompiler &compiler = ShaderCompilerInstance( );

    // We use this vector to make sure register spaces are ordered correctly, the order of the root parameters is also how the Top Level Argument Buffer expects them
    std::vector<RegisterSpaceRange>              localRegisterSpaceRanges;
    std::vector<RegisterSpaceRange>              registerSpaceRanges;
    std::vector<std::unique_ptr<CompiledShader>> dxilShaders;
    std::vector<D3D12_SHADER_INPUT_BIND_DESC>    processedInputs; // Handle duplicate inputs

    ReflectionState state{ };

    for ( int shaderIndex = 0; shaderIndex < m_desc.Shaders.NumElements( ); ++shaderIndex )
    {
        auto       &shader        = m_desc.Shaders.GetElement( shaderIndex );
        CompileDesc compileDesc   = { };
        compileDesc.Path          = shader.Path;
        compileDesc.Defines       = shader.Defines;
        compileDesc.EntryPoint    = shader.EntryPoint;
        compileDesc.Stage         = shader.Stage;
        compileDesc.EnableCaching = compileDesc.EnableCaching;
        compileDesc.TargetIL      = TargetIL::DXIL;
        compileDesc.RayTracing    = shader.RayTracing;
        auto compiledShader       = compiler.CompileHLSL( compileDesc );
        state.ShaderDesc          = &shader;
        state.CompiledShader      = compiledShader.get( );

        auto processResources = [ & ]( D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc, int i )
        {
            if ( IsResourceAlreadyProcessed( processedInputs, shaderInputBindDesc ) )
            {
                return;
            }
            processedInputs.push_back( shaderInputBindDesc );
            bool isLocal = IsBindingLocalTo( shader, shaderInputBindDesc );
            if ( isLocal )
            {
                ContainerUtilities::EnsureSize( localRegisterSpaceRanges, shaderInputBindDesc.Space );
            }
            else
            {
                ContainerUtilities::EnsureSize( registerSpaceRanges, shaderInputBindDesc.Space );
            }

            auto              &registerSpaceRange = isLocal ? localRegisterSpaceRanges[ shaderInputBindDesc.Space ] : registerSpaceRanges[ shaderInputBindDesc.Space ];
            IRShaderVisibility shaderVisibility   = ShaderStageToShaderVisibility( shader.Stage );

            if ( registerSpaceRange.ShaderVisibility != 0 && registerSpaceRange.ShaderVisibility != shaderVisibility )
            {
                registerSpaceRange.ShaderVisibility = IRShaderVisibilityAll;
            }
            else
            {
                registerSpaceRange.ShaderVisibility = shaderVisibility;
            }

            IRDescriptorRangeType descriptorRangeType = ShaderTypeToIRDescriptorType( shaderInputBindDesc.Type );
            if ( ( isLocal || shaderInputBindDesc.Space == DZConfiguration::Instance( ).RootConstantRegisterSpace ) && shaderInputBindDesc.Type == D3D_SIT_CBUFFER )
            {
                IRRootConstants &rootConstants = registerSpaceRange.RootConstants.emplace_back( );
                rootConstants.RegisterSpace    = shaderInputBindDesc.Space;
                rootConstants.ShaderRegister   = shaderInputBindDesc.BindPoint;

                ReflectionDesc rootConstantReflection;
                FillReflectionData( state, rootConstantReflection, i );
                rootConstants.Num32BitValues = rootConstantReflection.NumBytes / 4;
            }
            else if ( shaderInputBindDesc.Space == DZConfiguration::Instance( ).RootLevelBufferRegisterSpace )
            {
                if ( isLocal )
                {
                    LOG( ERROR ) << "Local root level buffers are not supported, use root constants instead.";
                }
                IRRootDescriptor &rootDescriptor = registerSpaceRange.RootArguments.emplace_back( );
                rootDescriptor.RegisterSpace     = shaderInputBindDesc.Space;
                rootDescriptor.ShaderRegister    = shaderInputBindDesc.BindPoint;

                registerSpaceRange.RootArgumentTypes.push_back( IRDescriptorRangeTypeToIRRootParameterType( descriptorRangeType ) );
            }
            else
            {
                IRDescriptorRange1 descriptorRange = CreateDescriptorRange( shaderInputBindDesc );

                switch ( descriptorRange.RangeType )
                {
                case IRDescriptorRangeTypeCBV:
                case IRDescriptorRangeTypeSRV:
                case IRDescriptorRangeTypeUAV:
                    registerSpaceRange.CbvSrvUavRanges.push_back( descriptorRange );
                    break;
                case IRDescriptorRangeTypeSampler:
                    registerSpaceRange.SamplerRanges.push_back( descriptorRange );
                    break;
                }
            }
        };
        IterateBoundResources( compiledShader.get( ), state, processResources );
        dxilShaders.push_back( std::move( compiledShader ) );
    }

    m_metalDescriptorOffsets.resize( registerSpaceRanges.size( ) );
    m_localMetalDescriptorOffsets.resize( localRegisterSpaceRanges.size( ) );

    CompileMslDesc compileMslDesc{ };
    compileMslDesc.RootSignature      = CreateRootSignature( registerSpaceRanges, m_metalDescriptorOffsets, false );
    compileMslDesc.LocalRootSignature = CreateRootSignature( localRegisterSpaceRanges, m_localMetalDescriptorOffsets, true );
    compileMslDesc.RayTracing         = m_desc.RayTracing;

    for ( int shaderIndex = 0; shaderIndex < m_desc.Shaders.NumElements( ); ++shaderIndex )
    {
        auto &shader = m_desc.Shaders.GetElement( shaderIndex );

        CompileDesc compileDesc = { };
        compileDesc.Path        = shader.Path;
        compileDesc.Defines     = shader.Defines;
        compileDesc.EntryPoint  = shader.EntryPoint;
        compileDesc.Stage       = shader.Stage;
        compileDesc.TargetIL    = TargetIL::MSL;
        compileDesc.RayTracing  = shader.RayTracing;

        auto     &compiledShader = dxilShaders[ shaderIndex ];
        IDxcBlob *mslBlob        = compiler.DxilToMsl( compileDesc, compiledShader->Blob, compileMslDesc );
        compiledShader->Blob->Release( );
        compiledShader->Blob       = mslBlob;
        compiledShader->RayTracing = shader.RayTracing;

        m_compiledShaders.push_back( std::move( compiledShader ) );
    }

    IRRootSignatureDestroy( compileMslDesc.LocalRootSignature );
    IRRootSignatureDestroy( compileMslDesc.RootSignature );
}
#endif

const ShaderCompiler &ShaderProgram::ShaderCompilerInstance( ) const
{
    static ShaderCompiler compiler;
    return compiler;
}

InteropArray<CompiledShader *> ShaderProgram::CompiledShaders( ) const
{
    InteropArray<CompiledShader *> compiledShaders;
    for ( auto &shader : m_compiledShaders )
    {
        compiledShaders.AddElement( shader.get( ) );
    }
    return std::move( compiledShaders );
}

Format MaskToFormat( const uint32_t mask )
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

ResourceDescriptor ReflectTypeToRootSignatureType( const D3D_SHADER_INPUT_TYPE type, const D3D_SRV_DIMENSION dimension )
{
    switch ( type )
    {
    case D3D_SIT_RTACCELERATIONSTRUCTURE:
        return ResourceDescriptor::AccelerationStructure;
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
        switch ( dimension )
        {
        case D3D_SRV_DIMENSION_BUFFER:
            return ResourceDescriptor::RWBuffer;
        case D3D_SRV_DIMENSION_TEXTURE1D:
        case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
        case D3D_SRV_DIMENSION_TEXTURE2D:
        case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
        case D3D_SRV_DIMENSION_TEXTURE2DMS:
        case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
        case D3D_SRV_DIMENSION_TEXTURE3D:
        case D3D_SRV_DIMENSION_TEXTURECUBE:
        case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
            return ResourceDescriptor::RWTexture;
        default:
            break;
        }
        return ResourceDescriptor::RWBuffer;
    case D3D_SIT_UAV_FEEDBACKTEXTURE:
        break;
    }
    LOG( ERROR ) << "Unknown resource type";
    return ResourceDescriptor::Texture;
}

ResourceBindingType ShaderProgram::ReflectTypeToBufferBindingType( const D3D_SHADER_INPUT_TYPE type ) const
{
    switch ( type )
    {
    case D3D_SIT_CBUFFER:
        return ResourceBindingType::ConstantBuffer;
    case D3D_SIT_TEXTURE:
        return ResourceBindingType::ShaderResource;
    case D3D_SIT_SAMPLER:
        return ResourceBindingType::Sampler;
    case D3D_SIT_TBUFFER:
    case D3D_SIT_BYTEADDRESS:
    case D3D_SIT_STRUCTURED:
    case D3D_SIT_RTACCELERATIONSTRUCTURE:
        return ResourceBindingType::ShaderResource;
    case D3D_SIT_UAV_APPEND_STRUCTURED:
    case D3D_SIT_UAV_CONSUME_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED:
    case D3D_SIT_UAV_RWTYPED:
    case D3D_SIT_UAV_RWBYTEADDRESS:
    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
        return ResourceBindingType::UnorderedAccess;
    case D3D_SIT_UAV_FEEDBACKTEXTURE:
        break;
    }
    LOG( ERROR ) << "Unknown resource type";
    return ResourceBindingType::ConstantBuffer;
}

ShaderReflectDesc ShaderProgram::Reflect( ) const
{
    ShaderReflectDesc result{ };

    result.LocalRootSignatures.Resize( m_compiledShaders.size( ) );

    InputLayoutDesc   &inputLayout   = result.InputLayout;
    RootSignatureDesc &rootSignature = result.RootSignature;

    // TODO These don't really need to be stored this way
    std::vector<uint32_t> descriptorTableLocations;
    std::vector<uint32_t> localDescriptorTableLocations;

    ReflectionState reflectionState               = { };
    reflectionState.RootSignatureDesc             = &rootSignature;
    reflectionState.InputLayoutDesc               = &inputLayout;
    reflectionState.DescriptorTableLocations      = &descriptorTableLocations;
    reflectionState.LocalDescriptorTableLocations = &localDescriptorTableLocations;

    for ( int shaderIndex = 0; shaderIndex < m_compiledShaders.size( ); ++shaderIndex )
    {
        auto &shader                         = m_compiledShaders[ shaderIndex ];
        reflectionState.CompiledShader       = shader.get( );
        reflectionState.ShaderDesc           = &m_desc.Shaders.GetElement( shaderIndex );
        LocalRootSignatureDesc &recordLayout = result.LocalRootSignatures.GetElement( shaderIndex );
        reflectionState.LocalRootSignature   = &recordLayout;

        IDxcBlob       *reflectionBlob = shader->Reflection;
        const DxcBuffer reflectionBuffer{
            .Ptr      = reflectionBlob->GetBufferPointer( ),
            .Size     = reflectionBlob->GetBufferSize( ),
            .Encoding = 0,
        };

        ID3D12ShaderReflection  *shaderReflection  = nullptr;
        ID3D12LibraryReflection *libraryReflection = nullptr;

        switch ( shader->Stage )
        {
        case ShaderStage::AnyHit:
        case ShaderStage::ClosestHit:
        case ShaderStage::Callable:
        case ShaderStage::Intersection:
        case ShaderStage::Raygen:
        case ShaderStage::Miss:
            DXC_CHECK_RESULT( ShaderCompilerInstance( ).DxcUtils( )->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &libraryReflection ) ) );
            reflectionState.LibraryReflection = libraryReflection;
            ReflectLibrary( reflectionState );
            break;
        case ShaderStage::Vertex:
        default:
            DXC_CHECK_RESULT( ShaderCompilerInstance( ).DxcUtils( )->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &shaderReflection ) ) );
            reflectionState.ShaderReflection = shaderReflection;
            ReflectShader( reflectionState );
            break;
        }

#ifdef BUILD_METAL
        IRShaderReflectionDestroy( reflectionState.IRReflection );
#endif
        if ( shaderReflection )
        {
            shaderReflection->Release( );
        }
        if ( libraryReflection )
        {
            libraryReflection->Release( );
        }
    }

#ifndef NDEBUG
    DumpReflectionInfo( result );
#endif
    return result;
}

ShaderProgramDesc ShaderProgram::Desc( ) const
{
    return m_desc;
}

void ShaderProgram::ReflectShader( ReflectionState &state ) const
{
    ID3D12ShaderReflection *shaderReflection = state.ShaderReflection;

    D3D12_SHADER_DESC shaderDesc{ };
    DXC_CHECK_RESULT( shaderReflection->GetDesc( &shaderDesc ) );

    if ( state.ShaderDesc->Stage == ShaderStage::Vertex )
    {
        InitInputLayout( shaderReflection, *state.InputLayoutDesc, shaderDesc );
    }

    DXC_CHECK_RESULT( shaderReflection->GetDesc( &shaderDesc ) );
#ifdef BUILD_METAL
    IRObject           *ir           = dynamic_cast<MetalDxcBlob_Impl *>( state.CompiledShader->Blob )->IrObject;
    IRShaderReflection *irReflection = IRShaderReflectionCreate( );
    IRObjectGetReflection( ir, ShaderCompiler::ConvertIrShaderStage( state.CompiledShader->Stage ), irReflection );

    std::vector<IRResourceLocation> resources( IRShaderReflectionGetResourceCount( irReflection ) );
    IRShaderReflectionGetResourceLocations( irReflection, resources.data( ) );
    state.IRReflection = irReflection;
#endif

    for ( const uint32_t i : std::views::iota( 0u, shaderDesc.BoundResources ) )
    {
        D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };
        DXC_CHECK_RESULT( shaderReflection->GetResourceBindingDesc( i, &shaderInputBindDesc ) );
        ProcessBoundResource( state, shaderInputBindDesc, i );
    }
}

void ShaderProgram::ReflectLibrary( ReflectionState &state ) const
{
    // It is common to include the same shader in multiple libraries in raytracing shaders.
    // TODO Check if this is a good way to go about it
    ID3D12LibraryReflection *libraryReflection = state.LibraryReflection;

    D3D12_LIBRARY_DESC libraryDesc{ };
    DXC_CHECK_RESULT( libraryReflection->GetDesc( &libraryDesc ) );

    for ( const uint32_t i : std::views::iota( 0u, libraryDesc.FunctionCount ) )
    {
        ID3D12FunctionReflection *functionReflection = libraryReflection->GetFunctionByIndex( i );
        D3D12_FUNCTION_DESC       functionDesc{ };
        DXC_CHECK_RESULT( functionReflection->GetDesc( &functionDesc ) );

        { // Only process the matching function
            // Check if the name is not mangled to start with
            std::string_view mangledName( functionDesc.Name );
            const bool       isMangled = mangledName.starts_with( "\01?" ) || mangledName.starts_with( "\x1?" );
            if ( !isMangled && functionDesc.Name != state.CompiledShader->EntryPoint.Get( ) )
            {
                continue;
            }
            if ( const size_t nameEnd = mangledName.find( '@' ); nameEnd != std::string_view::npos && isMangled )
            {
                if ( const std::string_view demangledName = mangledName.substr( 2, nameEnd - 2 ); demangledName != state.CompiledShader->EntryPoint.Get( ) )
                {
                    continue;
                }
            }
        }
        state.FunctionReflection = functionReflection;
        for ( const uint32_t j : std::views::iota( 0u, functionDesc.BoundResources ) )
        {
            D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };
            DXC_CHECK_RESULT( functionReflection->GetResourceBindingDesc( j, &shaderInputBindDesc ) );
            ProcessBoundResource( state, shaderInputBindDesc, j );
        }
    }
}

void ShaderProgram::ProcessBoundResource( ReflectionState &state, D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc, const int resourceIndex ) const
{
    if ( UpdateBoundResourceStage( state, shaderInputBindDesc ) )
    {
        return;
    }
    bool                      isLocal     = IsBindingLocalTo( *state.ShaderDesc, shaderInputBindDesc );
    const ResourceBindingType bindingType = ReflectTypeToBufferBindingType( shaderInputBindDesc.Type );
    // Root constants are reserved for a specific register space
    // PS: Constant buffers in local root signatures are already handled as root constants
    if ( shaderInputBindDesc.Space == DZConfiguration::Instance( ).RootConstantRegisterSpace && !isLocal )
    {
        ReflectionDesc rootConstantReflection;
        FillReflectionData( state, rootConstantReflection, resourceIndex );
        if ( rootConstantReflection.Type != ReflectionBindingType::Pointer && rootConstantReflection.Type != ReflectionBindingType::Struct )
        {
            LOG( FATAL ) << "Root constant reflection type mismatch. RegisterSpace [" << shaderInputBindDesc.Space
                         << "] is reserved for root constants. Which cannot be samplers or textures.";
        }
        RootConstantResourceBindingDesc &rootConstantBinding = state.RootSignatureDesc->RootConstants.EmplaceElement( );
        rootConstantBinding.Name                             = shaderInputBindDesc.Name;
        rootConstantBinding.Binding                          = shaderInputBindDesc.BindPoint;
        rootConstantBinding.Stages.AddElement( state.ShaderDesc->Stage );
        rootConstantBinding.NumBytes   = rootConstantReflection.NumBytes;
        rootConstantBinding.Reflection = rootConstantReflection;
        return;
    }

    // If this register space is configured to be a LocalRootSignature, then populate the corresponding Bindings.
    InteropArray<ResourceBindingDesc> *resourceBindings = &state.RootSignatureDesc->ResourceBindings;
    if ( isLocal )
    {
        resourceBindings = &state.LocalRootSignature->ResourceBindings;
    }

    ResourceBindingDesc &resourceBindingDesc = resourceBindings->EmplaceElement( );
    resourceBindingDesc.Name                 = shaderInputBindDesc.Name;
    resourceBindingDesc.Binding              = shaderInputBindDesc.BindPoint;
    resourceBindingDesc.RegisterSpace        = shaderInputBindDesc.Space;
    resourceBindingDesc.ArraySize            = shaderInputBindDesc.BindCount;
    resourceBindingDesc.BindingType          = bindingType;
    resourceBindingDesc.Descriptor           = ReflectTypeToRootSignatureType( shaderInputBindDesc.Type, shaderInputBindDesc.Dimension );
    resourceBindingDesc.Stages.AddElement( state.ShaderDesc->Stage );
    FillReflectionData( state, resourceBindingDesc.Reflection, resourceIndex );
#ifdef BUILD_METAL
    /*
     * This reflection information is unfortunately required to hint the MetalResourceBindGroup where a binding(i.e. b0, space0) lies in the top level argument buffer.
     */
    auto &metalDescriptorOffsets = isLocal ? m_localMetalDescriptorOffsets : m_metalDescriptorOffsets;

    if ( isLocal && resourceBindingDesc.BindingType == ResourceBindingType::ConstantBuffer )
    {
        resourceBindingDesc.Reflection.LocalCbvOffset = state.LocalCbvOffset;
        state.LocalCbvOffset += resourceBindingDesc.Reflection.NumBytes;
        return;
    }
    if ( resourceBindingDesc.RegisterSpace == DZConfiguration::Instance( ).RootLevelBufferRegisterSpace )
    {
        uint32_t hash = Utilities::HashInts( BindingTypeToIRRootParameterType( resourceBindingDesc.BindingType ), shaderInputBindDesc.Space, shaderInputBindDesc.BindPoint );
        resourceBindingDesc.Reflection.TLABOffset = metalDescriptorOffsets[ shaderInputBindDesc.Space ].UniqueTLABIndex.at( hash );
        return;
    }
    // Hint metal resource bind group where descriptor table lies in the top level argument buffer
    switch ( resourceBindingDesc.Reflection.Type )
    {
    case ReflectionBindingType::Pointer:
    case ReflectionBindingType::Struct:
    case ReflectionBindingType::Texture:
        resourceBindingDesc.Reflection.TLABOffset = metalDescriptorOffsets[ shaderInputBindDesc.Space ].CbvSrvUavOffset;
        break;
    case ReflectionBindingType::SamplerDesc:
        resourceBindingDesc.Reflection.TLABOffset = metalDescriptorOffsets[ shaderInputBindDesc.Space ].SamplerOffset;
        break;
    }

    std::vector<uint32_t> *tableIndexes = isLocal ? state.LocalDescriptorTableLocations : state.DescriptorTableLocations;
    ContainerUtilities::EnsureSize( ( *tableIndexes ), resourceBindingDesc.Reflection.TLABOffset );
    uint32_t &locationHint                              = ( *tableIndexes )[ resourceBindingDesc.Reflection.TLABOffset ];
    resourceBindingDesc.Reflection.DescriptorTableIndex = locationHint++;
#endif
}

bool ShaderProgram::IsBindingLocalTo( const ShaderDesc &shaderDesc, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc ) const
{
    const auto &bindings = shaderDesc.RayTracing.LocalBindings;
    for ( int i = 0; i < bindings.NumElements( ); ++i )
    {
        if ( auto &element = bindings.GetElement( i ); element.Binding == shaderInputBindDesc.BindPoint && element.RegisterSpace == shaderInputBindDesc.Space &&
                                                       element.Type == ReflectTypeToBufferBindingType( shaderInputBindDesc.Type ) )
        {
            return true;
        }
    }
    return false;
}

bool ShaderProgram::UpdateBoundResourceStage( const ReflectionState &state, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc ) const
{
    const ResourceBindingType bindingType = ReflectTypeToBufferBindingType( shaderInputBindDesc.Type );
    // Check if Resource is already bound, if so add the stage to the existing binding and continue
    bool found = false;

    // Check if it is a root constant:
    if ( shaderInputBindDesc.Space == DZConfiguration::Instance( ).RootConstantRegisterSpace )
    {
        for ( int bindingIndex = 0; bindingIndex < state.RootSignatureDesc->RootConstants.NumElements( ); ++bindingIndex )
        {
            if ( auto &boundBinding = state.RootSignatureDesc->RootConstants.GetElement( bindingIndex ); boundBinding.Binding == shaderInputBindDesc.BindPoint )
            {
                found = true;
                boundBinding.Stages.AddElement( state.ShaderDesc->Stage );
                break;
            }
        }
        return found;
    }

    for ( int bindingIndex = 0; bindingIndex < state.RootSignatureDesc->ResourceBindings.NumElements( ); ++bindingIndex )
    {
        auto &boundBinding  = state.RootSignatureDesc->ResourceBindings.GetElement( bindingIndex );
        bool  isSameBinding = boundBinding.RegisterSpace == shaderInputBindDesc.Space;
        isSameBinding       = isSameBinding && boundBinding.Binding == shaderInputBindDesc.BindPoint;
        isSameBinding       = isSameBinding && boundBinding.BindingType == bindingType;
        isSameBinding       = isSameBinding && strcmp( boundBinding.Name.Get( ), shaderInputBindDesc.Name ) == 0;
        if ( isSameBinding )
        {
            found            = true;
            bool stageExists = false;
            for ( int stageIndex = 0; stageIndex < boundBinding.Stages.NumElements( ); ++stageIndex )
            {
                if ( boundBinding.Stages.GetElement( stageIndex ) == state.ShaderDesc->Stage )
                {
                    stageExists = true;
                    break;
                }
            }
            if ( !stageExists )
            {
                boundBinding.Stages.AddElement( state.ShaderDesc->Stage );
            }
        }
    }
    return found;
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

void ShaderProgram::FillTypeInfo( ID3D12ShaderReflectionType *reflType, InteropArray<ReflectionResourceField> &fields, const uint32_t parentIndex, const uint32_t level ) const
{
    D3D12_SHADER_TYPE_DESC typeDesc;
    DXC_CHECK_RESULT( reflType->GetDesc( &typeDesc ) );

    if ( typeDesc.Members > 0 )
    {
        for ( UINT i = 0; i < typeDesc.Members; i++ )
        {
            ID3D12ShaderReflectionType *memberType = reflType->GetMemberTypeByIndex( i );
            D3D12_SHADER_TYPE_DESC      memberTypeDesc;
            DXC_CHECK_RESULT( memberType->GetDesc( &memberTypeDesc ) );

            const uint32_t           currentIndex = fields.NumElements( );
            ReflectionResourceField &memberField  = fields.EmplaceElement( );
            memberField.Name                      = reflType->GetMemberTypeName( i );
            memberField.Type                      = DXCVariableTypeToReflectionType( memberTypeDesc.Type );
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

void ShaderProgram::FillReflectionData( const ReflectionState &state, ReflectionDesc &reflectionDesc, const int resourceIndex ) const
{
    D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };
    if ( state.ShaderReflection )
    {
        DXC_CHECK_RESULT( state.ShaderReflection->GetResourceBindingDesc( resourceIndex, &shaderInputBindDesc ) );
    }
    else if ( state.FunctionReflection )
    {
        DXC_CHECK_RESULT( state.FunctionReflection->GetResourceBindingDesc( resourceIndex, &shaderInputBindDesc ) );
    }
    else
    {
        LOG( FATAL ) << "No shader reflection object found, make sure no compilation errors occurred.";
    }

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

    DZ_RETURN_IF( reflectionDesc.Type != ReflectionBindingType::Struct );

    ID3D12ShaderReflectionConstantBuffer *constantBuffer = nullptr;
    if ( state.ShaderReflection )
    {
        constantBuffer = state.ShaderReflection->GetConstantBufferByIndex( resourceIndex );
    }
    else if ( state.FunctionReflection )
    {
        constantBuffer = state.FunctionReflection->GetConstantBufferByIndex( resourceIndex );
    }

    D3D12_SHADER_BUFFER_DESC bufferDesc;
    DXC_CHECK_RESULT( constantBuffer->GetDesc( &bufferDesc ) );
    reflectionDesc.NumBytes = bufferDesc.Size;

    for ( const uint32_t i : std::views::iota( 0u, bufferDesc.Variables ) )
    {
        ID3D12ShaderReflectionVariable *variable = constantBuffer->GetVariableByIndex( i );
        D3D12_SHADER_VARIABLE_DESC      variableDesc;
        DXC_CHECK_RESULT( variable->GetDesc( &variableDesc ) );

        ID3D12ShaderReflectionType *reflectionType = variable->GetType( );
        D3D12_SHADER_TYPE_DESC      typeDesc;
        DXC_CHECK_RESULT( reflectionType->GetDesc( &typeDesc ) );

        const uint32_t           currentIndex = reflectionDesc.Fields.NumElements( );
        ReflectionResourceField &field        = reflectionDesc.Fields.EmplaceElement( );
        field.Name                            = variableDesc.Name;
        field.Type                            = DXCVariableTypeToReflectionType( typeDesc.Type );
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

void ShaderProgram::InitInputLayout( ID3D12ShaderReflection *shaderReflection, InputLayoutDesc &inputLayoutDesc, const D3D12_SHADER_DESC &shaderDesc ) const
{
    constexpr D3D_NAME providedSemantics[ 8 ] = {
        D3D_NAME_VERTEX_ID, D3D_NAME_INSTANCE_ID,   D3D_NAME_PRIMITIVE_ID, D3D_NAME_RENDER_TARGET_ARRAY_INDEX, D3D_NAME_VIEWPORT_ARRAY_INDEX,
        D3D_NAME_VERTEX_ID, D3D_NAME_CLIP_DISTANCE, D3D_NAME_CULL_DISTANCE
    };

    std::vector<InputLayoutElementDesc> inputElements;
    for ( const uint32_t parameterIndex : std::views::iota( 0u, shaderDesc.InputParameters ) )
    {
        D3D12_SIGNATURE_PARAMETER_DESC signatureParameterDesc{ };
        DXC_CHECK_RESULT( shaderReflection->GetInputParameterDesc( parameterIndex, &signatureParameterDesc ) );

        bool isSemanticProvided = false;
        for ( const uint32_t i : std::views::iota( 0u, 8u ) )
        {
            if ( signatureParameterDesc.SystemValueType == providedSemantics[ i ] )
            {
                isSemanticProvided = true;
                break;
            }
        }

        if ( isSemanticProvided )
        {
            continue;
        }

        InputLayoutElementDesc &inputElementDesc = inputElements.emplace_back( );
        inputElementDesc.Semantic                = SemanticFromString( signatureParameterDesc.SemanticName );
        inputElementDesc.SemanticIndex           = signatureParameterDesc.SemanticIndex;
        inputElementDesc.Format                  = MaskToFormat( signatureParameterDesc.Mask );
    }

    if ( !inputElements.empty( ) )
    {
        auto &inputElementsArray = inputLayoutDesc.InputGroups.EmplaceElement( );
        for ( int i = 0; i < inputElements.size( ); ++i )
        {
            inputElementsArray.Elements.AddElement( inputElements[ i ] );
        }
    }
}

ShaderProgram::~ShaderProgram( )
{
    for ( const auto &shader : m_compiledShaders )
    {
        if ( shader->Blob )
        {
            shader->Blob->Release( );
        }
        if ( shader->Reflection )
        {
            shader->Reflection->Release( );
        }
    }
}

// Debug Code:
void ShaderProgram::DumpReflectionInfo( const ShaderReflectDesc &reflection ) const
{
    std::stringstream output;

    output << "\n\n=== Global Root Signature ===\n";
    DumpRootSignature( output, reflection.RootSignature );

    output << "\n=== Local Root Signatures ===\n";
    for ( int i = 0; i < reflection.LocalRootSignatures.NumElements( ); ++i )
    {
        if ( auto localRootSignatureDesc = reflection.LocalRootSignatures.GetElement( i ); localRootSignatureDesc.ResourceBindings.NumElements( ) > 0 )
        {
            output << "\nLocal Root Signature " << i << "\n";
            DumpResourceBindings( output, localRootSignatureDesc.ResourceBindings );
        }
    }

    output << "\n\n";
    LOG( INFO ) << output.str( );
}

void ShaderProgram::DumpResourceBindings( std::stringstream &output, const InteropArray<ResourceBindingDesc> &resourceBindings ) const
{
    if ( resourceBindings.NumElements( ) == 0 )
    {
        return;
    }

    output << "\n=== Resource Bindings ===\n";
    output << std::string( 100, '=' ) << '\n';
    output << std::setw( 40 ) << std::left << "Name" << std::setw( 15 ) << "Type" << std::setw( 10 ) << "Space" << std::setw( 10 ) << "Binding" << std::setw( 10 ) << "Size"
           << "Stages\n";
    output << std::string( 100, '-' ) << '\n';

    for ( int i = 0; i < resourceBindings.NumElements( ); ++i )
    {
        const auto &binding = resourceBindings.GetElement( i );

        output << std::setw( 40 ) << std::left << binding.Name.Get( ) << std::setw( 15 ) << GetBindingTypeString( binding.BindingType ) << std::setw( 10 ) << binding.RegisterSpace
               << std::setw( 10 ) << binding.Binding << std::setw( 10 ) << binding.Reflection.NumBytes << GetStagesString( binding.Stages ) << '\n';

        if ( binding.Reflection.Fields.NumElements( ) > 0 )
        {
            output << std::string( 100, '-' ) << '\n';
            output << "  Fields for " << binding.Name.Get( ) << ":\n";
            output << "  " << std::string( 90, '-' ) << '\n';
            output << "  " << std::setw( 38 ) << std::left << "Field Name" << std::setw( 15 ) << "Type" << std::setw( 12 ) << "Columns"
                   << "Rows\n";
            output << "  " << std::string( 90, '-' ) << '\n';
            DumpStructFields( output, binding.Reflection.Fields );
            output << std::string( 100, '=' ) << '\n';
        }
    }
}

void ShaderProgram::DumpRootSignature( std::stringstream &output, const RootSignatureDesc &sig ) const
{
    DumpResourceBindings( output, sig.ResourceBindings );

    output << "\n--- Root Constants --- \n";
    for ( int i = 0; i < sig.RootConstants.NumElements( ); ++i )
    {
        const auto &constant = sig.RootConstants.GetElement( i );
        output << std::setw( 40 ) << constant.Name.Get( ) << std::setw( 10 ) << constant.Binding << std::setw( 10 ) << constant.NumBytes << " "
               << GetStagesString( constant.Stages ) << "\n";
    }
}

void ShaderProgram::DumpStructFields( std::stringstream &output, const InteropArray<ReflectionResourceField> &fields ) const
{
    for ( int i = 0; i < fields.NumElements( ); ++i )
    {
        const auto &field = fields.GetElement( i );

        std::string indent( 2 * field.Level, ' ' );
        output << indent << std::setw( 38 - indent.length( ) ) << std::left << field.Name.Get( ) << std::setw( 15 ) << GetFieldTypeString( field.Type ) << std::setw( 12 )
               << field.NumColumns << std::setw( 10 ) << field.NumRows << "offset:" << std::setw( 6 ) << field.Offset;

        if ( field.Elements > 0 )
        {
            output << " [" << field.Elements << "]";
        }
        if ( field.ParentIndex != UINT32_MAX )
        {
            output << " (parent: " << field.ParentIndex << ")";
        }
        output << '\n';
    }
}

std::string ShaderProgram::GetFieldTypeString( const ReflectionFieldType type ) const
{
    switch ( type )
    {
    case ReflectionFieldType::Undefined:
        return "Undefined";
    case ReflectionFieldType::Void:
        return "Void";
    case ReflectionFieldType::Bool:
        return "Bool";
    case ReflectionFieldType::Int:
        return "Int";
    case ReflectionFieldType::Float:
        return "Float";
    case ReflectionFieldType::String:
        return "String";
    case ReflectionFieldType::Texture:
        return "Texture";
    case ReflectionFieldType::Texture1D:
        return "Texture1D";
    case ReflectionFieldType::Texture2D:
        return "Texture2D";
    case ReflectionFieldType::Texture3D:
        return "Texture3D";
    case ReflectionFieldType::TextureCube:
        return "TextureCube";
    case ReflectionFieldType::Sampler:
        return "Sampler";
    case ReflectionFieldType::Sampler1d:
        return "Sampler1d";
    case ReflectionFieldType::Sampler2d:
        return "Sampler2d";
    case ReflectionFieldType::Sampler3d:
        return "Sampler3d";
    case ReflectionFieldType::SamplerCube:
        return "SamplerCube";
    case ReflectionFieldType::PixelFragment:
        return "PixelFragment";
    case ReflectionFieldType::VertexFragment:
        return "VertexFragment";
    case ReflectionFieldType::Uint:
        return "Uint";
    case ReflectionFieldType::Uint8:
        return "Uint8";
    case ReflectionFieldType::DepthStencil:
        return "DepthStencil";
    case ReflectionFieldType::Blend:
        return "Blend";
    case ReflectionFieldType::Buffer:
        return "Buffer";
    case ReflectionFieldType::CBuffer:
        return "CBuffer";
    case ReflectionFieldType::TBuffer:
        return "TBuffer";
    case ReflectionFieldType::Texture1DArray:
        return "Texture1DArray";
    case ReflectionFieldType::Texture2DArray:
        return "Texture2DArray";
    case ReflectionFieldType::RenderTargetView:
        return "RenderTargetView";
    case ReflectionFieldType::DepthStencilView:
        return "DepthStencilView";
    case ReflectionFieldType::Texture2Dms:
        return "Texture2Dms";
    case ReflectionFieldType::Texture2DmsArray:
        return "Texture2DmsArray";
    case ReflectionFieldType::TextureCubeArray:
        return "TextureCubeArray";
    case ReflectionFieldType::InterfacePointer:
        return "InterfacePointer";
    case ReflectionFieldType::Double:
        return "Double";
    case ReflectionFieldType::RWTexture1D:
        return "RWTexture1D";
    case ReflectionFieldType::RWTexture1DArray:
        return "RWTexture1DArray";
    case ReflectionFieldType::RWTexture2D:
        return "RWTexture2D";
    case ReflectionFieldType::RWTexture2DArray:
        return "RWTexture2DArray";
    case ReflectionFieldType::RWTexture3D:
        return "RWTexture3D";
    case ReflectionFieldType::RWBuffer:
        return "RWBuffer";
    case ReflectionFieldType::ByteAddressBuffer:
        return "ByteAddressBuffer";
    case ReflectionFieldType::RWByteAddressBuffer:
        return "RWByteAddressBuffer";
    case ReflectionFieldType::StructuredBuffer:
        return "StructuredBuffer";
    case ReflectionFieldType::RWStructuredBuffer:
        return "RWStructuredBuffer";
    case ReflectionFieldType::AppendStructuredBuffer:
        return "AppendStructuredBuffer";
    case ReflectionFieldType::ConsumeStructuredBuffer:
        return "ConsumeStructuredBuffer";
    case ReflectionFieldType::Min8Float:
        return "Min8Float";
    case ReflectionFieldType::Min10Float:
        return "Min10Float";
    case ReflectionFieldType::Min16Float:
        return "Min16Float";
    case ReflectionFieldType::Min12Int:
        return "Min12Int";
    case ReflectionFieldType::Min16Int:
        return "Min16Int";
    case ReflectionFieldType::Min16UInt:
        return "Min16UInt";
    case ReflectionFieldType::Int16:
        return "Int16";
    case ReflectionFieldType::UInt16:
        return "UInt16";
    case ReflectionFieldType::Float16:
        return "Float16";
    case ReflectionFieldType::Int64:
        return "Int64";
    case ReflectionFieldType::UInt64:
        return "UInt64";
    case ReflectionFieldType::PixelShader:
        return "PixelShader";
    case ReflectionFieldType::VertexShader:
        return "VertexShader";
    case ReflectionFieldType::GeometryShader:
        return "GeometryShader";
    case ReflectionFieldType::HullShader:
        return "HullShader";
    case ReflectionFieldType::DomainShader:
        return "DomainShader";
    case ReflectionFieldType::ComputeShader:
        return "ComputeShader";
    }

    return "";
}

std::string ShaderProgram::GetBindingTypeString( const ResourceBindingType type ) const
{
    switch ( type )
    {
    case ResourceBindingType::ConstantBuffer:
        return "CBV";
    case ResourceBindingType::ShaderResource:
        return "SRV";
    case ResourceBindingType::UnorderedAccess:
        return "UAV";
    case ResourceBindingType::Sampler:
        return "Sampler";
    default:
        return "Unknown";
    }
}

std::string ShaderProgram::GetStagesString( const InteropArray<ShaderStage> &stages ) const
{
    std::string result;
    for ( int i = 0; i < stages.NumElements( ); ++i )
    {
        if ( i > 0 )
            result += "|";
        switch ( stages.GetElement( i ) )
        {
        case ShaderStage::Vertex:
            result += "Vertex";
            break;
        case ShaderStage::Pixel:
            result += "Pixel";
            break;
        case ShaderStage::Compute:
            result += "Compute";
            break;
        case ShaderStage::Raygen:
            result += "Raygen";
            break;
        case ShaderStage::ClosestHit:
            result += "ClosestHit";
            break;
        case ShaderStage::Geometry:
            result += "Geometry";
            break;
        case ShaderStage::Hull:
            result += "Hull";
            break;
        case ShaderStage::Domain:
            result += "Domain";
            break;
        case ShaderStage::AllGraphics:
            result += "AllGraphics";
            break;
        case ShaderStage::All:
            result += "All";
            break;
        case ShaderStage::AnyHit:
            result += "AnyHit";
            break;
        case ShaderStage::Miss:
            result += "Miss";
            break;
        case ShaderStage::Intersection:
            result += "Intersection";
            break;
        case ShaderStage::Callable:
            result += "Callable";
            break;
        case ShaderStage::Task:
            result += "Task";
            break;
        case ShaderStage::Mesh:
            result += "Mesh";
            break;
        }
    }
    return result;
}
//
