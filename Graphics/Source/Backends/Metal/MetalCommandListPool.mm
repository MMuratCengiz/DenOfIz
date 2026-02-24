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

#import "DenOfIzGraphicsInternal/Backends/Metal/MetalCommandListPool.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/MetalCommandQueue.h"

using namespace DenOfIz;

MetalCommandListPool::MetalCommandListPool( MetalContext *context, DenOfIz_CommandListPoolDesc desc ) : m_context( context ), m_desc( desc )
{
    MetalCommandQueue *commandQueue = dynamic_cast<MetalCommandQueue *>( DENOFIZ_FROM_HANDLE( ICommandQueue, desc.CommandQueue ) );

    DenOfIz_CommandListDesc commandListDesc;
    commandListDesc.QueueType = commandQueue->GetQueueType( );

    for ( uint32_t i = 0; i < m_desc.NumCommandLists; i++ )
    {
        m_commandLists.push_back( std::make_unique<MetalCommandList>( m_context, commandQueue->GetQueue(), commandListDesc ) );
        m_commandListPtrs.push_back( reinterpret_cast<DenOfIz_CommandList>( m_commandLists[ i ].get( ) ) );
    }
}

DenOfIz_CommandListArray MetalCommandListPool::GetCommandLists( )
{
    DenOfIz_CommandListArray commandLists;
    commandLists.Elements = m_commandListPtrs.data( );
    commandLists.NumElements = static_cast<uint32_t>( m_commandListPtrs.size( ) );
    return commandLists;
}