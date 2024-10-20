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

#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>
#include <DenOfIzGraphics/Backends/Interface/RayTracing/RayTracingData.h>
#include "DX12Context.h"

namespace DenOfIz
{

    class DX12EnumConverter
    {
    public:
        static D3D12_DESCRIPTOR_RANGE_TYPE                         ConvertResourceDescriptorToDescriptorRangeType( const BitSet<ResourceDescriptor> &descriptor );
        static D3D12_COMMAND_LIST_TYPE                             ConvertQueueType( QueueType queueType );
        static D3D12_HEAP_TYPE                                     ConvertHeapType( const HeapType &heapType );
        static uint32_t                                            ConvertSampleCount( const MSAASampleCount &sampleCount );
        static DXGI_FORMAT                                         ConvertFormat( const Format &format );
        static D3D12_SHADER_VISIBILITY                             ConvertShaderStageToShaderVisibility( const ShaderStage &stage );
        static D3D12_COMPARISON_FUNC                               ConvertCompareOp( const CompareOp &op );
        static D3D12_PRIMITIVE_TOPOLOGY_TYPE                       ConvertPrimitiveTopologyToType( const PrimitiveTopology &topology );
        static D3D12_PRIMITIVE_TOPOLOGY                            ConvertPrimitiveTopology( const PrimitiveTopology &topology );
        static D3D12_STENCIL_OP                                    ConvertStencilOp( const StencilOp &op );
        static D3D12_CULL_MODE                                     ConvertCullMode( CullMode mode );
        static D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE             ConvertLoadOp( const LoadOp &op );
        static D3D12_RENDER_PASS_ENDING_ACCESS_TYPE                ConvertStoreOp( const StoreOp &op );
        static D3D12_BLEND_OP                                      ConvertBlendOp( const BlendOp &op );
        static D3D12_LOGIC_OP                                      ConvertLogicOp( const LogicOp &op );
        static D3D12_BLEND                                         ConvertBlend( const Blend &factor );
        static D3D12_RESOURCE_STATES                               ConvertResourceState( const BitSet<ResourceState> &state );
        static D3D12_BARRIER_LAYOUT                                ConvertResourceStateToBarrierLayout( const BitSet<ResourceState> &state, const QueueType &queueType );
        static D3D12_BARRIER_ACCESS                                ConvertResourceStateToBarrierAccess( const BitSet<ResourceState> &state );
        static D3D12_TEXTURE_ADDRESS_MODE                          ConvertSamplerAddressMode( const SamplerAddressMode &mode );
        static D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS ConvertAccelerationStructureBuildFlags( const BitSet<ASBuildFlags> &flags );
    };

} // namespace DenOfIz
