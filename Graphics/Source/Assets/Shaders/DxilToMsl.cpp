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

#include "DenOfIzGraphics/Assets/Shaders/DxilToMsl.h"
#include <functional>
#include <unordered_map>
#include "DenOfIzGraphics/Assets/Shaders/ShaderCompiler.h"
#include "DenOfIzGraphics/Backends/Interface/IRootSignature.h"
#include "DenOfIzGraphics/Backends/Interface/RayTracing/ILocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/DxcEnumConverter.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/ReflectionDebugOutput.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/ShaderReflectionHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/ContainerUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

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
    IRRootSignature     *RootSignature;
    IRRootSignature     *LocalRootSignature;
    ShaderRayTracingDesc RayTracing;
};

class DxilToMsl::Impl
{
public:
    ShaderCompiler m_compiler;

    ID3D12ShaderReflection   *m_shaderReflection   = nullptr;
    ID3D12LibraryReflection  *m_libraryReflection  = nullptr;
    ID3D12FunctionReflection *m_functionReflection = nullptr;

    IRCompiler *m_irCompiler = nullptr;

    Impl( );
    ~Impl( );
    InteropArray<InteropArray<Byte>> Convert( const DxilToMslDesc &desc );

private:
    [[nodiscard]] InteropArray<Byte> Compile( const CompileDesc &compileDesc, const InteropArray<Byte> &dxil, const CompileMslDesc &compileMslDesc,
                                              const RayTracingShaderDesc &rayTracingShaderDesc ) const;

    IRRootSignature *CreateRootSignature( std::vector<RegisterSpaceRange> &registerSpaceRanges, bool isLocal ) const;
    void             IterateBoundResources( CompiledShaderStage *shader, ReflectionCallback &callback );
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
void DxilToMsl::Impl::IterateBoundResources( CompiledShaderStage *shader, ReflectionCallback &callback )
{
    IDxcUtils *dxcUtils = nullptr;
    HRESULT    hr       = DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( &dxcUtils ) );
    if ( FAILED( hr ) )
    {
        LOG( ERROR ) << "Failed to create DxcUtils for reflection";
        return;
    }

    auto            reflectionBlob = shader->Reflection;
    const DxcBuffer reflectionBuffer{
        .Ptr      = reflectionBlob.Data( ),
        .Size     = reflectionBlob.NumElements( ),
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
        break;
    default:
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
        break;
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

    IRError         *error         = nullptr;
    IRRootSignature *rootSignature = IRRootSignatureCreateFromDescriptor( &desc, &error );

    if ( error )
    {
        LOG( ERROR ) << "Error producing IRRootSignature, error code [" << IRErrorGetCode( error ) << "]";
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

InteropArray<InteropArray<Byte>> DxilToMsl::Impl::Convert( const DxilToMslDesc &desc )
{
    IDxcUtils    *dxcUtils = nullptr;
    const HRESULT hr       = DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( &dxcUtils ) );
    if ( FAILED( hr ) )
    {
        LOG( ERROR ) << "Failed to create DxcUtils";
        return { };
    }

    const InteropArray<CompiledShaderStage *> &dxilShaders = desc.DXILShaders;
    // We use this vector to make sure register spaces are ordered correctly, the order of the root parameters is also how the Top Level Argument Buffer expects them
    std::vector<RegisterSpaceRange>           localRegisterSpaceRanges;
    std::vector<RegisterSpaceRange>           registerSpaceRanges;
    std::vector<D3D12_SHADER_INPUT_BIND_DESC> processedInputs; // Handle duplicate inputs

    for ( int shaderIndex = 0; shaderIndex < dxilShaders.NumElements( ); ++shaderIndex )
    {
        auto        dxilShader       = dxilShaders.GetElement( shaderIndex );
        const auto &shaderDesc       = desc.Shaders.GetElement( shaderIndex );
        auto        processResources = [ & ]( const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc, const int i )
        {
            if ( IsResourceAlreadyProcessed( processedInputs, shaderInputBindDesc ) )
            {
                return;
            }
            processedInputs.push_back( shaderInputBindDesc );
            const bool          isLocal      = ShaderReflectionHelper::IsBindingLocalTo( dxilShader->RayTracing, shaderInputBindDesc );
            const bool          isBindless   = ShaderReflectionHelper::IsBindingBindless( shaderDesc.Bindless, shaderInputBindDesc );
            const BindlessSlot *bindlessSlot = isBindless ? ShaderReflectionHelper::GetBindlessSlot( shaderDesc.Bindless, shaderInputBindDesc ) : nullptr;
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
            if ( ( isLocal || shaderInputBindDesc.Space == DZConfiguration::Instance( ).RootConstantRegisterSpace ) && shaderInputBindDesc.Type == D3D_SIT_CBUFFER )
            {
                IRRootConstants &rootConstants = registerSpaceRange.RootConstants.emplace_back( );
                rootConstants.RegisterSpace    = shaderInputBindDesc.Space;
                rootConstants.ShaderRegister   = shaderInputBindDesc.BindPoint;

                ReflectionDesc rootConstantReflection;
                ShaderReflectionHelper::FillReflectionData( m_shaderReflection, m_functionReflection, rootConstantReflection, i );
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

    auto result = InteropArray<InteropArray<Byte>>( desc.Shaders.NumElements( ) );

    CompileMslDesc compileMslDesc{ };
    compileMslDesc.RootSignature      = CreateRootSignature( registerSpaceRanges, false );
    compileMslDesc.LocalRootSignature = CreateRootSignature( localRegisterSpaceRanges, true );
    compileMslDesc.RayTracing         = desc.RayTracing;

    for ( int shaderIndex = 0; shaderIndex < desc.Shaders.NumElements( ); ++shaderIndex )
    {
        const auto &shader = desc.Shaders.GetElement( shaderIndex );

        CompileDesc compileDesc = { };
        compileDesc.Path        = shader.Path;
        compileDesc.Data        = shader.Data;
        compileDesc.Defines     = shader.Defines;
        compileDesc.EntryPoint  = shader.EntryPoint;
        compileDesc.Stage       = shader.Stage;
        compileDesc.TargetIL    = TargetIL::MSL;

        auto &dxilShader = dxilShaders.GetElement( shaderIndex );
        if ( dxilShader->Reflection.NumElements( ) > 0 )
        {
            const DxcBuffer reflectionBuffer{
                .Ptr      = dxilShader->Reflection.Data( ),
                .Size     = dxilShader->Reflection.NumElements( ),
                .Encoding = 0,
            };
            if ( m_shaderReflection )
            {
                m_shaderReflection->Release( );
                m_shaderReflection = nullptr;
            }
            dxcUtils->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &m_shaderReflection ) );
        }

        auto mslBlob = Compile( compileDesc, dxilShader->DXIL, compileMslDesc, shader.RayTracing );
        result.SetElement( shaderIndex, mslBlob );
    }

    IRRootSignatureDestroy( compileMslDesc.LocalRootSignature );
    IRRootSignatureDestroy( compileMslDesc.RootSignature );

    if ( dxcUtils )
    {
        dxcUtils->Release( );
    }

    return result;
}

InteropArray<Byte> DxilToMsl::Impl::Compile( const CompileDesc &compileDesc, const InteropArray<Byte> &dxil, const CompileMslDesc &compileMslDesc,
                                             const RayTracingShaderDesc &rayTracingShaderDesc ) const
{
    const IRRootSignature *rootSignature  = compileMslDesc.RootSignature;
    const IRRootSignature *localSignature = compileMslDesc.LocalRootSignature;

    IRCompiler *irCompiler = IRCompilerCreate( );
    IRCompilerSetEntryPointName( irCompiler, compileDesc.EntryPoint.Get( ) );
    IRCompilerSetMinimumDeploymentTarget( irCompiler, IROperatingSystem_macOS, "15.1" );
    IRCompilerSetGlobalRootSignature( irCompiler, rootSignature );
    IRCompilerSetLocalRootSignature( irCompiler, localSignature );

    PrimitiveTopology meshTopology = PrimitiveTopology::Triangle;
    if ( compileDesc.Stage == ShaderStage::Mesh && m_shaderReflection )
    {
        meshTopology = ShaderReflectionHelper::ExtractMeshOutputTopology( m_shaderReflection );
        IRCompilerSetCompatibilityFlags( irCompiler, IRCompatibilityFlagPositionInvariance );
    }
    switch ( meshTopology )
    {
    case PrimitiveTopology::Point:
        IRCompilerSetInputTopology( irCompiler, IRInputTopologyPoint );
        break;
    case PrimitiveTopology::Line:
        IRCompilerSetInputTopology( irCompiler, IRInputTopologyLine );
        break;
    case PrimitiveTopology::Triangle:
        IRCompilerSetInputTopology( irCompiler, IRInputTopologyTriangle );
        break;
    case PrimitiveTopology::Patch:
        IRCompilerSetInputTopology( irCompiler, IRInputTopologyPatch );
        break;
    }

    switch ( rayTracingShaderDesc.HitGroupType )
    {
    case HitGroupType::AABBs:
        IRCompilerSetHitgroupType( irCompiler, IRHitGroupTypeProceduralPrimitive );
        break;
    default:
        IRCompilerSetHitgroupType( irCompiler, IRHitGroupTypeTriangles );
        break;
    }

    IRCompilerSetRayTracingPipelineArguments( irCompiler, compileMslDesc.RayTracing.MaxNumAttributeBytes, IRRaytracingPipelineFlagNone, IRIntrinsicMaskClosestHitAll,
                                              IRIntrinsicMaskMissShaderAll, IRIntrinsicMaskAnyHitShaderAll, IRIntrinsicMaskCallableShaderAll,
                                              compileMslDesc.RayTracing.MaxRecursionDepth, IRRayGenerationCompilationVisibleFunction,
                                              IRIntersectionFunctionCompilationVisibleFunction );

    IRObject       *irDxil  = IRObjectCreateFromDXIL( static_cast<const uint8_t *>( dxil.Data( ) ), dxil.NumElements( ), IRBytecodeOwnershipNone );
    IRError        *irError = nullptr;
    const IRObject *outIr   = IRCompilerAllocCompileAndLink( irCompiler, compileDesc.EntryPoint.Get( ), irDxil, &irError );

    if ( !outIr )
    {
        LOG( ERROR ) << "Failed to compile and link DXIL to MSL, ErrorCode[" << IRErrorGetCode( irError ) << "]";
        IRErrorDestroy( irError );
    }

    IRMetalLibBinary   *metalLib = IRMetalLibBinaryCreate( );
    const IRShaderStage stage    = IRObjectGetMetalIRShaderStage( outIr );
    IRObjectGetMetalLibBinary( outIr, stage, metalLib );
    const size_t metalLibSize     = IRMetalLibGetBytecodeSize( metalLib );
    const auto   metalLibByteCode = new uint8_t[ metalLibSize ];
    IRMetalLibGetBytecode( metalLib, metalLibByteCode );

    InteropArray<Byte> mslBlob{ };
    mslBlob.MemCpy( metalLibByteCode, metalLibSize );

    IRMetalLibBinaryDestroy( metalLib );
    IRObjectDestroy( irDxil );
    IRCompilerDestroy( irCompiler );
    return mslBlob;
}

void DxilToMsl::Impl::DxcCheckResult( const HRESULT hr ) const
{
    if ( FAILED( hr ) )
    {
        LOG( ERROR ) << "DXC Error: " << hr;
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

InteropArray<InteropArray<Byte>> DxilToMsl::Convert( const DxilToMslDesc &desc )
{
    return m_pImpl->Convert( desc );
}
