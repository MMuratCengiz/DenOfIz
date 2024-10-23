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

#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12ShaderBindingTable.h>

using namespace DenOfIz;

DX12ShaderBindingTable::DX12ShaderBindingTable( DX12Context *context, const ShaderBindingTableDesc &desc ) : m_context( context )
{
    m_pipeline = dynamic_cast<DX12Pipeline *>( desc.Pipeline );
    Resize( desc.SizeDesc );
}

void DX12ShaderBindingTable::Resize( const SBTSizeDesc &desc )
{
    uint32_t totalNumEntries = desc.NumRayGenerationShaders + desc.NumMissShaders + ( desc.NumHitGroups * desc.NumInstances * desc.NumRayTypes );
    m_numBufferBytes         = totalNumEntries * D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

    BufferDesc bufferDesc   = { };
    bufferDesc.NumBytes     = m_numBufferBytes;
    bufferDesc.HeapType     = HeapType::GPU_CPU;
    bufferDesc.InitialState = ResourceState::CopySrc;
    bufferDesc.Descriptor   = ResourceDescriptor::Buffer;

    m_stagingBuffer = std::make_unique<DX12BufferResource>( m_context, bufferDesc );
    m_mappedMemory  = m_stagingBuffer->MapMemory( );

    if ( !m_mappedMemory )
    {
        LOG( ERROR ) << "Failed to map memory for shader binding table.";
    }

    bufferDesc.HeapType     = HeapType::GPU;
    bufferDesc.InitialState = ResourceState::CopyDst;
    m_buffer                = std::make_unique<DX12BufferResource>( m_context, bufferDesc );
}

void DX12ShaderBindingTable::BindRayGenerationShader( const RayGenerationBindingDesc &desc )
{
    const void *shaderIdentifier = m_pipeline->GetShaderIdentifier( desc.ShaderName.Get( ) );
    memcpy( m_mappedMemory, shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );

    m_rayGenerationShaderRange.StartAddress = m_buffer->Resource( )->GetGPUVirtualAddress( );
    m_rayGenerationShaderRange.SizeInBytes  = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
}

void DX12ShaderBindingTable::BindHitGroup( const HitGroupBindingDesc &desc )
{
    const uint32_t offset        = ( desc.InstanceIndex * desc.GeometryIndex * desc.RayTypeIndex ) * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    void          *hitGroupEntry = static_cast<Byte *>( m_mappedMemory ) + offset;

    const void *closestHitIdentifier   = m_pipeline->GetShaderIdentifier( desc.ClosestHitShaderName.Get( ) );
    const void *anyHitIdentifier       = m_pipeline->GetShaderIdentifier( desc.AnyHitShaderName.Get( ) );
    const void *intersectionIdentifier = m_pipeline->GetShaderIdentifier( desc.IntersectionShaderName.Get( ) );

    if ( desc.ClosestHitShaderName.IsEmpty( ) )
    {
        LOG( ERROR ) << "Closest hit shader name cannot be empty.";
    }
    uint32_t numHitGroupShaders = 1;
    memcpy( hitGroupEntry, closestHitIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );
    if ( !desc.AnyHitShaderName.IsEmpty( ) )
    {
        memcpy( static_cast<Byte *>( hitGroupEntry ) + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, anyHitIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );
        numHitGroupShaders++;
    }

    if ( !desc.IntersectionShaderName.IsEmpty( ) )
    {
        memcpy( static_cast<Byte *>( hitGroupEntry ) + 2 * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, intersectionIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );
        numHitGroupShaders++;
    }

    m_hitGroupShaderRange.StartAddress  = m_buffer->Resource( )->GetGPUVirtualAddress( ) + offset;
    m_hitGroupShaderRange.SizeInBytes   = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES * numHitGroupShaders;
    m_hitGroupShaderRange.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES * numHitGroupShaders;;
}

void DX12ShaderBindingTable::BindMissShader( const MissBindingDesc &desc )
{
    const uint32_t offset           = m_desc.SizeDesc.NumRayGenerationShaders * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    void          *missShaderEntry  = static_cast<Byte *>( m_mappedMemory ) + offset;
    const void    *shaderIdentifier = m_pipeline->GetShaderIdentifier( desc.ShaderName.Get( ) );
    memcpy( missShaderEntry, shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );

    m_missShaderRange.StartAddress  = m_buffer->Resource( )->GetGPUVirtualAddress( ) + offset;
    m_missShaderRange.SizeInBytes   = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    m_missShaderRange.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
}

void DX12ShaderBindingTable::Build( )
{
    m_stagingBuffer->UnmapMemory( );

    const auto commandList = m_context->CopyCommandList;
    commandList->Reset( m_context->CopyCommandListAllocator.get( ), nullptr );
    commandList->CopyBufferRegion( m_buffer->Resource( ), 0, m_buffer->Resource( ), 0, m_numBufferBytes );
    // Pipeline barrier to shader resource:
    D3D12_RESOURCE_BARRIER barrier = { };
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = m_buffer->Resource( );
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier( 1, &barrier );
    commandList->Close( );
}

IBufferResource *DX12ShaderBindingTable::Buffer( ) const
{
    return m_buffer.get( );
}

D3D12_GPU_VIRTUAL_ADDRESS_RANGE DX12ShaderBindingTable::RayGenerationShaderRange( ) const
{
    return m_rayGenerationShaderRange;
}

D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE DX12ShaderBindingTable::HitGroupShaderRange( ) const
{
    return m_hitGroupShaderRange;
}

D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE DX12ShaderBindingTable::MissShaderRange( ) const
{
    return m_missShaderRange;
}
