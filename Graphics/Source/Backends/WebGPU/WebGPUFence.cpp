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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUFence.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#if defined( __EMSCRIPTEN__ )
#include <emscripten.h>
#elif defined( WEBGPU_BACKEND_WGPU )
#include <webgpu/wgpu.h>
#endif

using namespace DenOfIz;

WebGPUFence::WebGPUFence( WebGPUContext *context ) : m_context( context )
{
}

WebGPUFence::~WebGPUFence( ) = default;

void WebGPUFence::Wait( )
{
    if ( m_signaled.load( ) )
    {
        return;
    }

#if defined( __EMSCRIPTEN__ )
    while ( !m_signaled.load( ) )
    {
        emscripten_sleep( 10 );
    }
#elif defined( WEBGPU_BACKEND_WGPU )
    while ( !m_signaled.load( ) )
    {
        wgpuDevicePoll( m_context->Device, true, nullptr );
    }
#else
    std::unique_lock lock( m_mutex );
    while ( !m_signaled.load( ) )
    {
        if ( m_cv.wait_for( lock, std::chrono::milliseconds( 1 ) ) == std::cv_status::timeout )
        {
            lock.unlock( );
            wgpuInstanceProcessEvents( m_context->Instance );
            lock.lock( );
        }
    }
#endif
}

void WebGPUFence::Reset( )
{
#ifndef __EMSCRIPTEN__
    std::lock_guard lock( m_mutex );
#endif
    m_signaled           = false;
    m_callbackRegistered = false;
}

void WebGPUFence::PrepareForSignal( )
{
#ifndef __EMSCRIPTEN__
    std::lock_guard lock( m_mutex );
#endif
    m_signaled           = false;
    m_callbackRegistered = true;
}

#if DZ_WEBGPU_USE_DAWN_API
void WebGPUFence::OnWorkDone( const WGPUQueueWorkDoneStatus status, void *userdata1, void *userdata2 )
{
    if ( status == WGPUQueueWorkDoneStatus_Success )
    {
        auto *fence = static_cast<WebGPUFence *>( userdata1 );
#ifndef __EMSCRIPTEN__
        {
            std::lock_guard lock( fence->m_mutex );
            fence->m_signaled = true;
        }
        fence->m_cv.notify_all( );
#else
        fence->m_signaled = true;
#endif
    }
}
#else
void WebGPUFence::OnWorkDone( const WGPUQueueWorkDoneStatus status, void *userdata )
{
    if ( status == WGPUQueueWorkDoneStatus_Success )
    {
        auto *fence       = static_cast<WebGPUFence *>( userdata );
        fence->m_signaled = true;
    }
}
#endif
