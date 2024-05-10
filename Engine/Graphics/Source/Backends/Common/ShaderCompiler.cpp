#include <DenOfIzGraphics/Backends/Common/ShaderCompiler.h>
#include <DenOfIzCore/Logger.h>
#include <DenOfIzCore/Utilities.h>

using namespace Microsoft::WRL;

namespace DenOfIz
{

    Result<Unit> ShaderCompiler::Init()
    {
        glslang::InitializeProcess();

        HRESULT result = DxcCreateInstance( CLSID_DxcLibrary, IID_PPV_ARGS( &dxcLibrary ) );
        if ( FAILED( result ) )
        {
            return Error( "Failed to initialize DXC Library" );
        }

        result = DxcCreateInstance( CLSID_DxcCompiler, IID_PPV_ARGS( &dxcCompiler ) );
        if ( FAILED( result ) )
        {
            return Error( "Failed to initialize DXC Compiler" );
        }

        result = DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( &dxcUtils ) );
        if ( FAILED( result ) )
        {
            return Error( "Failed to initialize DXC Utils" );
        }

        return Success( {} );
    }

    void ShaderCompiler::Destroy()
    {
        glslang::FinalizeProcess();
    }

    void ShaderCompiler::InitResources( TBuiltInResource &Resources )
    {
        Resources.maxLights = 32;
        Resources.maxClipPlanes = 6;
        Resources.maxTextureUnits = 32;
        Resources.maxTextureCoords = 32;
        Resources.maxVertexAttribs = 64;
        Resources.maxVertexUniformComponents = 4096;
        Resources.maxVaryingFloats = 64;
        Resources.maxVertexTextureImageUnits = 32;
        Resources.maxCombinedTextureImageUnits = 80;
        Resources.maxTextureImageUnits = 32;
        Resources.maxFragmentUniformComponents = 4096;
        Resources.maxDrawBuffers = 32;
        Resources.maxVertexUniformVectors = 128;
        Resources.maxVaryingVectors = 8;
        Resources.maxFragmentUniformVectors = 16;
        Resources.maxVertexOutputVectors = 16;
        Resources.maxFragmentInputVectors = 15;
        Resources.minProgramTexelOffset = -8;
        Resources.maxProgramTexelOffset = 7;
        Resources.maxClipDistances = 8;
        Resources.maxComputeWorkGroupCountX = 65535;
        Resources.maxComputeWorkGroupCountY = 65535;
        Resources.maxComputeWorkGroupCountZ = 65535;
        Resources.maxComputeWorkGroupSizeX = 1024;
        Resources.maxComputeWorkGroupSizeY = 1024;
        Resources.maxComputeWorkGroupSizeZ = 64;
        Resources.maxComputeUniformComponents = 1024;
        Resources.maxComputeTextureImageUnits = 16;
        Resources.maxComputeImageUniforms = 8;
        Resources.maxComputeAtomicCounters = 8;
        Resources.maxComputeAtomicCounterBuffers = 1;
        Resources.maxVaryingComponents = 60;
        Resources.maxVertexOutputComponents = 64;
        Resources.maxGeometryInputComponents = 64;
        Resources.maxGeometryOutputComponents = 128;
        Resources.maxFragmentInputComponents = 128;
        Resources.maxImageUnits = 8;
        Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
        Resources.maxCombinedShaderOutputResources = 8;
        Resources.maxImageSamples = 0;
        Resources.maxVertexImageUniforms = 0;
        Resources.maxTessControlImageUniforms = 0;
        Resources.maxTessEvaluationImageUniforms = 0;
        Resources.maxGeometryImageUniforms = 0;
        Resources.maxFragmentImageUniforms = 8;
        Resources.maxCombinedImageUniforms = 8;
        Resources.maxGeometryTextureImageUnits = 16;
        Resources.maxGeometryOutputVertices = 256;
        Resources.maxGeometryTotalOutputComponents = 1024;
        Resources.maxGeometryUniformComponents = 1024;
        Resources.maxGeometryVaryingComponents = 64;
        Resources.maxTessControlInputComponents = 128;
        Resources.maxTessControlOutputComponents = 128;
        Resources.maxTessControlTextureImageUnits = 16;
        Resources.maxTessControlUniformComponents = 1024;
        Resources.maxTessControlTotalOutputComponents = 4096;
        Resources.maxTessEvaluationInputComponents = 128;
        Resources.maxTessEvaluationOutputComponents = 128;
        Resources.maxTessEvaluationTextureImageUnits = 16;
        Resources.maxTessEvaluationUniformComponents = 1024;
        Resources.maxTessPatchComponents = 120;
        Resources.maxPatchVertices = 32;
        Resources.maxTessGenLevel = 64;
        Resources.maxViewports = 16;
        Resources.maxVertexAtomicCounters = 0;
        Resources.maxTessControlAtomicCounters = 0;
        Resources.maxTessEvaluationAtomicCounters = 0;
        Resources.maxGeometryAtomicCounters = 0;
        Resources.maxFragmentAtomicCounters = 8;
        Resources.maxCombinedAtomicCounters = 8;
        Resources.maxAtomicCounterBindings = 1;
        Resources.maxVertexAtomicCounterBuffers = 0;
        Resources.maxTessControlAtomicCounterBuffers = 0;
        Resources.maxTessEvaluationAtomicCounterBuffers = 0;
        Resources.maxGeometryAtomicCounterBuffers = 0;
        Resources.maxFragmentAtomicCounterBuffers = 1;
        Resources.maxCombinedAtomicCounterBuffers = 1;
        Resources.maxAtomicCounterBufferSize = 16384;
        Resources.maxTransformFeedbackBuffers = 4;
        Resources.maxTransformFeedbackInterleavedComponents = 64;
        Resources.maxCullDistances = 8;
        Resources.maxCombinedClipAndCullDistances = 8;
        Resources.maxSamples = 4;
        Resources.maxMeshOutputVerticesNV = 256;
        Resources.maxMeshOutputPrimitivesNV = 512;
        Resources.maxMeshWorkGroupSizeX_NV = 32;
        Resources.maxMeshWorkGroupSizeY_NV = 1;
        Resources.maxMeshWorkGroupSizeZ_NV = 1;
        Resources.maxTaskWorkGroupSizeX_NV = 32;
        Resources.maxTaskWorkGroupSizeY_NV = 1;
        Resources.maxTaskWorkGroupSizeZ_NV = 1;
        Resources.maxMeshViewCountNV = 4;
        Resources.limits.nonInductiveForLoops = true;
        Resources.limits.whileLoops = true;
        Resources.limits.doWhileLoops = true;
        Resources.limits.generalUniformIndexing = true;
        Resources.limits.generalAttributeMatrixVectorIndexing = true;
        Resources.limits.generalVaryingIndexing = true;
        Resources.limits.generalSamplerIndexing = true;
        Resources.limits.generalVariableIndexing = true;
        Resources.limits.generalConstantMatrixVectorIndexing = true;
    }

    EShLanguage ShaderCompiler::FindLanguage( const ShaderStage shader_type )
    {
        switch ( shader_type )
        {
        case ShaderStage::Vertex:
            return EShLangVertex;
        case ShaderStage::TessellationControl:
            return EShLangTessControl;
        case ShaderStage::TessellationEvaluation:
            return EShLangTessEvaluation;
        case ShaderStage::Geometry:
            return EShLangGeometry;
        case ShaderStage::Fragment:
            return EShLangFragment;
        case ShaderStage::Compute:
            return EShLangCompute;
        default:
            return EShLangVertex;
        }
    }

    std::vector<uint32_t> ShaderCompiler::GLSLtoSPV( const ShaderStage shaderType, const std::string &shaderPath )
    {
        auto glslContents = Utilities::ReadFile( shaderPath );

        EShLanguage stage = FindLanguage( shaderType );

        TBuiltInResource resources = {};
        InitResources( resources );

        glslang::TShader shader( stage );
        glslang::TProgram program;

        const char *shaderStrings[ 1 ];
        shaderStrings[ 0 ] = glslContents.c_str();
        shader.setStrings( shaderStrings, 1 );

        auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

        std::vector<uint32_t> spirv;
        if ( !shader.parse( &resources, 100, false, messages ) )
        {
            LOG( Verbosity::Warning, "Graphics", std::string(shader.getInfoLog()) );
            LOG( Verbosity::Warning, "Graphics", std::string(shader.getInfoDebugLog()) );
            return spirv;
        }

        program.addShader( &shader );
        if ( !program.link( messages ) )
        {
            LOG( Verbosity::Warning, "Graphics", std::string(shader.getInfoLog()) );
            LOG( Verbosity::Warning, "Graphics", std::string(shader.getInfoDebugLog()) );
            return spirv;
        }

        glslang::TIntermediate *intermediate = program.getIntermediate( stage );
        if ( intermediate == nullptr )
        {
            return spirv;
        }
        GlslangToSpv( *intermediate, spirv );
        return std::move( spirv );
    }

    std::vector<uint32_t> ShaderCompiler::HLSLtoSPV( const ShaderStage shaderType, const std::string &shaderPath ) const
    {
        // Attribute to source: https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/hlsl.adoc
        // https://github.com/KhronosGroup/Vulkan-Guide
        uint32_t codePage = DXC_CP_ACP;
        ComPtr<IDxcBlobEncoding> sourceBlob;
        std::wstring wsShaderPath( shaderPath.begin(), shaderPath.end() );
        HRESULT result = dxcUtils->LoadFile( wsShaderPath.c_str(), &codePage, &sourceBlob );
        if ( FAILED( result ) )
        {
            throw std::runtime_error( &"Could not load shader file"[ GetLastError() ] );
        }

        LPCWSTR targetProfile{};
        switch ( shaderType )
        {
        case ShaderStage::Vertex:
            targetProfile = L"vs_6_1";
            break;
        case ShaderStage::TessellationControl:
            break;
        case ShaderStage::TessellationEvaluation:
            break;
        case ShaderStage::Geometry:
            break;
        case ShaderStage::Fragment:
            targetProfile = L"ps_6_1";
            break;
        case ShaderStage::Compute:
            break;
        case ShaderStage::AllGraphics:
            break;
        case ShaderStage::All:
            break;
        }

        // Configure the compiler arguments for compiling the HLSL shader to SPIR-V
        std::vector<LPCWSTR> arguments = {
            // (Optional) name of the shader file to be displayed e.g. in an error message
            wsShaderPath.c_str(),
            // Shader main entry point
            L"-E", L"main",
            // Shader target profile
            L"-T", targetProfile,
            // Compile to SPIRV
            L"-spirv" };

        DxcBuffer buffer{};
        buffer.Encoding = DXC_CP_ACP; // or? DXC_CP_UTF8;
        buffer.Ptr = sourceBlob->GetBufferPointer();
        buffer.Size = sourceBlob->GetBufferSize();

        ComPtr<IDxcResult> dxcResult{ nullptr };
        result = dxcCompiler->Compile( &buffer, arguments.data(), static_cast<uint32_t>(arguments.size()), nullptr, IID_PPV_ARGS( &dxcResult ) );

        if ( SUCCEEDED( result ) )
        {
            dxcResult->GetStatus( &result );
        }

        if ( FAILED( result ) && (dxcResult) )
        {
            ComPtr<IDxcBlobEncoding> errorBlob;
            result = dxcResult->GetErrorBuffer( &errorBlob );
            if ( SUCCEEDED( result ) && errorBlob )
            {
                std::cerr << "Shader compilation failed :\n\n" << static_cast<const char *>(errorBlob->GetBufferPointer());
                throw std::runtime_error( "Compilation failed" );
            }
        }

        ComPtr<IDxcBlob> code;
        dxcResult->GetResult( &code );

        std::vector<uint32_t> codeToUVec( static_cast<uint32_t *>(code->GetBufferPointer()),
                                          static_cast<uint32_t *>(code->GetBufferPointer()) + code->GetBufferSize() / sizeof( uint32_t ) );

        code->Release();
        return codeToUVec;
    }

}
