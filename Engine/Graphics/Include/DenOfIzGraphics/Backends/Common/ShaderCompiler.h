#pragma once

#include <DenOfIzCore/Common.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>
#include "wsl/wrladapter.h"
#include "directx-dxc/dxcapi.h"

using namespace Microsoft::WRL;

namespace DenOfIz
{

class ShaderCompiler
{
private:
	ComPtr<IDxcLibrary> dxcLibrary;
	ComPtr<IDxcCompiler3> dxcCompiler;
	ComPtr<IDxcUtils> dxcUtils;
public:
	Result<Unit> Init();
	void Destroy();
	void InitResources(TBuiltInResource& Resources);
	EShLanguage FindLanguage(const ShaderStage shaderType);
	std::vector<uint32_t> HLSLtoSPV(ShaderStage shaderType, const std::string& filename);
	std::vector<uint32_t> GLSLtoSPV(ShaderStage shaderType, const std::string& filename);
};

}
