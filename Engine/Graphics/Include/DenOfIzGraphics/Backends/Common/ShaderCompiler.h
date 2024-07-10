#pragma once

#include <DenOfIzCore/Common.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#if defined(__APPLE__)
#include "Metal/Metal.h"
#include "MetalKit/MetalKit.h"
#include <metal_irconverter/metal_irconverter.h>
#include <simd/simd.h>
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

    class ShaderCompiler
    {
    private:
        IDxcLibrary   *m_dxcLibrary;
        IDxcCompiler3 *m_dxcCompiler;
        IDxcUtils     *m_dxcUtils;

#if defined(__APPLE__)
        IRCompiler *mp_compiler;
#endif

    public:
        bool                  Init();
        void                  Destroy();
        void                  InitResources(TBuiltInResource &Resources) const;
        EShLanguage           FindLanguage(ShaderStage shaderType) const;
        IDxcBlob             *CompileHLSL(const std::string &filename, const CompileOptions &compileOptions) const;
        std::vector<uint32_t> CompileGLSL(const std::string &filename, const CompileOptions &compileOptions) const;
    };

} // namespace DenOfIz
