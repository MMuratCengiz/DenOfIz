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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12CommandListPool.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12CommandQueue.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12EnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12CommandListPool::DX12CommandListPool( DX12Context *context, CommandListPoolDesc desc ) : m_desc( desc )
{
    DZ_NOT_NULL( context );
    DZ_ASSERTM( desc.NumCommandLists > 0, "CommandListCount must be greater than 0" );

    m_context                            = context;
    const DX12CommandQueue *commandQueue = dynamic_cast<DX12CommandQueue *>( desc.CommandQueue );

    for ( uint32_t i = 0; i < desc.NumCommandLists; i++ )
    {
        const D3D12_COMMAND_LIST_TYPE commandListType = DX12EnumConverter::ConvertQueueType( commandQueue->GetQueueType( ) );

        wil::com_ptr<ID3D12CommandAllocator> commandAllocator;
        DX_CHECK_RESULT( context->D3DDevice->CreateCommandAllocator( commandListType, IID_PPV_ARGS( commandAllocator.put( ) ) ) );

        wil::com_ptr<ID3D12GraphicsCommandList> dx12CommandList;
        DX_CHECK_RESULT( context->D3DDevice->CreateCommandList( 0, commandListType, commandAllocator.get( ), nullptr, IID_PPV_ARGS( dx12CommandList.put( ) ) ) );
        dx12CommandList->Close( );

        m_dx12CommandLists.push_back( std::move( dx12CommandList ) );
        m_commandAllocators.push_back( std::move( commandAllocator ) );
    }

    CommandListDesc commandListCreateInfo{ };
    commandListCreateInfo.QueueType = commandQueue->GetQueueType( );

    for ( uint32_t i = 0; i < desc.NumCommandLists; i++ )
    {
        m_commandLists.push_back( std::make_unique<DX12CommandList>( m_context, m_commandAllocators[ i ], m_dx12CommandLists[ i ], commandListCreateInfo ) );
        m_commandListPtrs.push_back( m_commandLists[ i ].get( ) );
    }
}

ICommandListArray DX12CommandListPool::GetCommandLists( )
{
    ICommandListArray commandLists{ };
    commandLists.Elements    = reinterpret_cast<ICommandList **>( m_commandListPtrs.data( ) );
    commandLists.NumElements = static_cast<uint32_t>( m_commandListPtrs.size( ) );
    return commandLists;
}
