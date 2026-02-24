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
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetReader.h"

#include <deque>
#include <map>
#include <ranges>
#include <utility>
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/StringStore.h"

#ifndef DZ_DXC_NOT_SUPPORTED
#include "DenOfIzGraphics/Assets/Shaders/ShaderCompiler.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/DxcEnumConverter.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/DxilToMsl.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/ReflectionDebugOutput.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/SPIRVToWGSL.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/ShaderReflectionHelper.h"

#ifdef _WIN32
#include <wrl/client.h>
#include "DenOfIzGraphics/Utilities/Common_Windows.h"
#else
#define __EMULATE_UUID
#include "WinAdapter.h"
#endif

#include "dxcapi.h"
#endif

using namespace DenOfIz;

#define SHADER_PROGRAM_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ShaderProgram, handle )

namespace DenOfIz
{
    struct BindGroupLayoutState
    {
        std::map<uint32_t, std::vector<DenOfIz_BindingDesc>> BindingsBySpace;
        std::vector<DenOfIz_RootConstantBindingDesc>         RootConstants;
    };

#ifndef DZ_DXC_NOT_SUPPORTED
#define DXC_CHECK_RESULT( result )                                                                                                                                                 \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        if ( FAILED( result ) )                                                                                                                                                    \
        {                                                                                                                                                                          \
            spdlog::error( "DXC Error: {}", result );                                                                                                                              \
        }                                                                                                                                                                          \
    }                                                                                                                                                                              \
    while ( false )

    struct ReflectionState
    {
        DenOfIz_InputLayoutDesc                       *InputLayoutDesc;
        std::vector<DenOfIz_LocalResourceBindingDesc> *LocalResourceBindings;
        DenOfIz_ShaderStageDesc const                 *ShaderDesc;
        DenOfIz_CompiledShaderStage                   *CompiledShader;
        ID3D12ShaderReflection                        *ShaderReflection;
        ID3D12LibraryReflection                       *LibraryReflection;
        ID3D12FunctionReflection                      *FunctionReflection;
        int                                            ShaderIndex;
    };
#endif

    class ShaderProgram
    {
    public:
        // Member variables
#ifndef DZ_DXC_NOT_SUPPORTED
        DenOfIz_ShaderCompiler m_compiler = DENOFIZ_NULL_HANDLE;
#endif
        std::vector<std::unique_ptr<DenOfIz_CompiledShaderStage>>  m_compiledShaders;
        std::vector<DenOfIz_CompiledShaderStage *>                 m_compiledShaderPtrs;
        std::vector<DenOfIz_ShaderStageDesc>                       m_shaderDescs; // Index matched with m_compiledShaders
        DenOfIz_ShaderReflectDesc                                  m_reflectDesc;
        DenOfIz_ShaderProgramDesc                                  m_desc;
        std::vector<DenOfIz_ByteArray>                             m_dataToClean;
        BindGroupLayoutState                                       m_bindGroupLayoutState{ };
        std::vector<DenOfIz_BindGroupLayoutDesc>                   m_bindGroupLayoutDescs;
        std::vector<std::vector<DenOfIz_BindingDesc>>              m_bindingsBySpace;
        std::vector<DenOfIz_ResourceBindingSlotArray>              m_localBindingsToClean;
        std::vector<std::vector<DenOfIz_LocalResourceBindingDesc>> m_localResourceBindings;
        std::vector<DenOfIz_LocalRootSignatureDesc>                m_localRootSignatures;
        std::vector<DenOfIz_ThreadGroupDesc>                       m_threadGroupInfos;
        uint32_t                                                   m_semanticStringIndex = 0;
        StringStore                                                m_stringStore;
        uint32_t                                                   m_bindingNameIndex = 0;
        std::vector<DenOfIz_InputGroupDesc>                        m_inputGroups;
        std::vector<std::vector<DenOfIz_InputLayoutElementDesc>>   m_inputElements;
        DenOfIz_ShaderAsset                                        m_shaderAsset = DENOFIZ_NULL_HANDLE;
        std::unique_ptr<DenOfIz_CompiledShader>                    m_shaderAssetCompiled{ };

        std::vector<DenOfIz_ShaderStageDesc>                  m_copiedShaderStages;
        std::vector<std::vector<DenOfIz_StringView>>          m_copiedDefines;
        std::vector<std::vector<DenOfIz_ResourceBindingSlot>> m_copiedLocalBindings;
        std::vector<std::vector<DenOfIz_BindlessSlot>>        m_copiedBindlessArrays;

        ShaderProgram( const DenOfIz_ShaderProgramDesc &desc );
        ShaderProgram( DenOfIz_ShaderAssetReader reader );
        ~ShaderProgram( );

        DenOfIz_CompiledShaderStageArray CompiledShaders( );
        const DenOfIz_ShaderReflectDesc &Reflect( ) const;
        DenOfIz_ShaderProgramDesc       &Desc( );

        void DeepCopyDesc( const DenOfIz_ShaderProgramDesc &desc );
        void Compile( );
        void CreateReflectionData( );
        void ProcessBindlessArrays( );
        void FinalizeBindGroupLayouts( );

#ifndef DZ_DXC_NOT_SUPPORTED
        void InitInputLayout( ID3D12ShaderReflection *shaderReflection, DenOfIz_InputLayoutDesc &inputLayoutDesc, const D3D12_SHADER_DESC &shaderDesc );
        void ReflectShader( ReflectionState &state );
        void ReflectLibrary( ReflectionState &state );
        void ProcessInputBindingDesc( ReflectionState &state, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc, int resourceIndex );
        bool UpdateBoundResourceStage( const ReflectionState &state, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc );
#endif
    };

} // namespace DenOfIz

ShaderProgram::ShaderProgram( const DenOfIz_ShaderProgramDesc &desc ) : m_reflectDesc( { } ), m_desc( { } )
{
    DeepCopyDesc( desc );
#ifndef DZ_DXC_NOT_SUPPORTED
    m_compiler = DenOfIz_ShaderCompiler_Create( );
#endif
    Compile( );
    CreateReflectionData( );
}

void ShaderProgram::DeepCopyDesc( const DenOfIz_ShaderProgramDesc &desc )
{
    m_desc.RayTracing     = desc.RayTracing;
    m_desc.IncludeHandler = desc.IncludeHandler;

    const size_t numStages = desc.ShaderStages.NumElements;
    m_copiedShaderStages.resize( numStages );
    m_copiedDefines.resize( numStages );
    m_copiedLocalBindings.resize( numStages );
    m_copiedBindlessArrays.resize( numStages );

    for ( size_t i = 0; i < numStages; ++i )
    {
        const DenOfIz_ShaderStageDesc &srcStage = desc.ShaderStages.Elements[ i ];
        DenOfIz_ShaderStageDesc       &dstStage = m_copiedShaderStages[ i ];

        dstStage.Stage    = srcStage.Stage;
        dstStage.CodePage = srcStage.CodePage;

        if ( srcStage.Path.NumChars > 0 && srcStage.Path.Chars != nullptr )
        {
            dstStage.Path = m_stringStore.Store( srcStage.Path.Chars, srcStage.Path.NumChars );
        }
        else
        {
            dstStage.Path = { nullptr, 0 };
        }

        if ( srcStage.EntryPoint.NumChars > 0 && srcStage.EntryPoint.Chars != nullptr )
        {
            dstStage.EntryPoint = m_stringStore.Store( srcStage.EntryPoint.Chars, srcStage.EntryPoint.NumChars );
        }
        else
        {
            dstStage.EntryPoint = { nullptr, 0 };
        }

        if ( srcStage.Data.NumElements > 0 && srcStage.Data.Elements != nullptr )
        {
            DenOfIz_ByteArray copiedData{ };
            copiedData.NumElements = srcStage.Data.NumElements;
            copiedData.Elements    = static_cast<Byte *>( std::malloc( copiedData.NumElements ) );
            std::memcpy( copiedData.Elements, srcStage.Data.Elements, copiedData.NumElements );
            dstStage.Data = copiedData;
            m_dataToClean.push_back( copiedData );
        }
        else
        {
            dstStage.Data = { nullptr, 0 };
        }

        if ( srcStage.Defines.NumElements > 0 && srcStage.Defines.Elements != nullptr )
        {
            m_copiedDefines[ i ].resize( srcStage.Defines.NumElements );
            for ( size_t j = 0; j < srcStage.Defines.NumElements; ++j )
            {
                const DenOfIz_StringView &srcDefine = srcStage.Defines.Elements[ j ];
                m_copiedDefines[ i ][ j ]           = m_stringStore.Store( srcDefine.Chars, srcDefine.NumChars );
            }
            dstStage.Defines.Elements    = m_copiedDefines[ i ].data( );
            dstStage.Defines.NumElements = m_copiedDefines[ i ].size( );
        }
        else
        {
            dstStage.Defines = { nullptr, 0 };
        }

        dstStage.RayTracing.HitGroupType = srcStage.RayTracing.HitGroupType;
        if ( srcStage.RayTracing.LocalBindings.NumElements > 0 && srcStage.RayTracing.LocalBindings.Elements != nullptr )
        {
            m_copiedLocalBindings[ i ].resize( srcStage.RayTracing.LocalBindings.NumElements );
            std::memcpy( m_copiedLocalBindings[ i ].data( ), srcStage.RayTracing.LocalBindings.Elements,
                         srcStage.RayTracing.LocalBindings.NumElements * sizeof( DenOfIz_ResourceBindingSlot ) );
            dstStage.RayTracing.LocalBindings.Elements    = m_copiedLocalBindings[ i ].data( );
            dstStage.RayTracing.LocalBindings.NumElements = m_copiedLocalBindings[ i ].size( );
        }
        else
        {
            dstStage.RayTracing.LocalBindings = { nullptr, 0 };
        }

        if ( srcStage.Bindless.BindlessArrays.NumElements > 0 && srcStage.Bindless.BindlessArrays.Elements != nullptr )
        {
            m_copiedBindlessArrays[ i ].resize( srcStage.Bindless.BindlessArrays.NumElements );
            std::memcpy( m_copiedBindlessArrays[ i ].data( ), srcStage.Bindless.BindlessArrays.Elements,
                         srcStage.Bindless.BindlessArrays.NumElements * sizeof( DenOfIz_BindlessSlot ) );
            dstStage.Bindless.BindlessArrays.Elements    = m_copiedBindlessArrays[ i ].data( );
            dstStage.Bindless.BindlessArrays.NumElements = m_copiedBindlessArrays[ i ].size( );
        }
        else
        {
            dstStage.Bindless.BindlessArrays = { nullptr, 0 };
        }
    }

    m_desc.ShaderStages.Elements    = m_copiedShaderStages.data( );
    m_desc.ShaderStages.NumElements = m_copiedShaderStages.size( );
}

ShaderProgram::ShaderProgram( DenOfIz_ShaderAssetReader reader ) : m_reflectDesc( { } ), m_desc( { } )
{
    m_shaderAsset = DenOfIz_ShaderAssetReader_Read( reader );
    for ( size_t i = 0; i < DenOfIz_ShaderAsset_NumStages( m_shaderAsset ); ++i )
    {
        const DenOfIz_ShaderStageAsset *stageAsset          = DenOfIz_ShaderAsset_GetStage( m_shaderAsset, i );
        const auto                      compiledShaderStage = new DenOfIz_CompiledShaderStage( );
        compiledShaderStage->Stage                          = stageAsset->Stage;
        compiledShaderStage->EntryPoint                     = stageAsset->EntryPoint;
        compiledShaderStage->RayTracing                     = stageAsset->RayTracing;
        compiledShaderStage->DXIL                           = stageAsset->DXIL;
        compiledShaderStage->MSL                            = stageAsset->MSL;
        compiledShaderStage->SPIRV                          = stageAsset->SPIRV;
        compiledShaderStage->WGSL                           = stageAsset->WGSL;
        compiledShaderStage->Reflection                     = stageAsset->Reflection;
        m_compiledShaders.emplace_back( std::unique_ptr<DenOfIz_CompiledShaderStage>( compiledShaderStage ) );
        m_compiledShaderPtrs.push_back( compiledShaderStage );
    }

    m_reflectDesc     = *DenOfIz_ShaderAsset_GetReflectDesc( m_shaderAsset );
    m_desc            = { };
    m_desc.RayTracing = *DenOfIz_ShaderAsset_GetRayTracing( m_shaderAsset );
}

ShaderProgram::~ShaderProgram( )
{
    for ( const auto &data : m_dataToClean )
    {
        if ( data.Elements )
        {
            free( data.Elements );
        }
    }
#ifndef DZ_DXC_NOT_SUPPORTED
    DenOfIz_ShaderCompiler_Destroy( m_compiler );
#endif
    if ( DENOFIZ_HANDLE_IS_VALID( m_shaderAsset ) )
    {
        DenOfIz_ShaderAsset_Destroy( m_shaderAsset );
    }
}

DenOfIz_CompiledShaderStageArray ShaderProgram::CompiledShaders( )
{
    DenOfIz_CompiledShaderStageArray compiledShaders{ };
    compiledShaders.NumElements = m_compiledShaders.size( );
    compiledShaders.Elements    = m_compiledShaderPtrs.data( );
    return compiledShaders;
}

const DenOfIz_ShaderReflectDesc &ShaderProgram::Reflect( ) const
{
    return m_reflectDesc;
}

DenOfIz_ShaderProgramDesc &ShaderProgram::Desc( )
{
    return m_desc;
}

/**
 * \brief Compiles the shaders targeting MSL/DXIL/SPIR-V. MSL is double compiled, first time to DXIL and reflect and provide a root signature to the second compilation.
 */
void ShaderProgram::Compile( )
{
#ifdef DZ_DXC_NOT_SUPPORTED
    spdlog::error( "ShaderProgram: Runtime shader compilation is not supported when DXC is disabled. Use pre-compiled shader assets instead." );
    return;
#else
    std::vector<DenOfIz_CompiledShaderStage *> dxilShaders;

    for ( uint32_t i = 0; i < m_desc.ShaderStages.NumElements; ++i )
    {
        const auto &stage = m_desc.ShaderStages.Elements[ i ];
        // Validate Shader
        if ( stage.Path.NumChars == 0 && stage.Data.NumElements == 0 )
        {
            spdlog::error( "Either stage.Path or stage.Data must be set for stage {} ", i );
            continue;
        }

        DenOfIz_CompileDesc compileDesc = { };
        compileDesc.Path                = stage.Path;
        compileDesc.CodePage            = stage.CodePage;
        compileDesc.Data                = stage.Data;
        compileDesc.Defines             = stage.Defines;
        compileDesc.EntryPoint          = stage.EntryPoint;
        compileDesc.Stage               = stage.Stage;
        compileDesc.TargetIL            = DENOFIZ_TARGET_IL_DXIL;
        compileDesc.IncludeHandler      = m_desc.IncludeHandler;

        DenOfIz_CompileResult compilationResult{ };
        DenOfIz_ShaderCompiler_CompileHLSL( m_compiler, &compileDesc, &compilationResult );
        auto [ dxil, reflection ] = compilationResult;

        m_dataToClean.push_back( dxil );
        m_dataToClean.push_back( reflection );

        compileDesc.TargetIL                   = DENOFIZ_TARGET_IL_SPIRV;
        compileDesc.SupportNonZeroBaseInstance = true;
        DenOfIz_CompileResult spirvCompilation{ };
        DenOfIz_ShaderCompiler_CompileHLSL( m_compiler, &compileDesc, &spirvCompilation );
        auto [ spirv, _ ] = spirvCompilation;
        m_dataToClean.push_back( spirv );

        const auto &compiledShader = m_compiledShaders.emplace_back( std::make_unique<DenOfIz_CompiledShaderStage>( ) );
        m_compiledShaderPtrs.push_back( compiledShader.get( ) );

        compiledShader->Stage      = stage.Stage;
        compiledShader->EntryPoint = stage.EntryPoint;
        compiledShader->RayTracing = stage.RayTracing;
        compiledShader->Reflection = reflection;
        compiledShader->DXIL       = dxil;
        compiledShader->SPIRV      = spirv;
        compiledShader->MSL        = { }; // Set below
        compiledShader->WGSL       = { }; // Set below

        m_shaderDescs.push_back( stage );
    }

#if ( defined( _WIN32 ) || defined( __APPLE__ ) ) && !defined( DZ_NO_METAL_SHADER_CONVERTER )
    DxilToMslDesc dxilToMslDesc{ };
    dxilToMslDesc.Shaders    = m_desc.ShaderStages;
    dxilToMslDesc.RayTracing = m_desc.RayTracing;

    dxilToMslDesc.DXILShaders.NumElements = m_compiledShaders.size( );
    dxilToMslDesc.DXILShaders.Elements =
        static_cast<DenOfIz_CompiledShaderStage **>( std::malloc( dxilToMslDesc.DXILShaders.NumElements * sizeof( DenOfIz_CompiledShaderStage * ) ) );
    for ( uint32_t i = 0; i < dxilToMslDesc.DXILShaders.NumElements; ++i )
    {
        dxilToMslDesc.DXILShaders.Elements[ i ] = m_compiledShaders[ i ].get( );
    }

    std::vector<DenOfIz_ByteArray> mslShaders( m_compiledShaders.size( ) );
    DenOfIz_ByteArrayArray         mslShadersArray{ };
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
    spdlog::warn( "MSL compilation is not supported on this platform" );
#endif

    std::vector<DenOfIz_CompiledShaderStage> wgslSpirvShaders( m_compiledShaders.size( ) );
    std::vector<DenOfIz_ByteArray>           wgslSpirvData( m_compiledShaders.size( ) );

    for ( uint32_t i = 0; i < m_desc.ShaderStages.NumElements; ++i )
    {
        const auto         &stage              = m_desc.ShaderStages.Elements[ i ];
        DenOfIz_CompileDesc compileDesc        = { };
        compileDesc.Path                       = stage.Path;
        compileDesc.CodePage                   = stage.CodePage;
        compileDesc.Data                       = stage.Data;
        compileDesc.Defines                    = stage.Defines;
        compileDesc.EntryPoint                 = stage.EntryPoint;
        compileDesc.Stage                      = stage.Stage;
        compileDesc.TargetIL                   = DENOFIZ_TARGET_IL_SPIRV;
        compileDesc.SupportNonZeroBaseInstance = false;
        compileDesc.IncludeHandler             = m_desc.IncludeHandler;

        DenOfIz_CompileResult spirvCompilation{ };
        DenOfIz_ShaderCompiler_CompileHLSL( m_compiler, &compileDesc, &spirvCompilation );
        wgslSpirvData[ i ] = spirvCompilation.Code;
        m_dataToClean.push_back( spirvCompilation.Code );

        wgslSpirvShaders[ i ].Stage      = stage.Stage;
        wgslSpirvShaders[ i ].EntryPoint = stage.EntryPoint;
        wgslSpirvShaders[ i ].SPIRV      = wgslSpirvData[ i ];
    }

    std::vector<DenOfIz_CompiledShaderStage *> wgslSpirvShaderPtrs( wgslSpirvShaders.size( ) );
    for ( uint32_t i = 0; i < wgslSpirvShaders.size( ); ++i )
    {
        wgslSpirvShaderPtrs[ i ] = &wgslSpirvShaders[ i ];
    }

    SPIRVToWGSLDesc spirvToWgslDesc{ };
    spirvToWgslDesc.Shaders                  = m_desc.ShaderStages;
    spirvToWgslDesc.SPIRVShaders.NumElements = wgslSpirvShaderPtrs.size( );
    spirvToWgslDesc.SPIRVShaders.Elements    = wgslSpirvShaderPtrs.data( );

    std::vector<DenOfIz_ByteArray> wgslShaders( m_compiledShaders.size( ) );
    DenOfIz_ByteArrayArray         wgslShadersArray{ };
    wgslShadersArray.NumElements   = wgslShaders.size( );
    wgslShadersArray.Elements      = wgslShaders.data( );
    spirvToWgslDesc.OutWGSLShaders = &wgslShadersArray;

    const SPIRVToWGSL spirvToWgsl{ };
    spirvToWgsl.Convert( spirvToWgslDesc );

    for ( uint32_t i = 0; i < wgslShaders.size( ); ++i )
    {
        m_compiledShaders[ i ]->WGSL = wgslShaders[ i ];
        m_dataToClean.push_back( wgslShaders[ i ] );
    }
#endif // DZ_DXC_NOT_SUPPORTED
}

void ShaderProgram::CreateReflectionData( )
{
#ifdef DZ_DXC_NOT_SUPPORTED
    spdlog::info( "ShaderProgram: Using pre-compiled reflection data from shader asset" );
    return;
#else
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
    m_threadGroupInfos.resize( m_compiledShaders.size( ) );

    DenOfIz_InputLayoutDesc &inputLayout = m_reflectDesc.InputLayout;

    ReflectionState reflectionState = { };
    reflectionState.InputLayoutDesc = &inputLayout;

    for ( int stageIndex = 0; stageIndex < m_compiledShaders.size( ); ++stageIndex )
    {
        auto &shader                                 = m_compiledShaders[ stageIndex ];
        reflectionState.CompiledShader               = shader.get( );
        reflectionState.ShaderDesc                   = &m_desc.ShaderStages.Elements[ stageIndex ];
        reflectionState.ShaderIndex                  = stageIndex;
        DenOfIz_LocalRootSignatureDesc &recordLayout = m_localRootSignatures[ stageIndex ];
        reflectionState.LocalResourceBindings        = &m_localResourceBindings[ stageIndex ];

        const auto reflectionBlob = shader->Reflection;
        DxcBuffer  reflectionBuffer{
             .Ptr      = reflectionBlob.Elements,
             .Size     = reflectionBlob.NumElements,
             .Encoding = 0,
        };

        ID3D12ShaderReflection  *shaderReflection  = nullptr;
        ID3D12LibraryReflection *libraryReflection = nullptr;

        const bool isRayTracingStage = ( shader->Stage & ( DENOFIZ_SHADER_STAGE_ANY_HIT_BIT | DENOFIZ_SHADER_STAGE_CLOSEST_HIT_BIT | DENOFIZ_SHADER_STAGE_CALLABLE_BIT |
                                                           DENOFIZ_SHADER_STAGE_INTERSECTION_BIT | DENOFIZ_SHADER_STAGE_RAY_GEN_BIT | DENOFIZ_SHADER_STAGE_MISS_BIT ) ) != 0;
        if ( isRayTracingStage )
        {
            DXC_CHECK_RESULT( dxcUtils->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &libraryReflection ) ) );
            reflectionState.LibraryReflection = libraryReflection;
            ReflectLibrary( reflectionState );
        }
        else
        {
            DXC_CHECK_RESULT( dxcUtils->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &shaderReflection ) ) );
            reflectionState.ShaderReflection = shaderReflection;
            ReflectShader( reflectionState );

            if ( shader->Stage & ( DENOFIZ_SHADER_STAGE_COMPUTE_BIT | DENOFIZ_SHADER_STAGE_MESH_BIT | DENOFIZ_SHADER_STAGE_TASK_BIT ) )
            {
                const DenOfIz_ThreadGroupDesc threadGroup = ShaderReflectionHelper::ExtractThreadGroupSize( shaderReflection, nullptr );
                shader->ThreadGroup                       = threadGroup;
                m_threadGroupInfos[ stageIndex ]          = threadGroup;
            }
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
    }

    ProcessBindlessArrays( );
    FinalizeBindGroupLayouts( );

#ifndef NDEBUG
    ReflectionDebugOutput::DumpReflectionInfo( m_reflectDesc );
#endif

    if ( dxcUtils )
    {
        dxcUtils->Release( );
    }

    m_reflectDesc.InputLayout.InputGroups.NumElements = m_inputGroups.size( );
    m_reflectDesc.InputLayout.InputGroups.Elements    = m_inputGroups.data( );

    m_reflectDesc.BindGroupLayouts.NumElements = m_bindGroupLayoutDescs.size( );
    m_reflectDesc.BindGroupLayouts.Elements    = m_bindGroupLayoutDescs.data( );

    m_reflectDesc.RootConstants.NumElements = m_bindGroupLayoutState.RootConstants.size( );
    m_reflectDesc.RootConstants.Elements    = m_bindGroupLayoutState.RootConstants.data( );

    m_reflectDesc.LocalRootSignatures.NumElements = m_localRootSignatures.size( );
    m_reflectDesc.LocalRootSignatures.Elements    = m_localRootSignatures.data( );
    m_reflectDesc.ThreadGroups.NumElements        = m_threadGroupInfos.size( );
    m_reflectDesc.ThreadGroups.Elements           = m_threadGroupInfos.data( );
#endif // DZ_DXC_NOT_SUPPORTED
}

#ifndef DZ_DXC_NOT_SUPPORTED
DenOfIz_Format MaskToFormat( const D3D_REGISTER_COMPONENT_TYPE componentType, const uint32_t mask )
{
    switch ( componentType )
    {
    case D3D_REGISTER_COMPONENT_UINT32:
        switch ( mask )
        {
        case 1:
            return DENOFIZ_FORMAT_R32_UINT;
        case 3:
            return DENOFIZ_FORMAT_R32G32_UINT;
        case 7:
            return DENOFIZ_FORMAT_R32G32B32_UINT;
        case 15:
            return DENOFIZ_FORMAT_R32G32B32A32_UINT;
        default:
            return DENOFIZ_FORMAT_UNDEFINED;
        }
    case D3D_REGISTER_COMPONENT_SINT32:
        switch ( mask )
        {
        case 1:
            return DENOFIZ_FORMAT_R32_SINT;
        case 3:
            return DENOFIZ_FORMAT_R32G32_SINT;
        case 7:
            return DENOFIZ_FORMAT_R32G32B32_SINT;
        case 15:
            return DENOFIZ_FORMAT_R32G32B32A32_SINT;
        default:
            return DENOFIZ_FORMAT_UNDEFINED;
        }
    case D3D_REGISTER_COMPONENT_FLOAT32:
        switch ( mask )
        {
        case 1:
            return DENOFIZ_FORMAT_R32_FLOAT;
        case 3:
            return DENOFIZ_FORMAT_R32G32_FLOAT;
        case 7:
            return DENOFIZ_FORMAT_R32G32B32_FLOAT;
        case 15:
            return DENOFIZ_FORMAT_R32G32B32A32_FLOAT;
        default:
            return DENOFIZ_FORMAT_UNDEFINED;
        }
#ifdef D3D_READY // Removed because they used to work with Directx Headers, however the dependency is now removed depending on DXC version which is a bit behind and doesn't have
                 // these values
    case D3D_REGISTER_COMPONENT_UINT16:
        switch ( mask )
        {
        case 1:
            return DENOFIZ_FORMAT_R16_UINT;
        case 3:
            return DENOFIZ_FORMAT_R16G16_UINT;
        case 15:
            return DENOFIZ_FORMAT_R16G16B16A16_UINT;
        default:
            return DENOFIZ_FORMAT_UNDEFINED;
        }
    case D3D_REGISTER_COMPONENT_SINT16:
        switch ( mask )
        {
        case 1:
            return DENOFIZ_FORMAT_R16_SINT;
        case 3:
            return DENOFIZ_FORMAT_R16G16_SINT;
        case 15:
            return DENOFIZ_FORMAT_R16G16B16A16_SINT;
        default:
            return DENOFIZ_FORMAT_UNDEFINED;
        }
    case D3D_REGISTER_COMPONENT_FLOAT16:
        switch ( mask )
        {
        case 1:
            return DENOFIZ_FORMAT_R16_FLOAT;
        case 3:
            return DENOFIZ_FORMAT_R16G16_FLOAT;
        case 15:
            return DENOFIZ_FORMAT_R16G16B16A16_FLOAT;
        default:
            return DENOFIZ_FORMAT_UNDEFINED;
        }
#endif
    default:
        return DENOFIZ_FORMAT_UNDEFINED;
    }
}

void ShaderProgram::ReflectShader( ReflectionState &state )
{
    ID3D12ShaderReflection *shaderReflection = state.ShaderReflection;

    D3D12_SHADER_DESC shaderDesc{ };
    DXC_CHECK_RESULT( shaderReflection->GetDesc( &shaderDesc ) );

    if ( state.ShaderDesc->Stage == DENOFIZ_SHADER_STAGE_VERTEX_BIT )
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

void ShaderProgram::ReflectLibrary( ReflectionState &state )
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
            std::string      entryPoint( state.CompiledShader->EntryPoint.Chars, state.CompiledShader->EntryPoint.NumChars );
            if ( !isMangled && functionDesc.Name != entryPoint )
            {
                continue;
            }
            if ( const size_t nameEnd = mangledName.find( '@' ); nameEnd != std::string_view::npos && isMangled )
            {
                if ( const std::string_view demangledName = mangledName.substr( 2, nameEnd - 2 ); demangledName != entryPoint )
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

void ShaderProgram::ProcessInputBindingDesc( ReflectionState &state, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc, const int resourceIndex )
{
    if ( UpdateBoundResourceStage( state, shaderInputBindDesc ) )
    {
        return;
    }
    const bool                        isLocal     = ShaderReflectionHelper::IsBindingLocalTo( state.ShaderDesc->RayTracing, shaderInputBindDesc );
    const DenOfIz_ResourceBindingType bindingType = DxcEnumConverter::ReflectTypeToBufferBindingType( shaderInputBindDesc.Type );
    if ( shaderInputBindDesc.Space == DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE && !isLocal )
    {
        DenOfIz_RootConstantBindingDesc &rootConstantBinding = m_bindGroupLayoutState.RootConstants.emplace_back( DenOfIz_RootConstantBindingDesc{ } );
        rootConstantBinding.Name                             = m_stringStore.Store( shaderInputBindDesc.Name );
        rootConstantBinding.Binding                          = shaderInputBindDesc.BindPoint;
        rootConstantBinding.Stages                           = state.ShaderDesc->Stage;
        rootConstantBinding.NumBytes                         = ShaderReflectionHelper::GetConstantBufferSize( state.ShaderReflection, state.FunctionReflection, resourceIndex );
        return;
    }

    const bool isBindless      = ShaderReflectionHelper::IsBindingBindless( state.ShaderDesc->Bindless, shaderInputBindDesc );
    bool       isBindlessArray = false;
    for ( uint32_t i = 0; i < state.ShaderDesc->Bindless.BindlessArrays.NumElements; ++i )
    {
        const auto                       &bindlessSlot     = state.ShaderDesc->Bindless.BindlessArrays.Elements[ i ];
        const DenOfIz_ResourceBindingType bindlessSlotType = DenOfIz_ResourceBindingType_FromDescriptor( bindlessSlot.Descriptor );
        if ( bindlessSlot.Binding == shaderInputBindDesc.BindPoint && bindlessSlot.RegisterSpace == shaderInputBindDesc.Space &&
             bindlessSlotType == DxcEnumConverter::ReflectTypeToBufferBindingType( shaderInputBindDesc.Type ) )
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

    // If this register space is configured to be a LocalRootSignature, then populate the corresponding Bindings.
    if ( isLocal )
    {
        DenOfIz_LocalResourceBindingDesc &localBindingDesc = state.LocalResourceBindings->emplace_back( );
        localBindingDesc.Binding                           = shaderInputBindDesc.BindPoint;
        localBindingDesc.RegisterSpace                     = shaderInputBindDesc.Space;
        localBindingDesc.ArraySize                         = isBindless ? UINT_MAX : shaderInputBindDesc.BindCount;
        localBindingDesc.Descriptor                        = DxcEnumConverter::ReflectTypeToRootSignatureType( shaderInputBindDesc.Type, shaderInputBindDesc.Dimension );
        localBindingDesc.Stages                            = state.ShaderDesc->Stage;
        localBindingDesc.NumBytes                          = 0;
        if ( shaderInputBindDesc.Type == D3D_SIT_CBUFFER )
        {
            localBindingDesc.NumBytes = ShaderReflectionHelper::GetConstantBufferSize( state.ShaderReflection, state.FunctionReflection, resourceIndex );
        }
    }
    else
    {
        auto                &spaceBindings       = m_bindGroupLayoutState.BindingsBySpace[ shaderInputBindDesc.Space ];
        DenOfIz_BindingDesc &resourceBindingDesc = spaceBindings.emplace_back( );
        resourceBindingDesc.Binding              = shaderInputBindDesc.BindPoint;
        resourceBindingDesc.ArraySize            = isBindless ? UINT_MAX : shaderInputBindDesc.BindCount;
        resourceBindingDesc.Descriptor           = DxcEnumConverter::ReflectTypeToRootSignatureType( shaderInputBindDesc.Type, shaderInputBindDesc.Dimension );
        resourceBindingDesc.Stages               = state.ShaderDesc->Stage;
    }
}

bool ShaderProgram::UpdateBoundResourceStage( const ReflectionState &state, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc )
{
    const DenOfIz_ResourceBindingType bindingType = DxcEnumConverter::ReflectTypeToBufferBindingType( shaderInputBindDesc.Type );
    bool                              found       = false;

    if ( shaderInputBindDesc.Space == DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE )
    {
        for ( auto &boundBinding : m_bindGroupLayoutState.RootConstants )
        {
            if ( boundBinding.Binding == shaderInputBindDesc.BindPoint )
            {
                found = true;
                boundBinding.Stages |= state.ShaderDesc->Stage;
                break;
            }
        }
        return found;
    }

    auto it = m_bindGroupLayoutState.BindingsBySpace.find( shaderInputBindDesc.Space );
    if ( it != m_bindGroupLayoutState.BindingsBySpace.end( ) )
    {
        for ( auto &boundBinding : it->second )
        {
            const DenOfIz_ResourceBindingType boundBindingType = DenOfIz_ResourceBindingType_FromDescriptor( boundBinding.Descriptor );
            bool                              isSameBinding    = boundBinding.Binding == shaderInputBindDesc.BindPoint;
            isSameBinding                                      = isSameBinding && boundBindingType == bindingType;
            if ( isSameBinding )
            {
                found = true;
                boundBinding.Stages |= state.ShaderDesc->Stage;
                break;
            }
        }
    }
    return found;
}

void ShaderProgram::InitInputLayout( ID3D12ShaderReflection *shaderReflection, DenOfIz_InputLayoutDesc &inputLayoutDesc, const D3D12_SHADER_DESC &shaderDesc )
{
    constexpr D3D_NAME providedSemantics[ 8 ] = {
        D3D_NAME_VERTEX_ID, D3D_NAME_INSTANCE_ID,   D3D_NAME_PRIMITIVE_ID, D3D_NAME_RENDER_TARGET_ARRAY_INDEX, D3D_NAME_VIEWPORT_ARRAY_INDEX,
        D3D_NAME_VERTEX_ID, D3D_NAME_CLIP_DISTANCE, D3D_NAME_CULL_DISTANCE
    };

    std::vector<DenOfIz_InputLayoutElementDesc> inputElements;
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

        DenOfIz_InputLayoutElementDesc &inputElementDesc = inputElements.emplace_back( DenOfIz_InputLayoutElementDesc{ } );
        inputElementDesc.Semantic                        = m_stringStore.Store( signatureParameterDesc.SemanticName );
        inputElementDesc.SemanticIndex                   = signatureParameterDesc.SemanticIndex;
        inputElementDesc.Format                          = MaskToFormat( signatureParameterDesc.ComponentType, signatureParameterDesc.Mask );
    }

    if ( !inputElements.empty( ) )
    {
        DenOfIz_InputGroupDesc inputGroup{ };
        inputGroup.StepRate = DENOFIZ_STEP_RATE_PER_VERTEX;

        m_inputElements.emplace_back( );
        auto &elementsVector = m_inputElements.back( );
        elementsVector       = std::move( inputElements );

        inputGroup.Elements.Elements    = elementsVector.data( );
        inputGroup.Elements.NumElements = elementsVector.size( );
        inputGroup.Stride               = 0;
        m_inputGroups.push_back( inputGroup );
    }
}

#endif // DZ_DXC_NOT_SUPPORTED

void ShaderProgram::ProcessBindlessArrays( )
{
    for ( uint32_t stageIndex = 0; stageIndex < m_desc.ShaderStages.NumElements; ++stageIndex )
    {
        const auto &shaderStage = m_desc.ShaderStages.Elements[ stageIndex ];
        for ( uint32_t i = 0; i < shaderStage.Bindless.BindlessArrays.NumElements; ++i )
        {
            const auto &bindlessSlot = shaderStage.Bindless.BindlessArrays.Elements[ i ];
            bool        found        = false;

            auto it = m_bindGroupLayoutState.BindingsBySpace.find( bindlessSlot.RegisterSpace );
            if ( it != m_bindGroupLayoutState.BindingsBySpace.end( ) )
            {
                for ( auto &existing : it->second )
                {
                    if ( existing.IsBindless && existing.Binding == bindlessSlot.Binding && existing.Descriptor == bindlessSlot.Descriptor )
                    {
                        existing.Stages |= shaderStage.Stage;
                        found = true;
                        break;
                    }
                }
            }

            if ( !found )
            {
                auto                &spaceBindings    = m_bindGroupLayoutState.BindingsBySpace[ bindlessSlot.RegisterSpace ];
                DenOfIz_BindingDesc &bindlessResource = spaceBindings.emplace_back( DenOfIz_BindingDesc{ } );
                bindlessResource.Binding              = bindlessSlot.Binding;
                bindlessResource.Descriptor           = bindlessSlot.Descriptor;
                bindlessResource.ArraySize            = bindlessSlot.MaxArraySize;
                bindlessResource.IsBindless           = true;
                bindlessResource.Stages               = shaderStage.Stage;
            }
        }
    }
}

void ShaderProgram::FinalizeBindGroupLayouts( )
{
    m_bindingsBySpace.clear( );
    m_bindGroupLayoutDescs.clear( );

    for ( auto &[ space, bindings ] : m_bindGroupLayoutState.BindingsBySpace )
    {
        m_bindingsBySpace.push_back( std::move( bindings ) );
    }

    uint32_t spaceIndex = 0;
    for ( auto &[ space, _ ] : m_bindGroupLayoutState.BindingsBySpace )
    {
        DenOfIz_BindGroupLayoutDesc layoutDesc{ };
        layoutDesc.RegisterSpace        = space;
        layoutDesc.Bindings.Elements    = m_bindingsBySpace[ spaceIndex ].data( );
        layoutDesc.Bindings.NumElements = m_bindingsBySpace[ spaceIndex ].size( );
        m_bindGroupLayoutDescs.push_back( layoutDesc );
        ++spaceIndex;
    }
}

extern "C"
{

    DenOfIz_ShaderProgram DenOfIz_ShaderProgram_Create( const DenOfIz_ShaderProgramDesc *desc )
    {
        if ( desc == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        auto *shaderProgram = new ShaderProgram( *desc );
        return DENOFIZ_TO_HANDLE( shaderProgram );
    }

    DenOfIz_ShaderProgram DenOfIz_ShaderProgram_CreateFromAsset( DenOfIz_ShaderAssetReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        auto *shaderProgram = new ShaderProgram( reader );
        return DENOFIZ_TO_HANDLE( shaderProgram );
    }

    void DenOfIz_ShaderProgram_CompiledShaders( DenOfIz_ShaderProgram shaderProgram, DenOfIz_CompiledShaderStageArray *outCompiledShaders )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderProgram ) || outCompiledShaders == NULL )
        {
            return;
        }
        *outCompiledShaders = SHADER_PROGRAM_IMPL( shaderProgram )->CompiledShaders( );
    }

    void DenOfIz_ShaderProgram_Reflect( DenOfIz_ShaderProgram shaderProgram, DenOfIz_ShaderReflectDesc *outReflectDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderProgram ) || outReflectDesc == NULL )
        {
            return;
        }
        const DenOfIz_ShaderReflectDesc &cppReflect = SHADER_PROGRAM_IMPL( shaderProgram )->Reflect( );
        outReflectDesc->InputLayout                 = cppReflect.InputLayout;
        outReflectDesc->BindGroupLayouts            = cppReflect.BindGroupLayouts;
        outReflectDesc->RootConstants               = cppReflect.RootConstants;
        outReflectDesc->LocalRootSignatures         = cppReflect.LocalRootSignatures;
        outReflectDesc->ThreadGroups                = cppReflect.ThreadGroups;
    }

    void DenOfIz_ShaderProgram_Desc( DenOfIz_ShaderProgram shaderProgram, DenOfIz_ShaderProgramDesc *outDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderProgram ) || outDesc == NULL )
        {
            return;
        }
        *outDesc = SHADER_PROGRAM_IMPL( shaderProgram )->Desc( );
    }

    void DenOfIz_ShaderProgram_Destroy( DenOfIz_ShaderProgram shaderProgram )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderProgram ) )
        {
            return;
        }
        delete SHADER_PROGRAM_IMPL( shaderProgram );
    }
}
