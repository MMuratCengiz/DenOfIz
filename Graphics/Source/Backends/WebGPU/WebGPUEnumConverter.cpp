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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUEnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUContext.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

WGPUTextureFormat DenOfIz_WebGPUEnumConverter_ConvertFormat( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_UNDEFINED:
        return WGPUTextureFormat_Undefined;
    case DENOFIZ_FORMAT_R8_UNORM:
        return WGPUTextureFormat_R8Unorm;
    case DENOFIZ_FORMAT_R8_SNORM:
        return WGPUTextureFormat_R8Snorm;
    case DENOFIZ_FORMAT_R8_UINT:
        return WGPUTextureFormat_R8Uint;
    case DENOFIZ_FORMAT_R8_SINT:
        return WGPUTextureFormat_R8Sint;
    case DENOFIZ_FORMAT_R16_UINT:
        return WGPUTextureFormat_R16Uint;
    case DENOFIZ_FORMAT_R16_SINT:
        return WGPUTextureFormat_R16Sint;
    case DENOFIZ_FORMAT_R16_FLOAT:
        return WGPUTextureFormat_R16Float;
    case DENOFIZ_FORMAT_R8G8_UNORM:
        return WGPUTextureFormat_RG8Unorm;
    case DENOFIZ_FORMAT_R8G8_SNORM:
        return WGPUTextureFormat_RG8Snorm;
    case DENOFIZ_FORMAT_R8G8_UINT:
        return WGPUTextureFormat_RG8Uint;
    case DENOFIZ_FORMAT_R8G8_SINT:
        return WGPUTextureFormat_RG8Sint;
    case DENOFIZ_FORMAT_R32_FLOAT:
        return WGPUTextureFormat_R32Float;
    case DENOFIZ_FORMAT_R32_UINT:
        return WGPUTextureFormat_R32Uint;
    case DENOFIZ_FORMAT_R32_SINT:
        return WGPUTextureFormat_R32Sint;
    case DENOFIZ_FORMAT_R16G16_UINT:
        return WGPUTextureFormat_RG16Uint;
    case DENOFIZ_FORMAT_R16G16_SINT:
        return WGPUTextureFormat_RG16Sint;
    case DENOFIZ_FORMAT_R16G16_FLOAT:
        return WGPUTextureFormat_RG16Float;
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM:
        return WGPUTextureFormat_RGBA8Unorm;
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB:
        return WGPUTextureFormat_RGBA8UnormSrgb;
    case DENOFIZ_FORMAT_R8G8B8A8_SNORM:
        return WGPUTextureFormat_RGBA8Snorm;
    case DENOFIZ_FORMAT_R8G8B8A8_UINT:
        return WGPUTextureFormat_RGBA8Uint;
    case DENOFIZ_FORMAT_R8G8B8A8_SINT:
        return WGPUTextureFormat_RGBA8Sint;
    case DENOFIZ_FORMAT_B8G8R8A8_UNORM:
        return WGPUTextureFormat_BGRA8Unorm;
    case DENOFIZ_FORMAT_R10G10B10A2_UNORM:
        return WGPUTextureFormat_RGB10A2Unorm;
    case DENOFIZ_FORMAT_R32G32_FLOAT:
        return WGPUTextureFormat_RG32Float;
    case DENOFIZ_FORMAT_R32G32_UINT:
        return WGPUTextureFormat_RG32Uint;
    case DENOFIZ_FORMAT_R32G32_SINT:
        return WGPUTextureFormat_RG32Sint;
    case DENOFIZ_FORMAT_R16G16B16A16_UINT:
        return WGPUTextureFormat_RGBA16Uint;
    case DENOFIZ_FORMAT_R16G16B16A16_SINT:
        return WGPUTextureFormat_RGBA16Sint;
    case DENOFIZ_FORMAT_R16G16B16A16_FLOAT:
        return WGPUTextureFormat_RGBA16Float;
    case DENOFIZ_FORMAT_R32G32B32A32_FLOAT:
        return WGPUTextureFormat_RGBA32Float;
    case DENOFIZ_FORMAT_R32G32B32A32_UINT:
        return WGPUTextureFormat_RGBA32Uint;
    case DENOFIZ_FORMAT_R32G32B32A32_SINT:
        return WGPUTextureFormat_RGBA32Sint;
    case DENOFIZ_FORMAT_D16_UNORM:
        return WGPUTextureFormat_Depth16Unorm;
    case DENOFIZ_FORMAT_D32_FLOAT:
        return WGPUTextureFormat_Depth32Float;
    case DENOFIZ_FORMAT_D24_UNORM_S8_UINT:
        return WGPUTextureFormat_Depth24PlusStencil8;
    case DENOFIZ_FORMAT_BC1_UNORM:
        return WGPUTextureFormat_BC1RGBAUnorm;
    case DENOFIZ_FORMAT_BC1_UNORM_SRGB:
        return WGPUTextureFormat_BC1RGBAUnormSrgb;
    case DENOFIZ_FORMAT_BC2_UNORM:
        return WGPUTextureFormat_BC2RGBAUnorm;
    case DENOFIZ_FORMAT_BC2_UNORM_SRGB:
        return WGPUTextureFormat_BC2RGBAUnormSrgb;
    case DENOFIZ_FORMAT_BC3_UNORM:
        return WGPUTextureFormat_BC3RGBAUnorm;
    case DENOFIZ_FORMAT_BC3_UNORM_SRGB:
        return WGPUTextureFormat_BC3RGBAUnormSrgb;
    case DENOFIZ_FORMAT_BC4_UNORM:
        return WGPUTextureFormat_BC4RUnorm;
    case DENOFIZ_FORMAT_BC4_SNORM:
        return WGPUTextureFormat_BC4RSnorm;
    case DENOFIZ_FORMAT_BC5_UNORM:
        return WGPUTextureFormat_BC5RGUnorm;
    case DENOFIZ_FORMAT_BC5_SNORM:
        return WGPUTextureFormat_BC5RGSnorm;
    case DENOFIZ_FORMAT_BC6H_UFLOAT16:
        return WGPUTextureFormat_BC6HRGBUfloat;
    case DENOFIZ_FORMAT_BC6H_SFLOAT16:
        return WGPUTextureFormat_BC6HRGBFloat;
    case DENOFIZ_FORMAT_BC7_UNORM:
        return WGPUTextureFormat_BC7RGBAUnorm;
    case DENOFIZ_FORMAT_BC7_UNORM_SRGB:
        return WGPUTextureFormat_BC7RGBAUnormSrgb;
    default:
        spdlog::error( "WebGPU: Unsupported texture format" );
        return WGPUTextureFormat_RGBA8Unorm;
    }
}

WGPUVertexFormat DenOfIz_WebGPUEnumConverter_ConvertVertexFormat( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_R8_UINT:
        return WGPUVertexFormat_Uint8x2; // Closest match
    case DENOFIZ_FORMAT_R8_SINT:
        return WGPUVertexFormat_Sint8x2; // Closest match
    case DENOFIZ_FORMAT_R8G8_UINT:
        return WGPUVertexFormat_Uint8x2;
    case DENOFIZ_FORMAT_R8G8_SINT:
        return WGPUVertexFormat_Sint8x2;
    case DENOFIZ_FORMAT_R8G8B8A8_UINT:
        return WGPUVertexFormat_Uint8x4;
    case DENOFIZ_FORMAT_R8G8B8A8_SINT:
        return WGPUVertexFormat_Sint8x4;
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM:
        return WGPUVertexFormat_Unorm8x4;
    case DENOFIZ_FORMAT_R8G8B8A8_SNORM:
        return WGPUVertexFormat_Snorm8x4;
    case DENOFIZ_FORMAT_R16_UINT:
        return WGPUVertexFormat_Uint16x2; // Closest match
    case DENOFIZ_FORMAT_R16_SINT:
        return WGPUVertexFormat_Sint16x2; // Closest match
    case DENOFIZ_FORMAT_R16_FLOAT:
        return WGPUVertexFormat_Float16x2; // Closest match
    case DENOFIZ_FORMAT_R16G16_UINT:
        return WGPUVertexFormat_Uint16x2;
    case DENOFIZ_FORMAT_R16G16_SINT:
        return WGPUVertexFormat_Sint16x2;
    case DENOFIZ_FORMAT_R16G16_FLOAT:
        return WGPUVertexFormat_Float16x2;
    case DENOFIZ_FORMAT_R16G16B16A16_UINT:
        return WGPUVertexFormat_Uint16x4;
    case DENOFIZ_FORMAT_R16G16B16A16_SINT:
        return WGPUVertexFormat_Sint16x4;
    case DENOFIZ_FORMAT_R16G16B16A16_FLOAT:
        return WGPUVertexFormat_Float16x4;
    case DENOFIZ_FORMAT_R32_FLOAT:
        return WGPUVertexFormat_Float32;
    case DENOFIZ_FORMAT_R32_UINT:
        return WGPUVertexFormat_Uint32;
    case DENOFIZ_FORMAT_R32_SINT:
        return WGPUVertexFormat_Sint32;
    case DENOFIZ_FORMAT_R32G32_FLOAT:
        return WGPUVertexFormat_Float32x2;
    case DENOFIZ_FORMAT_R32G32_UINT:
        return WGPUVertexFormat_Uint32x2;
    case DENOFIZ_FORMAT_R32G32_SINT:
        return WGPUVertexFormat_Sint32x2;
    case DENOFIZ_FORMAT_R32G32B32_FLOAT:
        return WGPUVertexFormat_Float32x3;
    case DENOFIZ_FORMAT_R32G32B32_UINT:
        return WGPUVertexFormat_Uint32x3;
    case DENOFIZ_FORMAT_R32G32B32_SINT:
        return WGPUVertexFormat_Sint32x3;
    case DENOFIZ_FORMAT_R32G32B32A32_FLOAT:
        return WGPUVertexFormat_Float32x4;
    case DENOFIZ_FORMAT_R32G32B32A32_UINT:
        return WGPUVertexFormat_Uint32x4;
    case DENOFIZ_FORMAT_R32G32B32A32_SINT:
        return WGPUVertexFormat_Sint32x4;
    default:
        spdlog::error( "WebGPU: Unsupported vertex format" );
        return WGPUVertexFormat_Float32x4;
    }
}

WGPUTextureDimension DenOfIz_WebGPUEnumConverter_ConvertTextureDimension( uint32_t width, const uint32_t height, const uint32_t depth )
{
    if ( depth > 1 )
    {
        return WGPUTextureDimension_3D;
    }
    else
    {
        return WGPUTextureDimension_2D;
    }
}

WGPUTextureViewDimension DenOfIz_WebGPUEnumConverter_ConvertTextureViewDimension( const uint32_t arraySize, const uint32_t depth )
{
    if ( depth > 1 )
    {
        return WGPUTextureViewDimension_3D;
    }
    else if ( arraySize > 1 )
    {
        return WGPUTextureViewDimension_2DArray;
    }
    else
    {
        return WGPUTextureViewDimension_2D;
    }
}

WGPUCompareFunction DenOfIz_WebGPUEnumConverter_ConvertCompareOp( const DenOfIz_CompareOp op )
{
    switch ( op )
    {
    case DENOFIZ_COMPARE_OP_NEVER:
        return WGPUCompareFunction_Never;
    case DENOFIZ_COMPARE_OP_LESS:
        return WGPUCompareFunction_Less;
    case DENOFIZ_COMPARE_OP_EQUAL:
        return WGPUCompareFunction_Equal;
    case DENOFIZ_COMPARE_OP_LESS_OR_EQUAL:
        return WGPUCompareFunction_LessEqual;
    case DENOFIZ_COMPARE_OP_GREATER:
        return WGPUCompareFunction_Greater;
    case DENOFIZ_COMPARE_OP_NOT_EQUAL:
        return WGPUCompareFunction_NotEqual;
    case DENOFIZ_COMPARE_OP_GREATER_OR_EQUAL:
        return WGPUCompareFunction_GreaterEqual;
    default:
        return WGPUCompareFunction_Always;
    }
}

WGPUStencilOperation DenOfIz_WebGPUEnumConverter_ConvertStencilOp( const DenOfIz_StencilOp op )
{
    switch ( op )
    {
    case DENOFIZ_STENCIL_OP_KEEP:
        return WGPUStencilOperation_Keep;
    case DENOFIZ_STENCIL_OP_ZERO:
        return WGPUStencilOperation_Zero;
    case DENOFIZ_STENCIL_OP_REPLACE:
        return WGPUStencilOperation_Replace;
    case DENOFIZ_STENCIL_OP_INCREMENT_AND_CLAMP:
        return WGPUStencilOperation_IncrementClamp;
    case DENOFIZ_STENCIL_OP_DECREMENT_AND_CLAMP:
        return WGPUStencilOperation_DecrementClamp;
    case DENOFIZ_STENCIL_OP_INVERT:
        return WGPUStencilOperation_Invert;
    case DENOFIZ_STENCIL_OP_INCREMENT_AND_WRAP:
        return WGPUStencilOperation_IncrementWrap;
    case DENOFIZ_STENCIL_OP_DECREMENT_AND_WRAP:
        return WGPUStencilOperation_DecrementWrap;
    default:
        return WGPUStencilOperation_Keep;
    }
}

WGPUAddressMode DenOfIz_WebGPUEnumConverter_ConvertSamplerAddressMode( const DenOfIz_SamplerAddressMode mode )
{
    switch ( mode )
    {
    case DENOFIZ_SAMPLER_ADDRESS_MODE_REPEAT:
        return WGPUAddressMode_Repeat;
    case DENOFIZ_SAMPLER_ADDRESS_MODE_MIRROR:
        return WGPUAddressMode_MirrorRepeat;
    default:
        return WGPUAddressMode_ClampToEdge;
    }
}

WGPUFilterMode DenOfIz_WebGPUEnumConverter_ConvertFilter( const DenOfIz_Filter filter )
{
    switch ( filter )
    {
    case DENOFIZ_FILTER_NEAREST:
        return WGPUFilterMode_Nearest;
    default:
        return WGPUFilterMode_Linear;
    }
}

WGPUMipmapFilterMode DenOfIz_WebGPUEnumConverter_ConvertMipmapMode( const DenOfIz_MipmapMode mode )
{
    switch ( mode )
    {
    case DENOFIZ_MIPMAP_MODE_NEAREST:
        return WGPUMipmapFilterMode_Nearest;
    default:
        return WGPUMipmapFilterMode_Linear;
    }
}

WGPULoadOp DenOfIz_WebGPUEnumConverter_ConvertLoadOp( const DenOfIz_LoadOp op )
{
    switch ( op )
    {
    case DENOFIZ_LOAD_OP_CLEAR:
        return WGPULoadOp_Clear;
    case DENOFIZ_LOAD_OP_LOAD:
        return WGPULoadOp_Load;
    default:
        return WGPULoadOp_Clear;
    }
}

WGPUStoreOp DenOfIz_WebGPUEnumConverter_ConvertStoreOp( const DenOfIz_StoreOp op )
{
    switch ( op )
    {
    case DENOFIZ_STORE_OP_STORE:
        return WGPUStoreOp_Store;
    case DENOFIZ_STORE_OP_NONE:
    case DENOFIZ_STORE_OP_DONT_CARE:
        return WGPUStoreOp_Discard;
    default:
        return WGPUStoreOp_Store;
    }
}

WGPUPrimitiveTopology DenOfIz_WebGPUEnumConverter_ConvertPrimitiveTopology( const DenOfIz_PrimitiveTopology topology )
{
    switch ( topology )
    {
    case DENOFIZ_PRIMITIVE_TOPOLOGY_POINT:
        return WGPUPrimitiveTopology_PointList;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_LINE:
        return WGPUPrimitiveTopology_LineList;
    default:
        return WGPUPrimitiveTopology_TriangleList;
    }
}

WGPUIndexFormat DenOfIz_WebGPUEnumConverter_ConvertIndexType( const DenOfIz_IndexType type )
{
    switch ( type )
    {
    case DENOFIZ_INDEX_TYPE_UINT16:
        return WGPUIndexFormat_Uint16;
    default:
        return WGPUIndexFormat_Uint32;
    }
}

WGPUCullMode DenOfIz_WebGPUEnumConverter_ConvertCullMode( const DenOfIz_CullMode mode )
{
    switch ( mode )
    {
    case DENOFIZ_CULL_MODE_NONE:
        return WGPUCullMode_None;
    case DENOFIZ_CULL_MODE_FRONT_FACE:
        return WGPUCullMode_Front;
    case DENOFIZ_CULL_MODE_BACK_FACE:
        return WGPUCullMode_Back;
    default:
        return WGPUCullMode_None;
    }
}

WGPUBlendFactor DenOfIz_WebGPUEnumConverter_ConvertBlend( const DenOfIz_Blend factor )
{
    switch ( factor )
    {
    case DENOFIZ_BLEND_ZERO:
        return WGPUBlendFactor_Zero;
    case DENOFIZ_BLEND_ONE:
        return WGPUBlendFactor_One;
    case DENOFIZ_BLEND_SRC_COLOR:
        return WGPUBlendFactor_Src;
    case DENOFIZ_BLEND_INV_SRC_COLOR:
        return WGPUBlendFactor_OneMinusSrc;
    case DENOFIZ_BLEND_SRC_ALPHA:
        return WGPUBlendFactor_SrcAlpha;
    case DENOFIZ_BLEND_INV_SRC_ALPHA:
        return WGPUBlendFactor_OneMinusSrcAlpha;
    case DENOFIZ_BLEND_DST_ALPHA:
        return WGPUBlendFactor_DstAlpha;
    case DENOFIZ_BLEND_INV_DST_ALPHA:
        return WGPUBlendFactor_OneMinusDstAlpha;
    case DENOFIZ_BLEND_DST_COLOR:
        return WGPUBlendFactor_Dst;
    case DENOFIZ_BLEND_INV_DST_COLOR:
        return WGPUBlendFactor_OneMinusDst;
    case DENOFIZ_BLEND_SRC_ALPHA_SATURATE:
        return WGPUBlendFactor_SrcAlphaSaturated;
    case DENOFIZ_BLEND_BLEND_FACTOR:
        return WGPUBlendFactor_Constant;
    case DENOFIZ_BLEND_INV_BLEND_FACTOR:
        return WGPUBlendFactor_OneMinusConstant;
    case DENOFIZ_BLEND_SRC1_COLOR:
        return WGPUBlendFactor_Src; // WebGPU doesn't support dual source blending
    case DENOFIZ_BLEND_INV_SRC1_COLOR:
        return WGPUBlendFactor_OneMinusSrc;
    case DENOFIZ_BLEND_SRC1_ALPHA:
        return WGPUBlendFactor_SrcAlpha;
    case DENOFIZ_BLEND_INV_SRC1_ALPHA:
        return WGPUBlendFactor_OneMinusSrcAlpha;
    default:
        return WGPUBlendFactor_One;
    }
}

WGPUBlendOperation DenOfIz_WebGPUEnumConverter_ConvertBlendOp( const DenOfIz_BlendOp op )
{
    switch ( op )
    {
    case DENOFIZ_BLEND_OP_ADD:
        return WGPUBlendOperation_Add;
    case DENOFIZ_BLEND_OP_SUBTRACT:
        return WGPUBlendOperation_Subtract;
    case DENOFIZ_BLEND_OP_REVERSE_SUBTRACT:
        return WGPUBlendOperation_ReverseSubtract;
    case DENOFIZ_BLEND_OP_MIN:
        return WGPUBlendOperation_Min;
    case DENOFIZ_BLEND_OP_MAX:
        return WGPUBlendOperation_Max;
    default:
        return WGPUBlendOperation_Add;
    }
}

WGPUBufferUsage DenOfIz_WebGPUEnumConverter_ConvertBufferUsage( const uint32_t descriptor, const DenOfIz_HeapType heapType, const uint32_t usages )
{
    uint32_t usage = 0;
    if ( heapType == DENOFIZ_HEAP_TYPE_CPU )
    {
        usage = WGPUBufferUsage_MapWrite | WGPUBufferUsage_CopySrc;
        return static_cast<WGPUBufferUsage>( usage );
    }
    if ( usages & DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT )
    {
        usage |= WGPUBufferUsage_CopySrc;
    }
    if ( usages & DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT )
    {
        usage |= WGPUBufferUsage_CopyDst;
    }
    if ( usages & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT )
    {
        usage |= WGPUBufferUsage_Storage;
    }

    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_VERTEX_BUFFER_BIT )
    {
        usage |= WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_INDEX_BUFFER_BIT )
    {
        usage |= WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_UNIFORM_BUFFER_BIT )
    {
        usage |= WGPUBufferUsage_Uniform;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_STRUCTURED_BUFFER_BIT )
    {
        usage |= WGPUBufferUsage_Storage;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_BUFFER_BIT )
    {
        usage |= WGPUBufferUsage_Storage;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT )
    {
        usage |= WGPUBufferUsage_Storage;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_INDIRECT_BUFFER_BIT )
    {
        usage |= WGPUBufferUsage_Indirect;
    }

    if ( heapType == DENOFIZ_HEAP_TYPE_GPU_CPU )
    {
        usage |= WGPUBufferUsage_MapRead | WGPUBufferUsage_CopySrc;
    }

    if ( heapType == DENOFIZ_HEAP_TYPE_CPU_GPU )
    {
        usage |= WGPUBufferUsage_CopyDst;
    }

    return static_cast<WGPUBufferUsage>( usage );
}

WGPUTextureUsage DenOfIz_WebGPUEnumConverter_ConvertTextureUsage( const DenOfIz_ResourceDescriptorFlags descriptor, const DenOfIz_HeapType heapType )
{
    uint32_t usage = WGPUTextureUsage_CopyDst;

    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT )
    {
        usage |= WGPUTextureUsage_TextureBinding;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_RW_TEXTURE_BIT )
    {
        usage |= WGPUTextureUsage_StorageBinding;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_RENDER_TARGET_BIT )
    {
        usage |= WGPUTextureUsage_RenderAttachment;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_DEPTH_STENCIL_BIT )
    {
        usage |= WGPUTextureUsage_RenderAttachment;
    }

    if ( heapType == DENOFIZ_HEAP_TYPE_GPU_CPU )
    {
        usage |= WGPUTextureUsage_CopySrc;
    }

    return static_cast<WGPUTextureUsage>( usage );
}

WGPUTextureUsage DenOfIz_WebGPUEnumConverter_ConvertSwapChainTextureUsage( const DenOfIz_ResourceDescriptorFlags descriptor, const DenOfIz_ResourceUsageFlags usages )
{
    uint32_t usage = 0;

    if ( ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT ) || ( usages & DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT ) )
    {
        usage |= WGPUTextureUsage_TextureBinding;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_RW_TEXTURE_BIT )
    {
        usage |= WGPUTextureUsage_StorageBinding;
    }
    if ( ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_RENDER_TARGET_BIT ) || ( usages & DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT ) )
    {
        usage |= WGPUTextureUsage_RenderAttachment;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_DEPTH_STENCIL_BIT )
    {
        usage |= WGPUTextureUsage_RenderAttachment;
    }
    if ( usages & DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT )
    {
        usage |= WGPUTextureUsage_CopySrc;
    }
    if ( usages & DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT )
    {
        usage |= WGPUTextureUsage_CopyDst;
    }
    if ( usages & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT )
    {
        usage |= WGPUTextureUsage_StorageBinding;
    }

    return static_cast<WGPUTextureUsage>( usage );
}

WGPUTextureAspect DenOfIz_WebGPUEnumConverter_ConvertTextureAspect( const DenOfIz_TextureAspect aspect )
{
    switch ( aspect )
    {
    case DENOFIZ_TEXTURE_ASPECT_COLOR:
        return WGPUTextureAspect_All;
    case DENOFIZ_TEXTURE_ASPECT_DEPTH:
        return WGPUTextureAspect_DepthOnly;
    case DENOFIZ_TEXTURE_ASPECT_STENCIL:
        return WGPUTextureAspect_StencilOnly;
    default:
        return WGPUTextureAspect_All;
    }
}

WGPUTextureAspect DenOfIz_WebGPUEnumConverter_GetTextureAspectFromFormat( const DenOfIz_Format format )
{
    if ( DenOfIz_Format_IsDepthStencil( format ) )
    {
        return WGPUTextureAspect_All;
    }
    if ( DenOfIz_Format_IsDepth( format ) )
    {
        return WGPUTextureAspect_DepthOnly;
    }
    return WGPUTextureAspect_All;
}

WGPUVertexStepMode DenOfIz_WebGPUEnumConverter_ConvertStepRate( const DenOfIz_StepRate stepRate )
{
    switch ( stepRate )
    {
    case DENOFIZ_STEP_RATE_PER_VERTEX:
        return WGPUVertexStepMode_Vertex;
    case DENOFIZ_STEP_RATE_PER_INSTANCE:
        return WGPUVertexStepMode_Instance;
    default:
        return WGPUVertexStepMode_Vertex;
    }
}

uint32_t DenOfIz_WebGPUEnumConverter_ConvertSampleCount( const DenOfIz_MSAASampleCount sampleCount )
{
    switch ( sampleCount )
    {
    case DENOFIZ_MSAA_SAMPLE_COUNT_1:
        return 1;
    case DENOFIZ_MSAA_SAMPLE_COUNT_2:
        return 2;
    case DENOFIZ_MSAA_SAMPLE_COUNT_4:
        return 4;
    case DENOFIZ_MSAA_SAMPLE_COUNT_8:
        return 8;
    case DENOFIZ_MSAA_SAMPLE_COUNT_16:
        return 16;
    case DENOFIZ_MSAA_SAMPLE_COUNT_32:
        return 32;
    case DENOFIZ_MSAA_SAMPLE_COUNT_64:
        return 64;
    default:
        return 1;
    }
}

WGPUShaderStage DenOfIz_WebGPUEnumConverter_ConvertShaderStage( const DenOfIz_ShaderStageFlags stages )
{
    uint32_t result = 0;

    if ( stages & DENOFIZ_SHADER_STAGE_VERTEX_BIT )
    {
        result |= WGPUShaderStage_Vertex;
    }
    if ( stages & DENOFIZ_SHADER_STAGE_PIXEL_BIT )
    {
        result |= WGPUShaderStage_Fragment;
    }
    if ( stages & DENOFIZ_SHADER_STAGE_COMPUTE_BIT )
    {
        result |= WGPUShaderStage_Compute;
    }

    return static_cast<WGPUShaderStage>( result );
}
