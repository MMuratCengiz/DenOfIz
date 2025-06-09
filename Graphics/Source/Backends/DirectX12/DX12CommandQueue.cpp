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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12CommandQueue.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Fence.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Semaphore.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12CommandQueue::DX12CommandQueue( DX12Context *context, const CommandQueueDesc &desc ) : m_context( context ), m_desc( desc )
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = { };

    switch ( desc.QueueType )
    {
    case QueueType::Graphics:
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        break;
    case QueueType::Compute:
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        break;
    case QueueType::Copy:
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        break;
    default:
        spdlog::error( "Unknown queue type" );
        break;
    }

    switch ( desc.Priority )
    {
    case QueuePriority::Low:
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        break;
    case QueuePriority::Normal:
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
        break;
    case QueuePriority::High:
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME;
        break;
    }

    queueDesc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;

    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( m_commandQueue.put( ) ) ) );
    DX_CHECK_RESULT( m_context->D3DDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( m_fence.put( ) ) ) );
}

DX12CommandQueue::~DX12CommandQueue( )
{
    WaitIdle( );
}

void DX12CommandQueue::WaitIdle( )
{
    const uint64_t fenceValue = m_fence->GetCompletedValue( ) + 1;
    DX_CHECK_RESULT( m_commandQueue->Signal( m_fence.get( ), fenceValue ) );
    if ( m_fence->GetCompletedValue( ) < fenceValue )
    {
        const HANDLE eventHandle = CreateEvent( nullptr, FALSE, FALSE, nullptr );
        DX_CHECK_RESULT( m_fence->SetEventOnCompletion( fenceValue, eventHandle ) );
        WaitForSingleObject( eventHandle, INFINITE );
        CloseHandle( eventHandle );
    }
}

void DX12CommandQueue::ExecuteCommandLists( const ExecuteCommandListsDesc &executeCommandListsDesc )
{
    for ( uint32_t i = 0; i < executeCommandListsDesc.WaitSemaphores.NumElements; i++ )
    {
        const auto *dx12Semaphore = dynamic_cast<DX12Semaphore *>( executeCommandListsDesc.WaitSemaphores.Elements[ i ] );
        DX_CHECK_RESULT( m_commandQueue->Wait( dx12Semaphore->GetFence( ), dx12Semaphore->GetCurrentValue( ) ) );
    }

    std::vector<ID3D12CommandList *> d3dCommandLists;
    d3dCommandLists.reserve( executeCommandListsDesc.CommandLists.NumElements );

    for ( uint32_t i = 0; i < executeCommandListsDesc.CommandLists.NumElements; i++ )
    {
        const auto *dx12CmdList = dynamic_cast<DX12CommandList *>( executeCommandListsDesc.CommandLists.Elements[ i ] );
        d3dCommandLists.push_back( dx12CmdList->GetCommandList( ) );
    }

    m_commandQueue->ExecuteCommandLists( static_cast<UINT>( d3dCommandLists.size( ) ), d3dCommandLists.data( ) );

    if ( executeCommandListsDesc.Signal )
    {
        auto *dx12Fence = dynamic_cast<DX12Fence *>( executeCommandListsDesc.Signal );
        dx12Fence->NotifyCommandQueue( m_commandQueue.get( ) );
    }

    for ( uint32_t i = 0; i < executeCommandListsDesc.SignalSemaphores.NumElements; i++ )
    {
        auto *dx12Semaphore = dynamic_cast<DX12Semaphore *>( executeCommandListsDesc.SignalSemaphores.Elements[ i ] );
        dx12Semaphore->NotifyCommandQueue( m_commandQueue.get( ) );
    }
}

ID3D12CommandQueue *DX12CommandQueue::GetCommandQueue( ) const
{
    return m_commandQueue.get( );
}

QueueType DX12CommandQueue::GetQueueType( ) const
{
    return m_desc.QueueType;
}
