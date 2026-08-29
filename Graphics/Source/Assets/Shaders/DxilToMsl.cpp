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

#include "DenOfIzGraphicsInternal/Assets/Shaders/DxilToMsl.h"
#include <functional>
#include <ranges>
#include <unordered_map>
#include "DenOfIzGraphics/Assets/Shaders/ShaderCompiler.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/DxcEnumConverter.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/ReflectionDebugOutput.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/ShaderReflectionHelper.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/IRootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/ILocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Utilities/ContainerUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/StringStore.h"

#ifdef _WIN32
#include <wrl/client.h>
#include "DenOfIzGraphics/Utilities/Common_Windows.h"
#else
#define __EMULATE_UUID
#include "WinAdapter.h"
#endif

#include "dxcapi.h"
#include "metal_irconverter/metal_irconverter.h"

using namespace DenOfIz;

struct MetalDescriptorOffsets
{
    // -1 is used for debugging purposes to show that no descriptor table exists in this root signature of that type
    int                                    CbvSrvUavOffset      = -1;
    int                                    SamplerOffset        = -1;
    int                                    LocalCbvSrvUavOffset = -1;
    int                                    LocalSamplerOffset   = -1;
    std::unordered_map<uint32_t, uint32_t> UniqueTLABIndex{ };
};

// This is used to order the root parameters in the root signature as metal top level argument buffer expects them in the same order
// Binding goes from 0 till max register space.
struct RegisterSpaceRange
{
    std::vector<IRRootConstants>     RootConstants;
    std::vector<IRRootDescriptor>    RootArguments;
    std::vector<IRRootParameterType> RootArgumentTypes;
    std::vector<IRDescriptorRange1>  CbvSrvUavRanges;
    std::vector<IRDescriptorRange1>  SamplerRanges;
    IRShaderVisibility               ShaderVisibility;
    bool                             HasBindlessResources = false;
};

typedef const std::function<void( D3D12_SHADER_INPUT_BIND_DESC &, int )> ReflectionCallback;

struct CompileMslDesc
{
    IRRootSignature             *RootSignature;
    IRRootSignature             *LocalRootSignature;
    DenOfIz_ShaderRayTracingDesc RayTracing;
};

class DxilToMsl::Impl
{
public:
    DenOfIz_ShaderCompiler m_compiler;

    ID3D12ShaderReflection   *m_shaderReflection   = nullptr;
    ID3D12LibraryReflection  *m_libraryReflection  = nullptr;
    ID3D12FunctionReflection *m_functionReflection = nullptr;

    IRCompiler *m_irCompiler = nullptr;

    Impl( );
    ~Impl( );
    void Convert( const DxilToMslDesc &desc );

private:
    [[nodiscard]] DenOfIz_ByteArray Compile( const DenOfIz_CompileDesc &compileDesc, const DenOfIz_ByteArray &dxil, const CompileMslDesc &compileMslDesc,
                                             const DenOfIz_RayTracingShaderDesc &rayTracingShaderDesc ) const;

    IRRootSignature *CreateRootSignature( std::vector<RegisterSpaceRange> &registerSpaceRanges, bool isLocal ) const;
    void             IterateBoundResources( DenOfIz_CompiledShaderStage *shader, ReflectionCallback &callback );
    void             DxcCheckResult( HRESULT hr ) const;
};

void PutRootParameterDescriptorTable( std::vector<IRRootParameter1> &rootParameters, const IRShaderVisibility visibility, std::vector<IRDescriptorRange1> &ranges )
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

// Todo perhaps share this with main reflection code
void DxilToMsl::Impl::IterateBoundResources( DenOfIz_CompiledShaderStage *shader, ReflectionCallback &callback )
{
    IDxcUtils *dxcUtils = nullptr;
    HRESULT    hr       = DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( &dxcUtils ) );
    if ( FAILED( hr ) )
    {
        spdlog::error( "Failed to create DxcUtils for reflection" );
        return;
    }

    auto            reflectionBlob = shader->Reflection;
    const DxcBuffer reflectionBuffer{
        .Ptr      = reflectionBlob.Elements,
        .Size     = reflectionBlob.NumElements,
        .Encoding = 0,
    };

    const bool isRayTracingStage = shader->Stage & ( DENOFIZ_SHADER_STAGE_ANY_HIT_BIT | DENOFIZ_SHADER_STAGE_INTERSECTION_BIT | DENOFIZ_SHADER_STAGE_CLOSEST_HIT_BIT |
                                                     DENOFIZ_SHADER_STAGE_RAY_GEN_BIT | DENOFIZ_SHADER_STAGE_MISS_BIT );
    if ( isRayTracingStage )
    {
        DxcCheckResult( dxcUtils->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &m_libraryReflection ) ) );
        D3D12_LIBRARY_DESC libraryDesc{ };
        DxcCheckResult( m_libraryReflection->GetDesc( &libraryDesc ) );

        for ( const uint32_t i : std::views::iota( 0u, libraryDesc.FunctionCount ) )
        {
            m_functionReflection = m_libraryReflection->GetFunctionByIndex( i );
            D3D12_FUNCTION_DESC functionDesc{ };
            DxcCheckResult( m_functionReflection->GetDesc( &functionDesc ) );
            for ( const uint32_t j : std::views::iota( 0u, functionDesc.BoundResources ) )
            {
                D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };
                DxcCheckResult( m_functionReflection->GetResourceBindingDesc( j, &shaderInputBindDesc ) );

                callback( shaderInputBindDesc, j );
            }
        }
    }
    else
    {
        DxcCheckResult( dxcUtils->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &m_shaderReflection ) ) );

        D3D12_SHADER_DESC shaderDesc{ };
        DxcCheckResult( m_shaderReflection->GetDesc( &shaderDesc ) );

        for ( const uint32_t i : std::views::iota( 0u, shaderDesc.BoundResources ) )
        {
            D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };
            DxcCheckResult( m_shaderReflection->GetResourceBindingDesc( i, &shaderInputBindDesc ) );

            callback( shaderInputBindDesc, i );
        }
    }

    if ( dxcUtils )
    {
        dxcUtils->Release( );
    }
}

bool IsResourceAlreadyProcessed( const std::vector<D3D12_SHADER_INPUT_BIND_DESC> &processedInputs, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc )
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

IRDescriptorRange1 CreateDescriptorRange( const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc, bool isBindless = false, uint32_t maxArraySize = 0 )
{
    IRDescriptorRange1 descriptorRange                = { };
    descriptorRange.BaseShaderRegister                = shaderInputBindDesc.BindPoint;
    descriptorRange.NumDescriptors                    = shaderInputBindDesc.BindCount;
    descriptorRange.RegisterSpace                     = shaderInputBindDesc.Space;
    descriptorRange.OffsetInDescriptorsFromTableStart = IRDescriptorRangeOffsetAppend;
    descriptorRange.RangeType                         = DxcEnumConverter::ShaderTypeToIRDescriptorType( shaderInputBindDesc.Type );

    if ( isBindless )
    {
        descriptorRange.Flags          = IRDescriptorRangeFlagDescriptorsVolatile;
        descriptorRange.NumDescriptors = maxArraySize;
    }
    else
    {
        descriptorRange.Flags = IRDescriptorRangeFlagNone;
    }

    return descriptorRange;
}

IRRootSignature *DxilToMsl::Impl::CreateRootSignature( std::vector<RegisterSpaceRange> &registerSpaceRanges, const bool isLocal ) const
{
    // MetalBindGroupLayout encodes descriptor tables in CBV -> SRV -> UAV order, sorted by register. Descriptor range order in the root
    // signature dictates the table layout the converted shader reads (Metal Shader Converter 3.x lays out append ranges strictly in
    // declaration order), so the ranges must be declared in that same order with explicit offsets.
    constexpr auto rangeTypeOrder = []( const IRDescriptorRangeType type )
    {
        switch ( type )
        {
        case IRDescriptorRangeTypeCBV:
            return 0;
        case IRDescriptorRangeTypeSRV:
            return 1;
        case IRDescriptorRangeTypeUAV:
            return 2;
        case IRDescriptorRangeTypeSampler:
            return 3;
        }
        return 4;
    };
    for ( auto &registerSpaceRange : registerSpaceRanges )
    {
        std::ranges::sort( registerSpaceRange.CbvSrvUavRanges,
                           [ & ]( const IRDescriptorRange1 &a, const IRDescriptorRange1 &b )
                           {
                               if ( a.RangeType != b.RangeType )
                               {
                                   return rangeTypeOrder( a.RangeType ) < rangeTypeOrder( b.RangeType );
                               }
                               return a.BaseShaderRegister < b.BaseShaderRegister;
                           } );
        std::ranges::sort( registerSpaceRange.SamplerRanges,
                           []( const IRDescriptorRange1 &a, const IRDescriptorRange1 &b ) { return a.BaseShaderRegister < b.BaseShaderRegister; } );

        uint32_t tableOffset = 0;
        for ( auto &range : registerSpaceRange.CbvSrvUavRanges )
        {
            range.OffsetInDescriptorsFromTableStart = tableOffset;
            tableOffset += range.NumDescriptors;
        }
        tableOffset = 0;
        for ( auto &range : registerSpaceRange.SamplerRanges )
        {
            range.OffsetInDescriptorsFromTableStart = tableOffset;
            tableOffset += range.NumDescriptors;
        }
    }

    std::vector<IRRootParameter1> rootParameters;
    int                           registerSpace = 0;
    for ( auto &registerSpaceRange : registerSpaceRanges )
    {
        int rootConstantIndex = 0;
        for ( const auto &rootConstant : registerSpaceRange.RootConstants )
        {
            IRRootParameter1 &rootParameter        = rootParameters.emplace_back( );
            rootParameter.ParameterType            = IRRootParameterType32BitConstants;
            rootParameter.ShaderVisibility         = IRShaderVisibilityAll;
            rootParameter.Constants.Num32BitValues = rootConstant.Num32BitValues;
            rootParameter.Constants.RegisterSpace  = rootConstant.RegisterSpace;
            rootParameter.Constants.ShaderRegister = rootConstant.ShaderRegister;
            ++rootConstantIndex;
        }
    }

    for ( auto &registerSpaceRange : registerSpaceRanges )
    {
        PutRootParameterDescriptorTable( rootParameters, registerSpaceRange.ShaderVisibility, registerSpaceRange.CbvSrvUavRanges );

        if ( !registerSpaceRange.SamplerRanges.empty( ) )
        {
            PutRootParameterDescriptorTable( rootParameters, registerSpaceRange.ShaderVisibility, registerSpaceRange.SamplerRanges );
        }

        int rootArgumentIndex = 0;
        for ( const auto &rootArgument : registerSpaceRange.RootArguments )
        {
            IRRootParameter1 &rootParameter         = rootParameters.emplace_back( );
            rootParameter.ParameterType             = registerSpaceRange.RootArgumentTypes[ rootArgumentIndex ];
            rootParameter.ShaderVisibility          = IRShaderVisibilityAll;
            rootParameter.Descriptor.RegisterSpace  = rootArgument.RegisterSpace;
            rootParameter.Descriptor.ShaderRegister = rootArgument.ShaderRegister;
            ++rootArgumentIndex;
        }

        ++registerSpace;
    }

#ifndef NDEBUG
    if ( isLocal )
    {
        ReflectionDebugOutput::DumpIRRootParameters( rootParameters, "Metal Local Root Signature" );
    }
    else
    {
        ReflectionDebugOutput::DumpIRRootParameters( rootParameters, "Metal Global Root Signature" );
    }
#endif

    IRVersionedRootSignatureDescriptor desc;
    desc.version        = IRRootSignatureVersion_1_1;
    desc.desc_1_1.Flags = static_cast<IRRootSignatureFlags>( IRRootSignatureFlagCBVSRVUAVHeapDirectlyIndexed | IRRootSignatureFlagSamplerHeapDirectlyIndexed );
    // TODO we also need to handle root constants
    desc.desc_1_1.NumParameters = rootParameters.size( );
    desc.desc_1_1.pParameters   = rootParameters.data( );
    // TODO:
    desc.desc_1_1.NumStaticSamplers = 0;
    desc.desc_1_1.pStaticSamplers   = nullptr;

    if ( const char *dumpDir = getenv( "DZ_DUMP_SHADERS" ) )
    {
        if ( const char *json = IRVersionedRootSignatureDescriptorCopyJSONString( &desc ) )
        {
            const std::string path = std::string( dumpDir ) + ( isLocal ? "/local_rs.json" : "/global_rs.json" );
            if ( FILE *f = fopen( path.c_str( ), "wb" ) )
            {
                fwrite( json, 1, strlen( json ), f );
                fclose( f );
            }
            IRVersionedRootSignatureDescriptorFreeString( json );
        }
    }

    IRError         *error         = nullptr;
    IRRootSignature *rootSignature = IRRootSignatureCreateFromDescriptor( &desc, &error );

    if ( error )
    {
        spdlog::error( "Error producing IRRootSignature, error code [ {} ]", IRErrorGetCode( error ) );
        IRErrorDestroy( error );
    }

    return rootSignature;
}
// For metal, we need to produce a root signature to compile a correct metal lib
// We also keep track of how the root parameter layout looks like so
DxilToMsl::Impl::~Impl( )
{
    if ( m_shaderReflection )
    {
        m_shaderReflection->Release( );
        m_shaderReflection = nullptr;
    }

    if ( m_libraryReflection )
    {
        m_libraryReflection->Release( );
        m_libraryReflection = nullptr;
    }
}

void DxilToMsl::Impl::Convert( const DxilToMslDesc &desc )
{
    // Check for empty input
    if ( desc.Shaders.NumElements == 0 || desc.DXILShaders.NumElements == 0 )
    {
        spdlog::error( "Empty input" );
        return;
    }
    if ( desc.Shaders.NumElements != desc.OutMSLShaders->NumElements )
    {
        spdlog::error( "desc.Shaders.NumElements != desc.OutMSLShaders.NumElements" );
        return;
    }

    IDxcUtils    *dxcUtils = nullptr;
    const HRESULT hr       = DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( &dxcUtils ) );
    if ( FAILED( hr ) )
    {
        spdlog::error( "Failed to create DxcUtils" );
        return;
    }

    const DenOfIz_CompiledShaderStageArray &dxilShaders = desc.DXILShaders;
    // We use this vector to make sure register spaces are ordered correctly, the order of the root parameters is also how the Top Level Argument Buffer expects them
    std::vector<RegisterSpaceRange>           localRegisterSpaceRanges;
    std::vector<RegisterSpaceRange>           registerSpaceRanges;
    std::vector<D3D12_SHADER_INPUT_BIND_DESC> processedInputs; // Handle duplicate inputs

    for ( uint32_t shaderIndex = 0; shaderIndex < dxilShaders.NumElements; ++shaderIndex )
    {
        auto        dxilShader       = dxilShaders.Elements[ shaderIndex ];
        const auto &shaderDesc       = desc.Shaders.Elements[ shaderIndex ];
        auto        processResources = [ & ]( const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc, const int i )
        {
            if ( IsResourceAlreadyProcessed( processedInputs, shaderInputBindDesc ) )
            {
                return;
            }
            processedInputs.push_back( shaderInputBindDesc );
            const bool                  isLocal      = ShaderReflectionHelper::IsBindingLocalTo( dxilShader->RayTracing, shaderInputBindDesc );
            const bool                  isBindless   = ShaderReflectionHelper::IsBindingBindless( shaderDesc.Bindless, shaderInputBindDesc );
            const DenOfIz_BindlessSlot *bindlessSlot = isBindless ? ShaderReflectionHelper::GetBindlessSlot( shaderDesc.Bindless, shaderInputBindDesc ) : nullptr;
            if ( isLocal )
            {
                ContainerUtilities::EnsureSize( localRegisterSpaceRanges, shaderInputBindDesc.Space );
            }
            else
            {
                ContainerUtilities::EnsureSize( registerSpaceRanges, shaderInputBindDesc.Space );
            }

            auto &registerSpaceRange = isLocal ? localRegisterSpaceRanges[ shaderInputBindDesc.Space ] : registerSpaceRanges[ shaderInputBindDesc.Space ];

            if ( const IRShaderVisibility shaderVisibility = DxcEnumConverter::ShaderStageToShaderVisibility( dxilShader->Stage );
                 registerSpaceRange.ShaderVisibility != 0 && registerSpaceRange.ShaderVisibility != shaderVisibility )
            {
                registerSpaceRange.ShaderVisibility = IRShaderVisibilityAll;
            }
            else
            {
                registerSpaceRange.ShaderVisibility = shaderVisibility;
            }

            const IRDescriptorRangeType descriptorRangeType = DxcEnumConverter::ShaderTypeToIRDescriptorType( shaderInputBindDesc.Type );
            if ( ( isLocal || shaderInputBindDesc.Space == DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE ) && shaderInputBindDesc.Type == D3D_SIT_CBUFFER )
            {
                IRRootConstants &rootConstants = registerSpaceRange.RootConstants.emplace_back( );
                rootConstants.RegisterSpace    = shaderInputBindDesc.Space;
                rootConstants.ShaderRegister   = shaderInputBindDesc.BindPoint;

                uint64_t numBytes            = ShaderReflectionHelper::GetConstantBufferSize( m_shaderReflection, m_functionReflection, i );
                rootConstants.Num32BitValues = numBytes / 4;
            }
            else if ( shaderInputBindDesc.Space == DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE )
            {
                if ( isLocal )
                {
                    spdlog::error( "Local root level buffers are not supported, use root constants instead." );
                }
                IRRootDescriptor &rootDescriptor = registerSpaceRange.RootArguments.emplace_back( );
                rootDescriptor.RegisterSpace     = shaderInputBindDesc.Space;
                rootDescriptor.ShaderRegister    = shaderInputBindDesc.BindPoint;

                registerSpaceRange.RootArgumentTypes.push_back( DxcEnumConverter::IRDescriptorRangeTypeToIRRootParameterType( descriptorRangeType ) );
            }
            else
            {
                const uint32_t           maxArraySize    = bindlessSlot ? bindlessSlot->MaxArraySize : 0;
                const IRDescriptorRange1 descriptorRange = CreateDescriptorRange( shaderInputBindDesc, isBindless, maxArraySize );
                if ( isBindless )
                {
                    registerSpaceRange.HasBindlessResources = true;
                }

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

        IterateBoundResources( dxilShader, processResources );
    }

    for ( uint32_t i = 0; i < desc.OutMSLShaders->NumElements; ++i )
    {
        desc.OutMSLShaders->Elements[ i ] = { nullptr, 0 };
    }

    CompileMslDesc compileMslDesc{ };
    compileMslDesc.RootSignature      = CreateRootSignature( registerSpaceRanges, false );
    compileMslDesc.LocalRootSignature = CreateRootSignature( localRegisterSpaceRanges, true );
    compileMslDesc.RayTracing         = desc.RayTracing;

    for ( uint32_t shaderIndex = 0; shaderIndex < desc.Shaders.NumElements; ++shaderIndex )
    {
        const auto &shader = desc.Shaders.Elements[ shaderIndex ];

        DenOfIz_CompileDesc compileDesc = { };
        compileDesc.Path                = shader.Path;
        compileDesc.Data                = shader.Data;
        compileDesc.Defines             = shader.Defines;
        compileDesc.EntryPoint          = shader.EntryPoint;
        compileDesc.Stage               = shader.Stage;
        compileDesc.TargetIL            = DENOFIZ_TARGET_IL_MSL;

        auto &dxilShader = dxilShaders.Elements[ shaderIndex ];
        if ( dxilShader->Reflection.NumElements > 0 )
        {
            const DxcBuffer reflectionBuffer{
                .Ptr      = dxilShader->Reflection.Elements,
                .Size     = dxilShader->Reflection.NumElements,
                .Encoding = 0,
            };
            if ( m_shaderReflection )
            {
                m_shaderReflection->Release( );
                m_shaderReflection = nullptr;
            }
            dxcUtils->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &m_shaderReflection ) );
        }

        const auto mslBlob                          = Compile( compileDesc, dxilShader->DXIL, compileMslDesc, shader.RayTracing );
        desc.OutMSLShaders->Elements[ shaderIndex ] = mslBlob;
    }

    IRRootSignatureDestroy( compileMslDesc.LocalRootSignature );
    IRRootSignatureDestroy( compileMslDesc.RootSignature );

    if ( dxcUtils )
    {
        dxcUtils->Release( );
    }
}

DenOfIz_ByteArray DxilToMsl::Impl::Compile( const DenOfIz_CompileDesc &compileDesc, const DenOfIz_ByteArray &dxil, const CompileMslDesc &compileMslDesc,
                                            const DenOfIz_RayTracingShaderDesc &rayTracingShaderDesc ) const
{
    const IRRootSignature *rootSignature  = compileMslDesc.RootSignature;
    const IRRootSignature *localSignature = compileMslDesc.LocalRootSignature;

    IRCompiler       *irCompiler = IRCompilerCreate( );
    const std::string entryPoint( compileDesc.EntryPoint.Chars, compileDesc.EntryPoint.NumChars );
    IRCompilerSetEntryPointName( irCompiler, entryPoint.c_str( ) );
    IRCompilerSetMinimumDeploymentTarget( irCompiler, IROperatingSystem_macOS, "15.1" );
    IRCompilerSetGlobalRootSignature( irCompiler, rootSignature );
    IRCompilerSetLocalRootSignature( irCompiler, localSignature );

    auto meshTopology = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
    if ( compileDesc.Stage == DENOFIZ_SHADER_STAGE_MESH_BIT && m_shaderReflection )
    {
        meshTopology = ShaderReflectionHelper::ExtractMeshOutputTopology( m_shaderReflection );
        IRCompilerSetCompatibilityFlags( irCompiler, IRCompatibilityFlagPositionInvariance );
    }
    switch ( meshTopology )
    {
    case DENOFIZ_PRIMITIVE_TOPOLOGY_POINT:
        IRCompilerSetInputTopology( irCompiler, IRInputTopologyPoint );
        break;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_LINE:
        IRCompilerSetInputTopology( irCompiler, IRInputTopologyLine );
        break;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE:
        IRCompilerSetInputTopology( irCompiler, IRInputTopologyTriangle );
        break;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_PATCH:
        IRCompilerSetInputTopology( irCompiler, IRInputTopologyPatch );
        break;
    }

    switch ( rayTracingShaderDesc.HitGroupType )
    {
    case DENOFIZ_HIT_GROUP_TYPE_AABBS:
        IRCompilerSetHitgroupType( irCompiler, IRHitGroupTypeProceduralPrimitive );
        break;
    default:
        IRCompilerSetHitgroupType( irCompiler, IRHitGroupTypeTriangles );
        break;
    }

    IRRayTracingPipelineConfiguration *rtPipelineConfig = IRRayTracingPipelineConfigurationCreate( );
    IRRayTracingPipelineConfigurationSetMaxAttributeSizeInBytes( rtPipelineConfig, compileMslDesc.RayTracing.MaxNumAttributeBytes );
    IRRayTracingPipelineConfigurationSetPipelineFlags( rtPipelineConfig, IRRaytracingPipelineFlagNone );
    IRRayTracingPipelineConfigurationSetIntrinsicMasks( rtPipelineConfig, IRIntrinsicMaskClosestHitAll, IRIntrinsicMaskMissShaderAll, IRIntrinsicMaskAnyHitShaderAll,
                                                        IRIntrinsicMaskCallableShaderAll );
    IRRayTracingPipelineConfigurationSetMaxRecursiveDepth( rtPipelineConfig, compileMslDesc.RayTracing.MaxRecursionDepth );
    IRRayTracingPipelineConfigurationSetRayGenerationCompilationMode( rtPipelineConfig, IRRayGenerationCompilationVisibleFunction );
    IRRayTracingPipelineConfigurationSetIntersectionFunctionCompilationMode( rtPipelineConfig, IRIntersectionFunctionCompilationVisibleFunction );
    IRCompilerSetRayTracingPipelineConfiguration( irCompiler, rtPipelineConfig );
    IRRayTracingPipelineConfigurationDestroy( rtPipelineConfig );

    IRObject       *irDxil  = IRObjectCreateFromDXIL( dxil.Elements, dxil.NumElements, IRBytecodeOwnershipNone );
    IRError        *irError = nullptr;
    const IRObject *outIr   = IRCompilerAllocCompileAndLink( irCompiler, entryPoint.c_str( ), irDxil, &irError );

    if ( !outIr )
    {
        spdlog::error( "Failed to compile and link DXIL to MSL, ErrorCode[ {} ]", IRErrorGetCode( irError ) );
        IRErrorDestroy( irError );
    }

    IRMetalLibBinary   *metalLib = IRMetalLibBinaryCreate( );
    const IRShaderStage stage    = IRObjectGetMetalIRShaderStage( outIr );
    IRObjectGetMetalLibBinary( outIr, stage, metalLib );
    const size_t metalLibSize     = IRMetalLibGetBytecodeSize( metalLib );
    const auto   metalLibByteCode = new uint8_t[ metalLibSize ];
    IRMetalLibGetBytecode( metalLib, metalLibByteCode );

    if ( const char *dumpDir = getenv( "DZ_DUMP_SHADERS" ) )
    {
        const std::string base = std::string( dumpDir ) + "/" + entryPoint;
        if ( FILE *f = fopen( ( base + ".dxil" ).c_str( ), "wb" ) )
        {
            fwrite( dxil.Elements, 1, dxil.NumElements, f );
            fclose( f );
        }
        if ( FILE *f = fopen( ( base + ".metallib" ).c_str( ), "wb" ) )
        {
            fwrite( metalLibByteCode, 1, metalLibSize, f );
            fclose( f );
        }
    }

    // Note: metalLibByteCode ownership is transferred to DenOfIz_ByteArray

    IRMetalLibBinaryDestroy( metalLib );
    IRObjectDestroy( irDxil );
    IRCompilerDestroy( irCompiler );

    const DenOfIz_ByteArray result{
        .Elements    = reinterpret_cast<Byte *>( metalLibByteCode ),
        .NumElements = metalLibSize,
    };
    return result;
}

void DxilToMsl::Impl::DxcCheckResult( const HRESULT hr ) const
{
    if ( FAILED( hr ) )
    {
        spdlog::error( "DXC Error: {}", hr );
    }
}

DxilToMsl::Impl::Impl( )
{
#ifdef __APPLE__
    m_irCompiler = IRCompilerCreate( );
#endif
}

// Public interface implementations
DxilToMsl::DxilToMsl( ) : m_pImpl( std::make_unique<Impl>( ) )
{
}

DxilToMsl::~DxilToMsl( ) = default;

void DxilToMsl::Convert( const DxilToMslDesc &desc ) const
{
    m_pImpl->Convert( desc );
}
