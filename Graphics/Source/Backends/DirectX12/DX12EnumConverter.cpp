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

#include <DenOfIzGraphics/Backends/DirectX12/DX12EnumConverter.h>

using namespace DenOfIz;

D3D12_DESCRIPTOR_RANGE_TYPE DX12EnumConverter::ConvertResourceDescriptorToDescriptorRangeType( const BitSet<ResourceDescriptor> &descriptor )
{
    if ( descriptor.IsSet( ResourceDescriptor::Sampler ) )
    {
        return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    }
    if ( descriptor.Any( { ResourceDescriptor::UniformBuffer, ResourceDescriptor::RootConstant } ) )
    {
        return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    }
    if ( descriptor.Any( { ResourceDescriptor::RWTexture, ResourceDescriptor::RWBuffer } ) )
    {
        return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    }
    if ( descriptor.Any( { ResourceDescriptor::Texture, ResourceDescriptor::Buffer, ResourceDescriptor::AccelerationStructure } ) )
    {
        return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    }

    return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
}

D3D12_COMMAND_LIST_TYPE DX12EnumConverter::ConvertQueueType( const QueueType queueType )
{
    switch ( queueType )
    {
    case QueueType::Graphics:
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case QueueType::Compute:
        return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    case QueueType::Copy:
        return D3D12_COMMAND_LIST_TYPE_COPY;
    }

    return D3D12_COMMAND_LIST_TYPE_DIRECT;
}

D3D12_HEAP_TYPE DX12EnumConverter::ConvertHeapType( const HeapType &heapType )
{
    switch ( heapType )
    {
    case HeapType::GPU:
        return D3D12_HEAP_TYPE_DEFAULT;
    case HeapType::CPU:
    case HeapType::CPU_GPU:
        return D3D12_HEAP_TYPE_UPLOAD;
    case HeapType::GPU_CPU:
        return D3D12_HEAP_TYPE_READBACK;
    }

    return D3D12_HEAP_TYPE_DEFAULT;
}

uint32_t DX12EnumConverter::ConvertSampleCount( const MSAASampleCount &sampleCount )
{
    switch ( sampleCount )
    {
    case MSAASampleCount::_0:
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
    case MSAASampleCount::_64:
        LOG( WARNING ) << "Exceeded the maximum sample count of 16 for this API, defaulting to 16.";
        return 16;
    }

    return 1;
}

DXGI_FORMAT DX12EnumConverter::ConvertFormat( const Format &format )
{
    switch ( format )
    {
    case Format::Undefined:
        return DXGI_FORMAT_UNKNOWN;
    case Format::R32G32B32A32Float:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case Format::R32G32B32A32Uint:
        return DXGI_FORMAT_R32G32B32A32_UINT;
    case Format::R32G32B32A32Sint:
        return DXGI_FORMAT_R32G32B32A32_SINT;
    case Format::R32G32B32Float:
        return DXGI_FORMAT_R32G32B32_FLOAT;
    case Format::R32G32B32Uint:
        return DXGI_FORMAT_R32G32B32_UINT;
    case Format::R32G32B32Sint:
        return DXGI_FORMAT_R32G32B32_SINT;
    case Format::R16G16B16A16Float:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case Format::R16G16B16A16Unorm:
        return DXGI_FORMAT_R16G16B16A16_UNORM;
    case Format::R16G16B16A16Uint:
        return DXGI_FORMAT_R16G16B16A16_UINT;
    case Format::R16G16B16A16Snorm:
        return DXGI_FORMAT_R16G16B16A16_SNORM;
    case Format::R16G16B16A16Sint:
        return DXGI_FORMAT_R16G16B16A16_SINT;
    case Format::R32G32Float:
        return DXGI_FORMAT_R32G32_FLOAT;
    case Format::R32G32Uint:
        return DXGI_FORMAT_R32G32_UINT;
    case Format::R32G32Sint:
        return DXGI_FORMAT_R32G32_SINT;
    case Format::R10G10B10A2Unorm:
        return DXGI_FORMAT_R10G10B10A2_UNORM;
    case Format::R10G10B10A2Uint:
        return DXGI_FORMAT_R10G10B10A2_UINT;
    case Format::R8G8B8A8Unorm:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case Format::R8G8B8A8UnormSrgb:
        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case Format::R8G8B8A8Uint:
        return DXGI_FORMAT_R8G8B8A8_UINT;
    case Format::R8G8B8A8Snorm:
        return DXGI_FORMAT_R8G8B8A8_SNORM;
    case Format::R8G8B8A8Sint:
        return DXGI_FORMAT_R8G8B8A8_SINT;
    case Format::R16G16Float:
        return DXGI_FORMAT_R16G16_FLOAT;
    case Format::R16G16Unorm:
        return DXGI_FORMAT_R16G16_UNORM;
    case Format::R16G16Uint:
        return DXGI_FORMAT_R16G16_UINT;
    case Format::R16G16Snorm:
        return DXGI_FORMAT_R16G16_SNORM;
    case Format::R16G16Sint:
        return DXGI_FORMAT_R16G16_SINT;
    case Format::D32Float:
        return DXGI_FORMAT_D32_FLOAT;
    case Format::R32Float:
        return DXGI_FORMAT_R32_FLOAT;
    case Format::R32Uint:
        return DXGI_FORMAT_R32_UINT;
    case Format::R32Sint:
        return DXGI_FORMAT_R32_SINT;
    case Format::D24UnormS8Uint:
        return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case Format::R8G8Unorm:
        return DXGI_FORMAT_R8G8_UNORM;
    case Format::R8G8Uint:
        return DXGI_FORMAT_R8G8_UINT;
    case Format::R8G8Snorm:
        return DXGI_FORMAT_R8G8_SNORM;
    case Format::R8G8Sint:
        return DXGI_FORMAT_R8G8_SINT;
    case Format::R16Float:
        return DXGI_FORMAT_R16_FLOAT;
    case Format::D16Unorm:
        return DXGI_FORMAT_D16_UNORM;
    case Format::R16Unorm:
        return DXGI_FORMAT_R16_UNORM;
    case Format::R16Uint:
        return DXGI_FORMAT_R16_UINT;
    case Format::R16Snorm:
        return DXGI_FORMAT_R16_SNORM;
    case Format::R16Sint:
        return DXGI_FORMAT_R16_SINT;
    case Format::R8Unorm:
        return DXGI_FORMAT_R8_UNORM;
    case Format::R8Uint:
        return DXGI_FORMAT_R8_UINT;
    case Format::R8Snorm:
        return DXGI_FORMAT_R8_SNORM;
    case Format::R8Sint:
        return DXGI_FORMAT_R8_SINT;
    case Format::BC1Unorm:
        return DXGI_FORMAT_BC1_UNORM;
    case Format::BC1UnormSrgb:
        return DXGI_FORMAT_BC1_UNORM_SRGB;
    case Format::BC2Unorm:
        return DXGI_FORMAT_BC2_UNORM;
    case Format::BC2UnormSrgb:
        return DXGI_FORMAT_BC2_UNORM_SRGB;
    case Format::BC3Unorm:
        return DXGI_FORMAT_BC3_UNORM;
    case Format::BC3UnormSrgb:
        return DXGI_FORMAT_BC3_UNORM_SRGB;
    case Format::BC4Unorm:
        return DXGI_FORMAT_BC4_UNORM;
    case Format::BC4Snorm:
        return DXGI_FORMAT_BC4_SNORM;
    case Format::BC5Unorm:
        return DXGI_FORMAT_BC5_UNORM;
    case Format::BC5Snorm:
        return DXGI_FORMAT_BC5_SNORM;
    case Format::B8G8R8A8Unorm:
        return DXGI_FORMAT_B8G8R8A8_UNORM;
    case Format::BC6HUfloat16:
        return DXGI_FORMAT_BC6H_UF16;
    case Format::BC6HSfloat16:
        return DXGI_FORMAT_BC6H_SF16;
    case Format::BC7Unorm:
        return DXGI_FORMAT_BC7_UNORM;
    case Format::BC7UnormSrgb:
        return DXGI_FORMAT_BC7_UNORM_SRGB;
    case Format::R32G32B32A32Typeless:
        return DXGI_FORMAT_R32G32B32A32_TYPELESS;
    case Format::R16G16B16A16Typeless:
        return DXGI_FORMAT_R16G16B16A16_TYPELESS;
    case Format::R32G32Typeless:
        return DXGI_FORMAT_R32G32_TYPELESS;
    case Format::R10G10B10A2Typeless:
        return DXGI_FORMAT_R10G10B10A2_TYPELESS;
    case Format::R8G8B8A8Typeless:
        return DXGI_FORMAT_R8G8B8A8_TYPELESS;
    case Format::R16G16Typeless:
        return DXGI_FORMAT_R16G16_TYPELESS;
    case Format::R32Typeless:
        return DXGI_FORMAT_R32_TYPELESS;
    case Format::R8G8Typeless:
        return DXGI_FORMAT_R8G8_TYPELESS;
    case Format::R16Typeless:
        return DXGI_FORMAT_R16_TYPELESS;
    case Format::R8Typeless:
        return DXGI_FORMAT_R8_TYPELESS;
    }

    return DXGI_FORMAT_UNKNOWN;
}

D3D12_SHADER_VISIBILITY DX12EnumConverter::ConvertShaderStageToShaderVisibility( const ShaderStage &stage )
{
    switch ( stage )
    {
    case ShaderStage::Vertex:
        return D3D12_SHADER_VISIBILITY_VERTEX;
    case ShaderStage::Hull:
        return D3D12_SHADER_VISIBILITY_HULL;
    case ShaderStage::Domain:
        return D3D12_SHADER_VISIBILITY_DOMAIN;
    case ShaderStage::Geometry:
        return D3D12_SHADER_VISIBILITY_GEOMETRY;
    case ShaderStage::Pixel:
        return D3D12_SHADER_VISIBILITY_PIXEL;
    case ShaderStage::Mesh:
        return D3D12_SHADER_VISIBILITY_MESH;
    default:
        return D3D12_SHADER_VISIBILITY_ALL;
    }
}

D3D12_COMPARISON_FUNC DX12EnumConverter::ConvertCompareOp( const CompareOp &op )
{
    switch ( op )
    {
    case CompareOp::Never:
        return D3D12_COMPARISON_FUNC_NEVER;
    case CompareOp::Equal:
        return D3D12_COMPARISON_FUNC_EQUAL;
    case CompareOp::NotEqual:
        return D3D12_COMPARISON_FUNC_NOT_EQUAL;
    case CompareOp::Always:
        return D3D12_COMPARISON_FUNC_ALWAYS;
    case CompareOp::Less:
        return D3D12_COMPARISON_FUNC_LESS;
    case CompareOp::LessOrEqual:
        return D3D12_COMPARISON_FUNC_LESS_EQUAL;
    case CompareOp::Greater:
        return D3D12_COMPARISON_FUNC_GREATER;
    case CompareOp::GreaterOrEqual:
        return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    }

    return D3D12_COMPARISON_FUNC_EQUAL;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE DX12EnumConverter::ConvertPrimitiveTopologyToType( const PrimitiveTopology &topology )
{
    switch ( topology )
    {
    case PrimitiveTopology::Point:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    case PrimitiveTopology::Line:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case PrimitiveTopology::Triangle:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case PrimitiveTopology::Patch:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    }

    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
}

D3D12_PRIMITIVE_TOPOLOGY DX12EnumConverter::ConvertPrimitiveTopology( const PrimitiveTopology &topology )
{
    switch ( topology )
    {
    case PrimitiveTopology::Point:
        return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    case PrimitiveTopology::Line:
        return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    case PrimitiveTopology::Triangle:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case PrimitiveTopology::Patch:
        return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST; // Todo could require more control points
    }

    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

D3D12_STENCIL_OP DX12EnumConverter::ConvertStencilOp( const StencilOp &op )
{
    switch ( op )
    {
    case StencilOp::Keep:
        return D3D12_STENCIL_OP_KEEP;
    case StencilOp::Zero:
        return D3D12_STENCIL_OP_ZERO;
    case StencilOp::Replace:
        return D3D12_STENCIL_OP_REPLACE;
    case StencilOp::IncrementAndClamp:
        return D3D12_STENCIL_OP_INCR_SAT;
    case StencilOp::DecrementAndClamp:
        return D3D12_STENCIL_OP_DECR_SAT;
    case StencilOp::Invert:
        return D3D12_STENCIL_OP_INVERT;
    case StencilOp::IncrementAndWrap:
        return D3D12_STENCIL_OP_INCR;
    case StencilOp::DecrementAndWrap:
        return D3D12_STENCIL_OP_DECR;
    }

    return D3D12_STENCIL_OP_KEEP;
}

D3D12_CULL_MODE DX12EnumConverter::ConvertCullMode( const CullMode mode )
{
    switch ( mode )
    {
    case CullMode::FrontFace:
        return D3D12_CULL_MODE_FRONT;
    case CullMode::BackFace:
        return D3D12_CULL_MODE_BACK;
    case CullMode::None:
        return D3D12_CULL_MODE_NONE;
    }

    return D3D12_CULL_MODE_NONE;
}

D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE DX12EnumConverter::ConvertLoadOp( const LoadOp &op )
{
    switch ( op )
    {
    case LoadOp::Clear:
        return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
    case LoadOp::Load:
        return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
    case LoadOp::DontCare:
        return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
    }

    return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
}

D3D12_RENDER_PASS_ENDING_ACCESS_TYPE DX12EnumConverter::ConvertStoreOp( const StoreOp &op )
{
    switch ( op )
    {
    case StoreOp::Store:
        return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
    case StoreOp::DontCare:
        return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
    case StoreOp::None:
        return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
    }

    return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
}

D3D12_BLEND_OP DX12EnumConverter::ConvertBlendOp( const BlendOp &op )
{
    switch ( op )
    {
    case BlendOp::Add:
        return D3D12_BLEND_OP_ADD;
    case BlendOp::Subtract:
        return D3D12_BLEND_OP_SUBTRACT;
    case BlendOp::ReverseSubtract:
        return D3D12_BLEND_OP_REV_SUBTRACT;
    case BlendOp::Min:
        return D3D12_BLEND_OP_MIN;
    case BlendOp::Max:
        return D3D12_BLEND_OP_MAX;
    }

    return D3D12_BLEND_OP_ADD;
}

D3D12_LOGIC_OP DX12EnumConverter::ConvertLogicOp( const LogicOp &op )
{
    switch ( op )
    {
    case LogicOp::Clear:
        return D3D12_LOGIC_OP_CLEAR;
    case LogicOp::Set:
        return D3D12_LOGIC_OP_SET;
    case LogicOp::Copy:
        return D3D12_LOGIC_OP_COPY;
    case LogicOp::CopyInverted:
        return D3D12_LOGIC_OP_COPY_INVERTED;
    case LogicOp::Noop:
        return D3D12_LOGIC_OP_NOOP;
    case LogicOp::Invert:
        return D3D12_LOGIC_OP_INVERT;
    case LogicOp::And:
        return D3D12_LOGIC_OP_AND;
    case LogicOp::Nand:
        return D3D12_LOGIC_OP_NAND;
    case LogicOp::Or:
        return D3D12_LOGIC_OP_OR;
    case LogicOp::Nor:
        return D3D12_LOGIC_OP_NOR;
    case LogicOp::Xor:
        return D3D12_LOGIC_OP_XOR;
    case LogicOp::Equiv:
        return D3D12_LOGIC_OP_EQUIV;
    case LogicOp::AndReverse:
        return D3D12_LOGIC_OP_AND_REVERSE;
    case LogicOp::AndInverted:
        return D3D12_LOGIC_OP_AND_INVERTED;
    case LogicOp::OrReverse:
        return D3D12_LOGIC_OP_OR_REVERSE;
    case LogicOp::OrInverted:
        return D3D12_LOGIC_OP_OR_INVERTED;
    }

    return D3D12_LOGIC_OP_CLEAR;
}

D3D12_BLEND DX12EnumConverter::ConvertBlend( const Blend &factor )
{
    switch ( factor )
    {
    case Blend::Zero:
        return D3D12_BLEND_ZERO;
    case Blend::One:
        return D3D12_BLEND_ONE;
    case Blend::SrcColor:
        return D3D12_BLEND_SRC_COLOR;
    case Blend::InvSrcColor:
        return D3D12_BLEND_INV_SRC_COLOR;
    case Blend::SrcAlpha:
        return D3D12_BLEND_SRC_ALPHA;
    case Blend::InvSrcAlpha:
        return D3D12_BLEND_INV_SRC_ALPHA;
    case Blend::DstAlpha:
        return D3D12_BLEND_DEST_ALPHA;
    case Blend::InvDstAlpha:
        return D3D12_BLEND_INV_DEST_ALPHA;
    case Blend::DstColor:
        return D3D12_BLEND_DEST_COLOR;
    case Blend::InvDstColor:
        return D3D12_BLEND_INV_DEST_COLOR;
    case Blend::SrcAlphaSaturate:
        return D3D12_BLEND_SRC_ALPHA_SAT;
    case Blend::Src1Color:
        return D3D12_BLEND_SRC1_COLOR;
    case Blend::InvSrc1Color:
        return D3D12_BLEND_INV_SRC1_COLOR;
    case Blend::Src1Alpha:
        return D3D12_BLEND_SRC1_ALPHA;
    case Blend::InvSrc1Alpha:
        return D3D12_BLEND_INV_SRC1_ALPHA;
    case Blend::BlendFactor:
        return D3D12_BLEND_BLEND_FACTOR;
    case Blend::InvBlendFactor:
        return D3D12_BLEND_INV_BLEND_FACTOR;
    }

    return D3D12_BLEND_ZERO;
}

D3D12_RESOURCE_STATES DX12EnumConverter::ConvertResourceState( const BitSet<ResourceState> &state )
{
    D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON;
    if ( state.IsSet( ResourceState::GenericRead ) )
    {
        return D3D12_RESOURCE_STATE_GENERIC_READ;
    }
    if ( state.IsSet( ResourceState::Common ) )
    {
        return D3D12_RESOURCE_STATE_COMMON;
    }
    if ( state.IsSet( ResourceState::Present ) )
    {
        return D3D12_RESOURCE_STATE_PRESENT;
    }

    if ( state.IsSet( ResourceState::VertexAndConstantBuffer ) )
    {
        result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    }
    if ( state.IsSet( ResourceState::IndexBuffer ) )
    {
        result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
    }
    if ( state.IsSet( ResourceState::RenderTarget ) )
    {
        result |= D3D12_RESOURCE_STATE_RENDER_TARGET;
    }
    if ( state.IsSet( ResourceState::UnorderedAccess ) )
    {
        result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }
    if ( state.IsSet( ResourceState::DepthWrite ) )
    {
        result |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
    }
    else if ( state.IsSet( ResourceState::DepthRead ) )
    {
        result |= D3D12_RESOURCE_STATE_DEPTH_READ;
    }

    if ( state.IsSet( ResourceState::StreamOut ) )
    {
        result |= D3D12_RESOURCE_STATE_STREAM_OUT;
    }
    if ( state.IsSet( ResourceState::IndirectArgument ) )
    {
        result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    }
    if ( state.IsSet( ResourceState::CopyDst ) )
    {
        result |= D3D12_RESOURCE_STATE_COPY_DEST;
    }
    if ( state.IsSet( ResourceState::CopySrc ) )
    {
        result |= D3D12_RESOURCE_STATE_COPY_SOURCE;
    }
    if ( state.IsSet( ResourceState::ShaderResource ) )
    {
        result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    }
    if ( state.IsSet( ResourceState::PixelShaderResource ) )
    {
        result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }
    if ( state.IsSet( ResourceState::AccelerationStructureRead ) || state.IsSet( ResourceState::AccelerationStructureWrite ) )
    {
        result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    }
    return result;
}

D3D12_BARRIER_LAYOUT DX12EnumConverter::ConvertResourceStateToBarrierLayout( const BitSet<ResourceState> &state, const QueueType &queueType )
{
    auto queueSpecificResult = [ = ]( const D3D12_BARRIER_LAYOUT direct, const D3D12_BARRIER_LAYOUT compute, const D3D12_BARRIER_LAYOUT other )
    {
        switch ( queueType )
        {
        case QueueType::Graphics:
            return direct;
        case QueueType::Compute:
            return compute;
        default:
            return other;
        }
    };

    if ( state.IsSet( ResourceState::Common ) || state.IsSet( ResourceState::Present ) )
    {
        return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COMMON, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COMMON, D3D12_BARRIER_LAYOUT_COMMON );
    }
    if ( state.IsSet( ResourceState::GenericRead ) )
    {
        return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_GENERIC_READ, D3D12_BARRIER_LAYOUT_GENERIC_READ );
    }
    if ( state.IsSet( ResourceState::CopySrc ) )
    {
        return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_SOURCE, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_SOURCE, D3D12_BARRIER_LAYOUT_COPY_SOURCE );
    }
    if ( state.IsSet( ResourceState::CopyDst ) )
    {
        return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_DEST, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_DEST, D3D12_BARRIER_LAYOUT_COPY_DEST );
    }
    if ( state.IsSet( ResourceState::UnorderedAccess ) )
    {
        return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_UNORDERED_ACCESS, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_UNORDERED_ACCESS,
                                    D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS );
    }
    if ( state.Any( { ResourceState::ShaderResource, ResourceState::PixelShaderResource } ) )
    {
        return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_SHADER_RESOURCE, D3D12_BARRIER_LAYOUT_SHADER_RESOURCE );
    }

    if ( state.IsSet( ResourceState::RenderTarget ) )
    {
        return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
    }
    if ( state.IsSet( ResourceState::DepthRead ) )
    {
        return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
    }
    if ( state.IsSet( ResourceState::DepthWrite ) )
    {
        return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
    }

    return queueSpecificResult( D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COMMON, D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COMMON, D3D12_BARRIER_LAYOUT_COMMON );
}

D3D12_BARRIER_ACCESS DX12EnumConverter::ConvertResourceStateToBarrierAccess( const BitSet<ResourceState> &state )
{
    D3D12_BARRIER_ACCESS result = D3D12_BARRIER_ACCESS_COMMON;
    if ( state.IsSet( ResourceState::GenericRead ) )
    {
        return result;
    }
    if ( state.IsSet( ResourceState::Common ) || state.IsSet( ResourceState::Present ) )
    {
        return D3D12_BARRIER_ACCESS_COMMON;
    }

    if ( state.IsSet( ResourceState::VertexAndConstantBuffer ) )
    {
        result |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER | D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
    }
    if ( state.IsSet( ResourceState::IndexBuffer ) )
    {
        result |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
    }
    if ( state.IsSet( ResourceState::RenderTarget ) )
    {
        result |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
    }
    if ( state.IsSet( ResourceState::UnorderedAccess ) )
    {
        result |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
    }
    if ( state.IsSet( ResourceState::DepthWrite ) )
    {
        result |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
    }
    else if ( state.IsSet( ResourceState::DepthRead ) )
    {
        result |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
    }

    if ( state.IsSet( ResourceState::StreamOut ) )
    {
        result |= D3D12_BARRIER_ACCESS_STREAM_OUTPUT;
    }
    if ( state.IsSet( ResourceState::IndirectArgument ) )
    {
        result |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
    }
    if ( state.IsSet( ResourceState::CopyDst ) )
    {
        result |= D3D12_BARRIER_ACCESS_COPY_DEST;
    }
    if ( state.IsSet( ResourceState::CopySrc ) )
    {
        result |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
    }
    if ( state.Any( { ResourceState::ShaderResource, ResourceState::PixelShaderResource } ) )
    {
        result |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
    }
    if ( state.IsSet( ResourceState::AccelerationStructureRead ) )
    {
        result |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
    }
    if ( state.IsSet( ResourceState::AccelerationStructureWrite ) )
    {
        result |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;
    }
    return result;
}

D3D12_TEXTURE_ADDRESS_MODE DX12EnumConverter::ConvertSamplerAddressMode( const SamplerAddressMode &mode )
{
    switch ( mode )
    {
    case SamplerAddressMode::Repeat:
        return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    case SamplerAddressMode::Mirror:
        return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    case SamplerAddressMode::ClampToEdge:
        return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    case SamplerAddressMode::ClampToBorder:
        return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    }
    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
}

D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DX12EnumConverter::ConvertAccelerationStructureBuildFlags( const BitSet<ASBuildFlags> &flags )
{
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS result = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
    if ( flags.IsSet( ASBuildFlags::AllowUpdate ) )
    {
        result |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
    }
    if ( flags.IsSet( ASBuildFlags::AllowCompaction ) )
    {
        result |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;
    }
    if ( flags.IsSet( ASBuildFlags::PreferFastTrace ) )
    {
        result |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    }
    if ( flags.IsSet( ASBuildFlags::PreferFastBuild ) )
    {
        result |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
    }
    if ( flags.IsSet( ASBuildFlags::MinimizeMemory ) )
    {
        result |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
    }
    if ( flags.IsSet( ASBuildFlags::PerformUpdate ) )
    {
        result |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
    }
    return result;
}
