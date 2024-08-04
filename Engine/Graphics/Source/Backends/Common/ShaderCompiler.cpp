#include <DenOfIzCore/Utilities.h>
#include <DenOfIzGraphics/Backends/Common/ShaderCompiler.h>

namespace DenOfIz
{

    bool ShaderCompiler::Init( )
    {
        glslang::InitializeProcess( );

        HRESULT result = DxcCreateInstance( CLSID_DxcLibrary, IID_PPV_ARGS( &m_dxcLibrary ) );
        if ( FAILED( result ) )
        {
            LOG( FATAL ) << "Failed to initialize DXC Library";
            return false;
        }

        result = DxcCreateInstance( CLSID_DxcCompiler, IID_PPV_ARGS( &m_dxcCompiler ) );
        if ( FAILED( result ) )
        {
            LOG( FATAL ) << "Failed to initialize DXC Compiler";
            return false;
        }

        result = DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( &m_dxcUtils ) );
        if ( FAILED( result ) )
        {
            LOG( FATAL ) << "Failed to initialize DXC Utils";
            return false;
        }

        return true;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void ShaderCompiler::Destroy( )
    {
        m_dxcUtils->Release( );
        m_dxcCompiler->Release( );
        m_dxcLibrary->Release( );

        glslang::FinalizeProcess( );
#ifdef __APPLE__
        IRCompilerDestroy( mp_compiler );
#endif
    }

    void ShaderCompiler::InitResources( TBuiltInResource &Resources ) const
    {
        Resources.maxLights                                   = 32;
        Resources.maxClipPlanes                               = 6;
        Resources.maxTextureUnits                             = 32;
        Resources.maxTextureCoords                            = 32;
        Resources.maxVertexAttribs                            = 64;
        Resources.maxVertexUniformComponents                  = 4096;
        Resources.maxVaryingFloats                            = 64;
        Resources.maxVertexTextureImageUnits                  = 32;
        Resources.maxCombinedTextureImageUnits                = 80;
        Resources.maxTextureImageUnits                        = 32;
        Resources.maxFragmentUniformComponents                = 4096;
        Resources.maxDrawBuffers                              = 32;
        Resources.maxVertexUniformVectors                     = 128;
        Resources.maxVaryingVectors                           = 8;
        Resources.maxFragmentUniformVectors                   = 16;
        Resources.maxVertexOutputVectors                      = 16;
        Resources.maxFragmentInputVectors                     = 15;
        Resources.minProgramTexelOffset                       = -8;
        Resources.maxProgramTexelOffset                       = 7;
        Resources.maxClipDistances                            = 8;
        Resources.maxComputeWorkGroupCountX                   = 65535;
        Resources.maxComputeWorkGroupCountY                   = 65535;
        Resources.maxComputeWorkGroupCountZ                   = 65535;
        Resources.maxComputeWorkGroupSizeX                    = 1024;
        Resources.maxComputeWorkGroupSizeY                    = 1024;
        Resources.maxComputeWorkGroupSizeZ                    = 64;
        Resources.maxComputeUniformComponents                 = 1024;
        Resources.maxComputeTextureImageUnits                 = 16;
        Resources.maxComputeImageUniforms                     = 8;
        Resources.maxComputeAtomicCounters                    = 8;
        Resources.maxComputeAtomicCounterBuffers              = 1;
        Resources.maxVaryingComponents                        = 60;
        Resources.maxVertexOutputComponents                   = 64;
        Resources.maxGeometryInputComponents                  = 64;
        Resources.maxGeometryOutputComponents                 = 128;
        Resources.maxFragmentInputComponents                  = 128;
        Resources.maxImageUnits                               = 8;
        Resources.maxCombinedImageUnitsAndFragmentOutputs     = 8;
        Resources.maxCombinedShaderOutputResources            = 8;
        Resources.maxImageSamples                             = 0;
        Resources.maxVertexImageUniforms                      = 0;
        Resources.maxTessControlImageUniforms                 = 0;
        Resources.maxTessEvaluationImageUniforms              = 0;
        Resources.maxGeometryImageUniforms                    = 0;
        Resources.maxFragmentImageUniforms                    = 8;
        Resources.maxCombinedImageUniforms                    = 8;
        Resources.maxGeometryTextureImageUnits                = 16;
        Resources.maxGeometryOutputVertices                   = 256;
        Resources.maxGeometryTotalOutputComponents            = 1024;
        Resources.maxGeometryUniformComponents                = 1024;
        Resources.maxGeometryVaryingComponents                = 64;
        Resources.maxTessControlInputComponents               = 128;
        Resources.maxTessControlOutputComponents              = 128;
        Resources.maxTessControlTextureImageUnits             = 16;
        Resources.maxTessControlUniformComponents             = 1024;
        Resources.maxTessControlTotalOutputComponents         = 4096;
        Resources.maxTessEvaluationInputComponents            = 128;
        Resources.maxTessEvaluationOutputComponents           = 128;
        Resources.maxTessEvaluationTextureImageUnits          = 16;
        Resources.maxTessEvaluationUniformComponents          = 1024;
        Resources.maxTessPatchComponents                      = 120;
        Resources.maxPatchVertices                            = 32;
        Resources.maxTessGenLevel                             = 64;
        Resources.maxViewports                                = 16;
        Resources.maxVertexAtomicCounters                     = 0;
        Resources.maxTessControlAtomicCounters                = 0;
        Resources.maxTessEvaluationAtomicCounters             = 0;
        Resources.maxGeometryAtomicCounters                   = 0;
        Resources.maxFragmentAtomicCounters                   = 8;
        Resources.maxCombinedAtomicCounters                   = 8;
        Resources.maxAtomicCounterBindings                    = 1;
        Resources.maxVertexAtomicCounterBuffers               = 0;
        Resources.maxTessControlAtomicCounterBuffers          = 0;
        Resources.maxTessEvaluationAtomicCounterBuffers       = 0;
        Resources.maxGeometryAtomicCounterBuffers             = 0;
        Resources.maxFragmentAtomicCounterBuffers             = 1;
        Resources.maxCombinedAtomicCounterBuffers             = 1;
        Resources.maxAtomicCounterBufferSize                  = 16384;
        Resources.maxTransformFeedbackBuffers                 = 4;
        Resources.maxTransformFeedbackInterleavedComponents   = 64;
        Resources.maxCullDistances                            = 8;
        Resources.maxCombinedClipAndCullDistances             = 8;
        Resources.maxSamples                                  = 4;
        Resources.maxMeshOutputVerticesNV                     = 256;
        Resources.maxMeshOutputPrimitivesNV                   = 512;
        Resources.maxMeshWorkGroupSizeX_NV                    = 32;
        Resources.maxMeshWorkGroupSizeY_NV                    = 1;
        Resources.maxMeshWorkGroupSizeZ_NV                    = 1;
        Resources.maxTaskWorkGroupSizeX_NV                    = 32;
        Resources.maxTaskWorkGroupSizeY_NV                    = 1;
        Resources.maxTaskWorkGroupSizeZ_NV                    = 1;
        Resources.maxMeshViewCountNV                          = 4;
        Resources.limits.nonInductiveForLoops                 = true;
        Resources.limits.whileLoops                           = true;
        Resources.limits.doWhileLoops                         = true;
        Resources.limits.generalUniformIndexing               = true;
        Resources.limits.generalAttributeMatrixVectorIndexing = true;
        Resources.limits.generalVaryingIndexing               = true;
        Resources.limits.generalSamplerIndexing               = true;
        Resources.limits.generalVariableIndexing              = true;
        Resources.limits.generalConstantMatrixVectorIndexing  = true;
    }

    EShLanguage ShaderCompiler::FindLanguage( const ShaderStage shader_type ) const
    {
        switch ( shader_type )
        {
        case ShaderStage::Vertex:
            return EShLangVertex;
        case ShaderStage::Hull:
            return EShLangTessControl;
        case ShaderStage::Domain:
            return EShLangTessEvaluation;
        case ShaderStage::Geometry:
            return EShLangGeometry;
        case ShaderStage::Pixel:
            return EShLangFragment;
        case ShaderStage::Compute:
            return EShLangCompute;
        default:
            return EShLangVertex;
        }
    }

    std::vector<uint32_t> ShaderCompiler::CompileGLSL( const std::string &filename, const CompileOptions &compileOptions ) const
    {
        const auto        glslContents = Utilities::ReadFile( filename );
        const ShaderStage shaderType   = compileOptions.Stage;
        const EShLanguage stage        = FindLanguage( shaderType );

        TBuiltInResource resources = { };
        InitResources( resources );

        glslang::TShader  shader( stage );
        glslang::TProgram program;

        const char *shaderStrings[ 1 ];
        shaderStrings[ 0 ] = glslContents.c_str( );
        shader.setStrings( shaderStrings, 1 );

        constexpr auto messages = static_cast<EShMessages>( EShMsgSpvRules | EShMsgVulkanRules );

        std::vector<uint32_t> spirv;
        if ( !shader.parse( &resources, 100, false, messages ) )
        {
            LOG( WARNING ) << std::string( shader.getInfoLog( ) );
            LOG( FATAL ) << std::string( shader.getInfoDebugLog( ) );
            return spirv;
        }

        program.addShader( &shader );
        if ( !program.link( messages ) )
        {
            LOG( WARNING ) << std::string( shader.getInfoLog( ) );
            LOG( WARNING ) << std::string( shader.getInfoDebugLog( ) );
            return spirv;
        }

        const glslang::TIntermediate *intermediate = program.getIntermediate( stage );
        if ( intermediate == nullptr )
        {
            return spirv;
        }
        GlslangToSpv( *intermediate, spirv );
        return std::move( spirv );
    }

    IDxcBlob *ShaderCompiler::CompileHLSL( const std::string &filename, const CompileOptions &compileOptions ) const
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

        dxcResult->Release( );
        sourceBlob->Release( );

        return code;
    }

} // namespace DenOfIz
