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
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

#define DX12_PIPELINE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::DX12Pipeline, handle )
#define DX12_SHADER_LOCAL_DATA_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::DX12ShaderLocalData, handle )

using namespace DenOfIz;

DX12ShaderBindingTable::DX12ShaderBindingTable( DX12Context *context, const DenOfIz_ShaderBindingTableDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_pipeline                   = DX12_PIPELINE_IMPL( desc.Pipeline );
    m_rayGenNumBytes             = Utilities::Align( D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + m_desc.MaxRayGenDataBytes, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT );
    m_hitGroupNumBytes           = Utilities::Align( D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + m_desc.MaxHitGroupDataBytes, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT );
    m_missNumBytes               = Utilities::Align( D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + m_desc.MaxMissDataBytes, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT );
    m_debugData.RayGenNumBytes   = m_rayGenNumBytes;
    m_debugData.MissNumBytes     = m_missNumBytes;
    m_debugData.HitGroupNumBytes = m_hitGroupNumBytes;
    Resize( desc.SizeDesc );
}

void DX12ShaderBindingTable::Resize( const DenOfIz_SBTSizeDesc &desc )
{
    const uint32_t rayGenerationShaderNumBytes = desc.NumRayGenerationShaders * m_rayGenNumBytes;
    const uint32_t hitGroupNumBytes            = desc.NumHitGroups * m_hitGroupNumBytes;
    const uint32_t missShaderNumBytes          = desc.NumMissShaders * m_missNumBytes;
    m_numBufferBytes                           = AlignRecord( rayGenerationShaderNumBytes ) + AlignRecord( hitGroupNumBytes ) + AlignRecord( missShaderNumBytes );

    DenOfIz_BufferDesc bufferDesc = { };
    bufferDesc.NumBytes           = m_numBufferBytes;
    bufferDesc.HeapType           = DENOFIZ_HEAP_TYPE_CPU_GPU;
    bufferDesc.Usage              = DENOFIZ_BUFFER_USAGE_COPY_SRC_BIT | DENOFIZ_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT;
    bufferDesc.DebugName          = DENOFIZ_STRING( "Shader Binding Table Staging Buffer" );

    m_stagingBuffer = std::make_unique<DX12Buffer>( m_context, bufferDesc );
    m_mappedMemory  = m_stagingBuffer->MapMemory( );

    if ( !m_mappedMemory )
    {
        spdlog::error( "Failed to map memory for shader binding table." );
    }

    bufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_COPY_DST_BIT | DENOFIZ_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT;
    bufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    bufferDesc.DebugName = DENOFIZ_STRING( "Shader Binding Table Buffer" );
    m_buffer             = std::make_unique<DX12Buffer>( m_context, bufferDesc );

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

void DX12ShaderBindingTable::BindRayGenerationShader( const DenOfIz_RayGenerationBindingDesc &desc )
{
    const std::string shaderName( desc.ShaderName.Chars, desc.ShaderName.NumChars );
    const void       *shaderIdentifier = m_pipeline->GetShaderIdentifier( shaderName );
    memcpy( m_mappedMemory, shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );

#ifndef NDEBUG
    m_debugData.RayGenerationShaders.push_back( { shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, 0, shaderName } );
#endif
}

void DX12ShaderBindingTable::BindHitGroup( const DenOfIz_HitGroupBindingDesc &desc )
{
    const uint32_t offset        = m_hitGroupOffset + desc.Offset * m_hitGroupNumBytes;
    void          *hitGroupEntry = static_cast<Byte *>( m_mappedMemory ) + offset;

    if ( desc.HitGroupExportName.Chars == nullptr || desc.HitGroupExportName.NumChars == 0 )
    {
        spdlog::error( "Hit group name cannot be empty." );
        return;
    }

    const std::string hitGroupExportName( desc.HitGroupExportName.Chars, desc.HitGroupExportName.NumChars );
    const void       *hitGroupIdentifier = m_pipeline->GetShaderIdentifier( hitGroupExportName );
    if ( !hitGroupIdentifier )
    {
        spdlog::error( "Hit group export not found in pipeline." );
        return;
    }
    memcpy( hitGroupEntry, hitGroupIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );
    uint32_t hitGroupDataSize = 0;
    if ( DENOFIZ_HANDLE_IS_VALID( desc.Data ) )
    {
        void                      *hitGroupData = static_cast<Byte *>( hitGroupEntry ) + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        const DX12ShaderLocalData *data         = DX12_SHADER_LOCAL_DATA_IMPL( desc.Data );
        hitGroupDataSize                        = data->DataNumBytes( );
        memcpy( hitGroupData, data->Data( ), hitGroupDataSize );
    }

#ifndef NDEBUG
    m_debugData.HitGroups.push_back( { hitGroupIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, hitGroupDataSize, hitGroupExportName } );
#endif
}

void DX12ShaderBindingTable::BindMissShader( const DenOfIz_MissBindingDesc &desc )
{
    const uint32_t    offset          = m_missGroupOffset + desc.Offset * m_missNumBytes;
    void             *missShaderEntry = static_cast<Byte *>( m_mappedMemory ) + offset;
    const std::string shaderName( desc.ShaderName.Chars, desc.ShaderName.NumChars );
    const void       *shaderIdentifier = m_pipeline->GetShaderIdentifier( shaderName );
    memcpy( missShaderEntry, shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );

#ifndef NDEBUG
    m_debugData.MissShaders.push_back( { shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, 0, shaderName } );
#endif
}

uint32_t DX12ShaderBindingTable::AlignRecord( const uint32_t size ) const
{
    return Utilities::Align( size, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT );
}

void DX12ShaderBindingTable::Build( )
{
#ifndef NDEBUG
    PrintShaderBindingTableDebugData( m_debugData );
#endif
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

    std::array<DenOfIz_BufferBarrierDesc, 2> stagingBarriers = { DenOfIz_BufferBarrierDesc{ .Resource = DENOFIZ_TO_HANDLE( m_stagingBuffer.get( ) ),
                                                                                            .OldState = DENOFIZ_RESOURCE_USAGE_COMMON_BIT,
                                                                                            .NewState = DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT },
                                                                 DenOfIz_BufferBarrierDesc{ .Resource = DENOFIZ_TO_HANDLE( m_buffer.get( ) ),
                                                                                            .OldState = DENOFIZ_RESOURCE_USAGE_COMMON_BIT,
                                                                                            .NewState = DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT } };
    DenOfIz_PipelineBarrierDesc              stagingBarrierDesc{ };
    stagingBarrierDesc.BufferBarriers.Elements    = stagingBarriers.data( );
    stagingBarrierDesc.BufferBarriers.NumElements = stagingBarriers.size( );
    DX12BarrierHelper::ExecuteResourceBarrier( m_context, commandList.get( ), DENOFIZ_QUEUE_TYPE_GRAPHICS, &stagingBarrierDesc );

    commandList->CopyBufferRegion( m_buffer->Resource( ), 0, m_stagingBuffer->Resource( ), 0, m_numBufferBytes );

    std::array<DenOfIz_BufferBarrierDesc, 1> bufferBarriers = { DenOfIz_BufferBarrierDesc{
        .Resource = DENOFIZ_TO_HANDLE( m_buffer.get( ) ), .OldState = DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT, .NewState = DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT } };
    DenOfIz_PipelineBarrierDesc              bufferBarrierDesc{ };
    bufferBarrierDesc.BufferBarriers.Elements    = bufferBarriers.data( );
    bufferBarrierDesc.BufferBarriers.NumElements = bufferBarriers.size( );
    DX12BarrierHelper::ExecuteResourceBarrier( m_context, commandList.get( ), DENOFIZ_QUEUE_TYPE_GRAPHICS, &bufferBarrierDesc );

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
