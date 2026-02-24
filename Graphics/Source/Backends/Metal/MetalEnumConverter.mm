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

#include "DenOfIzGraphicsInternal/Backends/Metal/MetalEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"


MTLBindingAccess DenOfIz_MetalEnumConverter_ConvertDescriptorToBindingAccess( const DenOfIz_ResourceDescriptorFlags descriptor )
{
    if ( descriptor & ( DENOFIZ_RESOURCE_DESCRIPTOR_RW_TEXTURE_BIT | DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT ) )
    {
        return MTLBindingAccessReadWrite;
    }

    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_INDIRECT_BUFFER_BIT )
    {
        return MTLBindingAccessReadOnly;
    }

    return MTLBindingAccessReadOnly;
}

MTLPixelFormat DenOfIz_MetalEnumConverter_ConvertFormat( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_UNDEFINED:
        return MTLPixelFormatInvalid;
    case DENOFIZ_FORMAT_R32G32B32A32_FLOAT:
        return MTLPixelFormatRGBA32Float;
    case DENOFIZ_FORMAT_R32G32B32A32_SINT:
        return MTLPixelFormatRGBA32Sint;
    case DENOFIZ_FORMAT_R32G32B32A32_TYPELESS:
        return MTLPixelFormatRGBA32Uint;
    case DENOFIZ_FORMAT_R32G32B32A32_UINT:
        return MTLPixelFormatRGBA32Uint;
    case DENOFIZ_FORMAT_R32G32B32_FLOAT:
    case DENOFIZ_FORMAT_R32G32B32_UINT:
    case DENOFIZ_FORMAT_R32G32B32_SINT:
        spdlog::warn("Unsupported DenOfIz_Format for Metal: R32G32B32..., Returning Undefined.");
        return MTLPixelFormatInvalid;
    case DENOFIZ_FORMAT_R16G16B16A16_FLOAT:
        return MTLPixelFormatRGBA16Float;
    case DENOFIZ_FORMAT_R16G16B16A16_SNORM:
        return MTLPixelFormatRGBA16Snorm;
    case DENOFIZ_FORMAT_R16G16B16A16_SINT:
        return MTLPixelFormatRGBA16Sint;
    case DENOFIZ_FORMAT_R16G16B16A16_TYPELESS:
        return MTLPixelFormatRGBA16Uint;
    case DENOFIZ_FORMAT_R16G16B16A16_UNORM:
        return MTLPixelFormatRGBA16Unorm;
    case DENOFIZ_FORMAT_R16G16B16A16_UINT:
        return MTLPixelFormatRGBA16Uint;
    case DENOFIZ_FORMAT_R32G32_FLOAT:
        return MTLPixelFormatRG32Float;
    case DENOFIZ_FORMAT_R32G32_SINT:
        return MTLPixelFormatRG32Sint;
    case DENOFIZ_FORMAT_R32G32_TYPELESS:
        return MTLPixelFormatRG32Uint;
    case DENOFIZ_FORMAT_R32G32_UINT:
        return MTLPixelFormatRG32Uint;
    case DENOFIZ_FORMAT_R10G10B10A2_TYPELESS:
        return MTLPixelFormatRGB10A2Uint;
    case DENOFIZ_FORMAT_R10G10B10A2_UINT:
        return MTLPixelFormatRGB10A2Uint;
    case DENOFIZ_FORMAT_R10G10B10A2_UNORM:
        return MTLPixelFormatRGB10A2Unorm;
    case DENOFIZ_FORMAT_R8G8B8A8_UINT:
        return MTLPixelFormatRGBA8Uint;
    case DENOFIZ_FORMAT_R8G8B8A8_SINT:
        return MTLPixelFormatRGBA8Sint;
    case DENOFIZ_FORMAT_R8G8B8A8_SNORM:
        return MTLPixelFormatRGBA8Snorm;
    case DENOFIZ_FORMAT_R8G8B8A8_TYPELESS:
        return MTLPixelFormatRGBA8Uint;
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM:
        return MTLPixelFormatRGBA8Unorm;
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB:
        return MTLPixelFormatRGBA8Unorm_sRGB;
    case DENOFIZ_FORMAT_R16G16_FLOAT:
        return MTLPixelFormatRG16Float;
    case DENOFIZ_FORMAT_R16G16_SINT:
        return MTLPixelFormatRG16Sint;
    case DENOFIZ_FORMAT_R16G16_SNORM:
        return MTLPixelFormatRG16Snorm;
    case DENOFIZ_FORMAT_R16G16_TYPELESS:
        return MTLPixelFormatRG16Uint;
    case DENOFIZ_FORMAT_R16G16_UINT:
        return MTLPixelFormatRG16Uint;
    case DENOFIZ_FORMAT_R16G16_UNORM:
        return MTLPixelFormatRG16Unorm;
    case DENOFIZ_FORMAT_D32_FLOAT:
        return MTLPixelFormatDepth32Float;
    case DENOFIZ_FORMAT_R32_FLOAT:
        return MTLPixelFormatR32Float;
    case DENOFIZ_FORMAT_R32_SINT:
        return MTLPixelFormatR32Sint;
    case DENOFIZ_FORMAT_R32_TYPELESS:
        return MTLPixelFormatR32Uint;
    case DENOFIZ_FORMAT_R32_UINT:
        return MTLPixelFormatR32Uint;
    case DENOFIZ_FORMAT_D24_UNORM_S8_UINT:
        return MTLPixelFormatDepth24Unorm_Stencil8;
    case DENOFIZ_FORMAT_R8G8_SNORM:
        return MTLPixelFormatRG8Snorm;
    case DENOFIZ_FORMAT_R8G8_TYPELESS:
        return MTLPixelFormatRG8Uint;
    case DENOFIZ_FORMAT_R8G8_UINT:
        return MTLPixelFormatRG8Uint;
    case DENOFIZ_FORMAT_R8G8_UNORM:
        return MTLPixelFormatRG8Unorm;
    case DENOFIZ_FORMAT_R8G8_SINT:
        return MTLPixelFormatRG8Sint;
    case DENOFIZ_FORMAT_D16_UNORM:
        return MTLPixelFormatDepth16Unorm;
    case DENOFIZ_FORMAT_R16_FLOAT:
        return MTLPixelFormatR16Float;
    case DENOFIZ_FORMAT_R16_SINT:
        return MTLPixelFormatR16Sint;
    case DENOFIZ_FORMAT_R16_SNORM:
        return MTLPixelFormatR16Snorm;
    case DENOFIZ_FORMAT_R16_TYPELESS:
        return MTLPixelFormatR16Uint;
    case DENOFIZ_FORMAT_R16_UINT:
        return MTLPixelFormatR16Uint;
    case DENOFIZ_FORMAT_R16_UNORM:
        return MTLPixelFormatR16Unorm;
    case DENOFIZ_FORMAT_R8_SINT:
        return MTLPixelFormatR8Sint;
    case DENOFIZ_FORMAT_R8_SNORM:
        return MTLPixelFormatR8Snorm;
    case DENOFIZ_FORMAT_R8_TYPELESS:
        return MTLPixelFormatR8Uint;
    case DENOFIZ_FORMAT_R8_UINT:
        return MTLPixelFormatR8Uint;
    case DENOFIZ_FORMAT_R8_UNORM:
        return MTLPixelFormatR8Unorm;
    case DENOFIZ_FORMAT_BC1_UNORM:
        return MTLPixelFormatBC1_RGBA;
    case DENOFIZ_FORMAT_BC1_UNORM_SRGB:
        return MTLPixelFormatBC1_RGBA_sRGB;
    case DENOFIZ_FORMAT_BC2_UNORM:
        return MTLPixelFormatBC2_RGBA;
    case DENOFIZ_FORMAT_BC2_UNORM_SRGB:
        return MTLPixelFormatBC2_RGBA_sRGB;
    case DENOFIZ_FORMAT_BC3_UNORM:
        return MTLPixelFormatBC3_RGBA;
    case DENOFIZ_FORMAT_BC3_UNORM_SRGB:
        return MTLPixelFormatBC3_RGBA_sRGB;
    case DENOFIZ_FORMAT_BC4_UNORM:
        return MTLPixelFormatBC4_RUnorm;
    case DENOFIZ_FORMAT_BC4_SNORM:
        return MTLPixelFormatBC4_RSnorm;
    case DENOFIZ_FORMAT_BC5_UNORM:
        return MTLPixelFormatBC5_RGUnorm;
    case DENOFIZ_FORMAT_BC5_SNORM:
        return MTLPixelFormatBC5_RGSnorm;
    case DENOFIZ_FORMAT_B8G8R8A8_UNORM:
        return MTLPixelFormatBGRA8Unorm;
    case DENOFIZ_FORMAT_BC6H_UFLOAT16:
        return MTLPixelFormatBC6H_RGBUfloat;
    case DENOFIZ_FORMAT_BC6H_SFLOAT16:
        return MTLPixelFormatBC6H_RGBFloat;
    case DENOFIZ_FORMAT_BC7_UNORM:
        return MTLPixelFormatBC7_RGBAUnorm;
    case DENOFIZ_FORMAT_BC7_UNORM_SRGB:
        return MTLPixelFormatBC7_RGBAUnorm_sRGB;
    }

    return MTLPixelFormatInvalid;
}

MTLPrimitiveTopologyClass DenOfIz_MetalEnumConverter_ConvertTopologyClass( const DenOfIz_PrimitiveTopology topology )
{
    switch ( topology )
    {
    case DENOFIZ_PRIMITIVE_TOPOLOGY_POINT:
        return MTLPrimitiveTopologyClassPoint;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_LINE:
        return MTLPrimitiveTopologyClassLine;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE:
        return MTLPrimitiveTopologyClassTriangle;
    default:
        break;
    }
    return MTLPrimitiveTopologyClassTriangle;
}

MTLPrimitiveType DenOfIz_MetalEnumConverter_ConvertTopologyToPrimitiveType( const DenOfIz_PrimitiveTopology topology )
{
    switch ( topology )
    {
        case DENOFIZ_PRIMITIVE_TOPOLOGY_POINT:
            return MTLPrimitiveTypePoint;
            break;
        case DENOFIZ_PRIMITIVE_TOPOLOGY_LINE:
            return MTLPrimitiveTypeLine;
            break;
        case DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE:
            return MTLPrimitiveTypeTriangle;
            break;
        default:
            break;
    }
    return MTLPrimitiveTypeTriangle;
}

MTLVertexFormat DenOfIz_MetalEnumConverter_ConvertFormatToVertexFormat( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_R32G32B32A32_FLOAT:
        return MTLVertexFormatFloat4;
    case DENOFIZ_FORMAT_R32G32B32A32_SINT:
        return MTLVertexFormatInt4;
    case DENOFIZ_FORMAT_R32G32B32A32_UINT:
        return MTLVertexFormatUInt4;
    case DENOFIZ_FORMAT_R32G32B32_FLOAT:
        return MTLVertexFormatFloat3;
    case DENOFIZ_FORMAT_R32G32B32_SINT:
        return MTLVertexFormatInt3;
    case DENOFIZ_FORMAT_R32G32B32_UINT:
        return MTLVertexFormatUInt3;
    case DENOFIZ_FORMAT_R16G16B16A16_FLOAT:
        return MTLVertexFormatHalf4;
    case DENOFIZ_FORMAT_R16G16B16A16_SNORM:
        return MTLVertexFormatShort4Normalized;
    case DENOFIZ_FORMAT_R16G16B16A16_SINT:
        return MTLVertexFormatShort4;
    case DENOFIZ_FORMAT_R16G16B16A16_UNORM:
        return MTLVertexFormatUShort4Normalized;
    case DENOFIZ_FORMAT_R16G16B16A16_UINT:
        return MTLVertexFormatUShort4;
    case DENOFIZ_FORMAT_R32G32_FLOAT:
        return MTLVertexFormatFloat2;
    case DENOFIZ_FORMAT_R32G32_SINT:
        return MTLVertexFormatInt2;
    case DENOFIZ_FORMAT_R32G32_UINT:
        return MTLVertexFormatUInt2;
    case DENOFIZ_FORMAT_R10G10B10A2_UINT:
        return MTLVertexFormatUInt1010102Normalized;
    case DENOFIZ_FORMAT_R10G10B10A2_UNORM:
        return MTLVertexFormatUInt1010102Normalized;
    case DENOFIZ_FORMAT_R8G8B8A8_UINT:
        return MTLVertexFormatUChar4;
    case DENOFIZ_FORMAT_R8G8B8A8_SINT:
        return MTLVertexFormatChar4;
    case DENOFIZ_FORMAT_R8G8B8A8_SNORM:
        return MTLVertexFormatChar4Normalized;
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM:
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB:
        return MTLVertexFormatUChar4Normalized;
    case DENOFIZ_FORMAT_R16G16_FLOAT:
        return MTLVertexFormatHalf2;
    case DENOFIZ_FORMAT_R16G16_SINT:
        return MTLVertexFormatShort2;
    case DENOFIZ_FORMAT_R16G16_SNORM:
        return MTLVertexFormatShort2Normalized;
    case DENOFIZ_FORMAT_R16G16_UNORM:
        return MTLVertexFormatUShort2Normalized;
    case DENOFIZ_FORMAT_R16G16_UINT:
        return MTLVertexFormatUShort2;
    case DENOFIZ_FORMAT_R32_FLOAT:
        return MTLVertexFormatFloat;
    case DENOFIZ_FORMAT_R32_SINT:
        return MTLVertexFormatInt;
    case DENOFIZ_FORMAT_R32_UINT:
        return MTLVertexFormatUInt;
    case DENOFIZ_FORMAT_R8G8_SNORM:
        return MTLVertexFormatChar2Normalized;
    case DENOFIZ_FORMAT_R8G8_UNORM:
        return MTLVertexFormatUChar2Normalized;
    case DENOFIZ_FORMAT_R8G8_SINT:
        return MTLVertexFormatChar2;
    case DENOFIZ_FORMAT_R8G8_UINT:
        return MTLVertexFormatUChar2;
    case DENOFIZ_FORMAT_R16_FLOAT:
        return MTLVertexFormatHalf;
    case DENOFIZ_FORMAT_R16_SINT:
        return MTLVertexFormatShort;
    case DENOFIZ_FORMAT_R16_SNORM:
        return MTLVertexFormatShortNormalized;
    case DENOFIZ_FORMAT_R16_UNORM:
        return MTLVertexFormatUShortNormalized;
    case DENOFIZ_FORMAT_R16_UINT:
        return MTLVertexFormatUShort;
    case DENOFIZ_FORMAT_R8_SINT:
        return MTLVertexFormatChar;
    case DENOFIZ_FORMAT_R8_SNORM:
        return MTLVertexFormatCharNormalized;
    case DENOFIZ_FORMAT_R8_UNORM:
        return MTLVertexFormatUCharNormalized;
    case DENOFIZ_FORMAT_R8_UINT:
        return MTLVertexFormatUChar;
    default:
        spdlog::warn("Warning: DenOfIz_Format does not have a corresponding MTLVertexFormat: {}", static_cast<int>( format ));
        return MTLVertexFormatInvalid;
    }
    spdlog::warn("Warning: Unknown format: {}", static_cast<int>( format ));
    return MTLVertexFormatInvalid;
}

MTLAttributeFormat DenOfIz_MetalEnumConverter_ConvertFormatToAttributeFormat( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_R32G32B32A32_FLOAT:
        return MTLAttributeFormatFloat4;
    case DENOFIZ_FORMAT_R32G32B32A32_SINT:
        return MTLAttributeFormatInt4;
    case DENOFIZ_FORMAT_R32G32B32A32_UINT:
        return MTLAttributeFormatUInt4;
    case DENOFIZ_FORMAT_R32G32B32_FLOAT:
        return MTLAttributeFormatFloat3;
    case DENOFIZ_FORMAT_R32G32B32_SINT:
        return MTLAttributeFormatInt3;
    case DENOFIZ_FORMAT_R32G32B32_UINT:
        return MTLAttributeFormatUInt3;
    case DENOFIZ_FORMAT_R16G16B16A16_FLOAT:
        return MTLAttributeFormatHalf4;
    case DENOFIZ_FORMAT_R16G16B16A16_SNORM:
        return MTLAttributeFormatShort4Normalized;
    case DENOFIZ_FORMAT_R16G16B16A16_SINT:
        return MTLAttributeFormatShort4;
    case DENOFIZ_FORMAT_R16G16B16A16_UNORM:
        return MTLAttributeFormatUShort4Normalized;
    case DENOFIZ_FORMAT_R16G16B16A16_UINT:
        return MTLAttributeFormatUShort4;
    case DENOFIZ_FORMAT_R32G32_FLOAT:
        return MTLAttributeFormatFloat2;
    case DENOFIZ_FORMAT_R32G32_SINT:
        return MTLAttributeFormatInt2;
    case DENOFIZ_FORMAT_R32G32_UINT:
        return MTLAttributeFormatUInt2;
    case DENOFIZ_FORMAT_R10G10B10A2_UINT:
        return MTLAttributeFormatUInt1010102Normalized;
    case DENOFIZ_FORMAT_R10G10B10A2_UNORM:
        return MTLAttributeFormatUInt1010102Normalized;
    case DENOFIZ_FORMAT_R8G8B8A8_UINT:
        return MTLAttributeFormatUChar4;
    case DENOFIZ_FORMAT_R8G8B8A8_SINT:
        return MTLAttributeFormatChar4;
    case DENOFIZ_FORMAT_R8G8B8A8_SNORM:
        return MTLAttributeFormatChar4Normalized;
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM:
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB:
        return MTLAttributeFormatUChar4Normalized;
    case DENOFIZ_FORMAT_R16G16_FLOAT:
        return MTLAttributeFormatHalf2;
    case DENOFIZ_FORMAT_R16G16_SINT:
        return MTLAttributeFormatShort2;
    case DENOFIZ_FORMAT_R16G16_SNORM:
        return MTLAttributeFormatShort2Normalized;
    case DENOFIZ_FORMAT_R16G16_UNORM:
        return MTLAttributeFormatUShort2Normalized;
    case DENOFIZ_FORMAT_R16G16_UINT:
        return MTLAttributeFormatUShort2;
    case DENOFIZ_FORMAT_R32_FLOAT:
        return MTLAttributeFormatFloat;
    case DENOFIZ_FORMAT_R32_SINT:
        return MTLAttributeFormatInt;
    case DENOFIZ_FORMAT_R32_UINT:
        return MTLAttributeFormatUInt;
    case DENOFIZ_FORMAT_R8G8_SNORM:
        return MTLAttributeFormatChar2Normalized;
    case DENOFIZ_FORMAT_R8G8_UNORM:
        return MTLAttributeFormatUChar2Normalized;
    case DENOFIZ_FORMAT_R8G8_SINT:
        return MTLAttributeFormatChar2;
    case DENOFIZ_FORMAT_R8G8_UINT:
        return MTLAttributeFormatUChar2;
    case DENOFIZ_FORMAT_R16_FLOAT:
        return MTLAttributeFormatHalf;
    case DENOFIZ_FORMAT_R16_SINT:
        return MTLAttributeFormatShort;
    case DENOFIZ_FORMAT_R16_SNORM:
        return MTLAttributeFormatShortNormalized;
    case DENOFIZ_FORMAT_R16_UNORM:
        return MTLAttributeFormatUShortNormalized;
    case DENOFIZ_FORMAT_R16_UINT:
        return MTLAttributeFormatUShort;
    case DENOFIZ_FORMAT_R8_SINT:
        return MTLAttributeFormatChar;
    case DENOFIZ_FORMAT_R8_SNORM:
        return MTLAttributeFormatCharNormalized;
    case DENOFIZ_FORMAT_R8_UNORM:
        return MTLAttributeFormatUCharNormalized;
    case DENOFIZ_FORMAT_R8_UINT:
        return MTLAttributeFormatUChar;
    default:
        break;
    }

    spdlog::warn("Warning: Unknown format: {}", static_cast<int>( format ));
    return MTLAttributeFormatInvalid;
}

MTLDataType DenOfIz_MetalEnumConverter_ConvertFormatToDataType( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_UNDEFINED:
        return MTLDataTypeNone;
    case DENOFIZ_FORMAT_R32G32B32A32_FLOAT:
        return MTLDataTypeFloat4;
    case DENOFIZ_FORMAT_R32G32B32A32_SINT:
        return MTLDataTypeInt4;
    case DENOFIZ_FORMAT_R32G32B32A32_UINT:
        return MTLDataTypeUInt4;
    case DENOFIZ_FORMAT_R32G32B32_FLOAT:
        return MTLDataTypeFloat3;
    case DENOFIZ_FORMAT_R32G32B32_SINT:
        return MTLDataTypeInt3;
    case DENOFIZ_FORMAT_R32G32B32_UINT:
        return MTLDataTypeUInt3;
    case DENOFIZ_FORMAT_R16G16B16A16_FLOAT:
        return MTLDataTypeHalf4;
    case DENOFIZ_FORMAT_R16G16B16A16_SNORM:
    case DENOFIZ_FORMAT_R16G16B16A16_SINT:
    case DENOFIZ_FORMAT_R16G16B16A16_TYPELESS:
    case DENOFIZ_FORMAT_R16G16B16A16_UNORM:
    case DENOFIZ_FORMAT_R16G16B16A16_UINT:
        return MTLDataTypeUShort4;
    case DENOFIZ_FORMAT_R32G32_FLOAT:
        return MTLDataTypeFloat2;
    case DENOFIZ_FORMAT_R32G32_SINT:
        return MTLDataTypeInt2;
    case DENOFIZ_FORMAT_R32G32_UINT:
        return MTLDataTypeUInt2;
    case DENOFIZ_FORMAT_R10G10B10A2_UINT:
        return MTLDataTypeUInt;
    case DENOFIZ_FORMAT_R10G10B10A2_UNORM:
        return MTLDataTypeUInt;
    case DENOFIZ_FORMAT_R8G8B8A8_UINT:
        return MTLDataTypeUChar4;
    case DENOFIZ_FORMAT_R8G8B8A8_SINT:
        return MTLDataTypeChar4;
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM:
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB:
        return MTLDataTypeUChar4;
    case DENOFIZ_FORMAT_R8G8B8A8_SNORM:
        return MTLDataTypeChar4;
    case DENOFIZ_FORMAT_R16G16_FLOAT:
        return MTLDataTypeHalf2;
    case DENOFIZ_FORMAT_R16G16_SINT:
        return MTLDataTypeShort2;
    case DENOFIZ_FORMAT_R16G16_UINT:
        return MTLDataTypeUShort2;
    case DENOFIZ_FORMAT_R16G16_UNORM:
        return MTLDataTypeUShort2;
    case DENOFIZ_FORMAT_R16G16_SNORM:
        return MTLDataTypeShort2;
    case DENOFIZ_FORMAT_D32_FLOAT:
        return MTLDataTypeFloat;
    case DENOFIZ_FORMAT_R32_FLOAT:
        return MTLDataTypeFloat;
    case DENOFIZ_FORMAT_R32_SINT:
        return MTLDataTypeInt;
    case DENOFIZ_FORMAT_R32_UINT:
        return MTLDataTypeUInt;
    case DENOFIZ_FORMAT_R8G8_UINT:
        return MTLDataTypeUChar2;
    case DENOFIZ_FORMAT_R8G8_SINT:
        return MTLDataTypeChar2;
    case DENOFIZ_FORMAT_R8G8_UNORM:
        return MTLDataTypeUChar2;
    case DENOFIZ_FORMAT_R8G8_SNORM:
        return MTLDataTypeChar2;
    case DENOFIZ_FORMAT_R16_FLOAT:
        return MTLDataTypeHalf;
    case DENOFIZ_FORMAT_R16_SINT:
        return MTLDataTypeShort;
    case DENOFIZ_FORMAT_R16_UINT:
        return MTLDataTypeUShort;
    case DENOFIZ_FORMAT_R16_UNORM:
        return MTLDataTypeUShort;
    case DENOFIZ_FORMAT_R16_SNORM:
        return MTLDataTypeShort;
    case DENOFIZ_FORMAT_R8_UINT:
        return MTLDataTypeUChar;
    case DENOFIZ_FORMAT_R8_SINT:
        return MTLDataTypeChar;
    case DENOFIZ_FORMAT_R8_UNORM:
        return MTLDataTypeUChar;
    case DENOFIZ_FORMAT_R8_SNORM:
        return MTLDataTypeChar;
    default:
        break;
    }

    return MTLDataTypeNone;
}

MTLSamplerMinMagFilter DenOfIz_MetalEnumConverter_ConvertFilter( const DenOfIz_Filter filter )
{
    switch ( filter )
    {
    case DENOFIZ_FILTER_NEAREST:
        return MTLSamplerMinMagFilterNearest;
    case DENOFIZ_FILTER_LINEAR:
        return MTLSamplerMinMagFilterLinear;
    }
    return MTLSamplerMinMagFilterLinear;
}

MTLSamplerAddressMode DenOfIz_MetalEnumConverter_ConvertSamplerAddressMode( const DenOfIz_SamplerAddressMode mode )
{
    switch ( mode )
    {
    case DENOFIZ_SAMPLER_ADDRESS_MODE_REPEAT:
        return MTLSamplerAddressModeRepeat;
    case DENOFIZ_SAMPLER_ADDRESS_MODE_MIRROR:
        return MTLSamplerAddressModeMirrorRepeat;
    case DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
        return MTLSamplerAddressModeClampToEdge;
    case DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
        return MTLSamplerAddressModeClampToZero;
    }
    return MTLSamplerAddressModeClampToEdge;
}

MTLSamplerMipFilter DenOfIz_MetalEnumConverter_ConvertMipMapFilter( const DenOfIz_MipmapMode mode )
{
    switch ( mode )
    {
    case DENOFIZ_MIPMAP_MODE_NEAREST:
        return MTLSamplerMipFilterNearest;
    case DENOFIZ_MIPMAP_MODE_LINEAR:
        return MTLSamplerMipFilterLinear;
    }
    return MTLSamplerMipFilterLinear;
}

MTLCompareFunction DenOfIz_MetalEnumConverter_ConvertCompareFunction( const DenOfIz_CompareOp op )
{
    switch ( op )
    {
    case DENOFIZ_COMPARE_OP_NEVER:
        return MTLCompareFunctionNever;
    case DENOFIZ_COMPARE_OP_LESS:
        return MTLCompareFunctionLess;
    case DENOFIZ_COMPARE_OP_EQUAL:
        return MTLCompareFunctionEqual;
    case DENOFIZ_COMPARE_OP_LESS_OR_EQUAL:
        return MTLCompareFunctionLessEqual;
    case DENOFIZ_COMPARE_OP_GREATER:
        return MTLCompareFunctionGreater;
    case DENOFIZ_COMPARE_OP_NOT_EQUAL:
        return MTLCompareFunctionNotEqual;
    case DENOFIZ_COMPARE_OP_GREATER_OR_EQUAL:
        return MTLCompareFunctionGreaterEqual;
    case DENOFIZ_COMPARE_OP_ALWAYS:
        return MTLCompareFunctionAlways;
    }
    return MTLCompareFunctionGreaterEqual;
}

MTLLoadAction DenOfIz_MetalEnumConverter_ConvertLoadAction( const DenOfIz_LoadOp op )
{
    switch ( op )
    {
    case DENOFIZ_LOAD_OP_LOAD:
        return MTLLoadActionLoad;
    case DENOFIZ_LOAD_OP_CLEAR:
        return MTLLoadActionClear;
    case DENOFIZ_LOAD_OP_DONT_CARE:
        return MTLLoadActionDontCare;
    }

    return MTLLoadActionDontCare;
}

MTLStoreAction DenOfIz_MetalEnumConverter_ConvertStoreAction( const DenOfIz_StoreOp op )
{
    switch ( op )
    {
    case DENOFIZ_STORE_OP_STORE:
        return MTLStoreActionStore;
    case DENOFIZ_STORE_OP_DONT_CARE:
    case DENOFIZ_STORE_OP_NONE:
        break;
    }

    return MTLStoreActionDontCare;
}

MTLBlendFactor DenOfIz_MetalEnumConverter_ConvertBlendFactor( const DenOfIz_Blend blend )
{
    switch ( blend )
    {
    case DENOFIZ_BLEND_ZERO:
        return MTLBlendFactorZero;
    case DENOFIZ_BLEND_ONE:
        return MTLBlendFactorOne;
    case DENOFIZ_BLEND_SRC_COLOR:
        return MTLBlendFactorSourceColor;
    case DENOFIZ_BLEND_INV_SRC_COLOR:
        return MTLBlendFactorOneMinusSourceColor;
    case DENOFIZ_BLEND_SRC_ALPHA:
        return MTLBlendFactorSourceAlpha;
    case DENOFIZ_BLEND_INV_SRC_ALPHA:
        return MTLBlendFactorOneMinusSourceAlpha;
    case DENOFIZ_BLEND_DST_ALPHA:
        return MTLBlendFactorDestinationAlpha;
    case DENOFIZ_BLEND_INV_DST_ALPHA:
        return MTLBlendFactorOneMinusDestinationAlpha;
    case DENOFIZ_BLEND_DST_COLOR:
        return MTLBlendFactorDestinationColor;
    case DENOFIZ_BLEND_INV_DST_COLOR:
        return MTLBlendFactorOneMinusDestinationColor;
    case DENOFIZ_BLEND_SRC_ALPHA_SATURATE:
        return MTLBlendFactorSourceAlphaSaturated;
    case DENOFIZ_BLEND_BLEND_FACTOR:
        return MTLBlendFactorBlendColor;
    case DENOFIZ_BLEND_INV_BLEND_FACTOR:
        return MTLBlendFactorOneMinusBlendColor;
    case DENOFIZ_BLEND_SRC1_COLOR:
        return MTLBlendFactorSource1Color;
    case DENOFIZ_BLEND_INV_SRC1_COLOR:
        return MTLBlendFactorOneMinusSource1Color;
    case DENOFIZ_BLEND_SRC1_ALPHA:
        return MTLBlendFactorSource1Alpha;
    case DENOFIZ_BLEND_INV_SRC1_ALPHA:
        return MTLBlendFactorOneMinusSource1Alpha;
    }
    return MTLBlendFactorZero;
}

MTLBlendOperation DenOfIz_MetalEnumConverter_ConvertBlendOp( const DenOfIz_BlendOp op )
{
    switch ( op )
    {
    case DENOFIZ_BLEND_OP_ADD:
        return MTLBlendOperationAdd;
    case DENOFIZ_BLEND_OP_SUBTRACT:
        return MTLBlendOperationSubtract;
    case DENOFIZ_BLEND_OP_REVERSE_SUBTRACT:
        return MTLBlendOperationReverseSubtract;
    case DENOFIZ_BLEND_OP_MIN:
        return MTLBlendOperationMin;
    case DENOFIZ_BLEND_OP_MAX:
        return MTLBlendOperationMax;
    }

    return MTLBlendOperationMin;
}

MTLStencilOperation DenOfIz_MetalEnumConverter_ConvertStencilOp( const DenOfIz_StencilOp op )
{
    switch ( op )
    {
    case DENOFIZ_STENCIL_OP_KEEP:
        return MTLStencilOperationKeep;
    case DENOFIZ_STENCIL_OP_ZERO:
        return MTLStencilOperationZero;
    case DENOFIZ_STENCIL_OP_REPLACE:
        return MTLStencilOperationReplace;
    case DENOFIZ_STENCIL_OP_INCREMENT_AND_CLAMP:
        return MTLStencilOperationIncrementClamp;
    case DENOFIZ_STENCIL_OP_DECREMENT_AND_CLAMP:
        return MTLStencilOperationDecrementClamp;
    case DENOFIZ_STENCIL_OP_INVERT:
        return MTLStencilOperationInvert;
    case DENOFIZ_STENCIL_OP_INCREMENT_AND_WRAP:
        return MTLStencilOperationIncrementWrap;
    case DENOFIZ_STENCIL_OP_DECREMENT_AND_WRAP:
        return MTLStencilOperationDecrementWrap;
    }

    return MTLStencilOperationZero;
}

MTLCompareFunction DenOfIz_MetalEnumConverter_ConvertCompareOp( const DenOfIz_CompareOp op )
{
    switch ( op )
    {
    case DENOFIZ_COMPARE_OP_NEVER:
        return MTLCompareFunctionNever;
    case DENOFIZ_COMPARE_OP_LESS:
        return MTLCompareFunctionLess;
    case DENOFIZ_COMPARE_OP_EQUAL:
        return MTLCompareFunctionEqual;
    case DENOFIZ_COMPARE_OP_LESS_OR_EQUAL:
        return MTLCompareFunctionLessEqual;
    case DENOFIZ_COMPARE_OP_GREATER:
        return MTLCompareFunctionGreater;
    case DENOFIZ_COMPARE_OP_NOT_EQUAL:
        return MTLCompareFunctionNotEqual;
    case DENOFIZ_COMPARE_OP_GREATER_OR_EQUAL:
        return MTLCompareFunctionGreaterEqual;
    case DENOFIZ_COMPARE_OP_ALWAYS:
        return MTLCompareFunctionAlways;
    }
    return MTLCompareFunctionGreaterEqual;
}
MTLRenderStages DenOfIz_MetalEnumConverter_ConvertRenderStages( const DenOfIz_ShaderStageFlags stages )
{
    MTLRenderStages mtlStages = 0;

    if ( stages & DENOFIZ_SHADER_STAGE_VERTEX_BIT )
    {
        mtlStages |= MTLRenderStageVertex;
    }
    if ( stages & DENOFIZ_SHADER_STAGE_PIXEL_BIT )
    {
        mtlStages |= MTLRenderStageFragment;
    }
    if ( stages & DENOFIZ_SHADER_STAGE_MESH_BIT )
    {
        mtlStages |= MTLRenderStageMesh;
    }

    return mtlStages != 0 ? mtlStages : MTLRenderStageVertex;
}
