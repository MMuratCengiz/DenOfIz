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

#include <DenOfIzGraphics/Backends/Interface/CommonData.h>

using namespace DenOfIz;

uint32_t DenOfIz::FormatNumBytes( const Format &format )
{
    switch ( format )
    {
    case Format::R32G32B32A32Float:
    case Format::R32G32B32A32Uint:
    case Format::R32G32B32A32Sint:
        return 16;
    case Format::R32G32B32Float:
    case Format::R32G32B32Uint:
    case Format::R32G32B32Sint:
        return 12;
    case Format::R16G16B16A16Float:
    case Format::R16G16B16A16Unorm:
    case Format::R16G16B16A16Uint:
    case Format::R16G16B16A16Snorm:
    case Format::R16G16B16A16Sint:
    case Format::R32G32Float:
    case Format::R32G32Uint:
    case Format::R32G32Sint:
        return 8;
    case Format::R10G10B10A2Unorm:
    case Format::R10G10B10A2Uint:
    case Format::R8G8B8A8Unorm:
    case Format::R8G8B8A8UnormSrgb:
    case Format::R8G8B8A8Uint:
    case Format::R8G8B8A8Snorm:
    case Format::R8G8B8A8Sint:
    case Format::R16G16Float:
    case Format::R16G16Unorm:
    case Format::R16G16Uint:
    case Format::R16G16Snorm:
    case Format::R16G16Sint:
    case Format::D32Float:
    case Format::R32Float:
    case Format::R32Uint:
    case Format::R32Sint:
    case Format::D24UnormS8Uint:
        return 4;
    case Format::R8G8Unorm:
    case Format::R8G8Uint:
    case Format::R8G8Snorm:
    case Format::R8G8Sint:
    case Format::R16Float:
    case Format::D16Unorm:
    case Format::R16Unorm:
    case Format::R16Uint:
        return 2;
    case Format::R16Snorm:
    case Format::R16Sint:
    case Format::R8Unorm:
    case Format::R8Uint:
    case Format::R8Snorm:
    case Format::R8Sint:
        return 1;
        // Recheck what the below are and what the expected sizes are.
    case Format::BC1Unorm:
    case Format::BC1UnormSrgb:
        return 8;
    case Format::BC2Unorm:
    case Format::BC2UnormSrgb:
    case Format::BC3Unorm:
    case Format::BC3UnormSrgb:
        return 16;
    case Format::BC4Unorm:
    case Format::BC4Snorm:
        return 8;
    case Format::BC5Unorm:
    case Format::BC5Snorm:
        return 16;
    case Format::B8G8R8A8Unorm:
        return 4;
    case Format::BC6HUfloat16:
    case Format::BC6HSfloat16:
    case Format::BC7Unorm:
    case Format::BC7UnormSrgb:
        return 16;
    default:
        return 1;
    }
}

Format DenOfIz::FormatToTypeless( const Format &format )
{
    switch ( format )
    {
    case Format::R32G32B32A32Float:
    case Format::R32G32B32A32Uint:
    case Format::R32G32B32A32Sint:
        return Format::R32G32B32A32Typeless;

    case Format::R16G16B16A16Float:
    case Format::R16G16B16A16Unorm:
    case Format::R16G16B16A16Uint:
    case Format::R16G16B16A16Snorm:
    case Format::R16G16B16A16Sint:
        return Format::R16G16B16A16Typeless;

    case Format::R32G32Float:
    case Format::R32G32Uint:
    case Format::R32G32Sint:
        return Format::R32G32Typeless;

    case Format::R10G10B10A2Unorm:
    case Format::R10G10B10A2Uint:
        return Format::R10G10B10A2Typeless;

    case Format::R8G8B8A8Unorm:
    case Format::R8G8B8A8Uint:
    case Format::R8G8B8A8Snorm:
    case Format::R8G8B8A8Sint:
        return Format::R8G8B8A8Typeless;

    case Format::R16G16Float:
    case Format::R16G16Unorm:
    case Format::R16G16Uint:
    case Format::R16G16Snorm:
    case Format::R16G16Sint:
        return Format::R16G16Typeless;

    case Format::R32Float:
    case Format::R32Uint:
    case Format::R32Sint:
        return Format::R32Typeless;

    case Format::R8G8Unorm:
    case Format::R8G8Uint:
    case Format::R8G8Snorm:
    case Format::R8G8Sint:
        return Format::R8G8Typeless;

    case Format::R16Float:
    case Format::R16Unorm:
    case Format::R16Uint:
    case Format::R16Snorm:
    case Format::R16Sint:
        return Format::R16Typeless;

    case Format::R8Unorm:
    case Format::R8Uint:
    case Format::R8Snorm:
    case Format::R8Sint:
        return Format::R8Typeless;

    default:
        return format;
    }
}

FormatSubType DenOfIz::GetFormatSubType( const Format &format )
{
    switch ( format )
    {
    case Format::Undefined:
        return FormatSubType::Undefined;
    case Format::R32G32B32A32Float:
    case Format::R32G32B32Float:
    case Format::R16G16B16A16Float:
    case Format::R32G32Float:
    case Format::R16G16Float:
    case Format::D32Float:
    case Format::R32Float:
    case Format::R16Float:
    case Format::BC6HUfloat16:
    case Format::BC6HSfloat16:
        return FormatSubType::Float;
    case Format::R32G32B32A32Sint:
    case Format::R32G32B32Sint:
    case Format::R16G16B16A16Sint:
    case Format::R32G32Sint:
    case Format::R8G8B8A8Sint:
    case Format::R16G16Sint:
    case Format::R32Sint:
    case Format::R8G8Sint:
    case Format::R16Sint:
    case Format::R8Sint:
        return FormatSubType::Sint;
    case Format::R32G32B32A32Typeless:
    case Format::R16G16B16A16Typeless:
    case Format::R32G32Typeless:
    case Format::R10G10B10A2Typeless:
    case Format::R8G8B8A8Typeless:
    case Format::R16G16Typeless:
    case Format::R32Typeless:
    case Format::R8G8Typeless:
    case Format::R16Typeless:
    case Format::R8Typeless:
        return FormatSubType::Typeless;
    case Format::R32G32B32A32Uint:
    case Format::R32G32B32Uint:
    case Format::R16G16B16A16Uint:
    case Format::R32G32Uint:
    case Format::R10G10B10A2Uint:
    case Format::R8G8B8A8Uint:
    case Format::R16G16Uint:
    case Format::R32Uint:
    case Format::D24UnormS8Uint:
    case Format::R8G8Uint:
    case Format::R16Uint:
    case Format::R8Uint:
        return FormatSubType::Uint;
    case Format::R16G16B16A16Snorm:
    case Format::R8G8B8A8Snorm:
    case Format::R16G16Snorm:
    case Format::R8G8Snorm:
    case Format::R16Snorm:
    case Format::R8Snorm:
    case Format::BC4Snorm:
    case Format::BC5Snorm:
        return FormatSubType::Snorm;
    case Format::R16G16B16A16Unorm:
    case Format::R10G10B10A2Unorm:
    case Format::R8G8B8A8Unorm:
    case Format::R16G16Unorm:
    case Format::R8G8Unorm:
    case Format::D16Unorm:
    case Format::R16Unorm:
    case Format::R8Unorm:
    case Format::BC1Unorm:
    case Format::BC2Unorm:
    case Format::BC3Unorm:
    case Format::BC4Unorm:
    case Format::BC5Unorm:
    case Format::B8G8R8A8Unorm:
    case Format::BC7Unorm:
    case Format::R8G8B8A8UnormSrgb:
    case Format::BC1UnormSrgb:
    case Format::BC2UnormSrgb:
    case Format::BC3UnormSrgb:
    case Format::BC7UnormSrgb:
        return FormatSubType::Unorm;
    }

    return FormatSubType::Undefined;
}

bool DenOfIz::IsFormatBC( const Format &format )
{
    switch ( format )
    {
    case Format::BC1Unorm:
    case Format::BC1UnormSrgb:
    case Format::BC2Unorm:
    case Format::BC2UnormSrgb:
    case Format::BC3Unorm:
    case Format::BC3UnormSrgb:
    case Format::BC4Unorm:
    case Format::BC4Snorm:
    case Format::BC5Unorm:
    case Format::BC5Snorm:
    case Format::BC6HUfloat16:
    case Format::BC6HSfloat16:
    case Format::BC7Unorm:
    case Format::BC7UnormSrgb:
        return true;
    default:
        return false;
    }
}

uint32_t DenOfIz::FormatBlockSize( const Format &format )
{
    return DenOfIz::IsFormatBC( format ) ? 4 : 1;
}

int DenOfIz::MSAASampleCountToNumSamples( const MSAASampleCount &sampleCount )
{
    switch ( sampleCount )
    {
    case MSAASampleCount::_1:
        return 1;
    case MSAASampleCount::_2:
        return 2;
    case MSAASampleCount::_4:
        return 4;
    case MSAASampleCount::_8:
        return 8;
    case MSAASampleCount::_16:
        return 16;
    case MSAASampleCount::_32:
        return 32;
    case MSAASampleCount::_64:
        return 64;
    default:
        return 1;
    }
}

ResourceBindingType DenOfIz::ResourceDescriptorBindingType( const BitSet<ResourceDescriptor> &descriptor )
{
    if ( descriptor.Any( { ResourceDescriptor::RWTexture, ResourceDescriptor::RWBuffer } ) )
    {
        return ResourceBindingType::UnorderedAccess;
    }
    if ( descriptor.IsSet( ResourceDescriptor::Sampler ) )
    {
        return ResourceBindingType::Sampler;
    }
    if ( descriptor.IsSet( ResourceDescriptor::UniformBuffer ) )
    {
        return ResourceBindingType::ConstantBuffer;
    }

    return ResourceBindingType::ShaderResource;
}
