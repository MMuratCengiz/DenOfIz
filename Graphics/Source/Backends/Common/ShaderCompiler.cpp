#include <DenOfIzGraphics/Backends/Common/ShaderCompiler.h>
#include <DenOfIzGraphics/Utilities/Utilities.h>
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
    result = m_dxcUtils->CreateDefaultIncludeHandler( &m_dxcIncludeHandler );
    if ( FAILED( result ) )
    {
        LOG( FATAL ) << "Failed to initialize DXC Include Handler";
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

    if ( compileDesc.EnableCaching )
    {
        if ( auto cachedBlob = LoadCachedShader( CachedShaderFile( compileDesc.Path.Get( ), compileDesc.EntryPoint.Get( ), compileDesc.TargetIL ) ) )
        {
            if ( auto cachedReflection = LoadCachedReflection( compileDesc.Path.Get( ), compileDesc.EntryPoint.Get( ) ) )
            { // We are heavily dependent on reflection data, so we need to ensure it is always available.
                auto *compiledShader       = new CompiledShader( );
                compiledShader->Path       = compileDesc.Path.Get( );
                compiledShader->Stage      = compileDesc.Stage;
                compiledShader->Blob       = std::move( cachedBlob );
                compiledShader->Reflection = std::move( cachedReflection );
                compiledShader->EntryPoint = compileDesc.EntryPoint.Get( );
                compiledShader->RayTracing = compileDesc.RayTracing;
                return std::unique_ptr<CompiledShader>( compiledShader );
            }
        }
    }

    // Attribute to reference: https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/hlsl.adoc
    // https://github.com/KhronosGroup/Vulkan-Guide
    std::string       path     = Utilities::AppPath( compileDesc.Path.Get( ) );
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
    case ShaderStage::Raygen:
    case ShaderStage::AnyHit:
    case ShaderStage::ClosestHit:
    case ShaderStage::Intersection:
    case ShaderStage::Miss:
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
    default:
        LOG( WARNING ) << "Invalid shader stage";
        break;
    }
    targetProfile += "_" + hlslVersion;

    std::vector<LPCWSTR> arguments;
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
    }
    for ( int i = 0; i < compileDesc.Defines.NumElements( ); ++i )
    {
        const auto &define = compileDesc.Defines.GetElement( i );
        arguments.push_back( L"-D" );
        arguments.push_back( reinterpret_cast<LPCWSTR>( define.Get( ) ) );
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
    arguments.push_back( L"-default-linkage" );
    arguments.push_back( L"external" );
    arguments.push_back( L"-export-shaders-only" );
    arguments.push_back( L"-exports" );
    arguments.push_back( wsEntryPoint.c_str( ) );

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

    CacheCompiledShader( compileDesc.Path.Get( ), compileDesc.EntryPoint.Get( ), compileDesc.TargetIL, code, reflection );

    auto *compiledShader       = new CompiledShader( );
    compiledShader->Path       = compileDesc.Path.Get( );
    compiledShader->Stage      = compileDesc.Stage;
    compiledShader->Blob       = code;
    compiledShader->Reflection = reflection;
    compiledShader->EntryPoint = compileDesc.EntryPoint.Get( );
    compiledShader->RayTracing = compileDesc.RayTracing;
    return std::unique_ptr<CompiledShader>( compiledShader );
}

#ifdef BUILD_METAL
IDxcBlob *ShaderCompiler::DxilToMsl( const CompileDesc &compileOptions, IDxcBlob *code, const CompileMslDesc &compileMslDesc ) const
{
    IRRootSignature *rootSignature  = compileMslDesc.RootSignature;
    IRRootSignature *localSignature = compileMslDesc.LocalRootSignature;

    IRCompiler *irCompiler = IRCompilerCreate( );
    IRCompilerSetEntryPointName( irCompiler, compileOptions.EntryPoint.Get( ) );
    IRCompilerSetMinimumDeploymentTarget( irCompiler, IROperatingSystem_macOS, "15.1" );
    IRCompilerSetGlobalRootSignature( irCompiler, rootSignature );
    IRCompilerSetLocalRootSignature( irCompiler, localSignature );
    // TODO some of these values are hardcoded because metal is odd. The only possible way I can think of is to move the compilation to pipeline creation.
    // But will try this way for now.
    IRCompilerSetRayTracingPipelineArguments( irCompiler, compileMslDesc.RayTracing.MaxNumAttributeBytes, IRRaytracingPipelineFlagNone, IRIntrinsicMaskClosestHitAll,
                                              IRIntrinsicMaskMissShaderAll, IRIntrinsicMaskAnyHitShaderAll, 255, compileMslDesc.RayTracing.MaxRecursionDepth,
                                              IRRayGenerationCompilationVisibleFunction );

    IRObject *irDxil = IRObjectCreateFromDXIL( (const uint8_t *)code->GetBufferPointer( ), code->GetBufferSize( ), IRBytecodeOwnershipNone );

    IRError  *irError = nullptr;
    IRObject *outIr   = IRCompilerAllocCompileAndLink( irCompiler, compileOptions.EntryPoint.Get( ), irDxil, &irError );

    if ( !outIr )
    {
        LOG( ERROR ) << "Failed to compile and link DXIL to MSL, ErrorCode[" << IRErrorGetCode( irError ) << "]";
        IRErrorDestroy( irError );
    }

    IRMetalLibBinary *metalLib = IRMetalLibBinaryCreate( );
    IRShaderStage     stage    = IRObjectGetMetalIRShaderStage( outIr );
    IRObjectGetMetalLibBinary( outIr, stage, metalLib );
    size_t   metalLibSize     = IRMetalLibGetBytecodeSize( metalLib );
    uint8_t *metalLibByteCode = new uint8_t[ metalLibSize ];
    IRMetalLibGetBytecode( metalLib, metalLibByteCode );

    MetalDxcBlob_Impl *mslBlob = new MetalDxcBlob_Impl( metalLibByteCode, metalLibSize );
    mslBlob->IrObject          = outIr;

    CacheCompiledShader( compileOptions.Path.Get( ), compileOptions.EntryPoint.Get( ), compileOptions.TargetIL, mslBlob, nullptr );

    IRMetalLibBinaryDestroy( metalLib );
    IRObjectDestroy( irDxil );
    IRCompilerDestroy( irCompiler );
    return mslBlob;
}
#endif

void ShaderCompiler::CacheCompiledShader( const std::string &filename, const std::string &entryPoint, const TargetIL &targetIL, IDxcBlob *code, IDxcBlob *reflection ) const
{
    // Cache the compiled shader into the matching binary format, it is dxil for hlsl and msl for metal,
    // Simply replace the extension with the corresponding value:
    const std::string appPath = Utilities::AppPath( CachedShaderFile( filename, entryPoint, targetIL ) );
    if ( std::ofstream compiledFile( appPath, std::ios::binary ); compiledFile.is_open( ) )
    {
        compiledFile.write( static_cast<const char *>( code->GetBufferPointer( ) ), code->GetBufferSize( ) );
        compiledFile.close( );
    }

    if ( reflection )
    {
        const std::string reflectionPath = Utilities::AppPath( CachedReflectionFile( filename, entryPoint ) );
        if ( std::ofstream reflectionFile( reflectionPath, std::ios::binary ); reflectionFile.is_open( ) )
        {
            reflectionFile.write( static_cast<const char *>( reflection->GetBufferPointer( ) ), reflection->GetBufferSize( ) );
            reflectionFile.close( );
        }
    }
}

std::string ShaderCompiler::CachedShaderFile( const std::string &filename, const std::string &entryPoint, const TargetIL &targetIL ) const
{
    std::string compiledFilename = filename.substr( 0, filename.find_last_of( '.' ) ) + "-" + entryPoint;
    if ( targetIL == TargetIL::SPIRV )
    {
        compiledFilename += ".spv";
    }
    else if ( targetIL == TargetIL::DXIL )
    {
        compiledFilename += ".dxil";
    }
    else if ( targetIL == TargetIL::MSL )
    {
        compiledFilename += ".metallib";
    }
    return Utilities::AppPath( compiledFilename );
}

std::string ShaderCompiler::CachedReflectionFile( const std::string &filename, const std::string &entryPoint ) const
{
    const std::string baseFilename = filename.substr( 0, filename.find_last_of( '.' ) );
    return baseFilename + "-" + entryPoint + ".reflection";
}

[[nodiscard]] IDxcBlob *ShaderCompiler::LoadCachedShader( const std::string &filename ) const
{
    if ( std::ifstream compiledFile( filename, std::ios::binary ); compiledFile.is_open( ) )
    {
        compiledFile.seekg( 0, std::ios::end );
        const size_t fileSize = compiledFile.tellg( );
        compiledFile.seekg( 0, std::ios::beg );
        std::vector<char> fileData( fileSize );
        compiledFile.read( fileData.data( ), fileSize );
        compiledFile.close( );
        IDxcBlobEncoding *blob = nullptr;

        HRESULT result = m_dxcLibrary->CreateBlobWithEncodingOnHeapCopy( fileData.data( ), fileSize, DXC_CP_ACP, &blob );
        if ( FAILED( result ) )
        {
            LOG( ERROR ) << "Failed to create blob from file";
        }
        return blob;
    }
    return nullptr;
}

[[nodiscard]] IDxcBlob *ShaderCompiler::LoadCachedReflection( const std::string &filename, const std::string &entryPoint ) const
{
    return LoadCachedShader( CachedReflectionFile( filename, entryPoint ) );
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

MetalDxcBlob_Impl::MetalDxcBlob_Impl( uint8_t *data, const size_t size ) : m_data( data ), m_size( size )
{
    m_refCount = 1;
}

HRESULT MetalDxcBlob_Impl::QueryInterface( const IID &riid, void **ppvObject )
{
    return E_NOINTERFACE;
}

ULONG MetalDxcBlob_Impl::AddRef( )
{
    return ++m_refCount;
}
ULONG MetalDxcBlob_Impl::Release( )
{
    if ( --m_refCount == 0 )
    {
#ifdef BUILD_METAL
        if ( IrObject )
        {
            IRObjectDestroy( IrObject );
        }
#endif
        delete[] m_data;
        delete this;
    }
    return m_refCount;
}

LPVOID MetalDxcBlob_Impl::GetBufferPointer( )
{
    return m_data;
}

SIZE_T MetalDxcBlob_Impl::GetBufferSize( )
{
    return m_size;
}
#endif
