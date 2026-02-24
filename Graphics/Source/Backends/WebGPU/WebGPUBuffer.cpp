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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUContext.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUEnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUShadowState.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/StringUtilities.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using namespace DenOfIz;

WebGPUBuffer::WebGPUBuffer( WebGPUContext *context, const DenOfIz_BufferDesc &desc ) : m_context( context ), m_desc( desc ), m_debugName( StringViewToStdString( desc.DebugName ) )
{
    if ( !m_debugName.empty( ) )
    {
        m_desc.DebugName = DenOfIz_StringView( m_debugName.c_str( ), static_cast<uint32_t>( m_debugName.size( ) ) );
    }
    else
    {
        m_desc.DebugName = { };
    }

    m_useShadowBuffer = desc.HeapType == DENOFIZ_HEAP_TYPE_CPU_GPU;

    if ( m_useShadowBuffer )
    {
        m_shadowBuffer.resize( desc.NumBytes );
        m_mappedData = m_shadowBuffer.data( );
    }

    uint32_t wgpuUsage = 0;

    if ( desc.HeapType == DENOFIZ_HEAP_TYPE_CPU )
    {
        wgpuUsage = WGPUBufferUsage_MapWrite | WGPUBufferUsage_CopySrc;
    }
    else
    {
        if ( desc.Usage & DENOFIZ_BUFFER_USAGE_COPY_SRC_BIT )
        {
            wgpuUsage |= WGPUBufferUsage_CopySrc;
        }
        if ( desc.Usage & DENOFIZ_BUFFER_USAGE_COPY_DST_BIT )
        {
            wgpuUsage |= WGPUBufferUsage_CopyDst;
        }
        if ( desc.Usage & DENOFIZ_BUFFER_USAGE_INDEX_BIT )
        {
            wgpuUsage |= WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
        }
        if ( desc.Usage & DENOFIZ_BUFFER_USAGE_VERTEX_BIT )
        {
            wgpuUsage |= WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        }
        if ( desc.Usage & DENOFIZ_BUFFER_USAGE_UNIFORM_BIT )
        {
            wgpuUsage |= WGPUBufferUsage_Uniform;
        }
        if ( ( desc.Usage & DENOFIZ_BUFFER_USAGE_STORAGE_BIT ) || desc.StructureDesc.Stride > 0 )
        {
            wgpuUsage |= WGPUBufferUsage_Storage;
        }
        if ( desc.Usage & DENOFIZ_BUFFER_USAGE_INDIRECT_BIT )
        {
            wgpuUsage |= WGPUBufferUsage_Indirect;
        }
        if ( desc.HeapType == DENOFIZ_HEAP_TYPE_GPU_CPU )
        {
            wgpuUsage |= WGPUBufferUsage_MapRead | WGPUBufferUsage_CopySrc;
        }
        if ( desc.HeapType == DENOFIZ_HEAP_TYPE_CPU_GPU )
        {
            wgpuUsage |= WGPUBufferUsage_CopyDst;
        }
    }

    WGPUBufferDescriptor bufferDesc = { };
    bufferDesc.size                 = desc.NumBytes;
    bufferDesc.mappedAtCreation     = desc.HeapType == DENOFIZ_HEAP_TYPE_CPU && !m_useShadowBuffer;
    bufferDesc.usage                = static_cast<WGPUBufferUsage>( wgpuUsage );
#if DZ_WEBGPU_USE_DAWN_API
    bufferDesc.label                = { m_debugName.empty( ) ? nullptr : m_debugName.c_str( ), m_debugName.size( ) };
#else
    bufferDesc.label                = m_debugName.empty( ) ? nullptr : m_debugName.c_str( );
#endif

    m_buffer = wgpuDeviceCreateBuffer( m_context->Device, &bufferDesc );
    if ( !m_buffer )
    {
        spdlog::critical( "WebGPU: Failed to create buffer" );
        return;
    }

    if ( bufferDesc.mappedAtCreation && !m_useShadowBuffer )
    {
        m_mappedData = wgpuBufferGetMappedRange( m_buffer, 0, desc.NumBytes );
        m_isMapped   = true;
    }
}

WebGPUBuffer::~WebGPUBuffer( )
{
    if ( m_buffer )
    {
        g_webGPUShadowState.RemoveBuffer( m_buffer );
        wgpuBufferRelease( m_buffer );
    }
}

void *WebGPUBuffer::MapMemory( )
{
    if ( m_useShadowBuffer )
    {
        m_isMapped = true;
        return m_shadowBuffer.data( );
    }

    if ( g_webGPUShadowState.IsBufferMapped( m_buffer ) )
    {
        const auto state = g_webGPUShadowState.GetBufferState( m_buffer );
        m_mappedData     = state.MappedData;
        m_isMapped       = true;
        return m_mappedData;
    }

    if ( m_isMapped )
    {
        return m_mappedData;
    }

    if ( m_desc.HeapType != DENOFIZ_HEAP_TYPE_CPU && m_desc.HeapType != DENOFIZ_HEAP_TYPE_CPU_GPU && m_desc.HeapType != DENOFIZ_HEAP_TYPE_GPU_CPU )
    {
        spdlog::error( "WebGPU: Cannot map GPU-only buffer" );
        return nullptr;
    }

    WGPUMapMode mapMode = static_cast<WGPUMapMode>( 0 );
    if ( m_desc.HeapType == DENOFIZ_HEAP_TYPE_CPU || m_desc.HeapType == DENOFIZ_HEAP_TYPE_CPU_GPU )
    {
        mapMode = WGPUMapMode_Write;
    }
    else
    {
        mapMode = WGPUMapMode_Read;
    }

#if DZ_WEBGPU_USE_DAWN_API
    struct MapData
    {
        bool               done   = false;
        WGPUMapAsyncStatus status = WGPUMapAsyncStatus_Success;
    } mapData;

    auto mapCallback = []( WGPUMapAsyncStatus status, WGPUStringView message, void *userdata1, void *userdata2 )
    {
        const auto data = static_cast<MapData *>( userdata1 );
        data->status    = status;
        data->done      = true;
    };

    WGPUBufferMapCallbackInfo callbackInfo = { };
#if defined( __EMSCRIPTEN__ )
    callbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
#else
    callbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
#endif
    callbackInfo.callback  = mapCallback;
    callbackInfo.userdata1 = &mapData;
    callbackInfo.userdata2 = nullptr;

    WGPUFuture mapFuture = wgpuBufferMapAsync( m_buffer, mapMode, 0, m_desc.NumBytes, callbackInfo );

#if defined( __EMSCRIPTEN__ )
    WGPUFutureWaitInfo waitInfo = { };
    waitInfo.future             = mapFuture;
    wgpuInstanceWaitAny( m_context->Instance, 1, &waitInfo, UINT64_MAX );
#else
    while ( !mapData.done )
    {
    }
#endif

    if ( mapData.status != WGPUMapAsyncStatus_Success )
    {
        spdlog::error( "WebGPU: Failed to map buffer" );
        return nullptr;
    }
#else
    struct MapData
    {
        bool                     done   = false;
        WGPUBufferMapAsyncStatus status = WGPUBufferMapAsyncStatus_Success;
    } mapData;

    auto mapCallback = []( WGPUBufferMapAsyncStatus status, void *userdata )
    {
        const auto data = static_cast<MapData *>( userdata );
        data->status    = status;
        data->done      = true;
    };

    wgpuBufferMapAsync( m_buffer, mapMode, 0, m_desc.NumBytes, mapCallback, &mapData );

    while ( !mapData.done )
    {
        emscripten_sleep( 1 );
    }

    if ( mapData.status != WGPUBufferMapAsyncStatus_Success )
    {
        spdlog::error( "WebGPU: Failed to map buffer" );
        return nullptr;
    }
#endif

    m_mappedData = wgpuBufferGetMappedRange( m_buffer, 0, m_desc.NumBytes );
    m_isMapped   = true;

    g_webGPUShadowState.SetBufferMapped( m_buffer, m_mappedData, m_desc.NumBytes, mapMode );
    return m_mappedData;
}

void WebGPUBuffer::UnmapMemory( )
{
    if ( m_useShadowBuffer )
    {
        m_isMapped = false;
        return;
    }

    if ( m_isMapped )
    {
        wgpuBufferUnmap( m_buffer );
        m_mappedData = nullptr;
        m_isMapped   = false;

        g_webGPUShadowState.SetBufferUnmapped( m_buffer );
    }
}

size_t WebGPUBuffer::NumBytes( ) const
{
    return m_desc.NumBytes;
}

const void *WebGPUBuffer::Data( ) const
{
    if ( m_useShadowBuffer )
    {
        return m_shadowBuffer.data( );
    }
    return m_mappedData;
}

DenOfIz_ByteArray WebGPUBuffer::GetData( ) const
{
    const void *dataPtr = nullptr;
    if ( m_useShadowBuffer )
    {
        dataPtr = m_shadowBuffer.data( );
    }
    else
    {
        dataPtr = m_mappedData;
    }

    if ( !dataPtr )
    {
        spdlog::error( "WebGPU: Buffer not mapped, cannot get data" );
        return { nullptr, 0 };
    }

    DenOfIz_ByteArray result{ };
    result.Elements    = new Byte[ m_desc.NumBytes ];
    result.NumElements = m_desc.NumBytes;
    std::memcpy( result.Elements, dataPtr, m_desc.NumBytes );
    return result;
}

void WebGPUBuffer::SetData( const DenOfIz_ByteArrayView &data, const bool keepMapped )
{
    void *mappedData = MapMemory( );
    if ( !mappedData )
    {
        return;
    }

    std::memcpy( mappedData, data.Elements, std::min( data.NumElements, m_desc.NumBytes ) );

    if ( !keepMapped )
    {
        UnmapMemory( );
    }
}

void WebGPUBuffer::WriteData( const DenOfIz_ByteArrayView &data, const uint32_t bufferOffset )
{
    if ( bufferOffset + data.NumElements > m_desc.NumBytes )
    {
        spdlog::error( "WebGPU: Write exceeds buffer size" );
        return;
    }

    wgpuQueueWriteBuffer( m_context->Queue, m_buffer, bufferOffset, data.Elements, data.NumElements );
}

WGPUBuffer WebGPUBuffer::GetBuffer( ) const
{
    return m_buffer;
}

size_t WebGPUBuffer::GetNumBytes( ) const
{
    return m_desc.NumBytes;
}

void WebGPUBuffer::SyncShadowBuffer( ) const
{
    if ( m_useShadowBuffer )
    {
        wgpuQueueWriteBuffer( m_context->Queue, m_buffer, 0, m_shadowBuffer.data( ), m_shadowBuffer.size( ) );
    }
}
