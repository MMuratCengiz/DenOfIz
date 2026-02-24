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

#import "DenOfIzGraphicsInternal/Backends/Metal/MetalQueryPool.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MetalQueryPool::MetalQueryPool( MetalContext *context, const DenOfIz_QueryPoolDesc &desc ) :
    m_context( context ), m_desc( desc ), m_counterSampleBuffer( nil ), m_visibilityResultBuffer( nil )
{
    switch( m_desc.Type ) 
    {
    case DENOFIZ_QUERY_TYPE_OCCLUSION:
        {
            NSUInteger bufferSize = sizeof( uint64_t ) * m_desc.NumQueries;
            m_visibilityResultBuffer = [m_context->Device newBufferWithLength:bufferSize options:MTLResourceStorageModeShared];
            m_visibilityResultBuffer.label = @"Occlusion Query Buffer";
        }
        break;

    case DENOFIZ_QUERY_TYPE_TIMESTAMP:
        {
            if( m_context->Device.counterSets.count > 0 )
            {
                MTLCounterSampleBufferDescriptor *desc = [MTLCounterSampleBufferDescriptor new];
                desc.counterSet = m_context->Device.counterSets[0];
                desc.sampleCount = m_desc.NumQueries * 2;
                desc.storageMode = MTLStorageModeShared;
                desc.label = @"Timestamp Query Buffer";
                
                NSError *error = nil;
                m_counterSampleBuffer = [m_context->Device newCounterSampleBufferWithDescriptor:desc error:&error];
                
                if( error || !m_counterSampleBuffer ) 
                {
                    spdlog::error( "MetalQueryPool: Failed to create counter sample buffer for timestamps" );
                }
            }
            else 
            {
                spdlog::error( "MetalQueryPool: No counter sets available for timestamp queries" );
            }
        }
        break;

    case DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS:
        spdlog::error( "MetalQueryPool: Pipeline statistics queries are not directly supported in Metal" );
        break;

    default:
        spdlog::error( "MetalQueryPool: Unknown query type" );
        break;
    }
}

MetalQueryPool::~MetalQueryPool( ) = default;

DenOfIz_QueryType MetalQueryPool::GetType( ) const
{
    return m_desc.Type;
}

uint32_t MetalQueryPool::GetNumQueries( ) const
{
    return m_desc.NumQueries;
}

DenOfIz_QueryData MetalQueryPool::GetQueryData( uint32_t queryIndex )
{
    DenOfIz_QueryData result{ };

    switch ( m_desc.Type )
    {
    case DENOFIZ_QUERY_TYPE_OCCLUSION:
        {
            if( !m_visibilityResultBuffer ) 
            {
                spdlog::error( "MetalQueryPool::GetQueryData: No visibility result buffer" );
                break;
            }

            uint64_t *results = static_cast<uint64_t*>( [m_visibilityResultBuffer contents] );
            result.Valid = true;
            result.OcclusionCounts = results[queryIndex];
            break;
        }
    case DENOFIZ_QUERY_TYPE_TIMESTAMP:
        {
            if( !m_counterSampleBuffer ) 
            {
                spdlog::error( "MetalQueryPool::GetQueryData: No counter sample buffer" );
                break;
            }
            NSUInteger actualIndex = queryIndex * 2;
            NSData *data = [m_counterSampleBuffer resolveCounterRange:NSMakeRange( actualIndex, 2 )];
            if( data && data.length >= 2 * sizeof( uint64_t ) ) 
            {
                const uint64_t *timestamps = static_cast<const uint64_t*>( [data bytes] );
                result.Valid = true;
                result.BeginTimestamp = timestamps[0];
                result.EndTimestamp = timestamps[1];
            }
            break;
        }
    case DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS:
        {
            spdlog::error( "MetalQueryPool::GetQueryData: Pipeline statistics queries are not supported in Metal" );
            break;
        }
    }

    return result;
}

void MetalQueryPool::WriteTimestamp( uint32_t queryIndex )
{
}

double MetalQueryPool::GetTimestampFrequency( )
{
    return 1000000000.0;
}


id<MTLCounterSampleBuffer> MetalQueryPool::GetCounterSampleBuffer( ) const
{
    return m_counterSampleBuffer;
}

id<MTLBuffer> MetalQueryPool::GetVisibilityResultBuffer( ) const
{
    return m_visibilityResultBuffer;
}