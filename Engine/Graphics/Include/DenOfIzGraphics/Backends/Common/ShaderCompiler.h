#pragma once

#include <DenOfIzCore/Common.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>

#ifdef _WIN32
#else
#define __EMULATE_UUID
#include "dxc/WinAdapter.h"
#include "dxc/dxcapi.h"
#endif
#if defined(__APPLE__)
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
#include <simd/simd.h>
#include <metal_irconverter/metal_irconverter.h>
#endif

namespace DenOfIz
{

class ShaderCompiler
{
private:
	CComPtr<IDxcLibrary> m_dxcLibrary;
	CComPtr<IDxcCompiler3> m_dxcCompiler;
	CComPtr<IDxcUtils> m_dxcUtils;

#if defined(__APPLE__)
    IRCompiler* mp_compiler;
#endif

public:
	Result<Unit> Init();
	void Destroy();
	void InitResources(TBuiltInResource& Resources);
	EShLanguage FindLanguage(ShaderStage shaderType);
	std::vector<uint32_t> HLSLtoSPV(ShaderStage shaderType, const std::string& filename) const;
	std::vector<uint32_t> GLSLtoSPV(ShaderStage shaderType, const std::string& filename);
};

}
