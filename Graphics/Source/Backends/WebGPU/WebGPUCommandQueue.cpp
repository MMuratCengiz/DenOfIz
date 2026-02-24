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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUCommandQueue.h"
#include <vector>
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUCommandList.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUContext.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUFence.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

WebGPUCommandQueue::WebGPUCommandQueue( WebGPUContext *context, const DenOfIz_CommandQueueDesc &desc ) :
    m_context( context ), m_desc( desc ), m_waitIdleFence( std::make_unique<WebGPUFence>( context ) )
{
    if ( !m_context || !m_context->Queue )
    {
        spdlog::critical( "WebGPUCommandQueue: Invalid context or queue" );
    }
}

WebGPUCommandQueue::~WebGPUCommandQueue( ) = default;

void WebGPUCommandQueue::WaitIdle( )
{
    m_waitIdleFence->PrepareForSignal( );

    wgpuQueueSubmit( m_context->Queue, 0, nullptr );

#if DZ_WEBGPU_USE_DAWN_API
    WGPUQueueWorkDoneCallbackInfo workDoneInfo = { };
    workDoneInfo.callback                      = WebGPUFence::OnWorkDone;
    workDoneInfo.userdata1                     = m_waitIdleFence.get( );
#if defined( __EMSCRIPTEN__ )
    workDoneInfo.mode = WGPUCallbackMode_AllowSpontaneous;
#else
    workDoneInfo.mode = WGPUCallbackMode_AllowProcessEvents;
#endif
    wgpuQueueOnSubmittedWorkDone( m_context->Queue, workDoneInfo );
#else
    wgpuQueueOnSubmittedWorkDone( m_context->Queue, WebGPUFence::OnWorkDone, m_waitIdleFence.get( ) );
#endif
    m_waitIdleFence->Wait( );
}

void WebGPUCommandQueue::ExecuteCommandLists( const DenOfIz_ExecuteCommandListsDesc &executeCommandListsDesc )
{
    if ( !m_context->Queue )
    {
        spdlog::critical( "WebGPUCommandQueue::ExecuteCommandLists: Invalid context or queue" );
        return;
    }

    std::vector<WGPUCommandBuffer> commandBuffers;
    commandBuffers.reserve( executeCommandListsDesc.CommandLists.NumElements );

    for ( size_t i = 0; i < executeCommandListsDesc.CommandLists.NumElements; i++ )
    {
        if ( const auto commandList = dynamic_cast<WebGPUCommandList *>( DENOFIZ_FROM_HANDLE( ICommandList, executeCommandListsDesc.CommandLists.Elements[ i ] ) ) )
        {
            if ( WGPUCommandBuffer commandBuffer = commandList->Finish( ) )
            {
                commandBuffers.push_back( commandBuffer );
            }
        }
    }

    WebGPUFence *fence = nullptr;
    if ( DENOFIZ_HANDLE_IS_VALID( executeCommandListsDesc.Signal ) )
    {
        fence = dynamic_cast<WebGPUFence *>( DENOFIZ_FROM_HANDLE( IFence, executeCommandListsDesc.Signal ) );
        if ( fence )
        {
            fence->PrepareForSignal( );
        }
    }
    if ( !commandBuffers.empty( ) )
    {
        wgpuQueueSubmit( m_context->Queue, commandBuffers.size( ), commandBuffers.data( ) );
        if ( fence )
        {
#if DZ_WEBGPU_USE_DAWN_API
            WGPUQueueWorkDoneCallbackInfo workDoneInfo = { };
            workDoneInfo.callback                      = WebGPUFence::OnWorkDone;
            workDoneInfo.userdata1                     = fence;
#if defined( __EMSCRIPTEN__ )
            workDoneInfo.mode = WGPUCallbackMode_AllowSpontaneous;
#else
            workDoneInfo.mode = WGPUCallbackMode_AllowProcessEvents;
#endif
            wgpuQueueOnSubmittedWorkDone( m_context->Queue, workDoneInfo );
#else
            wgpuQueueOnSubmittedWorkDone( m_context->Queue, WebGPUFence::OnWorkDone, fence );
#endif
        }
    }
}

WGPUQueue WebGPUCommandQueue::GetNativeQueue( ) const
{
    return m_context->Queue;
}

DenOfIz_QueueType WebGPUCommandQueue::GetQueueType( ) const
{
    return m_desc.QueueType;
}
