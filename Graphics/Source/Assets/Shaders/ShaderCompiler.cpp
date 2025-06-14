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
#include <fstream>
#include <ranges>
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

class ShaderCompiler::Impl
{
public:
    IDxcLibrary        *m_dxcLibrary        = nullptr;
    IDxcCompiler3      *m_dxcCompiler       = nullptr;
    IDxcUtils          *m_dxcUtils          = nullptr;
    IDxcIncludeHandler *m_dxcIncludeHandler = nullptr;

    Impl( );
    ~Impl( );
    [[nodiscard]] CompileResult CompileHLSL( const CompileDesc &compileDesc ) const;
};

ShaderCompiler::Impl::Impl( )
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
ShaderCompiler::Impl::~Impl( )
{
}

CompileResult ShaderCompiler::Impl::CompileHLSL( const CompileDesc &compileDesc ) const
{
    if ( compileDesc.TargetIL == TargetIL::MSL )
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
    case CodePage::ACP:
        codePage = DXC_CP_ACP;
        break;
    case CodePage::UTF8:
        codePage = DXC_CP_UTF8;
        break;
    case CodePage::UTF16:
        codePage = DXC_CP_UTF16;
        break;
    case CodePage::UTF32:
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
    else if ( !compileDesc.Path.IsEmpty( ) )
    {
        std::string  path = Utilities::AppPath( compileDesc.Path.Get( ) );
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
    switch ( compileDesc.Stage )
    {
    case ShaderStage::Raygen:
    case ShaderStage::AnyHit:
    case ShaderStage::ClosestHit:
    case ShaderStage::Intersection:
    case ShaderStage::Miss:
    case ShaderStage::Callable:
        targetProfile = "lib";
        break;
    case ShaderStage::Vertex:
        targetProfile = "vs";
        break;
    case ShaderStage::Hull:
        targetProfile = "hs";
        break;
    case ShaderStage::Domain:
        targetProfile = "ds";
        break;
    case ShaderStage::Geometry:
        targetProfile = "gs";
        break;
    case ShaderStage::Pixel:
        targetProfile = "ps";
        break;
    case ShaderStage::Compute:
        targetProfile = "cs";
        break;
    case ShaderStage::Mesh:
        targetProfile = "ms";
        break;
    case ShaderStage::Task:
        targetProfile = "as";
        break;
    default:
        spdlog::warn( "Invalid shader stage" );
        break;
    }
    targetProfile += "_" + hlslVersion;

    std::vector<LPCWSTR> arguments;
    std::wstring         wsShaderPath;
    if ( !compileDesc.Path.IsEmpty( ) )
    {
        std::string path = Utilities::AppPath( compileDesc.Path.Get( ) );
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
    if ( compileDesc.TargetIL == TargetIL::SPIRV )
    {
        arguments.push_back( L"-spirv" );
        arguments.push_back( L"-fspv-target-env=vulkan1.3" );
        arguments.push_back( L"-Wno-parameter-usage" );

        if ( compileDesc.Stage == ShaderStage::Raygen || compileDesc.Stage == ShaderStage::Miss || compileDesc.Stage == ShaderStage::ClosestHit ||
             compileDesc.Stage == ShaderStage::Intersection )
        {
            arguments.push_back( L"-fspv-extension=SPV_KHR_ray_tracing" );
            arguments.push_back( L"-fspv-extension=SPV_KHR_ray_query" );
        }

        // Vulkan requires unique binding for each descriptor, hlsl has a binding per buffer view.
        // Docs suggest shifting the binding to avoid conflicts.
        static const std::wstring VkShiftCbvWs     = std::to_wstring( VkShiftCbv );
        static const std::wstring VkShiftSrvWs     = std::to_wstring( VkShiftSrv );
        static const std::wstring VkShiftUavWs     = std::to_wstring( VkShiftUav );
        static const std::wstring VkShiftSamplerWs = std::to_wstring( VkShiftSampler );

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
        arguments.push_back( L"-fspv-debug=line" );
        arguments.push_back( L"-fvk-support-nonzero-base-instance" );
    }

    std::vector<std::string> defines( compileDesc.Defines.NumElements );
    for ( auto i = 0; i < compileDesc.Defines.NumElements; ++i )
    {
        auto               strView   = compileDesc.Defines.Elements[ i ];
        const std::string &defineStr = defines.emplace_back( strView.Chars, strView.Length );
        arguments.push_back( L"-D" );
        arguments.push_back( reinterpret_cast<LPCWSTR>( defineStr.c_str( ) ) );
    }
    arguments.push_back( L"-HV" );
    arguments.push_back( L"2021" );
#ifndef NDEBUG
    arguments.push_back( L"-Zi" );
#endif

    // Set the entry point, and export only the specific entry point, helps with Vulkan and Metal compatibility
    arguments.push_back( L"-E" );
    std::string  entryPoint = compileDesc.EntryPoint.Get( );
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

    IDxcResult *dxcResult{ nullptr };
    result = m_dxcCompiler->Compile( &buffer, arguments.data( ), static_cast<uint32_t>( arguments.size( ) ), m_dxcIncludeHandler, IID_PPV_ARGS( &dxcResult ) );

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
    if ( compileDesc.TargetIL == TargetIL::DXIL )
    {
        if ( FAILED( dxcResult->GetOutput( DXC_OUT_REFLECTION, IID_PPV_ARGS( &reflection ), nullptr ) ) )
        {
            spdlog::error( "Failed to get shader reflection" );
        }
    }

    dxcResult->Release( );
    sourceBlob->Release( );

    ByteArray resultCode{ };
    resultCode.Elements    = static_cast<Byte *>( std::malloc( code->GetBufferSize( ) ) );
    resultCode.NumElements = code->GetBufferSize( );
    std::memcpy( resultCode.Elements, code->GetBufferPointer( ), code->GetBufferSize( ) );
    code->Release( );
    ByteArray resultReflection{ .Elements = nullptr, .NumElements = 0 };
    if ( reflection )
    {
        resultReflection.Elements    = static_cast<Byte *>( std::malloc( reflection->GetBufferSize( ) ) );
        resultReflection.NumElements = reflection->GetBufferSize( );
        std::memcpy( resultReflection.Elements, reflection->GetBufferPointer( ), reflection->GetBufferSize( ) );
        reflection->Release( );
    }
    return { .Code = resultCode, .Reflection = resultReflection };
}

ShaderCompiler::ShaderCompiler( ) : m_pImpl( std::make_unique<Impl>( ) )
{
}

ShaderCompiler::~ShaderCompiler( ) = default;

CompileResult ShaderCompiler::CompileHLSL( const CompileDesc &compileDesc ) const
{
    return m_pImpl->CompileHLSL( compileDesc );
}
