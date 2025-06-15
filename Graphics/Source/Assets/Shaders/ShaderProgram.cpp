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

#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include <ranges>
#include <set>
#include <utility>
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetReader.h"
#include "DenOfIzGraphics/Assets/Shaders/DxilToMsl.h"
#include "DenOfIzGraphics/Assets/Shaders/ShaderCompiler.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/DxcEnumConverter.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/ReflectionDebugOutput.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/ShaderReflectionHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#ifdef _WIN32
#include <wrl/client.h>
#include "DenOfIzGraphics/Utilities/Common_Windows.h"
#else
#define __EMULATE_UUID
#include "WinAdapter.h"
#endif

#include "dxcapi.h"

using namespace DenOfIz;

#define DXC_CHECK_RESULT( result )                                                                                                                                                 \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        if ( FAILED( result ) )                                                                                                                                                    \
        {                                                                                                                                                                          \
            spdlog::error( "DXC Error: {}", result );                                                                                                                              \
        }                                                                                                                                                                          \
    }                                                                                                                                                                              \
    while ( false )

struct RootSignatureState
{
    std::vector<StaticSamplerDesc>               StaticSamplers;
    std::vector<RootConstantResourceBindingDesc> RootConstants;
    std::vector<ResourceBindingDesc>             ResourceBindings;
    std::vector<BindlessResourceDesc>            BindlessResources;
    std::vector<std::vector<ShaderStage>>        RootConstantStages;
    std::vector<std::vector<ShaderStage>>        ResourceBindingStages;
};

struct ReflectionState
{
    InputLayoutDesc                  *InputLayoutDesc;
    std::vector<ResourceBindingDesc> *LocalResourceBindings;
    ShaderStageDesc const            *ShaderDesc;
    CompiledShaderStage              *CompiledShader;
    ID3D12ShaderReflection           *ShaderReflection;
    ID3D12LibraryReflection          *LibraryReflection;
    ID3D12FunctionReflection         *FunctionReflection;
    int                               ShaderIndex;
};

class ShaderProgram::Impl
{
public:
    ShaderCompiler                                     m_compiler;
    std::vector<std::unique_ptr<CompiledShaderStage>>  m_compiledShaders;
    std::vector<CompiledShaderStage *>                 m_compiledShaderPtrs;
    std::vector<ShaderStageDesc>                       m_shaderDescs; // Index matched with m_compiledShaders
    ShaderReflectDesc                                  m_reflectDesc;
    ShaderProgramDesc                                  m_desc;
    std::vector<ByteArray>                             m_dataToClean;
    RootSignatureState                                 m_rootSignatureState{ };
    std::vector<ResourceBindingSlotArray>              m_localBindingsToClean;
    std::vector<std::vector<ResourceBindingDesc>>      m_localResourceBindings;
    std::vector<std::vector<std::vector<ShaderStage>>> m_localResourceBindingStages;
    std::vector<LocalRootSignatureDesc>                m_localRootSignatures;
    std::vector<ThreadGroupInfo>                       m_threadGroupInfos;
    std::vector<std::vector<ReflectionResourceField>>  m_reflectionFieldStorage;
    std::vector<InputGroupDesc>                        m_inputGroups;
    std::vector<std::vector<InputLayoutElementDesc>>   m_inputElements;

    explicit Impl( const ShaderProgramDesc &desc ) : m_desc( desc )
    {
    }
    explicit Impl( const ShaderAsset & ) : m_desc( { } )
    {
    }

    void Compile( );
    void CreateReflectionData( );
    void InitInputLayout( ID3D12ShaderReflection *shaderReflection, InputLayoutDesc &inputLayoutDesc, const D3D12_SHADER_DESC &shaderDesc );
    void ReflectShader( ReflectionState &state );
    void ReflectLibrary( ReflectionState &state );
    void ProcessInputBindingDesc( ReflectionState &state, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc, int resourceIndex );
    bool UpdateBoundResourceStage( const ReflectionState &state, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc );
    void ProcessBindlessArrays( RootSignatureDesc &rootSignature );
};

struct LocalRootSignatureState
{
    std::vector<ResourceBindingDesc> ResourceBindings;
    LocalRootSignatureDesc           Desc;
};

ShaderProgram::ShaderProgram( ShaderProgramDesc desc ) : m_pImpl( std::make_unique<Impl>( std::move( desc ) ) )
{
    m_pImpl->Compile( );
    m_pImpl->CreateReflectionData( );
}

ShaderProgram::ShaderProgram( const ShaderAsset &asset ) : m_pImpl( std::make_unique<Impl>( asset ) )
{
    const CompiledShader shader = ShaderAssetReader::ConvertToCompiledShader( asset );
    for ( uint32_t i = 0; i < shader.Stages.NumElements; ++i )
    {
        CompiledShaderStage *shaderStage = shader.Stages.Elements[ i ];
        const auto          &ownedShader = m_pImpl->m_compiledShaders.emplace_back( std::unique_ptr<CompiledShaderStage>( shaderStage ) );
        m_pImpl->m_compiledShaderPtrs.push_back( ownedShader.get( ) );
        // ShaderAssetReader doesn't clean this data because we may need it in the lifetime of the program
        m_pImpl->m_dataToClean.push_back( ownedShader->DXIL );
        m_pImpl->m_dataToClean.push_back( ownedShader->MSL );
        m_pImpl->m_dataToClean.push_back( ownedShader->SPIRV );
        m_pImpl->m_dataToClean.push_back( ownedShader->Reflection );
    }

    m_pImpl->m_reflectDesc     = shader.ReflectDesc;
    m_pImpl->m_desc            = { };
    m_pImpl->m_desc.RayTracing = shader.RayTracing;
}

ShaderProgram::~ShaderProgram( )
{
    for ( const auto &data : m_pImpl->m_dataToClean )
    {
        if ( data.Elements )
        {
            free( data.Elements );
        }
    }
}

CompiledShaderStageArray ShaderProgram::CompiledShaders( ) const
{
    CompiledShaderStageArray compiledShaders{ };
    compiledShaders.NumElements = m_pImpl->m_compiledShaders.size( );
    compiledShaders.Elements    = m_pImpl->m_compiledShaderPtrs.data( );
    return compiledShaders;
}

ShaderReflectDesc ShaderProgram::Reflect( ) const
{
    return m_pImpl->m_reflectDesc;
}

ShaderProgramDesc ShaderProgram::Desc( ) const
{
    return m_pImpl->m_desc;
}

/**
 * \brief Compiles the shaders targeting MSL/DXIL/SPIR-V. MSL is double compiled, first time to DXIL and reflect and provide a root signature to the second compilation.
 */
void ShaderProgram::Impl::Compile( )
{
    std::vector<CompiledShaderStage *> dxilShaders;

    for ( uint32_t i = 0; i < m_desc.ShaderStages.NumElements; ++i )
    {
        const auto &stage = m_desc.ShaderStages.Elements[ i ];
        // Validate Shader
        if ( stage.Path.IsEmpty( ) && stage.Data.NumElements == 0 )
        {
            spdlog::error( "Either stage.Path or stage.Data must be set for stage {} ", i );
            continue;
        }

        CompileDesc compileDesc = { };
        compileDesc.Path        = stage.Path;
        compileDesc.CodePage    = stage.CodePage;
        compileDesc.Data        = stage.Data;
        compileDesc.Defines     = stage.Defines;
        compileDesc.EntryPoint  = stage.EntryPoint;
        compileDesc.Stage       = stage.Stage;
        compileDesc.TargetIL    = TargetIL::DXIL;

        auto [ dxil, reflection ] = m_compiler.CompileHLSL( compileDesc );
        m_dataToClean.push_back( dxil );
        m_dataToClean.push_back( reflection );

        compileDesc.TargetIL = TargetIL::SPIRV;
        auto [ spirv, _ ]    = m_compiler.CompileHLSL( compileDesc );
        m_dataToClean.push_back( spirv );

        const auto &compiledShader = m_compiledShaders.emplace_back( std::make_unique<CompiledShaderStage>( ) );
        m_compiledShaderPtrs.push_back( compiledShader.get( ) );

        compiledShader->Stage      = stage.Stage;
        compiledShader->EntryPoint = stage.EntryPoint;
        compiledShader->RayTracing = stage.RayTracing;
        compiledShader->Reflection = reflection;
        compiledShader->DXIL       = dxil;
        compiledShader->SPIRV      = spirv;
        compiledShader->MSL        = { }; // Set below

        m_shaderDescs.push_back( stage );
    }

#if defined( _WIN32 ) || defined( __APPLE__ ) // TODO metal shader converter on linux: not yet supported
    DxilToMslDesc dxilToMslDesc{ };
    dxilToMslDesc.Shaders    = m_desc.ShaderStages;
    dxilToMslDesc.RayTracing = m_desc.RayTracing;

    dxilToMslDesc.DXILShaders.NumElements = m_compiledShaders.size( );
    dxilToMslDesc.DXILShaders.Elements    = static_cast<CompiledShaderStage **>( std::malloc( dxilToMslDesc.DXILShaders.NumElements * sizeof( CompiledShaderStage * ) ) );
    for ( uint32_t i = 0; i < dxilToMslDesc.DXILShaders.NumElements; ++i )
    {
        dxilToMslDesc.DXILShaders.Elements[ i ] = m_compiledShaders[ i ].get( );
    }

    std::vector<ByteArray> mslShaders( m_compiledShaders.size( ) );
    ByteArrayArray         mslShadersArray{ };
    mslShadersArray.NumElements = mslShaders.size( );
    mslShadersArray.Elements    = mslShaders.data( );
    dxilToMslDesc.OutMSLShaders = &mslShadersArray;
    const DxilToMsl dxilToMsl{ };
    dxilToMsl.Convert( dxilToMslDesc );
    if ( mslShaders.size( ) != m_desc.ShaderStages.NumElements )
    {
        spdlog::error( "Num DXIL shaders != Num MSL Shaders, probable bug in DxilToMsl" );
        std::free( dxilToMslDesc.DXILShaders.Elements );
        return;
    }

    for ( uint32_t i = 0; i < mslShaders.size( ); ++i )
    {
        m_compiledShaders[ i ]->MSL = mslShaders[ i ];
        m_dataToClean.push_back( mslShaders[ i ] );
    }
    std::free( dxilToMslDesc.DXILShaders.Elements );
#else
    spdlog::error( "MSL compilation is not supported on this platform" );
#endif
}

void ShaderProgram::Impl::CreateReflectionData( )
{
    IDxcUtils    *dxcUtils = nullptr;
    const HRESULT hr       = DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( &dxcUtils ) );
    if ( FAILED( hr ) )
    {
        spdlog::error( "Failed to create DxcUtils for reflection" );
        return;
    }
    m_reflectDesc = { };
    m_localRootSignatures.resize( m_compiledShaders.size( ) );
    m_localResourceBindings.resize( m_compiledShaders.size( ) );
    m_localResourceBindingStages.resize( m_compiledShaders.size( ) );
    m_threadGroupInfos.resize( m_compiledShaders.size( ) );

    InputLayoutDesc   &inputLayout   = m_reflectDesc.InputLayout;
    RootSignatureDesc &rootSignature = m_reflectDesc.RootSignature;

    // TODO These don't really need to be stored this way
    std::vector<uint32_t> descriptorTableLocations;
    std::vector<uint32_t> localDescriptorTableLocations;

    ReflectionState reflectionState = { };
    reflectionState.InputLayoutDesc = &inputLayout;

    for ( int stageIndex = 0; stageIndex < m_compiledShaders.size( ); ++stageIndex )
    {
        auto &shader                          = m_compiledShaders[ stageIndex ];
        reflectionState.CompiledShader        = shader.get( );
        reflectionState.ShaderDesc            = &m_desc.ShaderStages.Elements[ stageIndex ];
        reflectionState.ShaderIndex           = stageIndex;
        LocalRootSignatureDesc &recordLayout  = m_localRootSignatures[ stageIndex ];
        reflectionState.LocalResourceBindings = &m_localResourceBindings[ stageIndex ];

        const auto reflectionBlob = shader->Reflection;
        DxcBuffer  reflectionBuffer{
             .Ptr      = reflectionBlob.Elements,
             .Size     = reflectionBlob.NumElements,
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
            DXC_CHECK_RESULT( dxcUtils->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &libraryReflection ) ) );
            reflectionState.LibraryReflection = libraryReflection;
            ReflectLibrary( reflectionState );
            break;
        case ShaderStage::Vertex:
        default:
            DXC_CHECK_RESULT( dxcUtils->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &shaderReflection ) ) );
            reflectionState.ShaderReflection = shaderReflection;
            ReflectShader( reflectionState );

            if ( shader->Stage == ShaderStage::Compute || shader->Stage == ShaderStage::Mesh || shader->Stage == ShaderStage::Task )
            {
                const ThreadGroupInfo threadGroup = ShaderReflectionHelper::ExtractThreadGroupSize( shaderReflection, nullptr );
                shader->ThreadGroup               = threadGroup;
                m_threadGroupInfos[ stageIndex ]  = threadGroup;
            }
            break;
        }

        if ( shaderReflection )
        {
            shaderReflection->Release( );
        }
        if ( libraryReflection )
        {
            libraryReflection->Release( );
        }

        recordLayout.ResourceBindings.NumElements = m_localResourceBindings[ stageIndex ].size( );
        recordLayout.ResourceBindings.Elements    = m_localResourceBindings[ stageIndex ].data( );

        // Set up stages for local resource bindings
        for ( size_t i = 0; i < m_localResourceBindings[ stageIndex ].size( ); ++i )
        {
            auto &binding              = m_localResourceBindings[ stageIndex ][ i ];
            auto &stages               = m_localResourceBindingStages[ stageIndex ][ i ];
            binding.Stages.Elements    = stages.data( );
            binding.Stages.NumElements = stages.size( );
        }
    }

    ProcessBindlessArrays( rootSignature );

#ifndef NDEBUG
    ReflectionDebugOutput::DumpReflectionInfo( m_reflectDesc );
#endif

    if ( dxcUtils )
    {
        dxcUtils->Release( );
    }

    // Set up stages for root constants
    for ( size_t i = 0; i < m_rootSignatureState.RootConstants.size( ); ++i )
    {
        auto &binding              = m_rootSignatureState.RootConstants[ i ];
        auto &stages               = m_rootSignatureState.RootConstantStages[ i ];
        binding.Stages.Elements    = stages.data( );
        binding.Stages.NumElements = stages.size( );
    }

    // Set up stages for resource bindings
    for ( size_t i = 0; i < m_rootSignatureState.ResourceBindings.size( ); ++i )
    {
        auto &binding              = m_rootSignatureState.ResourceBindings[ i ];
        auto &stages               = m_rootSignatureState.ResourceBindingStages[ i ];
        binding.Stages.Elements    = stages.data( );
        binding.Stages.NumElements = stages.size( );
    }

    // Set up input layout arrays
    m_reflectDesc.InputLayout.InputGroups.NumElements = m_inputGroups.size( );
    m_reflectDesc.InputLayout.InputGroups.Elements    = m_inputGroups.data( );

    m_reflectDesc.RootSignature.RootConstants.NumElements     = m_rootSignatureState.RootConstants.size( );
    m_reflectDesc.RootSignature.RootConstants.Elements        = m_rootSignatureState.RootConstants.data( );
    m_reflectDesc.RootSignature.ResourceBindings.NumElements  = m_rootSignatureState.ResourceBindings.size( );
    m_reflectDesc.RootSignature.ResourceBindings.Elements     = m_rootSignatureState.ResourceBindings.data( );
    m_reflectDesc.RootSignature.BindlessResources.NumElements = m_rootSignatureState.BindlessResources.size( );
    m_reflectDesc.RootSignature.BindlessResources.Elements    = m_rootSignatureState.BindlessResources.data( );
    m_reflectDesc.LocalRootSignatures.NumElements             = m_localRootSignatures.size( );
    m_reflectDesc.LocalRootSignatures.Elements                = m_localRootSignatures.data( );
    m_reflectDesc.ThreadGroups.NumElements                    = m_threadGroupInfos.size( );
    m_reflectDesc.ThreadGroups.Elements                       = m_threadGroupInfos.data( );
}

Format MaskToFormat( const D3D_REGISTER_COMPONENT_TYPE componentType, const uint32_t mask )
{
    switch ( componentType )
    {
    case D3D_REGISTER_COMPONENT_UINT32:
        switch ( mask )
        {
        case 1:
            return Format::R32Uint;
        case 3:
            return Format::R32G32Uint;
        case 7:
            return Format::R32G32B32Uint;
        case 15:
            return Format::R32G32B32A32Uint;
        default:
            return Format::Undefined;
        }
    case D3D_REGISTER_COMPONENT_SINT32:
        switch ( mask )
        {
        case 1:
            return Format::R32Sint;
        case 3:
            return Format::R32G32Sint;
        case 7:
            return Format::R32G32B32Sint;
        case 15:
            return Format::R32G32B32A32Sint;
        default:
            return Format::Undefined;
        }
    case D3D_REGISTER_COMPONENT_FLOAT32:
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
#ifdef D3D_READY // Removed because they used to work with Directx Headers, however the dependency is now removed depending on DXC version which is a bit behind and doesn't have
                 // these values
    case D3D_REGISTER_COMPONENT_UINT16:
        switch ( mask )
        {
        case 1:
            return Format::R16Uint;
        case 3:
            return Format::R16G16Uint;
        case 15:
            return Format::R16G16B16A16Uint;
        default:
            return Format::Undefined;
        }
    case D3D_REGISTER_COMPONENT_SINT16:
        switch ( mask )
        {
        case 1:
            return Format::R16Sint;
        case 3:
            return Format::R16G16Sint;
        case 15:
            return Format::R16G16B16A16Sint;
        default:
            return Format::Undefined;
        }
    case D3D_REGISTER_COMPONENT_FLOAT16:
        switch ( mask )
        {
        case 1:
            return Format::R16Float;
        case 3:
            return Format::R16G16Float;
        case 15:
            return Format::R16G16B16A16Float;
        default:
            return Format::Undefined;
        }
#endif
    default:
        return Format::Undefined;
    }
}

void ShaderProgram::Impl::ReflectShader( ReflectionState &state )
{
    ID3D12ShaderReflection *shaderReflection = state.ShaderReflection;

    D3D12_SHADER_DESC shaderDesc{ };
    DXC_CHECK_RESULT( shaderReflection->GetDesc( &shaderDesc ) );

    if ( state.ShaderDesc->Stage == ShaderStage::Vertex )
    {
        InitInputLayout( shaderReflection, *state.InputLayoutDesc, shaderDesc );
    }

    DXC_CHECK_RESULT( shaderReflection->GetDesc( &shaderDesc ) );
    for ( const uint32_t i : std::views::iota( 0u, shaderDesc.BoundResources ) )
    {
        D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };
        DXC_CHECK_RESULT( shaderReflection->GetResourceBindingDesc( i, &shaderInputBindDesc ) );
        ProcessInputBindingDesc( state, shaderInputBindDesc, i );
    }
}

void ShaderProgram::Impl::ReflectLibrary( ReflectionState &state )
{
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
            ProcessInputBindingDesc( state, shaderInputBindDesc, j );
        }
    }
}

void ShaderProgram::Impl::ProcessInputBindingDesc( ReflectionState &state, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc, const int resourceIndex )
{
    if ( UpdateBoundResourceStage( state, shaderInputBindDesc ) )
    {
        return;
    }
    const bool                isLocal     = ShaderReflectionHelper::IsBindingLocalTo( state.ShaderDesc->RayTracing, shaderInputBindDesc );
    const ResourceBindingType bindingType = DxcEnumConverter::ReflectTypeToBufferBindingType( shaderInputBindDesc.Type );
    // Root constants are reserved for a specific register space
    // PS: Constant buffers in local root signatures are already handled as root constants
    if ( shaderInputBindDesc.Space == DZConfiguration::Instance( ).RootConstantRegisterSpace && !isLocal )
    {
        ReflectionDesc rootConstantReflection;
        ShaderReflectionHelper::FillReflectionData( state.ShaderReflection, state.FunctionReflection, rootConstantReflection, resourceIndex, m_reflectionFieldStorage );
        if ( rootConstantReflection.Type != ReflectionBindingType::Pointer && rootConstantReflection.Type != ReflectionBindingType::Struct )
        {
            spdlog::critical( "Root constant reflection type mismatch. RegisterSpace [ {} ] is reserved for root constants. Which cannot be samplers or textures.",
                              shaderInputBindDesc.Space );
        }
        RootConstantResourceBindingDesc &rootConstantBinding = m_rootSignatureState.RootConstants.emplace_back( RootConstantResourceBindingDesc{ } );
        rootConstantBinding.Name                             = shaderInputBindDesc.Name;
        rootConstantBinding.Binding                          = shaderInputBindDesc.BindPoint;
        m_rootSignatureState.RootConstantStages.emplace_back( ).push_back( state.ShaderDesc->Stage );
        rootConstantBinding.NumBytes   = rootConstantReflection.NumBytes;
        rootConstantBinding.Reflection = rootConstantReflection;
        return;
    }

    // If this register space is configured to be a LocalRootSignature, then populate the corresponding Bindings.
    auto *resourceBindings = &m_rootSignatureState.ResourceBindings;
    if ( isLocal )
    {
        resourceBindings = state.LocalResourceBindings;
    }

    const bool isBindless      = ShaderReflectionHelper::IsBindingBindless( state.ShaderDesc->Bindless, shaderInputBindDesc );
    bool       isBindlessArray = false;
    for ( uint32_t i = 0; i < state.ShaderDesc->Bindless.BindlessArrays.NumElements; ++i )
    {
        const auto &bindlessSlot = state.ShaderDesc->Bindless.BindlessArrays.Elements[ i ];
        if ( bindlessSlot.Binding == shaderInputBindDesc.BindPoint && bindlessSlot.RegisterSpace == shaderInputBindDesc.Space &&
             bindlessSlot.Type == DxcEnumConverter::ReflectTypeToBufferBindingType( shaderInputBindDesc.Type ) )
        {
            isBindlessArray = true;
            break;
        }
    }

    // Skip creating regular resource binding for bindless arrays as they're handled separately
    if ( isBindlessArray )
    {
        return;
    }

    ResourceBindingDesc &resourceBindingDesc = resourceBindings->emplace_back( ResourceBindingDesc{ } );
    resourceBindingDesc.Name                 = shaderInputBindDesc.Name;
    resourceBindingDesc.Binding              = shaderInputBindDesc.BindPoint;
    resourceBindingDesc.RegisterSpace        = shaderInputBindDesc.Space;
    resourceBindingDesc.ArraySize            = isBindless ? UINT_MAX : shaderInputBindDesc.BindCount;
    resourceBindingDesc.BindingType          = bindingType;
    resourceBindingDesc.Descriptor           = DxcEnumConverter::ReflectTypeToRootSignatureType( shaderInputBindDesc.Type, shaderInputBindDesc.Dimension );
    if ( resourceBindings == &m_rootSignatureState.ResourceBindings )
    {
        m_rootSignatureState.ResourceBindingStages.emplace_back( ).push_back( state.ShaderDesc->Stage );
    }
    else
    {
        // For local resource bindings, track the stage
        m_localResourceBindingStages[ state.ShaderIndex ].emplace_back( ).push_back( state.ShaderDesc->Stage );
    }
    resourceBindingDesc.IsBindless = isBindless;
    ShaderReflectionHelper::FillReflectionData( state.ShaderReflection, state.FunctionReflection, resourceBindingDesc.Reflection, resourceIndex, m_reflectionFieldStorage );
}

void ShaderProgram::Impl::ProcessBindlessArrays( RootSignatureDesc &rootSignature )
{
    for ( uint32_t stageIndex = 0; stageIndex < m_desc.ShaderStages.NumElements; ++stageIndex )
    {
        const auto &shaderStage = m_desc.ShaderStages.Elements[ stageIndex ];
        for ( uint32_t i = 0; i < shaderStage.Bindless.BindlessArrays.NumElements; ++i )
        {
            const auto &bindlessSlot  = shaderStage.Bindless.BindlessArrays.Elements[ i ];
            bool        alreadyExists = false;
            for ( uint32_t j = 0; j < rootSignature.BindlessResources.NumElements; ++j )
            {
                const auto &existing = rootSignature.BindlessResources.Elements[ j ];
                if ( existing.Binding == bindlessSlot.Binding && existing.RegisterSpace == bindlessSlot.RegisterSpace && existing.Type == bindlessSlot.Type )
                {
                    alreadyExists = true;
                    break;
                }
            }

            if ( !alreadyExists )
            {
                BindlessResourceDesc &bindlessResource = m_rootSignatureState.BindlessResources.emplace_back( BindlessResourceDesc{ } );
                bindlessResource.Binding               = bindlessSlot.Binding;
                bindlessResource.RegisterSpace         = bindlessSlot.RegisterSpace;
                bindlessResource.Type                  = bindlessSlot.Type;
                bindlessResource.MaxArraySize          = bindlessSlot.MaxArraySize;
                bindlessResource.IsDynamic             = true;
                bindlessResource.Name                  = InteropString( "BindlessArray_" )
                                            .Append( std::to_string( bindlessSlot.Binding ).c_str( ) )
                                            .Append( "_" )
                                            .Append( std::to_string( bindlessSlot.RegisterSpace ).c_str( ) );
            }
        }
    }
}

bool ShaderProgram::Impl::UpdateBoundResourceStage( const ReflectionState &state, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc )
{
    const ResourceBindingType bindingType = DxcEnumConverter::ReflectTypeToBufferBindingType( shaderInputBindDesc.Type );
    // Check if Resource is already bound, if so add the stage to the existing binding and continue
    bool found = false;

    // Check if it is a root constant:
    if ( shaderInputBindDesc.Space == DZConfiguration::Instance( ).RootConstantRegisterSpace )
    {
        for ( int bindingIndex = 0; bindingIndex < m_rootSignatureState.RootConstants.size( ); ++bindingIndex )
        {
            if ( auto &boundBinding = m_rootSignatureState.RootConstants[ bindingIndex ]; boundBinding.Binding == shaderInputBindDesc.BindPoint )
            {
                found = true;
                m_rootSignatureState.RootConstantStages[ bindingIndex ].push_back( state.ShaderDesc->Stage );
                break;
            }
        }
        return found;
    }

    for ( int bindingIndex = 0; bindingIndex < m_rootSignatureState.ResourceBindings.size( ); ++bindingIndex )
    {
        auto &boundBinding  = m_rootSignatureState.ResourceBindings[ bindingIndex ];
        bool  isSameBinding = boundBinding.RegisterSpace == shaderInputBindDesc.Space;
        isSameBinding       = isSameBinding && boundBinding.Binding == shaderInputBindDesc.BindPoint;
        isSameBinding       = isSameBinding && boundBinding.BindingType == bindingType;
        isSameBinding       = isSameBinding && strcmp( boundBinding.Name.Get( ), shaderInputBindDesc.Name ) == 0;
        if ( isSameBinding )
        {
            found             = true;
            auto &stages      = m_rootSignatureState.ResourceBindingStages[ bindingIndex ];
            bool  stageExists = false;
            for ( const auto &existingStage : stages )
            {
                if ( existingStage == state.ShaderDesc->Stage )
                {
                    stageExists = true;
                    break;
                }
            }
            if ( !stageExists )
            {
                stages.push_back( state.ShaderDesc->Stage );
            }
        }
    }
    return found;
}

void ShaderProgram::Impl::InitInputLayout( ID3D12ShaderReflection *shaderReflection, InputLayoutDesc &inputLayoutDesc, const D3D12_SHADER_DESC &shaderDesc )
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
        inputElementDesc.Semantic                = signatureParameterDesc.SemanticName;
        inputElementDesc.SemanticIndex           = signatureParameterDesc.SemanticIndex;
        inputElementDesc.Format                  = MaskToFormat( signatureParameterDesc.ComponentType, signatureParameterDesc.Mask );
    }

    if ( !inputElements.empty( ) )
    {
        m_inputGroups.emplace_back( );
        auto &inputGroup    = m_inputGroups.back( );
        inputGroup.StepRate = StepRate::PerVertex;

        m_inputElements.emplace_back( );
        auto &elementsVector = m_inputElements.back( );
        elementsVector       = std::move( inputElements );

        inputGroup.Elements.Elements    = elementsVector.data( );
        inputGroup.Elements.NumElements = elementsVector.size( );
    }
}
