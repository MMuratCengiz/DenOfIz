#pragma once

#include <DenOfIzCore/Common.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <dxcapi.h>
#if defined(__APPLE__)
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
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
        TargetIL TargetIL;

        std::vector<std::string> Defines;
    };

    class ShaderCompiler
    {
    private:
        wil::com_ptr<IDxcLibrary> m_dxcLibrary;
        wil::com_ptr<IDxcCompiler3> m_dxcCompiler;
        wil::com_ptr<IDxcUtils> m_dxcUtils;

#if defined(__APPLE__)
        IRCompiler *mp_compiler;
#endif

    public:
        bool Init();
        void Destroy();
        void InitResources(TBuiltInResource &Resources) const;
        EShLanguage FindLanguage(ShaderStage shaderType) const;
        wil::com_ptr<IDxcBlob> CompileHLSL(const std::string &filename, const CompileOptions &compileOptions) const;
        std::vector<uint32_t> CompileGLSL(const std::string &filename, const CompileOptions &compileOptions) const;
    };

} // namespace DenOfIz
