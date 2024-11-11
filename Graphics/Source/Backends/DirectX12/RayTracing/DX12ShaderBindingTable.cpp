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
#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12ShaderLocalData.h>

using namespace DenOfIz;

DX12ShaderBindingTable::DX12ShaderBindingTable( DX12Context *context, const ShaderBindingTableDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_pipeline          = dynamic_cast<DX12Pipeline *>( desc.Pipeline );
    m_rayGenNumBytes    = Utilities::Align( D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + m_desc.MaxRayGenDataBytes, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT );
    m_hitGroupNumBytes  = Utilities::Align( D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + m_desc.MaxHitGroupDataBytes, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT );
    m_missGroupNumBytes = Utilities::Align( D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + m_desc.MaxMissDataBytes, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT );
    Resize( desc.SizeDesc );
}

void DX12ShaderBindingTable::Resize( const SBTSizeDesc &desc )
{
    const uint32_t rayGenerationShaderNumBytes = AlignRecord( desc.NumRayGenerationShaders * m_rayGenNumBytes );
    const uint32_t hitGroupNumBytes            = AlignRecord( desc.NumInstances * desc.NumGeometries * desc.NumRayTypes * m_hitGroupNumBytes );
    const uint32_t missShaderNumBytes          = AlignRecord( desc.NumMissShaders * m_missGroupNumBytes );
    m_numBufferBytes                           = rayGenerationShaderNumBytes + hitGroupNumBytes + missShaderNumBytes;

    BufferDesc bufferDesc   = { };
    bufferDesc.NumBytes     = m_numBufferBytes;
    bufferDesc.HeapType     = HeapType::GPU_CPU;
    bufferDesc.InitialUsage = ResourceUsage::CopySrc;
    bufferDesc.Descriptor   = ResourceDescriptor::Buffer;
    bufferDesc.DebugName    = "Shader Binding Table Staging Buffer";

    m_stagingBuffer = std::make_unique<DX12BufferResource>( m_context, bufferDesc );
    m_mappedMemory  = m_stagingBuffer->MapMemory( );

    if ( !m_mappedMemory )
    {
        LOG( ERROR ) << "Failed to map memory for shader binding table.";
    }

    bufferDesc.HeapType     = HeapType::GPU;
    bufferDesc.InitialUsage = ResourceUsage::CopyDst;
    bufferDesc.DebugName    = "Shader Binding Table Buffer";
    m_buffer                = std::make_unique<DX12BufferResource>( m_context, bufferDesc );

    m_rayGenerationShaderRange.StartAddress = m_buffer->Resource( )->GetGPUVirtualAddress( );
    m_rayGenerationShaderRange.SizeInBytes  = rayGenerationShaderNumBytes;
    m_hitGroupOffset                        = m_rayGenerationShaderRange.SizeInBytes;

    m_hitGroupShaderRange.StartAddress  = m_buffer->Resource( )->GetGPUVirtualAddress( ) + m_hitGroupOffset;
    m_hitGroupShaderRange.SizeInBytes   = desc.NumInstances * desc.NumGeometries * desc.NumRayTypes * m_hitGroupNumBytes;
    m_hitGroupShaderRange.StrideInBytes = m_hitGroupNumBytes;

    m_missGroupOffset               = m_hitGroupOffset + hitGroupNumBytes;
    m_missShaderRange.StartAddress  = AlignRecord( m_buffer->Resource( )->GetGPUVirtualAddress( ) + m_missGroupOffset );
    m_missShaderRange.SizeInBytes   = missShaderNumBytes;
    m_missShaderRange.StrideInBytes = missShaderNumBytes;
}

void DX12ShaderBindingTable::BindRayGenerationShader( const RayGenerationBindingDesc &desc )
{
    const void *shaderIdentifier = m_pipeline->GetShaderIdentifier( desc.ShaderName.Get( ) );
    memcpy( m_mappedMemory, shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );
}

void DX12ShaderBindingTable::BindHitGroup( const HitGroupBindingDesc &desc )
{
    if ( BindHitGroupRecursive( desc ) )
    {
        return;
    }

    const uint32_t instanceOffset = desc.InstanceIndex * m_desc.SizeDesc.NumGeometries * m_desc.SizeDesc.NumRayTypes;
    const uint32_t geometryOffset = desc.GeometryIndex * m_desc.SizeDesc.NumRayTypes;
    const uint32_t rayTypeOffset  = desc.RayTypeIndex;

    const uint32_t offset        = m_hitGroupOffset + ( instanceOffset + geometryOffset + rayTypeOffset ) * m_hitGroupNumBytes;
    void          *hitGroupEntry = static_cast<Byte *>( m_mappedMemory ) + offset;

    if ( desc.HitGroupExportName.IsEmpty( ) )
    {
        LOG( ERROR ) << "Hit group name cannot be empty.";
        return;
    }

    const void *hitGroupIdentifier = m_pipeline->GetShaderIdentifier( desc.HitGroupExportName.Get( ) );
    if ( !hitGroupIdentifier )
    {
        LOG( ERROR ) << "Hit group export not found in pipeline.";
        return;
    }
    memcpy( hitGroupEntry, hitGroupIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );
    if ( desc.Data )
    {
        void                *hitGroupData = static_cast<Byte *>( hitGroupEntry ) + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        DX12ShaderLocalData *data         = dynamic_cast<DX12ShaderLocalData *>( desc.Data );
        memcpy( hitGroupData, data->Data( ), m_desc.MaxHitGroupDataBytes );
    }
}

bool DX12ShaderBindingTable::BindHitGroupRecursive( const HitGroupBindingDesc &desc )
{
    if ( desc.InstanceIndex == -1 )
    {
        if ( desc.GeometryIndex != -1 || desc.RayTypeIndex != -1 )
        {
            LOG( ERROR )
                << "InstanceIndex/GeometryIndex/RayTypeIndex work in a parent child relationship. If InstanceIndex == -1, then GeometryIndex/RayTypeIndex must also be -1.";
            return true;
        }

        for ( uint32_t i = 0; i < m_desc.SizeDesc.NumInstances; ++i )
        {
            HitGroupBindingDesc hitGroupDesc = desc;
            hitGroupDesc.InstanceIndex       = i;
            hitGroupDesc.GeometryIndex       = -1;
            hitGroupDesc.RayTypeIndex        = -1;
            BindHitGroupRecursive( hitGroupDesc );
        }

        return true;
    }

    if ( desc.GeometryIndex == -1 )
    {
        if ( desc.RayTypeIndex != -1 )
        {
            LOG( ERROR ) << "InstanceIndex/GeometryIndex/RayTypeIndex work in a parent child relationship. If GeometryIndex == -1, then RayTypeIndex must also be -1.";
            return true;
        }

        for ( uint32_t i = 0; i < m_desc.SizeDesc.NumGeometries; ++i )
        {
            HitGroupBindingDesc hitGroupDesc = desc;
            hitGroupDesc.GeometryIndex       = i;
            hitGroupDesc.RayTypeIndex        = -1;
            BindHitGroupRecursive( hitGroupDesc );
        }

        return true;
    }

    if ( desc.RayTypeIndex == -1 )
    {
        for ( uint32_t i = 0; i < m_desc.SizeDesc.NumRayTypes; ++i )
        {
            HitGroupBindingDesc hitGroupDesc = desc;
            hitGroupDesc.RayTypeIndex        = i;
            BindHitGroup( hitGroupDesc );
        }
        return true;
    }

    return false;
}

void DX12ShaderBindingTable::BindMissShader( const MissBindingDesc &desc )
{
    uint32_t    offset           = m_missGroupOffset + desc.RayTypeIndex * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    void       *missShaderEntry  = static_cast<Byte *>( m_mappedMemory ) + offset;
    const void *shaderIdentifier = m_pipeline->GetShaderIdentifier( desc.ShaderName.Get( ) );

    memcpy( missShaderEntry, shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );
}

uint32_t DX12ShaderBindingTable::AlignRecord( const uint32_t size ) const
{
    return Utilities::Align( size, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT );
}

void DX12ShaderBindingTable::Build( )
{
    m_stagingBuffer->UnmapMemory( );

    // Todo move this to some utility function:
    // Create a fence:
    wil::com_ptr<ID3D12Fence> copyFence;
    wil::com_ptr<ID3D12Fence> barrierFence;
    DX_CHECK_RESULT( m_context->D3DDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( copyFence.put( ) ) ) );
    DX_CHECK_RESULT( m_context->D3DDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( barrierFence.put( ) ) ) );

    const auto commandList = m_context->CopyCommandList;
    DX_CHECK_RESULT( commandList->Reset( m_context->CopyCommandListAllocator.get( ), nullptr ) );
    commandList->CopyBufferRegion( m_buffer->Resource( ), 0, m_stagingBuffer->Resource( ), 0, m_numBufferBytes );
    DX_CHECK_RESULT( commandList->Close( ) );
    m_context->CopyCommandQueue->ExecuteCommandLists( 1, CommandListCast( commandList.addressof( ) ) );
    DX_CHECK_RESULT( m_context->CopyCommandQueue->Signal( copyFence.get( ), 1 ) );

    // Create a direct command list to barrier the resource to shader resource state:
    wil::com_ptr<ID3D12CommandAllocator>    commandAllocator;
    wil::com_ptr<ID3D12GraphicsCommandList> directCommandList;

    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( commandAllocator.put( ) ) ) );
    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.get( ), nullptr, IID_PPV_ARGS( directCommandList.put( ) ) ) );

    DX_CHECK_RESULT( directCommandList->Close( ) );
    DX_CHECK_RESULT( commandAllocator->Reset( ) );
    DX_CHECK_RESULT( directCommandList->Reset( commandAllocator.get( ), nullptr ) );
    const auto &barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_buffer->Resource( ), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE );
    directCommandList->ResourceBarrier( 1, &barrier );
    DX_CHECK_RESULT( directCommandList->Close( ) );
    DX_CHECK_RESULT( m_context->GraphicsCommandQueue->Wait( copyFence.get( ), 1 ) );
    m_context->GraphicsCommandQueue->ExecuteCommandLists( 1, CommandListCast( directCommandList.addressof( ) ) );
    DX_CHECK_RESULT( m_context->GraphicsCommandQueue->Signal( barrierFence.get( ), 1 ) );
    DX_CHECK_RESULT( m_context->GraphicsCommandQueue->Wait( barrierFence.get( ), 1 ) );
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
