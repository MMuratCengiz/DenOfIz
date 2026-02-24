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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUCommandListPool.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUCommandList.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUCommandQueue.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

WebGPUCommandListPool::WebGPUCommandListPool( WebGPUContext *context, const DenOfIz_CommandListPoolDesc &desc ) : m_context( context ), m_desc( desc )
{
    if ( !m_context || !m_context->Device )
    {
        spdlog::critical( "WebGPUCommandListPool: Invalid context or device" );
        return;
    }

    m_commandLists.reserve( desc.NumCommandLists );
    m_commandListPtrs.reserve( desc.NumCommandLists );

    ICommandQueue          *commandQueuePtr = DENOFIZ_FROM_HANDLE( ICommandQueue, desc.CommandQueue );
    const auto             *commandQueue    = dynamic_cast<WebGPUCommandQueue *>( commandQueuePtr );
    DenOfIz_CommandListDesc commandListDesc{ };
    commandListDesc.QueueType = commandQueue->GetQueueType( );

    for ( uint32_t i = 0; i < desc.NumCommandLists; i++ )
    {
        m_commandLists.emplace_back( std::make_unique<WebGPUCommandList>( m_context, commandQueuePtr, commandListDesc ) );
        m_commandListPtrs.push_back( reinterpret_cast<DenOfIz_CommandList>( m_commandLists.back( ).get( ) ) );
    }
}

WebGPUCommandListPool::~WebGPUCommandListPool( ) = default;

DenOfIz_CommandListArray WebGPUCommandListPool::GetCommandLists( )
{
    DenOfIz_CommandListArray array{ };
    array.Elements    = m_commandListPtrs.data( );
    array.NumElements = static_cast<uint32_t>( m_commandListPtrs.size( ) );
    return array;
}
