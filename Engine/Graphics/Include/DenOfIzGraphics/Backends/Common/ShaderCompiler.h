#pragma once

#include <DenOfIzCore/Common.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#ifdef _WIN32
#include <directx-dxc/dxcapi.h>
#include <wrl/client.h>
#define CComPtr Microsoft::WRL::ComPtr
#else
#define __EMULATE_UUID
#include "dxc/WinAdapter.h"
#include "dxc/dxcapi.h"
#endif
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
        CComPtr<IDxcLibrary> m_dxcLibrary;
        CComPtr<IDxcCompiler3> m_dxcCompiler;
        CComPtr<IDxcUtils> m_dxcUtils;

#if defined(__APPLE__)
        IRCompiler *mp_compiler;
#endif

    public:
        Result<Unit> Init();
        void Destroy();
        void InitResources(TBuiltInResource &Resources) const;
        EShLanguage FindLanguage(ShaderStage shaderType) const;
        std::vector<uint32_t> CompileHLSL(const std::string &filename, const CompileOptions &compileOptions) const;
        std::vector<uint32_t> CompileGLSL(const std::string &filename, const CompileOptions &compileOptions) const;
    };

} // namespace DenOfIz
