/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <DenOfIzGraphics/Backends/Interface/IResource.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>

namespace DenOfIz
{

class DX12EnumConverter
{
public:
	static DXGI_FORMAT ConvertImageFormat(const ImageFormat& format)
	{
		switch (format)
		{
		case ImageFormat::Unknown:
			return DXGI_FORMAT_UNKNOWN;
		case ImageFormat::R32G32B32A32Float:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case ImageFormat::R32G32B32A32Uint:
			return DXGI_FORMAT_R32G32B32A32_UINT;
		case ImageFormat::R32G32B32A32Sint:
			return DXGI_FORMAT_R32G32B32A32_SINT;
		case ImageFormat::R32G32B32Float:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case ImageFormat::R32G32B32Uint:
			return DXGI_FORMAT_R32G32B32_UINT;
		case ImageFormat::R32G32B32Sint:
			return DXGI_FORMAT_R32G32B32_SINT;
		case ImageFormat::R16G16B16A16Float:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case ImageFormat::R16G16B16A16Unorm:
			return DXGI_FORMAT_R16G16B16A16_UNORM;
		case ImageFormat::R16G16B16A16Uint:
			return DXGI_FORMAT_R16G16B16A16_UINT;
		case ImageFormat::R16G16B16A16Snorm:
			return DXGI_FORMAT_R16G16B16A16_SNORM;
		case ImageFormat::R16G16B16A16Sint:
			return DXGI_FORMAT_R16G16B16A16_SINT;
		case ImageFormat::R32G32Float:
			return DXGI_FORMAT_R32G32_FLOAT;
		case ImageFormat::R32G32Uint:
			return DXGI_FORMAT_R32G32_UINT;
		case ImageFormat::R32G32Sint:
			return DXGI_FORMAT_R32G32_SINT;
		case ImageFormat::R10G10B10A2Unorm:
			return DXGI_FORMAT_R10G10B10A2_UNORM;
		case ImageFormat::R10G10B10A2Uint:
			return DXGI_FORMAT_R10G10B10A2_UINT;
		case ImageFormat::R8G8B8A8Unorm:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::R8G8B8A8UnormSrgb:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case ImageFormat::R8G8B8A8Uint:
			return DXGI_FORMAT_R8G8B8A8_UINT;
		case ImageFormat::R8G8B8A8Snorm:
			return DXGI_FORMAT_R8G8B8A8_SNORM;
		case ImageFormat::R8G8B8A8Sint:
			return DXGI_FORMAT_R8G8B8A8_SINT;
		case ImageFormat::R16G16Float:
			return DXGI_FORMAT_R16G16_FLOAT;
		case ImageFormat::R16G16Unorm:
			return DXGI_FORMAT_R16G16_UNORM;
		case ImageFormat::R16G16Uint:
			return DXGI_FORMAT_R16G16_UINT;
		case ImageFormat::R16G16Snorm:
			return DXGI_FORMAT_R16G16_SNORM;
		case ImageFormat::R16G16Sint:
			return DXGI_FORMAT_R16G16_SINT;
		case ImageFormat::D32Float:
			return DXGI_FORMAT_D32_FLOAT;
		case ImageFormat::R32Float:
			return DXGI_FORMAT_R32_FLOAT;
		case ImageFormat::R32Uint:
			return DXGI_FORMAT_R32_UINT;
		case ImageFormat::R32Sint:
			return DXGI_FORMAT_R32_SINT;
		case ImageFormat::D24UnormS8Uint:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case ImageFormat::R8G8Unorm:
			return DXGI_FORMAT_R8G8_UNORM;
		case ImageFormat::R8G8Uint:
			return DXGI_FORMAT_R8G8_UINT;
		case ImageFormat::R8G8Snorm:
			return DXGI_FORMAT_R8G8_SNORM;
		case ImageFormat::R8G8Sint:
			return DXGI_FORMAT_R8G8_SINT;
		case ImageFormat::R16Float:
			return DXGI_FORMAT_R16_FLOAT;
		case ImageFormat::D16Unorm:
			return DXGI_FORMAT_D16_UNORM;
		case ImageFormat::R16Unorm:
			return DXGI_FORMAT_R16_UNORM;
		case ImageFormat::R16Uint:
			return DXGI_FORMAT_R16_UINT;
		case ImageFormat::R16Snorm:
			return DXGI_FORMAT_R16_SNORM;
		case ImageFormat::R16Sint:
			return DXGI_FORMAT_R16_SINT;
		case ImageFormat::R8Unorm:
			return DXGI_FORMAT_R8_UNORM;
		case ImageFormat::R8Uint:
			return DXGI_FORMAT_R8_UINT;
		case ImageFormat::R8Snorm:
			return DXGI_FORMAT_R8_SNORM;
		case ImageFormat::R8Sint:
			return DXGI_FORMAT_R8_SINT;
		case ImageFormat::BC1Unorm:
			return DXGI_FORMAT_BC1_UNORM;
		case ImageFormat::BC1UnormSrgb:
			return DXGI_FORMAT_BC1_UNORM_SRGB;
		case ImageFormat::BC2Unorm:
			return DXGI_FORMAT_BC2_UNORM;
		case ImageFormat::BC2UnormSrgb:
			return DXGI_FORMAT_BC2_UNORM_SRGB;
		case ImageFormat::BC3Unorm:
			return DXGI_FORMAT_BC3_UNORM;
		case ImageFormat::BC3UnormSrgb:
			return DXGI_FORMAT_BC3_UNORM_SRGB;
		case ImageFormat::BC4Unorm:
			return DXGI_FORMAT_BC4_UNORM;
		case ImageFormat::BC4Snorm:
			return DXGI_FORMAT_BC4_SNORM;
		case ImageFormat::BC5Unorm:
			return DXGI_FORMAT_BC5_UNORM;
		case ImageFormat::BC5Snorm:
			return DXGI_FORMAT_BC5_SNORM;
		case ImageFormat::B8G8R8A8Unorm:
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case ImageFormat::BC6HUfloat16:
			return DXGI_FORMAT_BC6H_UF16;
		case ImageFormat::BC6HSfloat16:
			return DXGI_FORMAT_BC6H_SF16;
		case ImageFormat::BC7Unorm:
			return DXGI_FORMAT_BC7_UNORM;
		case ImageFormat::BC7UnormSrgb:
			return DXGI_FORMAT_BC7_UNORM_SRGB;
		};

		return DXGI_FORMAT_UNKNOWN;
	}
};

}