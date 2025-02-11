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

#include <DenOfIzGraphics/Backends/DirectX12/DX12Semaphore.h>

using namespace DenOfIz;

DX12Semaphore::DX12Semaphore( DX12Context *context ) : m_context( context ), m_fenceValue( 0 )
{
    DX_CHECK_RESULT( m_context->D3DDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( m_fence.put( ) ) ) );
}

DX12Semaphore::~DX12Semaphore( )
{
    m_fence = nullptr;
}

void DX12Semaphore::Notify( )
{
    DX_CHECK_RESULT( m_fence->Signal( ++m_fenceValue ) );
}

void DX12Semaphore::NotifyCommandQueue( ID3D12CommandQueue *commandQueue )
{
    DX_CHECK_RESULT( commandQueue->Signal( m_fence.get( ), ++m_fenceValue ) );
}

ID3D12Fence *DX12Semaphore::GetFence( ) const
{
    return m_fence.get( );
}

uint64_t DX12Semaphore::GetCurrentValue( ) const
{
    return m_fenceValue;
}
