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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUQueryPool.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

WebGPUQueryPool::WebGPUQueryPool( WebGPUContext *context, const DenOfIz_QueryPoolDesc &desc ) :
    m_context( context ), m_desc( desc ), m_querySet( nullptr ), m_readbackBuffer( nullptr )
{
    // Following The Forge's approach: Timestamp queries use 2 slots per query
    uint32_t actualQueryCount = m_desc.NumQueries;
    if ( m_desc.Type == DENOFIZ_QUERY_TYPE_TIMESTAMP )
    {
        actualQueryCount = m_desc.NumQueries * 2;
    }

    WGPUQuerySetDescriptor querySetDesc{ };
    querySetDesc.label = DZ_WEBGPU_STRING( "DenOfIz Query Set" );
    querySetDesc.count = actualQueryCount;

    switch ( m_desc.Type )
    {
    case DENOFIZ_QUERY_TYPE_OCCLUSION:
        querySetDesc.type = WGPUQueryType_Occlusion;
        break;
    case DENOFIZ_QUERY_TYPE_TIMESTAMP:
        querySetDesc.type = WGPUQueryType_Timestamp;
        break;
    case DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS:
        spdlog::error( "WebGPUQueryPool: Pipeline statistics queries are not supported in WebGPU" );
        return;
    default:
        querySetDesc.type = WGPUQueryType_Occlusion;
        break;
    }

    m_querySet = wgpuDeviceCreateQuerySet( m_context->Device, &querySetDesc );

    WGPUBufferDescriptor bufferDesc{ };
    bufferDesc.label            = DZ_WEBGPU_STRING( "Query Readback Buffer" );
    bufferDesc.size             = sizeof( uint64_t ) * actualQueryCount;
    bufferDesc.usage            = WGPUBufferUsage_QueryResolve | WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst;
    bufferDesc.mappedAtCreation = false;

    m_readbackBuffer = wgpuDeviceCreateBuffer( m_context->Device, &bufferDesc );
}

WebGPUQueryPool::~WebGPUQueryPool( )
{
    if ( m_querySet )
    {
        wgpuQuerySetRelease( m_querySet );
    }
    if ( m_readbackBuffer )
    {
        wgpuBufferRelease( m_readbackBuffer );
    }
}

DenOfIz_QueryType WebGPUQueryPool::GetType( ) const
{
    return m_desc.Type;
}

uint32_t WebGPUQueryPool::GetNumQueries( ) const
{
    return m_desc.NumQueries;
}

DenOfIz_QueryData WebGPUQueryPool::GetQueryData( uint32_t queryIndex )
{
    DenOfIz_QueryData result{ };

    // Note: WebGPU has limitations for synchronous query reading
    // Query results must be resolved to a buffer and read asynchronously
    // This implementation provides a synchronous interface by warning users
    // In a production system, this would require proper async handling

    switch ( m_desc.Type )
    {
    case DENOFIZ_QUERY_TYPE_OCCLUSION:
        {
            // Following The Forge's approach: WebGPU requires async buffer mapping for query results
            // For now, we return an invalid result and log a warning
            spdlog::warn( "WebGPUQueryPool::GetQueryData: Occlusion query results require async buffer mapping in WebGPU" );
            result.Valid = false;
            break;
        }
    case DENOFIZ_QUERY_TYPE_TIMESTAMP:
        {
            // Following The Forge's approach: WebGPU requires async buffer mapping for query results
            // Timestamp queries would use 2 slots per query
            spdlog::warn( "WebGPUQueryPool::GetQueryData: Timestamp query results require async buffer mapping in WebGPU" );
            result.Valid = false;
            break;
        }
    case DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS:
        {
            spdlog::error( "WebGPUQueryPool::GetQueryData: Pipeline statistics queries are not supported in WebGPU" );
            result.Valid = false;
            break;
        }
    }

    return result;
}

double WebGPUQueryPool::GetTimestampFrequency( )
{
    return 1000000000.0;
}
