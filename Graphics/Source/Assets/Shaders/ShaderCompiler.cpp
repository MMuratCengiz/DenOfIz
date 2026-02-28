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

#include "DenOfIzGraphics/Assets/Shaders/ShaderCompiler.h"
#include <atomic>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

#ifdef _WIN32
#include <wrl/client.h>
#include "DenOfIzGraphics/Utilities/Common_Windows.h" // Include this before to make sure NOMINMAX is defined
#else
#define __EMULATE_UUID
#include "WinAdapter.h"
#endif

#include "dxcapi.h"

using namespace DenOfIz;

namespace DenOfIz
{
    class ShaderIncludeHandler : public IDxcIncludeHandler
    {
    public:
        ShaderIncludeHandler( IDxcUtils *utils ) : m_refCount( 1 ), m_dxcUtils( utils )
        {
            if ( m_dxcUtils )
            {
                m_dxcUtils->AddRef( );
            }
        }

        virtual ~ShaderIncludeHandler( )
        {
            if ( m_dxcUtils )
            {
                m_dxcUtils->Release( );
            }
        }

        void AddFile( const std::string &path, const std::vector<Byte> &data )
        {
            m_files[ path ]            = data;
            std::string normalizedPath = NormalizePath( path );
            if ( normalizedPath != path )
            {
                m_files[ normalizedPath ] = data;
            }
        }

        HRESULT STDMETHODCALLTYPE LoadSource( _In_z_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob **ppIncludeSource ) override
        {
            *ppIncludeSource = nullptr;

            std::wstring wpath( pFilename );
            std::string  path( wpath.begin( ), wpath.end( ) );
            std::string  normalizedPath = NormalizePath( path );

            auto it = m_files.find( normalizedPath );
            if ( it == m_files.end( ) )
            {
                it = m_files.find( path );
            }

            if ( it != m_files.end( ) )
            {
                const auto &data = it->second;
                if ( m_dxcUtils )
                {
                    IDxcBlobEncoding *blob = nullptr;
                    HRESULT           hr   = m_dxcUtils->CreateBlob( data.data( ), static_cast<uint32_t>( data.size( ) ), DXC_CP_UTF8, &blob );
                    if ( SUCCEEDED( hr ) )
                    {
                        *ppIncludeSource = blob;
                    }
                    return hr;
                }
                return E_FAIL;
            }

            return E_FAIL;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void **ppvObject ) override
        {
            if ( !ppvObject )
            {
                return E_INVALIDARG;
            }
            *ppvObject = nullptr;
            if ( riid == __uuidof( IUnknown ) || riid == __uuidof( IDxcIncludeHandler ) )
            {
                *ppvObject = static_cast<IDxcIncludeHandler *>( this );
                AddRef( );
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef( ) override
        {
            return ++m_refCount;
        }

        ULONG STDMETHODCALLTYPE Release( ) override
        {
            ULONG count = --m_refCount;
            if ( count == 0 )
            {
                delete this;
            }
            return count;
        }

    private:
        std::string NormalizePath( const std::string &path )
        {
            std::string result = path;
            for ( auto &c : result )
            {
#ifdef _WIN32
                if ( c == '/' )
                {
                    c = '\\';
                }
#else
                if ( c == '\\' )
                {
                    c = '/';
                }
#endif
            }
            return result;
        }

        std::atomic<ULONG>                                 m_refCount;
        IDxcUtils                                         *m_dxcUtils;
        std::unordered_map<std::string, std::vector<Byte>> m_files;
    };

    class ShaderCompiler
    {
    public:
        IDxcLibrary        *m_dxcLibrary        = nullptr;
        IDxcCompiler3      *m_dxcCompiler       = nullptr;
        IDxcUtils          *m_dxcUtils          = nullptr;
        IDxcIncludeHandler *m_dxcIncludeHandler = nullptr;

        ShaderCompiler( );
        ~ShaderCompiler( );
        [[nodiscard]] DenOfIz_CompileResult CompileHLSL( const DenOfIz_CompileDesc &compileDesc, IDxcIncludeHandler *customIncludeHandler = nullptr ) const;
    };
} // namespace DenOfIz

#define SHADER_COMPILER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ShaderCompiler, handle )
#define INCLUDE_HANDLER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ShaderIncludeHandler, handle )

ShaderCompiler::ShaderCompiler( )
{
    HRESULT result = DxcCreateInstance( CLSID_DxcLibrary, IID_PPV_ARGS( &m_dxcLibrary ) );
    if ( FAILED( result ) )
    {
        spdlog::critical( "Failed to initialize DXC Library" );
    }

    result = DxcCreateInstance( CLSID_DxcCompiler, IID_PPV_ARGS( &m_dxcCompiler ) );
    if ( FAILED( result ) )
    {
        spdlog::critical( "Failed to initialize DXC Compiler" );
    }

    result = DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( &m_dxcUtils ) );
    if ( FAILED( result ) )
    {
        spdlog::critical( "Failed to initialize DXC Utils" );
    }
    result = m_dxcUtils->CreateDefaultIncludeHandler( &m_dxcIncludeHandler );
    if ( FAILED( result ) )
    {
        spdlog::critical( "Failed to initialize DXC Include Handler" );
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
ShaderCompiler::~ShaderCompiler( )
{
}

DenOfIz_CompileResult ShaderCompiler::CompileHLSL( const DenOfIz_CompileDesc &compileDesc, IDxcIncludeHandler *customIncludeHandler ) const
{
    if ( compileDesc.TargetIL == DENOFIZ_TARGET_IL_MSL )
    {
        spdlog::critical(
            "MSL requires a root signature to provide an accurate metallib with the context of all shaders. Using shader reflection create an IRRootSignature and pass "
            "it to the DxilToMsl class." );
        return { };
    }

    // Attribute to reference: https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/hlsl.adoc
    // https://github.com/KhronosGroup/Vulkan-Guide
    IDxcBlobEncoding *sourceBlob = nullptr;
    HRESULT           result     = S_OK;
    uint32_t          codePage   = DXC_CP_ACP;
    switch ( compileDesc.CodePage )
    {
    case DENOFIZ_CODE_PAGE_ACP:
        codePage = DXC_CP_ACP;
        break;
    case DENOFIZ_CODE_PAGE_UTF8:
        codePage = DXC_CP_UTF8;
        break;
    case DENOFIZ_CODE_PAGE_UTF16:
        codePage = DXC_CP_UTF16;
        break;
    case DENOFIZ_CODE_PAGE_UTF32:
#ifndef DXC_CP_UTF32
#define DXC_CP_UTF32 12000 // Undefined on CI for some reason
#endif
        codePage = DXC_CP_UTF32;
        break;
    }

    if ( compileDesc.Data.NumElements > 0 )
    {
        result = m_dxcLibrary->CreateBlobWithEncodingOnHeapCopy( compileDesc.Data.Elements, compileDesc.Data.NumElements, codePage, &sourceBlob );
        if ( FAILED( result ) )
        {
            spdlog::critical( "Could not create blob from memory data, error code: {}", result );
        }
    }
    else if ( compileDesc.Path.NumChars > 0 )
    {
        std::string  appPath( compileDesc.Path.Chars, compileDesc.Path.NumChars );
        std::string  path = Utilities::AppPath( appPath.c_str( ) );
        std::wstring wsShaderPath( path.begin( ), path.end( ) );
        result = m_dxcUtils->LoadFile( wsShaderPath.c_str( ), &codePage, &sourceBlob );

        if ( FAILED( result ) )
        {
            spdlog::critical( "Could not load shader file: {} error code: {}", path, GetLastError( ) );
        }
    }
    else
    {
        spdlog::critical( "Neither Path nor Data provided for shader compilation" );
    }

    std::string hlslVersion = "6_6";
    std::string targetProfile;
    if ( compileDesc.Stage & ( DENOFIZ_SHADER_STAGE_RAY_GEN_BIT | DENOFIZ_SHADER_STAGE_ANY_HIT_BIT | DENOFIZ_SHADER_STAGE_CLOSEST_HIT_BIT | DENOFIZ_SHADER_STAGE_INTERSECTION_BIT |
                               DENOFIZ_SHADER_STAGE_MISS_BIT | DENOFIZ_SHADER_STAGE_CALLABLE_BIT ) )
    {
        targetProfile = "lib";
    }
    else if ( compileDesc.Stage == DENOFIZ_SHADER_STAGE_VERTEX_BIT )
    {
        targetProfile = "vs";
    }
    else if ( compileDesc.Stage == DENOFIZ_SHADER_STAGE_HULL_BIT )
    {
        targetProfile = "hs";
    }
    else if ( compileDesc.Stage == DENOFIZ_SHADER_STAGE_DOMAIN_BIT )
    {
        targetProfile = "ds";
    }
    else if ( compileDesc.Stage == DENOFIZ_SHADER_STAGE_GEOMETRY_BIT )
    {
        targetProfile = "gs";
    }
    else if ( compileDesc.Stage == DENOFIZ_SHADER_STAGE_PIXEL_BIT )
    {
        targetProfile = "ps";
    }
    else if ( compileDesc.Stage == DENOFIZ_SHADER_STAGE_COMPUTE_BIT )
    {
        targetProfile = "cs";
    }
    else if ( compileDesc.Stage == DENOFIZ_SHADER_STAGE_MESH_BIT )
    {
        targetProfile = "ms";
    }
    else if ( compileDesc.Stage == DENOFIZ_SHADER_STAGE_TASK_BIT )
    {
        targetProfile = "as";
    }
    else
    {
        spdlog::warn( "Invalid shader stage" );
    }
    targetProfile += "_" + hlslVersion;

    std::vector<LPCWSTR> arguments;
    std::wstring         wsShaderPath;
    if ( compileDesc.Path.NumChars > 0 )
    {
        std::string appPath( compileDesc.Path.Chars, compileDesc.Path.NumChars );
        std::string path = Utilities::AppPath( appPath.c_str( ) );
        wsShaderPath     = std::wstring( path.begin( ), path.end( ) );
    }
    else
    {
        wsShaderPath = L"memory_shader";
    }
    arguments.push_back( wsShaderPath.c_str( ) );
    // Set shader stage
    arguments.push_back( L"-T" );
    std::wstring wsTargetProfile( targetProfile.begin( ), targetProfile.end( ) );
    arguments.push_back( wsTargetProfile.c_str( ) );
    arguments.push_back( L"-Zpr" ); // Row-major packing
    arguments.push_back( L"-all_resources_bound" );
    if ( compileDesc.TargetIL == DENOFIZ_TARGET_IL_SPIRV )
    {
        arguments.push_back( L"-spirv" );
        const bool needsVulkan13 =
            compileDesc.Stage & ( DENOFIZ_SHADER_STAGE_RAY_GEN_BIT | DENOFIZ_SHADER_STAGE_MISS_BIT | DENOFIZ_SHADER_STAGE_CLOSEST_HIT_BIT | DENOFIZ_SHADER_STAGE_ANY_HIT_BIT |
                                  DENOFIZ_SHADER_STAGE_INTERSECTION_BIT | DENOFIZ_SHADER_STAGE_MESH_BIT | DENOFIZ_SHADER_STAGE_HULL_BIT );
        if ( needsVulkan13 )
        {
            arguments.push_back( L"-fspv-target-env=vulkan1.3" );
            arguments.push_back( L"-fspv-extension=SPV_KHR_ray_tracing" );
            arguments.push_back( L"-fspv-extension=SPV_KHR_ray_query" );
            arguments.push_back( L"-fspv-extension=SPV_EXT_descriptor_indexing" );
            arguments.push_back( L"-fspv-extension=SPV_EXT_shader_viewport_index_layer" );
        }
        else
        {
            arguments.push_back( L"-fspv-target-env=vulkan1.1" );
            arguments.push_back( L"-fspv-extension=SPV_EXT_descriptor_indexing" );
            arguments.push_back( L"-fspv-extension=SPV_EXT_shader_viewport_index_layer" );
        }

        arguments.push_back( L"-Wno-parameter-usage" );
        // Vulkan requires unique binding for each descriptor, hlsl has a binding per buffer view.
        // Docs suggest shifting the binding to avoid conflicts.
        static const std::wstring VkShiftCbvWs     = std::to_wstring( DENOFIZ_VK_SHIFT_CBV );
        static const std::wstring VkShiftSrvWs     = std::to_wstring( DENOFIZ_VK_SHIFT_SRV );
        static const std::wstring VkShiftUavWs     = std::to_wstring( DENOFIZ_VK_SHIFT_UAV );
        static const std::wstring VkShiftSamplerWs = std::to_wstring( DENOFIZ_VK_SHIFT_SAMPLER );

        {
            // Shift Cbv for Vk
            arguments.push_back( L"-fvk-b-shift" );
            arguments.push_back( VkShiftCbvWs.c_str( ) );
            arguments.push_back( L"all" );
        }
        {
            // Shift Srv for Vk
            arguments.push_back( L"-fvk-t-shift" );
            arguments.push_back( VkShiftSrvWs.c_str( ) );
            arguments.push_back( L"all" );
        }
        {
            // Shift Uav for Vk
            arguments.push_back( L"-fvk-u-shift" );
            arguments.push_back( VkShiftUavWs.c_str( ) );
            arguments.push_back( L"all" );
        }
        {
            // Shift Sampler for Vk
            arguments.push_back( L"-fvk-s-shift" );
            arguments.push_back( VkShiftSamplerWs.c_str( ) );
            arguments.push_back( L"all" );
        }

        arguments.push_back( L"-fvk-use-dx-position-w" );
        arguments.push_back( L"-fvk-use-dx-layout" );
        if ( compileDesc.SupportNonZeroBaseInstance )
        {
            arguments.push_back( L"-fvk-support-nonzero-base-instance" );
        }
    }

    std::vector<std::string>  defines( compileDesc.Defines.NumElements );
    std::vector<std::wstring> wdefines( compileDesc.Defines.NumElements );
    for ( auto i = 0; i < compileDesc.Defines.NumElements; ++i )
    {
        auto strView  = compileDesc.Defines.Elements[ i ];
        defines[ i ]  = std::string( strView.Chars, strView.NumChars );
        wdefines[ i ] = std::wstring( defines[ i ].begin( ), defines[ i ].end( ) );
        arguments.push_back( L"-D" );
        arguments.push_back( wdefines[ i ].c_str( ) );
    }
    arguments.push_back( L"-HV" );
    arguments.push_back( L"2021" );
#ifndef NDEBUG
    if ( compileDesc.TargetIL == DENOFIZ_TARGET_IL_DXIL ) // Use Zi only on DXIL, for SPIRV this breaks naga compatibility
    {
        arguments.push_back( L"-Zi" );
    }
#endif

    // Set the entry point, and export only the specific entry point, helps with Vulkan and Metal compatibility
    arguments.push_back( L"-E" );
    std::string  entryPoint( compileDesc.EntryPoint.Chars, compileDesc.EntryPoint.NumChars );
    std::wstring wsEntryPoint( entryPoint.begin( ), entryPoint.end( ) );
    arguments.push_back( wsEntryPoint.c_str( ) );

    if ( targetProfile.starts_with( "lib" ) )
    {
        arguments.push_back( L"-default-linkage" );
        arguments.push_back( L"external" );
        arguments.push_back( L"-export-shaders-only" );
        arguments.push_back( L"-exports" );
        arguments.push_back( wsEntryPoint.c_str( ) );
    }

    DxcBuffer buffer{ };
    buffer.Encoding = DXC_CP_ACP; // or? DXC_CP_UTF8;
    buffer.Ptr      = sourceBlob->GetBufferPointer( );
    buffer.Size     = sourceBlob->GetBufferSize( );

    IDxcResult         *dxcResult{ nullptr };
    IDxcIncludeHandler *includeHandler = customIncludeHandler ? customIncludeHandler : m_dxcIncludeHandler;
    result = m_dxcCompiler->Compile( &buffer, arguments.data( ), static_cast<uint32_t>( arguments.size( ) ), includeHandler, IID_PPV_ARGS( &dxcResult ) );

    if ( SUCCEEDED( result ) )
    {
        if ( FAILED( dxcResult->GetStatus( &result ) ) )
        {
            spdlog::warn( "Unable to get shader status" );
        }
    }

    if ( FAILED( result ) && dxcResult )
    {
        IDxcBlobEncoding *errorBlob;
        result = dxcResult->GetErrorBuffer( &errorBlob );
        if ( SUCCEEDED( result ) && errorBlob )
        {
            spdlog::error( "Shader compilation failed :\n\n {}", static_cast<const char *>( errorBlob->GetBufferPointer( ) ) );
            errorBlob->Release( );
            throw std::runtime_error( "Compilation failed" );
        }
    }

    IDxcBlob *code = nullptr;
    if ( FAILED( dxcResult->GetResult( &code ) ) )
    {
        spdlog::error( "Failed to get shader code" );
    }

    IDxcBlob *reflection = nullptr;
    if ( compileDesc.TargetIL == DENOFIZ_TARGET_IL_DXIL )
    {
        if ( FAILED( dxcResult->GetOutput( DXC_OUT_REFLECTION, IID_PPV_ARGS( &reflection ), nullptr ) ) )
        {
            spdlog::error( "Failed to get shader reflection" );
        }
    }

    dxcResult->Release( );
    sourceBlob->Release( );

    DenOfIz_ByteArray resultCode{ };
    resultCode.Elements    = static_cast<Byte *>( std::malloc( code->GetBufferSize( ) ) );
    resultCode.NumElements = code->GetBufferSize( );
    std::memcpy( resultCode.Elements, code->GetBufferPointer( ), code->GetBufferSize( ) );
    code->Release( );
    DenOfIz_ByteArray resultReflection{ .Elements = nullptr, .NumElements = 0 };
    if ( reflection )
    {
        resultReflection.Elements    = static_cast<Byte *>( std::malloc( reflection->GetBufferSize( ) ) );
        resultReflection.NumElements = reflection->GetBufferSize( );
        std::memcpy( resultReflection.Elements, reflection->GetBufferPointer( ), reflection->GetBufferSize( ) );
        reflection->Release( );
    }

    return DenOfIz_CompileResult{ .Code = resultCode, .Reflection = resultReflection };
}

extern "C"
{

    DenOfIz_ShaderIncludeHandler DenOfIz_ShaderIncludeHandler_Create( )
    {
        IDxcUtils *utils  = nullptr;
        HRESULT    result = DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( &utils ) );
        if ( FAILED( result ) )
        {
            spdlog::critical( "Failed to initialize DXC Utils for ShaderIncludeHandler" );
            return DENOFIZ_NULL_HANDLE;
        }
        auto *handler = new ShaderIncludeHandler( utils );
        utils->Release( );
        return DENOFIZ_TO_HANDLE( handler );
    }

    void DenOfIz_ShaderIncludeHandler_AddFile( DenOfIz_ShaderIncludeHandler handler, const DenOfIz_StringView *path, const DenOfIz_ByteArrayView *data )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( handler ) || path == NULL || data == NULL )
        {
            return;
        }
        std::string       pathStr( path->Chars, path->NumChars );
        std::vector<Byte> dataVec( data->Elements, data->Elements + data->NumElements );
        INCLUDE_HANDLER_IMPL( handler )->AddFile( pathStr, dataVec );
    }

    void DenOfIz_ShaderIncludeHandler_Destroy( DenOfIz_ShaderIncludeHandler handler )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( handler ) )
        {
            return;
        }
        INCLUDE_HANDLER_IMPL( handler )->Release( );
    }

    DenOfIz_ShaderCompiler DenOfIz_ShaderCompiler_Create( )
    {
        auto *compiler = new ShaderCompiler( );
        return DENOFIZ_TO_HANDLE( compiler );
    }

    void DenOfIz_ShaderCompiler_CompileHLSL( DenOfIz_ShaderCompiler compiler, const DenOfIz_CompileDesc *compileDesc, DenOfIz_CompileResult *outResult )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( compiler ) || compileDesc == NULL || outResult == NULL )
        {
            return;
        }

        DenOfIz_CompileDesc cppDesc{ };
        cppDesc.Path                       = compileDesc->Path;
        cppDesc.CodePage                   = compileDesc->CodePage;
        cppDesc.Data                       = compileDesc->Data;
        cppDesc.EntryPoint                 = compileDesc->EntryPoint;
        cppDesc.Stage                      = compileDesc->Stage;
        cppDesc.TargetIL                   = compileDesc->TargetIL;
        cppDesc.Defines                    = compileDesc->Defines;
        cppDesc.SupportNonZeroBaseInstance = compileDesc->SupportNonZeroBaseInstance;

        IDxcIncludeHandler *includeHandler = nullptr;
        if ( DENOFIZ_HANDLE_IS_VALID( compileDesc->IncludeHandler ) )
        {
            includeHandler = INCLUDE_HANDLER_IMPL( compileDesc->IncludeHandler );
        }

        DenOfIz_CompileResult cppResult = SHADER_COMPILER_IMPL( compiler )->CompileHLSL( cppDesc, includeHandler );
        outResult->Code                 = cppResult.Code;
        outResult->Reflection           = cppResult.Reflection;
    }

    void DenOfIz_ShaderCompiler_Destroy( DenOfIz_ShaderCompiler compiler )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( compiler ) )
        {
            return;
        }
        delete SHADER_COMPILER_IMPL( compiler );
    }
}
