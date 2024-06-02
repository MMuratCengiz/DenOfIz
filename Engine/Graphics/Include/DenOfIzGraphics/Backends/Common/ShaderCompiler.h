#pragma once

#include <DenOfIzCore/Common.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>

#include <wrl/client.h>
#include "directx-dxc/dxcapi.h"

using namespace Microsoft::WRL;

namespace DenOfIz
{

class ShaderCompiler
{
	ComPtr<IDxcLibrary> m_dxcLibrary;
	ComPtr<IDxcCompiler3> m_dxcCompiler;
	ComPtr<IDxcUtils> m_dxcUtils;

public:
	Result<Unit> Init();
	void Destroy();
	void InitResources(TBuiltInResource& Resources);
	EShLanguage FindLanguage(ShaderStage shaderType);
	std::vector<uint32_t> HLSLtoSPV(ShaderStage shaderType, const std::string& filename) const;
	std::vector<uint32_t> GLSLtoSPV(ShaderStage shaderType, const std::string& filename);
};

}
