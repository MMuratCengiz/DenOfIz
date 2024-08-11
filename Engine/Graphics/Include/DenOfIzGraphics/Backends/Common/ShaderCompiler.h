#pragma once

#include <DenOfIzCore/Common.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#if defined( __APPLE__ )
#include <metal_irconverter/metal_irconverter.h>
#include <simd/simd.h>
#include "Metal/Metal.h"
#include "MetalKit/MetalKit.h"
#endif

namespace DenOfIz
{

    enum class TargetIL
    {
        DXIL,
        MSL,
        SPIRV
    };

    struct CompileOptions
    {
        std::string EntryPoint = "main";
        ShaderStage Stage;
        TargetIL    TargetIL;

        std::vector<std::string> Defines;
    };

    class ShaderCompiler final : public NonCopyable
    {
        IDxcLibrary   *m_dxcLibrary  = nullptr;
        IDxcCompiler3 *m_dxcCompiler = nullptr;
        IDxcUtils     *m_dxcUtils    = nullptr;

#if defined( __APPLE__ )
        IRCompiler *m_irCompiler = nullptr;
#endif

    public:
        static constexpr uint32_t VkShiftCbv     = 1000;
        static constexpr uint32_t VkShiftSrv     = 2000;
        static constexpr uint32_t VkShiftUav     = 3000;
        static constexpr uint32_t VkShiftSampler = 4000;
        IDxcUtils *DxcUtils( ) const;

        ShaderCompiler( );
        ~ShaderCompiler( );
        void                                          InitResources( TBuiltInResource &Resources ) const;
        [[nodiscard]] EShLanguage                     FindLanguage( ShaderStage shaderType ) const;
        [[nodiscard]] std::unique_ptr<CompiledShader> CompileHLSL( const std::string &path, const CompileOptions &compileOptions ) const;
        [[nodiscard]] std::vector<uint32_t>           CompileGLSL( const std::string &filename, const CompileOptions &compileOptions ) const;
        [[nodiscard]] IDxcBlob *const                 DxilToMsl( const CompileOptions &compileOptions, IDxcBlob *code ) const;
    };

} // namespace DenOfIz
