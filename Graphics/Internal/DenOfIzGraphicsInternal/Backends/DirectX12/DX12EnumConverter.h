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

#include <DenOfIzGraphics/Backends/Interface/RayTracing/RayTracingData.h>
#include <DenOfIzGraphics/Backends/Interface/ShaderData.h>
#include <DenOfIzGraphicsInternal/Backends/Interface/IPipeline.h>
#include "DX12Context.h"

D3D12_DESCRIPTOR_RANGE_TYPE             DenOfIz_DX12EnumConverter_ConvertResourceDescriptorToDescriptorRangeType( DenOfIz_ResourceDescriptorFlags descriptor );
D3D12_COMMAND_LIST_TYPE                 DenOfIz_DX12EnumConverter_ConvertQueueType( DenOfIz_QueueType queueType );
D3D12_HEAP_TYPE                         DenOfIz_DX12EnumConverter_ConvertHeapType( DenOfIz_HeapType heapType );
uint32_t                                DenOfIz_DX12EnumConverter_ConvertSampleCount( DenOfIz_MSAASampleCount sampleCount );
DXGI_FORMAT                             DenOfIz_DX12EnumConverter_ConvertFormat( DenOfIz_Format format );
D3D12_SHADER_VISIBILITY                 DenOfIz_DX12EnumConverter_ConvertShaderStageToShaderVisibility( DenOfIz_ShaderStageFlags stages );
D3D12_COMPARISON_FUNC                   DenOfIz_DX12EnumConverter_ConvertCompareOp( DenOfIz_CompareOp op );
D3D12_PRIMITIVE_TOPOLOGY_TYPE           DenOfIz_DX12EnumConverter_ConvertPrimitiveTopologyToType( DenOfIz_PrimitiveTopology topology );
D3D12_PRIMITIVE_TOPOLOGY                DenOfIz_DX12EnumConverter_ConvertPrimitiveTopology( DenOfIz_PrimitiveTopology topology );
D3D12_STENCIL_OP                        DenOfIz_DX12EnumConverter_ConvertStencilOp( DenOfIz_StencilOp op );
D3D12_CULL_MODE                         DenOfIz_DX12EnumConverter_ConvertCullMode( DenOfIz_CullMode mode );
D3D12_FILL_MODE                         DenOfIz_DX12EnumConverter_ConvertFillMode( DenOfIz_FillMode mode );
D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE DenOfIz_DX12EnumConverter_ConvertLoadOp( DenOfIz_LoadOp op );
D3D12_RENDER_PASS_ENDING_ACCESS_TYPE    DenOfIz_DX12EnumConverter_ConvertStoreOp( DenOfIz_StoreOp op );
D3D12_BLEND_OP                          DenOfIz_DX12EnumConverter_ConvertBlendOp( DenOfIz_BlendOp op );
D3D12_LOGIC_OP                          DenOfIz_DX12EnumConverter_ConvertLogicOp( DenOfIz_LogicOp op );
D3D12_BLEND                             DenOfIz_DX12EnumConverter_ConvertBlend( DenOfIz_Blend factor );
D3D12_RESOURCE_STATES                   DenOfIz_DX12EnumConverter_ConvertResourceUsage( DenOfIz_ResourceUsageFlags usage );
D3D12_BARRIER_LAYOUT       DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierLayout( DenOfIz_ResourceUsageFlags state, DenOfIz_QueueType queueType, bool enhancedBarriers );
D3D12_BARRIER_ACCESS       DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( DenOfIz_ResourceUsageFlags state, DenOfIz_QueueType queueType );
D3D12_TEXTURE_ADDRESS_MODE DenOfIz_DX12EnumConverter_ConvertSamplerAddressMode( DenOfIz_SamplerAddressMode mode );
D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DenOfIz_DX12EnumConverter_ConvertAccelerationStructureBuildFlags( DenOfIz_ASBuildFlags flags );
D3D12_RAYTRACING_GEOMETRY_TYPE                      DenOfIz_DX12EnumConverter_ConvertGeometryType( DenOfIz_HitGroupType type );
DXGI_FORMAT                                         DenOfIz_DX12EnumConverter_GetTypelessFormatForDepth( DenOfIz_Format format );
DXGI_FORMAT                                         DenOfIz_DX12EnumConverter_GetSrvFormatForDepth( DenOfIz_Format format );
DXGI_FORMAT                                         DenOfIz_DX12EnumConverter_GetDsvFormat( DenOfIz_Format format );

D3D12_RESOURCE_FLAGS DenOfIz_DX12EnumConverter_ConvertBufferUsageToResourceFlags( DenOfIz_BufferUsageFlags usage );
D3D12_RESOURCE_FLAGS DenOfIz_DX12EnumConverter_ConvertTextureUsageToResourceFlags( DenOfIz_TextureUsageFlags usage, DenOfIz_Format format );
