/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Fence.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Context.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12Fence::DX12Fence( DX12Context *context ) : m_context( context ), m_fenceValue( 0 )
{
    DX_CHECK_RESULT( m_context->D3DDevice->CreateFence( INITIAL_FENCE_VALUE, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( m_fence.put( ) ) ) );
    m_fenceEvent.Attach( CreateEvent( nullptr, FALSE, FALSE, nullptr ) );
}

DX12Fence::~DX12Fence( ) = default;

void DX12Fence::Wait( )
{
    if ( const uint64_t completedValue = m_fence->GetCompletedValue( ); completedValue < m_fenceValue )
    {
        DX_CHECK_RESULT( m_fence->SetEventOnCompletion( m_fenceValue, m_fenceEvent.Get( ) ) );
        WaitForSingleObjectEx( m_fenceEvent.Get( ), INFINITE, FALSE );
    }
}

void DX12Fence::Reset( )
{
    ++m_fenceValue;
}

void DX12Fence::NotifyCommandQueue( ID3D12CommandQueue *commandQueue )
{
    DX_CHECK_RESULT( commandQueue->Signal( m_fence.get( ), ++m_fenceValue ) );
}

ID3D12Fence *DX12Fence::GetFence( ) const
{
    return m_fence.get( );
}
