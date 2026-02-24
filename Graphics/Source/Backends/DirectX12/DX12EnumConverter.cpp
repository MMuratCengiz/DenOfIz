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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12EnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

D3D12_DESCRIPTOR_RANGE_TYPE DenOfIz_DX12EnumConverter_ConvertResourceDescriptorToDescriptorRangeType( const DenOfIz_ResourceDescriptorFlags descriptor )
{
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_SAMPLER_BIT )
    {
        return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    }
    if ( descriptor & ( DENOFIZ_RESOURCE_DESCRIPTOR_UNIFORM_BUFFER_BIT | DENOFIZ_RESOURCE_DESCRIPTOR_ROOT_CONSTANT_BIT ) )
    {
        return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    }
    if ( descriptor & ( DENOFIZ_RESOURCE_DESCRIPTOR_RW_TEXTURE_BIT | DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT ) )
    {
        return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    }
    if ( descriptor & ( DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT | DENOFIZ_RESOURCE_DESCRIPTOR_BUFFER_BIT | DENOFIZ_RESOURCE_DESCRIPTOR_ACCELERATION_STRUCTURE_BIT ) )
    {
        return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    }

    return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
}

D3D12_COMMAND_LIST_TYPE DenOfIz_DX12EnumConverter_ConvertQueueType( const DenOfIz_QueueType queueType )
{
    switch ( queueType )
    {
    case DENOFIZ_QUEUE_TYPE_GRAPHICS:
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case DENOFIZ_QUEUE_TYPE_COMPUTE:
        return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    case DENOFIZ_QUEUE_TYPE_COPY:
        return D3D12_COMMAND_LIST_TYPE_COPY;
    }

    return D3D12_COMMAND_LIST_TYPE_DIRECT;
}

D3D12_HEAP_TYPE DenOfIz_DX12EnumConverter_ConvertHeapType( const DenOfIz_HeapType heapType )
{
    switch ( heapType )
    {
    case DENOFIZ_HEAP_TYPE_GPU:
        return D3D12_HEAP_TYPE_DEFAULT;
    case DENOFIZ_HEAP_TYPE_CPU:
    case DENOFIZ_HEAP_TYPE_CPU_GPU:
        return D3D12_HEAP_TYPE_UPLOAD;
    case DENOFIZ_HEAP_TYPE_GPU_CPU:
        return D3D12_HEAP_TYPE_READBACK;
    }

    return D3D12_HEAP_TYPE_DEFAULT;
}

uint32_t DenOfIz_DX12EnumConverter_ConvertSampleCount( const DenOfIz_MSAASampleCount sampleCount )
{
    switch ( sampleCount )
    {
    case DENOFIZ_MSAA_SAMPLE_COUNT_0:
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
    case DENOFIZ_MSAA_SAMPLE_COUNT_64:
        spdlog::warn( "Exceeded the maximum sample count of 16 for this API, defaulting to 16." );
        return 16;
    }

    return 1;
}

DXGI_FORMAT DenOfIz_DX12EnumConverter_ConvertFormat( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_UNDEFINED:
        return DXGI_FORMAT_UNKNOWN;
    case DENOFIZ_FORMAT_R32G32B32A32_FLOAT:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case DENOFIZ_FORMAT_R32G32B32A32_UINT:
        return DXGI_FORMAT_R32G32B32A32_UINT;
    case DENOFIZ_FORMAT_R32G32B32A32_SINT:
        return DXGI_FORMAT_R32G32B32A32_SINT;
    case DENOFIZ_FORMAT_R32G32B32_FLOAT:
        return DXGI_FORMAT_R32G32B32_FLOAT;
    case DENOFIZ_FORMAT_R32G32B32_UINT:
        return DXGI_FORMAT_R32G32B32_UINT;
    case DENOFIZ_FORMAT_R32G32B32_SINT:
        return DXGI_FORMAT_R32G32B32_SINT;
    case DENOFIZ_FORMAT_R16G16B16A16_FLOAT:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case DENOFIZ_FORMAT_R16G16B16A16_UNORM:
        return DXGI_FORMAT_R16G16B16A16_UNORM;
    case DENOFIZ_FORMAT_R16G16B16A16_UINT:
        return DXGI_FORMAT_R16G16B16A16_UINT;
    case DENOFIZ_FORMAT_R16G16B16A16_SNORM:
        return DXGI_FORMAT_R16G16B16A16_SNORM;
    case DENOFIZ_FORMAT_R16G16B16A16_SINT:
        return DXGI_FORMAT_R16G16B16A16_SINT;
    case DENOFIZ_FORMAT_R32G32_FLOAT:
        return DXGI_FORMAT_R32G32_FLOAT;
    case DENOFIZ_FORMAT_R32G32_UINT:
        return DXGI_FORMAT_R32G32_UINT;
    case DENOFIZ_FORMAT_R32G32_SINT:
        return DXGI_FORMAT_R32G32_SINT;
    case DENOFIZ_FORMAT_R10G10B10A2_UNORM:
        return DXGI_FORMAT_R10G10B10A2_UNORM;
    case DENOFIZ_FORMAT_R10G10B10A2_UINT:
        return DXGI_FORMAT_R10G10B10A2_UINT;
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB:
        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case DENOFIZ_FORMAT_R8G8B8A8_UINT:
        return DXGI_FORMAT_R8G8B8A8_UINT;
    case DENOFIZ_FORMAT_R8G8B8A8_SNORM:
        return DXGI_FORMAT_R8G8B8A8_SNORM;
    case DENOFIZ_FORMAT_R8G8B8A8_SINT:
        return DXGI_FORMAT_R8G8B8A8_SINT;
    case DENOFIZ_FORMAT_R16G16_FLOAT:
        return DXGI_FORMAT_R16G16_FLOAT;
    case DENOFIZ_FORMAT_R16G16_UNORM:
        return DXGI_FORMAT_R16G16_UNORM;
    case DENOFIZ_FORMAT_R16G16_UINT:
        return DXGI_FORMAT_R16G16_UINT;
    case DENOFIZ_FORMAT_R16G16_SNORM:
        return DXGI_FORMAT_R16G16_SNORM;
    case DENOFIZ_FORMAT_R16G16_SINT:
        return DXGI_FORMAT_R16G16_SINT;
    case DENOFIZ_FORMAT_D32_FLOAT:
        return DXGI_FORMAT_D32_FLOAT;
    case DENOFIZ_FORMAT_R32_FLOAT:
        return DXGI_FORMAT_R32_FLOAT;
    case DENOFIZ_FORMAT_R32_UINT:
        return DXGI_FORMAT_R32_UINT;
    case DENOFIZ_FORMAT_R32_SINT:
        return DXGI_FORMAT_R32_SINT;
    case DENOFIZ_FORMAT_D24_UNORM_S8_UINT:
        return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case DENOFIZ_FORMAT_R8G8_UNORM:
        return DXGI_FORMAT_R8G8_UNORM;
    case DENOFIZ_FORMAT_R8G8_UINT:
        return DXGI_FORMAT_R8G8_UINT;
    case DENOFIZ_FORMAT_R8G8_SNORM:
        return DXGI_FORMAT_R8G8_SNORM;
    case DENOFIZ_FORMAT_R8G8_SINT:
        return DXGI_FORMAT_R8G8_SINT;
    case DENOFIZ_FORMAT_R16_FLOAT:
        return DXGI_FORMAT_R16_FLOAT;
    case DENOFIZ_FORMAT_D16_UNORM:
        return DXGI_FORMAT_D16_UNORM;
    case DENOFIZ_FORMAT_R16_UNORM:
        return DXGI_FORMAT_R16_UNORM;
    case DENOFIZ_FORMAT_R16_UINT:
        return DXGI_FORMAT_R16_UINT;
    case DENOFIZ_FORMAT_R16_SNORM:
        return DXGI_FORMAT_R16_SNORM;
    case DENOFIZ_FORMAT_R16_SINT:
        return DXGI_FORMAT_R16_SINT;
    case DENOFIZ_FORMAT_R8_UNORM:
        return DXGI_FORMAT_R8_UNORM;
    case DENOFIZ_FORMAT_R8_UINT:
        return DXGI_FORMAT_R8_UINT;
    case DENOFIZ_FORMAT_R8_SNORM:
        return DXGI_FORMAT_R8_SNORM;
    case DENOFIZ_FORMAT_R8_SINT:
        return DXGI_FORMAT_R8_SINT;
    case DENOFIZ_FORMAT_BC1_UNORM:
        return DXGI_FORMAT_BC1_UNORM;
    case DENOFIZ_FORMAT_BC1_UNORM_SRGB:
        return DXGI_FORMAT_BC1_UNORM_SRGB;
    case DENOFIZ_FORMAT_BC2_UNORM:
        return DXGI_FORMAT_BC2_UNORM;
    case DENOFIZ_FORMAT_BC2_UNORM_SRGB:
        return DXGI_FORMAT_BC2_UNORM_SRGB;
    case DENOFIZ_FORMAT_BC3_UNORM:
        return DXGI_FORMAT_BC3_UNORM;
    case DENOFIZ_FORMAT_BC3_UNORM_SRGB:
        return DXGI_FORMAT_BC3_UNORM_SRGB;
    case DENOFIZ_FORMAT_BC4_UNORM:
        return DXGI_FORMAT_BC4_UNORM;
    case DENOFIZ_FORMAT_BC4_SNORM:
        return DXGI_FORMAT_BC4_SNORM;
    case DENOFIZ_FORMAT_BC5_UNORM:
        return DXGI_FORMAT_BC5_UNORM;
    case DENOFIZ_FORMAT_BC5_SNORM:
        return DXGI_FORMAT_BC5_SNORM;
    case DENOFIZ_FORMAT_B8G8R8A8_UNORM:
        return DXGI_FORMAT_B8G8R8A8_UNORM;
    case DENOFIZ_FORMAT_BC6H_UFLOAT16:
        return DXGI_FORMAT_BC6H_UF16;
    case DENOFIZ_FORMAT_BC6H_SFLOAT16:
        return DXGI_FORMAT_BC6H_SF16;
    case DENOFIZ_FORMAT_BC7_UNORM:
        return DXGI_FORMAT_BC7_UNORM;
    case DENOFIZ_FORMAT_BC7_UNORM_SRGB:
        return DXGI_FORMAT_BC7_UNORM_SRGB;
    case DENOFIZ_FORMAT_R32G32B32A32_TYPELESS:
        return DXGI_FORMAT_R32G32B32A32_TYPELESS;
    case DENOFIZ_FORMAT_R16G16B16A16_TYPELESS:
        return DXGI_FORMAT_R16G16B16A16_TYPELESS;
    case DENOFIZ_FORMAT_R32G32_TYPELESS:
        return DXGI_FORMAT_R32G32_TYPELESS;
    case DENOFIZ_FORMAT_R10G10B10A2_TYPELESS:
        return DXGI_FORMAT_R10G10B10A2_TYPELESS;
    case DENOFIZ_FORMAT_R8G8B8A8_TYPELESS:
        return DXGI_FORMAT_R8G8B8A8_TYPELESS;
    case DENOFIZ_FORMAT_R16G16_TYPELESS:
        return DXGI_FORMAT_R16G16_TYPELESS;
    case DENOFIZ_FORMAT_R32_TYPELESS:
        return DXGI_FORMAT_R32_TYPELESS;
    case DENOFIZ_FORMAT_R8G8_TYPELESS:
        return DXGI_FORMAT_R8G8_TYPELESS;
    case DENOFIZ_FORMAT_R16_TYPELESS:
        return DXGI_FORMAT_R16_TYPELESS;
    case DENOFIZ_FORMAT_R8_TYPELESS:
        return DXGI_FORMAT_R8_TYPELESS;
    }

    return DXGI_FORMAT_UNKNOWN;
}

D3D12_SHADER_VISIBILITY DenOfIz_DX12EnumConverter_ConvertShaderStageToShaderVisibility( const DenOfIz_ShaderStageFlags stages )
{
    const int popCount = __popcnt( stages );
    if ( popCount > 1 )
    {
        return D3D12_SHADER_VISIBILITY_ALL;
    }

    if ( stages & DENOFIZ_SHADER_STAGE_VERTEX_BIT )
    {
        return D3D12_SHADER_VISIBILITY_VERTEX;
    }
    if ( stages & DENOFIZ_SHADER_STAGE_HULL_BIT )
    {
        return D3D12_SHADER_VISIBILITY_HULL;
    }
    if ( stages & DENOFIZ_SHADER_STAGE_DOMAIN_BIT )
    {
        return D3D12_SHADER_VISIBILITY_DOMAIN;
    }
    if ( stages & DENOFIZ_SHADER_STAGE_GEOMETRY_BIT )
    {
        return D3D12_SHADER_VISIBILITY_GEOMETRY;
    }
    if ( stages & DENOFIZ_SHADER_STAGE_PIXEL_BIT )
    {
        return D3D12_SHADER_VISIBILITY_PIXEL;
    }
    if ( stages & DENOFIZ_SHADER_STAGE_MESH_BIT )
    {
        return D3D12_SHADER_VISIBILITY_MESH;
    }
    return D3D12_SHADER_VISIBILITY_ALL;
}

D3D12_COMPARISON_FUNC DenOfIz_DX12EnumConverter_ConvertCompareOp( const DenOfIz_CompareOp op )
{
    switch ( op )
    {
    case DENOFIZ_COMPARE_OP_NEVER:
        return D3D12_COMPARISON_FUNC_NEVER;
    case DENOFIZ_COMPARE_OP_EQUAL:
        return D3D12_COMPARISON_FUNC_EQUAL;
    case DENOFIZ_COMPARE_OP_NOT_EQUAL:
        return D3D12_COMPARISON_FUNC_NOT_EQUAL;
    case DENOFIZ_COMPARE_OP_ALWAYS:
        return D3D12_COMPARISON_FUNC_ALWAYS;
    case DENOFIZ_COMPARE_OP_LESS:
        return D3D12_COMPARISON_FUNC_LESS;
    case DENOFIZ_COMPARE_OP_LESS_OR_EQUAL:
        return D3D12_COMPARISON_FUNC_LESS_EQUAL;
    case DENOFIZ_COMPARE_OP_GREATER:
        return D3D12_COMPARISON_FUNC_GREATER;
    case DENOFIZ_COMPARE_OP_GREATER_OR_EQUAL:
        return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    }

    return D3D12_COMPARISON_FUNC_EQUAL;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE DenOfIz_DX12EnumConverter_ConvertPrimitiveTopologyToType( const DenOfIz_PrimitiveTopology topology )
{
    switch ( topology )
    {
    case DENOFIZ_PRIMITIVE_TOPOLOGY_POINT:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_LINE:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_PATCH:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    }

    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
}

D3D12_PRIMITIVE_TOPOLOGY DenOfIz_DX12EnumConverter_ConvertPrimitiveTopology( const DenOfIz_PrimitiveTopology topology )
{
    switch ( topology )
    {
    case DENOFIZ_PRIMITIVE_TOPOLOGY_POINT:
        return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_LINE:
        return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_PATCH:
        return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST; // Todo could require more control points
    }

    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

D3D12_STENCIL_OP DenOfIz_DX12EnumConverter_ConvertStencilOp( const DenOfIz_StencilOp op )
{
    switch ( op )
    {
    case DENOFIZ_STENCIL_OP_KEEP:
        return D3D12_STENCIL_OP_KEEP;
    case DENOFIZ_STENCIL_OP_ZERO:
        return D3D12_STENCIL_OP_ZERO;
    case DENOFIZ_STENCIL_OP_REPLACE:
        return D3D12_STENCIL_OP_REPLACE;
    case DENOFIZ_STENCIL_OP_INCREMENT_AND_CLAMP:
        return D3D12_STENCIL_OP_INCR_SAT;
    case DENOFIZ_STENCIL_OP_DECREMENT_AND_CLAMP:
        return D3D12_STENCIL_OP_DECR_SAT;
    case DENOFIZ_STENCIL_OP_INVERT:
        return D3D12_STENCIL_OP_INVERT;
    case DENOFIZ_STENCIL_OP_INCREMENT_AND_WRAP:
        return D3D12_STENCIL_OP_INCR;
    case DENOFIZ_STENCIL_OP_DECREMENT_AND_WRAP:
        return D3D12_STENCIL_OP_DECR;
    }

    return D3D12_STENCIL_OP_KEEP;
}

D3D12_CULL_MODE DenOfIz_DX12EnumConverter_ConvertCullMode( const DenOfIz_CullMode mode )
{
    switch ( mode )
    {
    case DENOFIZ_CULL_MODE_FRONT_FACE:
        return D3D12_CULL_MODE_FRONT;
    case DENOFIZ_CULL_MODE_BACK_FACE:
        return D3D12_CULL_MODE_BACK;
    case DENOFIZ_CULL_MODE_NONE:
        return D3D12_CULL_MODE_NONE;
    }

    return D3D12_CULL_MODE_NONE;
}

D3D12_FILL_MODE DenOfIz_DX12EnumConverter_ConvertFillMode( const DenOfIz_FillMode mode )
{
    switch ( mode )
    {
    case DENOFIZ_FILL_MODE_SOLID:
        return D3D12_FILL_MODE_SOLID;
    case DENOFIZ_FILL_MODE_WIREFRAME:
        return D3D12_FILL_MODE_WIREFRAME;
    }
    return D3D12_FILL_MODE_SOLID;
}

D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE DenOfIz_DX12EnumConverter_ConvertLoadOp( const DenOfIz_LoadOp op )
{
    switch ( op )
    {
    case DENOFIZ_LOAD_OP_CLEAR:
        return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
    case DENOFIZ_LOAD_OP_LOAD:
        return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
    case DENOFIZ_LOAD_OP_DONT_CARE:
        return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
    }

    return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
}

D3D12_RENDER_PASS_ENDING_ACCESS_TYPE DenOfIz_DX12EnumConverter_ConvertStoreOp( const DenOfIz_StoreOp op )
{
    switch ( op )
    {
    case DENOFIZ_STORE_OP_STORE:
        return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
    case DENOFIZ_STORE_OP_DONT_CARE:
        return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
    case DENOFIZ_STORE_OP_NONE:
        return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
    }

    return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
}

D3D12_BLEND_OP DenOfIz_DX12EnumConverter_ConvertBlendOp( const DenOfIz_BlendOp op )
{
    switch ( op )
    {
    case DENOFIZ_BLEND_OP_ADD:
        return D3D12_BLEND_OP_ADD;
    case DENOFIZ_BLEND_OP_SUBTRACT:
        return D3D12_BLEND_OP_SUBTRACT;
    case DENOFIZ_BLEND_OP_REVERSE_SUBTRACT:
        return D3D12_BLEND_OP_REV_SUBTRACT;
    case DENOFIZ_BLEND_OP_MIN:
        return D3D12_BLEND_OP_MIN;
    case DENOFIZ_BLEND_OP_MAX:
        return D3D12_BLEND_OP_MAX;
    }

    return D3D12_BLEND_OP_ADD;
}

D3D12_LOGIC_OP DenOfIz_DX12EnumConverter_ConvertLogicOp( const DenOfIz_LogicOp op )
{
    switch ( op )
    {
    case DENOFIZ_LOGIC_OP_CLEAR:
        return D3D12_LOGIC_OP_CLEAR;
    case DENOFIZ_LOGIC_OP_SET:
        return D3D12_LOGIC_OP_SET;
    case DENOFIZ_LOGIC_OP_COPY:
        return D3D12_LOGIC_OP_COPY;
    case DENOFIZ_LOGIC_OP_COPY_INVERTED:
        return D3D12_LOGIC_OP_COPY_INVERTED;
    case DENOFIZ_LOGIC_OP_NO_OP:
        return D3D12_LOGIC_OP_NOOP;
    case DENOFIZ_LOGIC_OP_INVERT:
        return D3D12_LOGIC_OP_INVERT;
    case DENOFIZ_LOGIC_OP_AND:
        return D3D12_LOGIC_OP_AND;
    case DENOFIZ_LOGIC_OP_NAND:
        return D3D12_LOGIC_OP_NAND;
    case DENOFIZ_LOGIC_OP_OR:
        return D3D12_LOGIC_OP_OR;
    case DENOFIZ_LOGIC_OP_NOR:
        return D3D12_LOGIC_OP_NOR;
    case DENOFIZ_LOGIC_OP_XOR:
        return D3D12_LOGIC_OP_XOR;
    case DENOFIZ_LOGIC_OP_EQUIV:
        return D3D12_LOGIC_OP_EQUIV;
    case DENOFIZ_LOGIC_OP_AND_REVERSE:
        return D3D12_LOGIC_OP_AND_REVERSE;
    case DENOFIZ_LOGIC_OP_AND_INVERTED:
        return D3D12_LOGIC_OP_AND_INVERTED;
    case DENOFIZ_LOGIC_OP_OR_REVERSE:
        return D3D12_LOGIC_OP_OR_REVERSE;
    case DENOFIZ_LOGIC_OP_OR_INVERTED:
        return D3D12_LOGIC_OP_OR_INVERTED;
    }

    return D3D12_LOGIC_OP_CLEAR;
}

D3D12_BLEND DenOfIz_DX12EnumConverter_ConvertBlend( const DenOfIz_Blend factor )
{
    switch ( factor )
    {
    case DENOFIZ_BLEND_ZERO:
        return D3D12_BLEND_ZERO;
    case DENOFIZ_BLEND_ONE:
        return D3D12_BLEND_ONE;
    case DENOFIZ_BLEND_SRC_COLOR:
        return D3D12_BLEND_SRC_COLOR;
    case DENOFIZ_BLEND_INV_SRC_COLOR:
        return D3D12_BLEND_INV_SRC_COLOR;
    case DENOFIZ_BLEND_SRC_ALPHA:
        return D3D12_BLEND_SRC_ALPHA;
    case DENOFIZ_BLEND_INV_SRC_ALPHA:
        return D3D12_BLEND_INV_SRC_ALPHA;
    case DENOFIZ_BLEND_DST_ALPHA:
        return D3D12_BLEND_DEST_ALPHA;
    case DENOFIZ_BLEND_INV_DST_ALPHA:
        return D3D12_BLEND_INV_DEST_ALPHA;
    case DENOFIZ_BLEND_DST_COLOR:
        return D3D12_BLEND_DEST_COLOR;
    case DENOFIZ_BLEND_INV_DST_COLOR:
        return D3D12_BLEND_INV_DEST_COLOR;
    case DENOFIZ_BLEND_SRC_ALPHA_SATURATE:
        return D3D12_BLEND_SRC_ALPHA_SAT;
    case DENOFIZ_BLEND_SRC1_COLOR:
        return D3D12_BLEND_SRC1_COLOR;
    case DENOFIZ_BLEND_INV_SRC1_COLOR:
        return D3D12_BLEND_INV_SRC1_COLOR;
    case DENOFIZ_BLEND_SRC1_ALPHA:
        return D3D12_BLEND_SRC1_ALPHA;
    case DENOFIZ_BLEND_INV_SRC1_ALPHA:
        return D3D12_BLEND_INV_SRC1_ALPHA;
    case DENOFIZ_BLEND_BLEND_FACTOR:
        return D3D12_BLEND_BLEND_FACTOR;
    case DENOFIZ_BLEND_INV_BLEND_FACTOR:
        return D3D12_BLEND_INV_BLEND_FACTOR;
    }

    return D3D12_BLEND_ZERO;
}

D3D12_RESOURCE_STATES DenOfIz_DX12EnumConverter_ConvertResourceUsage( const DenOfIz_ResourceUsageFlags usage )
{
    D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON;
    if ( usage & DENOFIZ_RESOURCE_USAGE_GENERIC_READ_BIT )
    {
        return D3D12_RESOURCE_STATE_GENERIC_READ;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_COMMON_BIT )
    {
        return D3D12_RESOURCE_STATE_COMMON;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_PRESENT_BIT )
    {
        return D3D12_RESOURCE_STATE_PRESENT;
    }

    if ( usage & DENOFIZ_RESOURCE_USAGE_VERTEX_AND_CONSTANT_BUFFER_BIT )
    {
        result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_INDEX_BUFFER_BIT )
    {
        result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT )
    {
        result |= D3D12_RESOURCE_STATE_RENDER_TARGET;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT )
    {
        result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT )
    {
        result |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
    }
    else if ( usage & DENOFIZ_RESOURCE_USAGE_DEPTH_READ_BIT )
    {
        result |= D3D12_RESOURCE_STATE_DEPTH_READ;
    }

    if ( usage & DENOFIZ_RESOURCE_USAGE_STREAM_OUT_BIT )
    {
        result |= D3D12_RESOURCE_STATE_STREAM_OUT;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_INDIRECT_ARGUMENT_BIT )
    {
        result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT )
    {
        result |= D3D12_RESOURCE_STATE_COPY_DEST;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT )
    {
        result |= D3D12_RESOURCE_STATE_COPY_SOURCE;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT )
    {
        result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_PIXEL_SHADER_RESOURCE_BIT )
    {
        result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }
    if ( ( usage & DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT ) || ( usage & DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT ) )
    {
        result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    }
    return result;
}

D3D12_BARRIER_LAYOUT DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierLayout( const DenOfIz_ResourceUsageFlags state, const DenOfIz_QueueType queueType,
                                                                                    bool enhancedBarriers )
{
    auto queueSpecificResult = [ = ]( const D3D12_BARRIER_LAYOUT direct, const D3D12_BARRIER_LAYOUT compute, const D3D12_BARRIER_LAYOUT other )
    {
        if ( enhancedBarriers )
        {
            return other;
        }

        switch ( queueType )
        {
        case DENOFIZ_QUEUE_TYPE_GRAPHICS:
            return direct;
        case DENOFIZ_QUEUE_TYPE_COMPUTE:
            return compute;
        default:
            return other;
        }
    };

    if ( state & DENOFIZ_RESOURCE_USAGE_COMMON_BIT )
    {
        return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COMMON, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COMMON, D3D12_BARRIER_LAYOUT_COMMON );
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_PRESENT_BIT )
    {
        return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COMMON, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COMMON, D3D12_BARRIER_LAYOUT_PRESENT );
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_GENERIC_READ_BIT )
    {
        return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_GENERIC_READ, D3D12_BARRIER_LAYOUT_GENERIC_READ );
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT )
    {
        return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_SOURCE, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_SOURCE, D3D12_BARRIER_LAYOUT_COPY_SOURCE );
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT )
    {
        return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_DEST, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_DEST, D3D12_BARRIER_LAYOUT_COPY_DEST );
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT )
    {
        return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_UNORDERED_ACCESS, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_UNORDERED_ACCESS,
                                    D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS );
    }
    if ( state & ( DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT | DENOFIZ_RESOURCE_USAGE_PIXEL_SHADER_RESOURCE_BIT ) )
    {
        return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_SHADER_RESOURCE, D3D12_BARRIER_LAYOUT_SHADER_RESOURCE );
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT )
    {
        return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_DEPTH_READ_BIT )
    {
        return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT )
    {
        return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
    }

    return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COMMON, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COMMON, D3D12_BARRIER_LAYOUT_COMMON );
}

D3D12_BARRIER_ACCESS DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( const DenOfIz_ResourceUsageFlags state, const DenOfIz_QueueType queueType )
{
    D3D12_BARRIER_ACCESS result = D3D12_BARRIER_ACCESS_COMMON;
    if ( state & DENOFIZ_RESOURCE_USAGE_GENERIC_READ_BIT )
    {
        return result;
    }
    if ( ( state & DENOFIZ_RESOURCE_USAGE_COMMON_BIT ) || ( state & DENOFIZ_RESOURCE_USAGE_PRESENT_BIT ) )
    {
        return D3D12_BARRIER_ACCESS_COMMON;
    }

    if ( state & DENOFIZ_RESOURCE_USAGE_VERTEX_AND_CONSTANT_BUFFER_BIT )
    {
        result |= D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
        if ( queueType == DENOFIZ_QUEUE_TYPE_GRAPHICS )
        {
            result |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
        }
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_INDEX_BUFFER_BIT )
    {
        result |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT )
    {
        result |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT )
    {
        result |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT )
    {
        result |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
    }
    else if ( state & DENOFIZ_RESOURCE_USAGE_DEPTH_READ_BIT )
    {
        result |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
    }

    if ( state & DENOFIZ_RESOURCE_USAGE_STREAM_OUT_BIT )
    {
        result |= D3D12_BARRIER_ACCESS_STREAM_OUTPUT;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_INDIRECT_ARGUMENT_BIT )
    {
        result |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT )
    {
        result |= D3D12_BARRIER_ACCESS_COPY_DEST;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT )
    {
        result |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
    }
    if ( state & ( DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT | DENOFIZ_RESOURCE_USAGE_PIXEL_SHADER_RESOURCE_BIT ) )
    {
        result |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT )
    {
        result |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT )
    {
        result |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;
    }
    return result;
}

D3D12_TEXTURE_ADDRESS_MODE DenOfIz_DX12EnumConverter_ConvertSamplerAddressMode( const DenOfIz_SamplerAddressMode mode )
{
    switch ( mode )
    {
    case DENOFIZ_SAMPLER_ADDRESS_MODE_REPEAT:
        return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    case DENOFIZ_SAMPLER_ADDRESS_MODE_MIRROR:
        return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    case DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
        return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    case DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
        return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    }
    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
}

D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DenOfIz_DX12EnumConverter_ConvertAccelerationStructureBuildFlags( const DenOfIz_ASBuildFlags flags )
{
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS result = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
    if ( flags & DENOFIZ_AS_BUILD_ALLOW_UPDATE_BIT )
    {
        result |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
    }
    if ( flags & DENOFIZ_AS_BUILD_ALLOW_COMPACTION_BIT )
    {
        result |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;
    }
    if ( flags & DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT )
    {
        result |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    }
    if ( flags & DENOFIZ_AS_BUILD_PREFER_FAST_BUILD_BIT )
    {
        result |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
    }
    if ( flags & DENOFIZ_AS_BUILD_MINIMIZE_MEMORY_BIT )
    {
        result |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
    }
    if ( flags & DENOFIZ_AS_BUILD_PERFORM_UPDATE_BIT )
    {
        result |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
    }
    return result;
}

D3D12_RAYTRACING_GEOMETRY_TYPE DenOfIz_DX12EnumConverter_ConvertGeometryType( const DenOfIz_HitGroupType type )
{
    switch ( type )
    {
    case DENOFIZ_HIT_GROUP_TYPE_TRIANGLES:
        return D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    case DENOFIZ_HIT_GROUP_TYPE_AABBS:
        return D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
    }
    return D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
}

DXGI_FORMAT DenOfIz_DX12EnumConverter_GetTypelessFormatForDepth( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_D32_FLOAT:
        return DXGI_FORMAT_R32_TYPELESS;
    case DENOFIZ_FORMAT_D16_UNORM:
        return DXGI_FORMAT_R16_TYPELESS;
    case DENOFIZ_FORMAT_D24_UNORM_S8_UINT:
        return DXGI_FORMAT_R24G8_TYPELESS;
    default:
        return DenOfIz_DX12EnumConverter_ConvertFormat( format );
    }
}

DXGI_FORMAT DenOfIz_DX12EnumConverter_GetSrvFormatForDepth( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_D32_FLOAT:
        return DXGI_FORMAT_R32_FLOAT;
    case DENOFIZ_FORMAT_D16_UNORM:
        return DXGI_FORMAT_R16_UNORM;
    case DENOFIZ_FORMAT_D24_UNORM_S8_UINT:
        return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    default:
        return DenOfIz_DX12EnumConverter_ConvertFormat( format );
    }
}

DXGI_FORMAT DenOfIz_DX12EnumConverter_GetDsvFormat( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_D32_FLOAT:
        return DXGI_FORMAT_D32_FLOAT;
    case DENOFIZ_FORMAT_D16_UNORM:
        return DXGI_FORMAT_D16_UNORM;
    case DENOFIZ_FORMAT_D24_UNORM_S8_UINT:
        return DXGI_FORMAT_D24_UNORM_S8_UINT;
    default:
        return DenOfIz_DX12EnumConverter_ConvertFormat( format );
    }
}

D3D12_RESOURCE_FLAGS DenOfIz_DX12EnumConverter_ConvertBufferUsageToResourceFlags( const DenOfIz_BufferUsageFlags usage )
{
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

    if ( usage & DENOFIZ_BUFFER_USAGE_STORAGE_BIT )
    {
        flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }
    if ( usage & DENOFIZ_BUFFER_USAGE_ACCELERATION_STRUCTURE_BIT )
    {
        flags |= D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;
    }

    return flags;
}

D3D12_RESOURCE_FLAGS DenOfIz_DX12EnumConverter_ConvertTextureUsageToResourceFlags( const DenOfIz_TextureUsageFlags usage, const DenOfIz_Format format )
{
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

    if ( usage & DENOFIZ_TEXTURE_USAGE_STORAGE_BINDING_BIT )
    {
        flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }
    if ( usage & DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT )
    {
        if ( DenOfIz_Format_IsDepth( format ) || DenOfIz_Format_IsDepthStencil( format ) )
        {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        }
        else
        {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }
    }

    return flags;
}
