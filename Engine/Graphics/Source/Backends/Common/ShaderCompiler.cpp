#include <DenOfIzCore/Utilities.h>
#include <DenOfIzGraphics/Backends/Common/ShaderCompiler.h>
#include <fstream>
#include <ranges>

using namespace DenOfIz;

ShaderCompiler::ShaderCompiler( )
{
    HRESULT result = DxcCreateInstance( CLSID_DxcLibrary, IID_PPV_ARGS( &m_dxcLibrary ) );
    if ( FAILED( result ) )
    {
        LOG( FATAL ) << "Failed to initialize DXC Library";
    }

    result = DxcCreateInstance( CLSID_DxcCompiler, IID_PPV_ARGS( &m_dxcCompiler ) );
    if ( FAILED( result ) )
    {
        LOG( FATAL ) << "Failed to initialize DXC Compiler";
    }

    result = DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( &m_dxcUtils ) );
    if ( FAILED( result ) )
    {
        LOG( FATAL ) << "Failed to initialize DXC Utils";
    }

#ifdef __APPLE__
    m_irCompiler = IRCompilerCreate( );
#endif
}

// ReSharper disable once CppMemberFunctionMayBeConst
ShaderCompiler::~ShaderCompiler( )
{
#ifdef __APPLE__
    IRCompilerDestroy( m_irCompiler );
#endif
}

IDxcUtils *ShaderCompiler::DxcUtils( ) const
{
    return m_dxcUtils;
}

std::unique_ptr<CompiledShader> ShaderCompiler::CompileHLSL( const CompileDesc &compileDesc ) const
{
    if ( compileDesc.TargetIL == TargetIL::MSL )
    {
        LOG( FATAL ) << "MSL requires a root signature to provide an accurate metallib with the context of all shaders. Using shader reflection create an IRRootSignature and pass "
                        "it to the DxilToMsl function. See ShaderProgram::ProduceMSL().";
        return nullptr;
    }

    // Attribute to reference: https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/hlsl.adoc
    // https://github.com/KhronosGroup/Vulkan-Guide
    std::string       path     = Utilities::AppPath( compileDesc.Path );
    uint32_t          codePage = DXC_CP_ACP;
    IDxcBlobEncoding *sourceBlob;
    std::wstring      wsShaderPath( path.begin( ), path.end( ) );
    HRESULT           result = m_dxcUtils->LoadFile( wsShaderPath.c_str( ), &codePage, &sourceBlob );
    if ( FAILED( result ) )
    {
        LOG( FATAL ) << "Could not load shader file: " << path << " error code: " << GetLastError( );
    }

    std::string hlslVersion = "6_6";
    std::string targetProfile;
    switch ( compileDesc.Stage )
    {
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
    default:
        LOG( WARNING ) << "Invalid shader stage";
        break;
    }
    targetProfile += "_" + hlslVersion;

    std::vector<LPCWSTR> arguments;
    arguments.push_back( wsShaderPath.c_str( ) );
    // Set the entry point
    arguments.push_back( L"-E" );
    std::wstring wsEntryPoint( compileDesc.EntryPoint.begin( ), compileDesc.EntryPoint.end( ) );
    arguments.push_back( wsEntryPoint.c_str( ) );
    // Set shader stage
    arguments.push_back( L"-T" );
    std::wstring wsTargetProfile( targetProfile.begin( ), targetProfile.end( ) );
    arguments.push_back( wsTargetProfile.c_str( ) );
    if ( compileDesc.TargetIL == TargetIL::SPIRV )
    {
        arguments.push_back( L"-spirv" );
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
    }
    for ( const auto &define : compileDesc.Defines )
    {
        arguments.push_back( L"-D" );
        arguments.push_back( reinterpret_cast<LPCWSTR>( define.c_str( ) ) );
    }
    arguments.push_back( L"-HV" );
    arguments.push_back( L"2021" );
#ifndef NDEBUG
    arguments.push_back( L"-Zi" );
#endif

    DxcBuffer buffer{ };
    buffer.Encoding = DXC_CP_ACP; // or? DXC_CP_UTF8;
    buffer.Ptr      = sourceBlob->GetBufferPointer( );
    buffer.Size     = sourceBlob->GetBufferSize( );

    IDxcResult *dxcResult{ nullptr };
    result = m_dxcCompiler->Compile( &buffer, arguments.data( ), static_cast<uint32_t>( arguments.size( ) ), nullptr, IID_PPV_ARGS( &dxcResult ) );

    if ( SUCCEEDED( result ) )
    {
        if ( FAILED( dxcResult->GetStatus( &result ) ) )
        {
            LOG( WARNING ) << "Unable to get shader status";
        }
    }

    if ( FAILED( result ) && dxcResult )
    {
        IDxcBlobEncoding *errorBlob;
        result = dxcResult->GetErrorBuffer( &errorBlob );
        if ( SUCCEEDED( result ) && errorBlob )
        {
            LOG( ERROR ) << "Shader compilation failed :\n\n" << static_cast<const char *>( errorBlob->GetBufferPointer( ) );
            errorBlob->Release( );
            throw std::runtime_error( "Compilation failed" );
        }
    }

    IDxcBlob *code;
    if ( FAILED( dxcResult->GetResult( &code ) ) )
    {
        LOG( ERROR ) << "Failed to get shader code";
    }

    IDxcBlob *reflection;
    if ( compileDesc.TargetIL == TargetIL::SPIRV )
    {
        // Unfortunately, seems like reflection data using SPIRV doesn't work with DXC, so we need to double compile :/
        CompileDesc compileOptionsHLSL = compileDesc;
        compileOptionsHLSL.TargetIL    = TargetIL::DXIL;
        auto hlslBlob                  = CompileHLSL( compileOptionsHLSL );
        reflection                     = std::move( hlslBlob->Reflection );
        hlslBlob->Reflection           = nullptr;
    }
    else
    {
        if ( FAILED( dxcResult->GetOutput( DXC_OUT_REFLECTION, IID_PPV_ARGS( &reflection ), nullptr ) ) )
        {
            LOG( ERROR ) << "Failed to get shader reflection";
        }
    }

    dxcResult->Release( );
    sourceBlob->Release( );

    CacheCompiledShader( compileDesc.Path, compileDesc.TargetIL, code );

    auto *compiledShader       = new CompiledShader( );
    compiledShader->Stage      = compileDesc.Stage;
    compiledShader->Blob       = code;
    compiledShader->Reflection = reflection;
    compiledShader->EntryPoint = compileDesc.EntryPoint;
    return std::unique_ptr<CompiledShader>( compiledShader );
}

#ifdef BUILD_METAL
IDxcBlob *ShaderCompiler::DxilToMsl( const CompileDesc &compileOptions, IDxcBlob *code, IRRootSignature *rootSignature ) const
{
    IRCompiler *irCompiler = IRCompilerCreate( );
    IRCompilerSetEntryPointName( irCompiler, compileOptions.EntryPoint.c_str( ) );
    IRCompilerSetMinimumDeploymentTarget( irCompiler, IROperatingSystem_macOS, "14.0" );
    IRCompilerSetGlobalRootSignature( irCompiler, rootSignature );

    IRObject *irDxil = IRObjectCreateFromDXIL( (const uint8_t *)code->GetBufferPointer( ), code->GetBufferSize( ), IRBytecodeOwnershipNone );

    IRError  *irError = nullptr;
    IRObject *outIr   = IRCompilerAllocCompileAndLink( irCompiler, NULL, irDxil, &irError );

    if ( !outIr )
    {
        LOG( ERROR ) << "Failed to compile and link DXIL to MSL, ErrorCode[" << IRErrorGetCode( irError ) << "]";
        IRErrorDestroy( irError );
    }

    IRMetalLibBinary *metalLib = IRMetalLibBinaryCreate( );
    IRObjectGetMetalLibBinary( outIr, ConvertIrShaderStage( compileOptions.Stage ), metalLib );
    size_t   metalLibSize     = IRMetalLibGetBytecodeSize( metalLib );
    uint8_t *metalLibByteCode = new uint8_t[ metalLibSize ];
    IRMetalLibGetBytecode( metalLib, metalLibByteCode );

    MetalDxcBlob_Impl *mslBlob = new MetalDxcBlob_Impl( metalLibByteCode, metalLibSize );
    mslBlob->IrObject          = outIr;

    CacheCompiledShader( compileOptions.Path, compileOptions.TargetIL, mslBlob );

    IRMetalLibBinaryDestroy( metalLib );
    IRObjectDestroy( irDxil );
    IRCompilerDestroy( irCompiler );
    return mslBlob;
}
#endif

void ShaderCompiler::CacheCompiledShader( const std::string &filename, const TargetIL &targetIL, IDxcBlob *code ) const
{
    // Cache the compiled shader into the matching binary format, it is dxil for hlsl and msl for metal,
    // Simply replace the extension with the corresponding value:
    std::string  compiledFilename = filename;
    const size_t extensionLength  = filename.size( ) - filename.find_last_of( '.' );

    if ( targetIL == TargetIL::SPIRV )
    {
        compiledFilename.replace( compiledFilename.find_last_of( '.' ), extensionLength, ".spv" );
    }
    else if ( targetIL == TargetIL::DXIL )
    {
        compiledFilename.replace( compiledFilename.find_last_of( '.' ), extensionLength, ".dxil" );
    }
    else if ( targetIL == TargetIL::MSL )
    {
        compiledFilename.replace( compiledFilename.find_last_of( '.' ), extensionLength, ".metallib" );
    }

    const std::string appPath = Utilities::AppPath( compiledFilename );

    if ( std::ofstream compiledFile( appPath, std::ios::binary ); compiledFile.is_open( ) )
    {
        compiledFile.write( static_cast<const char *>( code->GetBufferPointer( ) ), code->GetBufferSize( ) );
        compiledFile.close( );
    }
}

#ifdef BUILD_METAL
IRShaderStage ShaderCompiler::ConvertIrShaderStage( const ShaderStage &stage )
{
    switch ( stage )
    {
    case ShaderStage::Vertex:
        return IRShaderStageVertex;
    case ShaderStage::Pixel:
        return IRShaderStageFragment;
    case ShaderStage::Hull:
        return IRShaderStageHull;
    case ShaderStage::Domain:
        return IRShaderStageDomain;
    case ShaderStage::Geometry:
        return IRShaderStageGeometry;
    case ShaderStage::Compute:
        return IRShaderStageCompute;
    case ShaderStage::AllGraphics:
    case ShaderStage::All:
    case ShaderStage::Task:
        // TODO
        LOG( WARNING ) << "Unsupported metal stage `All/AllGraphics/Task`";
        return IRShaderStageInvalid;
    case ShaderStage::Raygen:
        return IRShaderStageRayGeneration;
    case ShaderStage::AnyHit:
        return IRShaderStageAnyHit;
    case ShaderStage::ClosestHit:
        return IRShaderStageClosestHit;
    case ShaderStage::Miss:
        return IRShaderStageMiss;
    case ShaderStage::Intersection:
        return IRShaderStageIntersection;
    case ShaderStage::Callable:
        return IRShaderStageCallable;
    case ShaderStage::Mesh:
        return IRShaderStageMesh;
    }
}
#endif
