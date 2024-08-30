#include <DenOfIzCore/Utilities.h>
#include <DenOfIzGraphics/Backends/Common/ShaderCompiler.h>
#include <fstream>

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

std::unique_ptr<CompiledShader> ShaderCompiler::CompileHLSL( const std::string &filename, const CompileOptions &compileOptions ) const
{
    // Attribute to reference: https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/hlsl.adoc
    // https://github.com/KhronosGroup/Vulkan-Guide
    std::string       path     = Utilities::AppPath( filename );
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
    switch ( compileOptions.Stage )
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
        DZ_ASSERTM( false, "Invalid shader stage" );
        break;
    }
    targetProfile += "_" + hlslVersion;

    std::vector<LPCWSTR> arguments;
    arguments.push_back( wsShaderPath.c_str( ) );
    // Set the entry point
    arguments.push_back( L"-E" );
    std::wstring wsEntryPoint( compileOptions.EntryPoint.begin( ), compileOptions.EntryPoint.end( ) );
    arguments.push_back( wsEntryPoint.c_str( ) );
    // Set shader stage
    arguments.push_back( L"-T" );
    std::wstring wsTargetProfile( targetProfile.begin( ), targetProfile.end( ) );
    arguments.push_back( wsTargetProfile.c_str( ) );
    if ( compileOptions.TargetIL == TargetIL::SPIRV )
    {
        arguments.push_back( L"-spirv" );
        // TODO !IMPROVEMENT! uncomment the below was shader reflection is added. To help support properly.
        //            arguments.push_back( L"--fvk-stage-io-order=alpha" );
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
    for ( const auto &define : compileOptions.Defines )
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
            std::cerr << "Shader compilation failed :\n\n" << static_cast<const char *>( errorBlob->GetBufferPointer( ) );
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
    if ( FAILED( dxcResult->GetOutput( DXC_OUT_REFLECTION, IID_PPV_ARGS( &reflection ), nullptr ) ) )
    {
        LOG( ERROR ) << "Failed to get shader reflection";
    }

    dxcResult->Release( );
    sourceBlob->Release( );

#ifdef BUILD_METAL
    if ( compileOptions.TargetIL == TargetIL::MSL )
    {
        IDxcBlob *metalBlob = DxilToMsl( compileOptions, code );
        code->Release( );
        code = metalBlob;
    }
#endif

    CacheCompiledShader( filename, compileOptions.TargetIL, code );

    auto *compiledShader       = new CompiledShader( );
    compiledShader->Stage      = compileOptions.Stage;
    compiledShader->Blob       = code;
    compiledShader->Reflection = reflection;
    compiledShader->EntryPoint = compileOptions.EntryPoint;
    return std::unique_ptr<CompiledShader>( compiledShader );
}

IDxcBlob *ShaderCompiler::DxilToMsl( const CompileOptions &compileOptions, IDxcBlob *code ) const
{
#ifdef BUILD_METAL
    IRCompilerSetEntryPointName( this->m_irCompiler, compileOptions.EntryPoint.c_str( ) );
    IRCompilerSetMinimumDeploymentTarget( this->m_irCompiler, IROperatingSystem_macOS, "14.0" );

    IRObject *irDxil = IRObjectCreateFromDXIL( (const uint8_t *)code->GetBufferPointer( ), code->GetBufferSize( ), IRBytecodeOwnershipNone );

    IRError  *irError = nullptr;
    IRObject *outIr   = IRCompilerAllocCompileAndLink( this->m_irCompiler, NULL, irDxil, &irError );

    if ( !outIr )
    {
        uint32_t irCode = IRErrorGetCode( irError );

        switch ( irCode )
        {
        case IRErrorCodeNoError:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeNoError";
            break;
        case IRErrorCodeShaderRequiresRootSignature:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeShaderRequiresRootSignature";
            break;
        case IRErrorCodeUnrecognizedRootSignatureDescriptor:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeUnrecognizedRootSignatureDescriptor";
            break;
        case IRErrorCodeUnrecognizedParameterTypeInRootSignature:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeUnrecognizedParameterTypeInRootSignature";
            break;
        case IRErrorCodeResourceNotReferencedByRootSignature:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeResourceNotReferencedByRootSignature";
            break;
        case IRErrorCodeShaderIncompatibleWithDualSourceBlending:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeShaderIncompatibleWithDualSourceBlending";
            break;
        case IRErrorCodeUnsupportedWaveSize:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeUnsupportedWaveSize";
            break;
        case IRErrorCodeUnsupportedInstruction:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeUnsupportedInstruction";
            break;
        case IRErrorCodeCompilationError:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeCompilationError";
            break;
        case IRErrorCodeFailedToSynthesizeStageInFunction:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeFailedToSynthesizeStageInFunction";
            break;
        case IRErrorCodeFailedToSynthesizeStreamOutFunction:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeFailedToSynthesizeStreamOutFunction";
            break;
        case IRErrorCodeFailedToSynthesizeIndirectIntersectionFunction:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeFailedToSynthesizeIndirectIntersectionFunction";
            break;
        case IRErrorCodeUnableToVerifyModule:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeUnableToVerifyModule";
            break;
        case IRErrorCodeUnableToLinkModule:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeUnableToLinkModule";
            break;
        case IRErrorCodeUnrecognizedDXILHeader:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeUnrecognizedDXILHeader";
            break;
        case IRErrorCodeInvalidRaytracingAttribute:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeInvalidRaytracingAttribute";
            break;
        case IRErrorCodeNullHullShaderInputOutputMismatch:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeNullHullShaderInputOutputMismatch";
            break;
        case IRErrorCodeUnknown:
            LOG( WARNING ) << "DXIL to MSL Error: IRErrorCodeUnknown";
            break;
        }
        LOG( ERROR ) << "Failed to compile and link DXIL to MSL: " << irCode;
        IRErrorDestroy( irError );
    }

    IRMetalLibBinary *metalLib = IRMetalLibBinaryCreate( );
    IRObjectGetMetalLibBinary( outIr, ConvertIrShaderStage( compileOptions.Stage ), metalLib );
    size_t   metalLibSize     = IRMetalLibGetBytecodeSize( metalLib );
    uint8_t *metalLibByteCode = new uint8_t[ metalLibSize ];
    IRMetalLibGetBytecode( metalLib, metalLibByteCode );

    MetalDxcBlob_Impl *mslBlob = new MetalDxcBlob_Impl( metalLibByteCode, metalLibSize );
    mslBlob->IrObject          = outIr;

    IRMetalLibBinaryDestroy( metalLib );
    IRObjectDestroy( irDxil );
    return mslBlob;
#else
    return nullptr;
#endif
}

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
