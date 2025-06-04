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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Context.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Fence.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12Fence::DX12Fence( DX12Context *context ) : m_context( context ), m_fenceValue( 0 )
{
    DX_CHECK_RESULT( m_context->D3DDevice->CreateFence( INITIAL_FENCE_VALUE, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( m_fence.put( ) ) ) );
    m_fenceEvent.Attach( CreateEvent( nullptr, FALSE, FALSE, nullptr )  );
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

void DX12Fence:: NotifyCommandQueue( ID3D12CommandQueue *commandQueue )
{
    DX_CHECK_RESULT( commandQueue->Signal( m_fence.get( ), ++m_fenceValue ) );
}

ID3D12Fence *DX12Fence::GetFence( ) const
{
    return m_fence.get( );
}
