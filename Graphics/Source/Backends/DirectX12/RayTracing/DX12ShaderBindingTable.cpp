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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12ShaderBindingTable.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12BarrierHelper.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12ShaderLocalData.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12ShaderBindingTable::DX12ShaderBindingTable( DX12Context *context, const ShaderBindingTableDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_pipeline                   = dynamic_cast<DX12Pipeline *>( desc.Pipeline );
    m_rayGenNumBytes             = Utilities::Align( D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + m_desc.MaxRayGenDataBytes, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT );
    m_hitGroupNumBytes           = Utilities::Align( D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + m_desc.MaxHitGroupDataBytes, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT );
    m_missNumBytes               = Utilities::Align( D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + m_desc.MaxMissDataBytes, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT );
    m_debugData.RayGenNumBytes   = m_rayGenNumBytes;
    m_debugData.MissNumBytes     = m_missNumBytes;
    m_debugData.HitGroupNumBytes = m_hitGroupNumBytes;
    Resize( desc.SizeDesc );
}

void DX12ShaderBindingTable::Resize( const SBTSizeDesc &desc )
{
    const uint32_t rayGenerationShaderNumBytes = desc.NumRayGenerationShaders * m_rayGenNumBytes;
    const uint32_t hitGroupNumBytes            = desc.NumHitGroups * m_hitGroupNumBytes;
    const uint32_t missShaderNumBytes          = desc.NumMissShaders * m_missNumBytes;
    m_numBufferBytes                           = AlignRecord( rayGenerationShaderNumBytes ) + AlignRecord( hitGroupNumBytes ) + AlignRecord( missShaderNumBytes );

    BufferDesc bufferDesc = { };
    bufferDesc.NumBytes   = m_numBufferBytes;
    bufferDesc.HeapType   = HeapType::CPU_GPU;
    bufferDesc.Usages     = BitSet( ResourceUsage::CopySrc ) | ResourceUsage::ShaderBindingTable;
    bufferDesc.Descriptor = ResourceDescriptor::Buffer;
    bufferDesc.DebugName  = "Shader Binding Table Staging Buffer";

    m_stagingBuffer = std::make_unique<DX12BufferResource>( m_context, bufferDesc );
    m_mappedMemory  = m_stagingBuffer->MapMemory( );

    if ( !m_mappedMemory )
    {
        spdlog::error("Failed to map memory for shader binding table.");
    }

    bufferDesc.Usages    = BitSet( ResourceUsage::CopyDst ) | ResourceUsage::ShaderBindingTable;
    bufferDesc.HeapType  = HeapType::GPU;
    bufferDesc.DebugName = "Shader Binding Table Buffer";
    m_buffer             = std::make_unique<DX12BufferResource>( m_context, bufferDesc );

    const UINT64 gpuVA                      = m_buffer->Resource( )->GetGPUVirtualAddress( );
    m_rayGenerationShaderRange.StartAddress = gpuVA;
    m_rayGenerationShaderRange.SizeInBytes  = rayGenerationShaderNumBytes;
    m_hitGroupOffset                        = AlignRecord( m_rayGenerationShaderRange.SizeInBytes );

    m_hitGroupShaderRange.StartAddress  = gpuVA + m_hitGroupOffset;
    m_hitGroupShaderRange.SizeInBytes   = desc.NumHitGroups * m_hitGroupNumBytes;
    m_hitGroupShaderRange.StrideInBytes = m_hitGroupNumBytes;

    m_missGroupOffset               = AlignRecord( m_hitGroupOffset + hitGroupNumBytes );
    m_missShaderRange.StartAddress  = gpuVA + m_missGroupOffset;
    m_missShaderRange.SizeInBytes   = missShaderNumBytes;
    m_missShaderRange.StrideInBytes = m_missNumBytes;
}

void DX12ShaderBindingTable::BindRayGenerationShader( const RayGenerationBindingDesc &desc )
{
    const void *shaderIdentifier = m_pipeline->GetShaderIdentifier( desc.ShaderName.Get( ) );
    memcpy( m_mappedMemory, shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );

#ifndef NDEBUG
    m_debugData.RayGenerationShaders.AddElement( { shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, 0, desc.ShaderName.Get( ) } );
#endif
}

void DX12ShaderBindingTable::BindHitGroup( const HitGroupBindingDesc &desc )
{
    const uint32_t offset        = m_hitGroupOffset + desc.Offset * m_hitGroupNumBytes;
    void          *hitGroupEntry = static_cast<Byte *>( m_mappedMemory ) + offset;

    if ( desc.HitGroupExportName.IsEmpty( ) )
    {
        spdlog::error("Hit group name cannot be empty.");
        return;
    }

    const void *hitGroupIdentifier = m_pipeline->GetShaderIdentifier( desc.HitGroupExportName.Get( ) );
    if ( !hitGroupIdentifier )
    {
        spdlog::error("Hit group export not found in pipeline.");
        return;
    }
    memcpy( hitGroupEntry, hitGroupIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );
    uint32_t hitGroupDataSize = 0;
    if ( desc.Data )
    {
        void                      *hitGroupData = static_cast<Byte *>( hitGroupEntry ) + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        const DX12ShaderLocalData *data         = dynamic_cast<DX12ShaderLocalData *>( desc.Data );
        hitGroupDataSize                        = data->DataNumBytes( );
        memcpy( hitGroupData, data->Data( ), hitGroupDataSize );
    }

#ifndef NDEBUG
    m_debugData.HitGroups.AddElement( { hitGroupIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, hitGroupDataSize, desc.HitGroupExportName.Get( ) } );
#endif
}

void DX12ShaderBindingTable::BindMissShader( const MissBindingDesc &desc )
{
    const uint32_t offset           = m_missGroupOffset + desc.Offset * m_missNumBytes;
    void          *missShaderEntry  = static_cast<Byte *>( m_mappedMemory ) + offset;
    const void    *shaderIdentifier = m_pipeline->GetShaderIdentifier( desc.ShaderName.Get( ) );
    memcpy( missShaderEntry, shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );

#ifndef NDEBUG
    m_debugData.MissShaders.AddElement( { shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, 0, desc.ShaderName.Get( ) } );
#endif
}

uint32_t DX12ShaderBindingTable::AlignRecord( const uint32_t size ) const
{
    return Utilities::Align( size, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT );
}

void DX12ShaderBindingTable::Build( )
{
    PrintShaderBindingTableDebugData( m_debugData );
    m_stagingBuffer->UnmapMemory( );

    wil::com_ptr<ID3D12CommandAllocator>     commandAllocator;
    wil::com_ptr<ID3D12GraphicsCommandList7> commandList;

    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( commandAllocator.put( ) ) ) );
    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.get( ), nullptr, IID_PPV_ARGS( commandList.put( ) ) ) );

    wil::com_ptr<ID3D12Fence> fence;
    DX_CHECK_RESULT( m_context->D3DDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( fence.put( ) ) ) );

    DX_CHECK_RESULT( commandList->Close( ) );
    DX_CHECK_RESULT( commandAllocator->Reset( ) );
    DX_CHECK_RESULT( commandList->Reset( commandAllocator.get( ), nullptr ) );

    const auto stagingBarrier = PipelineBarrierDesc{ }
                                    .BufferBarrier( BufferBarrierDesc{ m_stagingBuffer.get( ), ResourceUsage::Common, ResourceUsage::CopySrc } )
                                    .BufferBarrier( BufferBarrierDesc{ m_buffer.get( ), ResourceUsage::Common, ResourceUsage::CopyDst } );
    DX12BarrierHelper::ExecuteResourceBarrier( m_context, commandList.get( ), QueueType::Graphics, stagingBarrier );

    commandList->CopyBufferRegion( m_buffer->Resource( ), 0, m_stagingBuffer->Resource( ), 0, m_numBufferBytes );

    const auto bufferBarrier = PipelineBarrierDesc{ }.BufferBarrier( BufferBarrierDesc{ m_buffer.get( ), ResourceUsage::CopyDst, ResourceUsage::ShaderResource } );
    DX12BarrierHelper::ExecuteResourceBarrier( m_context, commandList.get( ), QueueType::Graphics, bufferBarrier );

    DX_CHECK_RESULT( commandList->Close( ) );
    m_context->GraphicsCommandQueue->ExecuteCommandLists( 1, CommandListCast( commandList.addressof( ) ) );
    DX_CHECK_RESULT( m_context->GraphicsCommandQueue->Signal( fence.get( ), 1 ) );
    if ( fence->GetCompletedValue( ) < 1 )
    {
        const HANDLE eventHandle = CreateEventEx( nullptr, nullptr, 0, EVENT_ALL_ACCESS );
        DX_CHECK_RESULT( fence->SetEventOnCompletion( 1, eventHandle ) );
        WaitForSingleObject( eventHandle, INFINITE );
        CloseHandle( eventHandle );
    }
}

D3D12_GPU_VIRTUAL_ADDRESS_RANGE DX12ShaderBindingTable::RayGenerationShaderRecord( ) const
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
