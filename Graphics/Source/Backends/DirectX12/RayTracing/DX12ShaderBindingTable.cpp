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
    uint32_t totalNumEntries = desc.NumRayGenerationShaders + desc.NumMissShaders + ( desc.NumInstances * desc.NumGeometries * desc.NumRayTypes );
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

    m_rayGenerationShaderRange.StartAddress = Utilities::Align( m_buffer->Resource( )->GetGPUVirtualAddress( ), D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT );
    m_rayGenerationShaderRange.SizeInBytes  = Utilities::Align( D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT );
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

    const uint32_t offset        = ( instanceOffset + geometryOffset + rayTypeOffset ) * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    void          *hitGroupEntry = static_cast<Byte *>( m_mappedMemory ) + offset;

    const void *hitGroupIdentifier = m_pipeline->GetShaderIdentifier( desc.HitGroupExportName.Get( ) );
    if ( desc.HitGroupExportName.IsEmpty( ) )
    {
        LOG( ERROR ) << "Hit group name cannot be empty.";
        return;
    }

    memcpy( hitGroupEntry, hitGroupIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );

    m_hitGroupShaderRange.StartAddress  = m_buffer->Resource( )->GetGPUVirtualAddress( ) +  + ( offset & ~( D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT - 1 ) );
    m_hitGroupShaderRange.SizeInBytes   = Utilities::Align( D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT );
    m_hitGroupShaderRange.StrideInBytes = Utilities::Align( D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT );
}

bool DX12ShaderBindingTable::BindHitGroupRecursive( const HitGroupBindingDesc &desc )
{
    if ( desc.InstanceIndex == -1 )
    {
        if ( desc.GeometryIndex == -1 || desc.RayTypeIndex == -1 )
        {
            LOG( ERROR ) << "InstanceIndex/GeometryIndex/RayTypeIndex work in a parent child relationship. If InstanceIndex == -1, then GeometryIndex/RayTypeIndex cannot be -1.";
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
        if ( desc.RayTypeIndex == -1 )
        {
            LOG( ERROR ) << "InstanceIndex/GeometryIndex/RayTypeIndex work in a parent child relationship. If GeometryIndex == -1, then RayTypeIndex cannot be -1.";
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
    const uint32_t baseOffset       = m_desc.SizeDesc.NumRayGenerationShaders * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    const uint32_t offset           = baseOffset + desc.RayTypeIndex * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    void          *missShaderEntry  = static_cast<Byte *>( m_mappedMemory ) + offset;
    const void    *shaderIdentifier = m_pipeline->GetShaderIdentifier( desc.ShaderName.Get( ) );

    memcpy( missShaderEntry, shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );

    m_missShaderRange.StartAddress  = m_buffer->Resource( )->GetGPUVirtualAddress( ) + ( offset & ~( D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT - 1 ) );
    m_missShaderRange.SizeInBytes   = Utilities::Align( D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT );
    m_missShaderRange.StrideInBytes = Utilities::Align( D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT );
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
    commandList->Reset( m_context->CopyCommandListAllocator.get( ), nullptr );
    commandList->CopyBufferRegion( m_stagingBuffer->Resource( ), 0, m_buffer->Resource( ), 0, m_numBufferBytes );
    commandList->Close( );
    m_context->CopyCommandQueue->ExecuteCommandLists( 1, CommandListCast( commandList.addressof( ) ) );
    m_context->CopyCommandQueue->Signal( copyFence.get( ), 1 );

    // Create a direct command list to barrier the resource to shader resource state:
    wil::com_ptr<ID3D12CommandAllocator>    commandAllocator;
    wil::com_ptr<ID3D12GraphicsCommandList> directCommandList;

    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( commandAllocator.put( ) ) ) );
    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.get( ), nullptr, IID_PPV_ARGS( directCommandList.put( ) ) ) );

    directCommandList->Close( );
    commandAllocator->Reset( );
    directCommandList->Reset( commandAllocator.get( ), nullptr );
    const auto &barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_buffer->Resource( ), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE );
    directCommandList->ResourceBarrier( 1, &barrier );
    directCommandList->Close( );
    m_context->GraphicsCommandQueue->Wait( copyFence.get( ), 1 );
    m_context->GraphicsCommandQueue->ExecuteCommandLists( 1, CommandListCast( directCommandList.addressof( ) ) );
    m_context->GraphicsCommandQueue->Signal( barrierFence.get( ), 1 );
    m_context->GraphicsCommandQueue->Wait( barrierFence.get( ), 1 );
}

IBufferResource *DX12ShaderBindingTable::Buffer( ) const
{
    return m_buffer.get( );
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
