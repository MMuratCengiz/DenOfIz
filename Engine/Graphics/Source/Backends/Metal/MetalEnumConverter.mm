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

#include <DenOfIzGraphics/Backends/Metal/MetalEnumConverter.h>

using namespace DenOfIz;

MTLPixelFormat MetalEnumConverter::ConvertFormat( Format format )
{
    switch ( format )
    {
    case Format::Undefined:
        return MTLPixelFormatInvalid;
    case Format::R32G32B32A32Float:
        return MTLPixelFormatRGBA32Float;
    case Format::R32G32B32A32Sint:
        return MTLPixelFormatRGBA32Sint;
    case Format::R32G32B32A32Typeless:
        return MTLPixelFormatRGBA32Uint;
    case Format::R32G32B32A32Uint:
        return MTLPixelFormatRGBA32Uint;
    case Format::R32G32B32Float:
    case Format::R32G32B32Uint:
    case Format::R32G32B32Sint:
        LOG( WARNING ) << "Unsupported Format for Metal: R32G32B32..., Returning Undefined.";
        return MTLPixelFormatInvalid;
    case Format::R16G16B16A16Float:
        return MTLPixelFormatRGBA16Float;
    case Format::R16G16B16A16Snorm:
        return MTLPixelFormatRGBA16Snorm;
    case Format::R16G16B16A16Sint:
        return MTLPixelFormatRGBA16Sint;
    case Format::R16G16B16A16Typeless:
        return MTLPixelFormatRGBA16Uint;
    case Format::R16G16B16A16Unorm:
        return MTLPixelFormatRGBA16Unorm;
    case Format::R16G16B16A16Uint:
        return MTLPixelFormatRGBA16Uint;
    case Format::R32G32Float:
        return MTLPixelFormatRG32Float;
    case Format::R32G32Sint:
        return MTLPixelFormatRG32Sint;
    case Format::R32G32Typeless:
        return MTLPixelFormatRG32Uint;
    case Format::R32G32Uint:
        return MTLPixelFormatRG32Uint;
    case Format::R10G10B10A2Typeless:
        return MTLPixelFormatRGB10A2Uint;
    case Format::R10G10B10A2Uint:
        return MTLPixelFormatRGB10A2Uint;
    case Format::R10G10B10A2Unorm:
        return MTLPixelFormatRGB10A2Unorm;
    case Format::R8G8B8A8Uint:
        return MTLPixelFormatRGBA8Uint;
    case Format::R8G8B8A8Sint:
        return MTLPixelFormatRGBA8Sint;
    case Format::R8G8B8A8Snorm:
        return MTLPixelFormatRGBA8Snorm;
    case Format::R8G8B8A8Typeless:
        return MTLPixelFormatRGBA8Uint;
    case Format::R8G8B8A8Unorm:
        return MTLPixelFormatRGBA8Unorm;
    case Format::R8G8B8A8UnormSrgb:
        return MTLPixelFormatRGBA8Unorm_sRGB;
    case Format::R16G16Float:
        return MTLPixelFormatRG16Float;
    case Format::R16G16Sint:
        return MTLPixelFormatRG16Sint;
    case Format::R16G16Snorm:
        return MTLPixelFormatRG16Snorm;
    case Format::R16G16Typeless:
        return MTLPixelFormatRG16Uint;
    case Format::R16G16Uint:
        return MTLPixelFormatRG16Uint;
    case Format::R16G16Unorm:
        return MTLPixelFormatRG16Unorm;
    case Format::D32Float:
        return MTLPixelFormatDepth32Float;
    case Format::R32Float:
        return MTLPixelFormatR32Float;
    case Format::R32Sint:
        return MTLPixelFormatR32Sint;
    case Format::R32Typeless:
        return MTLPixelFormatR32Uint;
    case Format::R32Uint:
        return MTLPixelFormatR32Uint;
    case Format::D24UnormS8Uint:
        return MTLPixelFormatDepth24Unorm_Stencil8;
    case Format::R8G8Snorm:
        return MTLPixelFormatRG8Snorm;
    case Format::R8G8Typeless:
        return MTLPixelFormatRG8Uint;
    case Format::R8G8Uint:
        return MTLPixelFormatRG8Uint;
    case Format::R8G8Unorm:
        return MTLPixelFormatRG8Unorm;
    case Format::R8G8Sint:
        return MTLPixelFormatRG8Sint;
    case Format::D16Unorm:
        return MTLPixelFormatDepth16Unorm;
    case Format::R16Float:
        return MTLPixelFormatR16Float;
    case Format::R16Sint:
        return MTLPixelFormatR16Sint;
    case Format::R16Snorm:
        return MTLPixelFormatR16Snorm;
    case Format::R16Typeless:
        return MTLPixelFormatR16Uint;
    case Format::R16Uint:
        return MTLPixelFormatR16Uint;
    case Format::R16Unorm:
        return MTLPixelFormatR16Unorm;
    case Format::R8Sint:
        return MTLPixelFormatR8Sint;
    case Format::R8Snorm:
        return MTLPixelFormatR8Snorm;
    case Format::R8Typeless:
        return MTLPixelFormatR8Uint;
    case Format::R8Uint:
        return MTLPixelFormatR8Uint;
    case Format::R8Unorm:
        return MTLPixelFormatR8Unorm;
    case Format::BC1Unorm:
        return MTLPixelFormatBC1_RGBA;
    case Format::BC1UnormSrgb:
        return MTLPixelFormatBC1_RGBA_sRGB;
    case Format::BC2Unorm:
        return MTLPixelFormatBC2_RGBA;
    case Format::BC2UnormSrgb:
        return MTLPixelFormatBC2_RGBA_sRGB;
    case Format::BC3Unorm:
        return MTLPixelFormatBC3_RGBA;
    case Format::BC3UnormSrgb:
        return MTLPixelFormatBC3_RGBA_sRGB;
    case Format::BC4Unorm:
        return MTLPixelFormatBC4_RUnorm;
    case Format::BC4Snorm:
        return MTLPixelFormatBC4_RSnorm;
    case Format::BC5Unorm:
        return MTLPixelFormatBC5_RGUnorm;
    case Format::BC5Snorm:
        return MTLPixelFormatBC5_RGSnorm;
    case Format::B8G8R8A8Unorm:
        return MTLPixelFormatBGRA8Unorm;
    case Format::BC6HUfloat16:
        return MTLPixelFormatBC6H_RGBUfloat;
    case Format::BC6HSfloat16:
        return MTLPixelFormatBC6H_RGBFloat;
    case Format::BC7Unorm:
        return MTLPixelFormatBC7_RGBAUnorm;
    case Format::BC7UnormSrgb:
        return MTLPixelFormatBC7_RGBAUnorm;
    }

    return MTLPixelFormatInvalid;
}

MTLPrimitiveTopologyClass MetalEnumConverter::ConvertTopologyClass( PrimitiveTopology topology )
{
    switch ( topology )
    {
    case PrimitiveTopology::Point:
        return MTLPrimitiveTopologyClassPoint;
    case PrimitiveTopology::Line:
        return MTLPrimitiveTopologyClassLine;
    case PrimitiveTopology::Triangle:
        return MTLPrimitiveTopologyClassTriangle;
    default:
        break;
    }
    return MTLPrimitiveTopologyClassTriangle;
}

MTLVertexFormat MetalEnumConverter::ConvertFormatToVertexFormat( Format format )
{
    switch ( format )
    {
    case Format::R32G32B32A32Float:
        return MTLVertexFormatFloat4;
    case Format::R32G32B32A32Sint:
        return MTLVertexFormatInt4;
    case Format::R32G32B32A32Uint:
        return MTLVertexFormatUInt4;
    case Format::R32G32B32Float:
        return MTLVertexFormatFloat3;
    case Format::R32G32B32Sint:
        return MTLVertexFormatInt3;
    case Format::R32G32B32Uint:
        return MTLVertexFormatUInt3;
    case Format::R16G16B16A16Float:
        return MTLVertexFormatHalf4;
    case Format::R16G16B16A16Snorm:
        return MTLVertexFormatShort4Normalized;
    case Format::R16G16B16A16Sint:
        return MTLVertexFormatShort4;
    case Format::R16G16B16A16Unorm:
        return MTLVertexFormatUShort4Normalized;
    case Format::R16G16B16A16Uint:
        return MTLVertexFormatUShort4;
    case Format::R32G32Float:
        return MTLVertexFormatFloat2;
    case Format::R32G32Sint:
        return MTLVertexFormatInt2;
    case Format::R32G32Uint:
        return MTLVertexFormatUInt2;
    case Format::R10G10B10A2Uint:
        return MTLVertexFormatUInt1010102Normalized;
    case Format::R10G10B10A2Unorm:
        return MTLVertexFormatUInt1010102Normalized;
    case Format::R8G8B8A8Uint:
        return MTLVertexFormatUChar4;
    case Format::R8G8B8A8Sint:
        return MTLVertexFormatChar4;
    case Format::R8G8B8A8Snorm:
        return MTLVertexFormatChar4Normalized;
    case Format::R8G8B8A8Unorm:
    case Format::R8G8B8A8UnormSrgb:
        return MTLVertexFormatUChar4Normalized;
    case Format::R16G16Float:
        return MTLVertexFormatHalf2;
    case Format::R16G16Sint:
        return MTLVertexFormatShort2;
    case Format::R16G16Snorm:
        return MTLVertexFormatShort2Normalized;
    case Format::R16G16Unorm:
        return MTLVertexFormatUShort2Normalized;
    case Format::R16G16Uint:
        return MTLVertexFormatUShort2;
    case Format::R32Float:
        return MTLVertexFormatFloat;
    case Format::R32Sint:
        return MTLVertexFormatInt;
    case Format::R32Uint:
        return MTLVertexFormatUInt;
    case Format::R8G8Snorm:
        return MTLVertexFormatChar2Normalized;
    case Format::R8G8Unorm:
        return MTLVertexFormatUChar2Normalized;
    case Format::R8G8Sint:
        return MTLVertexFormatChar2;
    case Format::R8G8Uint:
        return MTLVertexFormatUChar2;
    case Format::R16Float:
        return MTLVertexFormatHalf;
    case Format::R16Sint:
        return MTLVertexFormatShort;
    case Format::R16Snorm:
        return MTLVertexFormatShortNormalized;
    case Format::R16Unorm:
        return MTLVertexFormatUShortNormalized;
    case Format::R16Uint:
        return MTLVertexFormatUShort;
    case Format::R8Sint:
        return MTLVertexFormatChar;
    case Format::R8Snorm:
        return MTLVertexFormatCharNormalized;
    case Format::R8Unorm:
        return MTLVertexFormatUCharNormalized;
    case Format::R8Uint:
        return MTLVertexFormatUChar;
    default:
        LOG( WARNING ) << "Warning: Format does not have a corresponding MTLVertexFormat: " << static_cast<int>( format );
        return MTLVertexFormatInvalid;
    }
    LOG( WARNING ) << "Warning: Unknown format: " << static_cast<int>( format );
    return MTLVertexFormatInvalid;
}

MTLDataType MetalEnumConverter::ConvertFormatToDataType( Format format )
{
    switch ( format )
    {
    case Format::Undefined:
        return MTLDataTypeNone;
    case Format::R32G32B32A32Float:
        return MTLDataTypeFloat4;
    case Format::R32G32B32A32Sint:
        return MTLDataTypeInt4;
    case Format::R32G32B32A32Uint:
        return MTLDataTypeUInt4;
    case Format::R32G32B32Float:
        return MTLDataTypeFloat3;
    case Format::R32G32B32Sint:
        return MTLDataTypeInt3;
    case Format::R32G32B32Uint:
        return MTLDataTypeUInt3;
    case Format::R16G16B16A16Float:
        return MTLDataTypeHalf4;
    case Format::R16G16B16A16Snorm:
    case Format::R16G16B16A16Sint:
    case Format::R16G16B16A16Typeless:
    case Format::R16G16B16A16Unorm:
    case Format::R16G16B16A16Uint:
        return MTLDataTypeUShort4;
    case Format::R32G32Float:
        return MTLDataTypeFloat2;
    case Format::R32G32Sint:
        return MTLDataTypeInt2;
    case Format::R32G32Uint:
        return MTLDataTypeUInt2;
    case Format::R10G10B10A2Uint:
        return MTLDataTypeUInt;
    case Format::R10G10B10A2Unorm:
        return MTLDataTypeUInt;
    case Format::R8G8B8A8Uint:
        return MTLDataTypeUChar4;
    case Format::R8G8B8A8Sint:
        return MTLDataTypeChar4;
    case Format::R8G8B8A8Unorm:
    case Format::R8G8B8A8UnormSrgb:
        return MTLDataTypeUChar4;
    case Format::R8G8B8A8Snorm:
        return MTLDataTypeChar4;
    case Format::R16G16Float:
        return MTLDataTypeHalf2;
    case Format::R16G16Sint:
        return MTLDataTypeShort2;
    case Format::R16G16Uint:
        return MTLDataTypeUShort2;
    case Format::R16G16Unorm:
        return MTLDataTypeUShort2;
    case Format::R16G16Snorm:
        return MTLDataTypeShort2;
    case Format::D32Float:
        return MTLDataTypeFloat;
    case Format::R32Float:
        return MTLDataTypeFloat;
    case Format::R32Sint:
        return MTLDataTypeInt;
    case Format::R32Uint:
        return MTLDataTypeUInt;
    case Format::R8G8Uint:
        return MTLDataTypeUChar2;
    case Format::R8G8Sint:
        return MTLDataTypeChar2;
    case Format::R8G8Unorm:
        return MTLDataTypeUChar2;
    case Format::R8G8Snorm:
        return MTLDataTypeChar2;
    case Format::R16Float:
        return MTLDataTypeHalf;
    case Format::R16Sint:
        return MTLDataTypeShort;
    case Format::R16Uint:
        return MTLDataTypeUShort;
    case Format::R16Unorm:
        return MTLDataTypeUShort;
    case Format::R16Snorm:
        return MTLDataTypeShort;
    case Format::R8Uint:
        return MTLDataTypeUChar;
    case Format::R8Sint:
        return MTLDataTypeChar;
    case Format::R8Unorm:
        return MTLDataTypeUChar;
    case Format::R8Snorm:
        return MTLDataTypeChar;
    default:
        break;
    }

    return MTLDataTypeNone;
}

MTLSamplerMinMagFilter MetalEnumConverter::ConvertFilter( Filter filter )
{
    switch ( filter )
    {
    case Filter::Nearest:
        return MTLSamplerMinMagFilterNearest;
    case Filter::Linear:
        return MTLSamplerMinMagFilterLinear;
    }
    return MTLSamplerMinMagFilterLinear;
}

MTLSamplerAddressMode MetalEnumConverter::ConvertSamplerAddressMode( SamplerAddressMode mode )
{
    switch ( mode )
    {
    case SamplerAddressMode::Repeat:
        return MTLSamplerAddressModeRepeat;
    case SamplerAddressMode::Mirror:
        return MTLSamplerAddressModeMirrorRepeat;
    case SamplerAddressMode::ClampToEdge:
        return MTLSamplerAddressModeClampToEdge;
    case SamplerAddressMode::ClampToBorder:
        return MTLSamplerAddressModeClampToZero;
    }
    return MTLSamplerAddressModeClampToEdge;
}

MTLSamplerMipFilter MetalEnumConverter::ConvertMipMapFilter( MipmapMode mode )
{
    switch ( mode )
    {
    case MipmapMode::Nearest:
        return MTLSamplerMipFilterNearest;
    case MipmapMode::Linear:
        return MTLSamplerMipFilterLinear;
    }
    return MTLSamplerMipFilterLinear;
}

MTLCompareFunction MetalEnumConverter::ConvertCompareFunction( CompareOp op )
{
    switch ( op )
    {
    case CompareOp::Never:
        return MTLCompareFunctionNever;
    case CompareOp::Less:
        return MTLCompareFunctionLess;
    case CompareOp::Equal:
        return MTLCompareFunctionEqual;
    case CompareOp::LessOrEqual:
        return MTLCompareFunctionLessEqual;
    case CompareOp::Greater:
        return MTLCompareFunctionGreater;
    case CompareOp::NotEqual:
        return MTLCompareFunctionNotEqual;
    case CompareOp::GreaterOrEqual:
        return MTLCompareFunctionGreaterEqual;
    case CompareOp::Always:
        return MTLCompareFunctionAlways;
    }
    return MTLCompareFunctionGreaterEqual;
}
