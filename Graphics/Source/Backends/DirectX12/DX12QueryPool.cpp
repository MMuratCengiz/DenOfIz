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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12QueryPool.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12QueryPool::DX12QueryPool( DX12Context *context, const DenOfIz_QueryPoolDesc &desc ) : m_context( context ), m_desc( desc ), m_timestampFrequency( 0 )
{
    DX_CHECK_RESULT( m_context->GraphicsCommandQueue->GetTimestampFrequency( &m_timestampFrequency ) );

    switch ( m_desc.Type )
    {
    case DENOFIZ_QUERY_TYPE_OCCLUSION:
        m_d3d12QueryType = D3D12_QUERY_TYPE_OCCLUSION;
        break;
    case DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS:
        m_d3d12QueryType = D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
        break;
    case DENOFIZ_QUERY_TYPE_TIMESTAMP:
        m_d3d12QueryType = D3D12_QUERY_TYPE_TIMESTAMP;
        break;
    default:
        m_d3d12QueryType = D3D12_QUERY_TYPE_OCCLUSION;
        break;
    }

    D3D12_QUERY_HEAP_DESC heapDesc{ };
    D3D12_QUERY_HEAP_TYPE heapType;
    switch ( m_d3d12QueryType )
    {
    case D3D12_QUERY_TYPE_OCCLUSION:
        heapType = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
        break;
    case D3D12_QUERY_TYPE_PIPELINE_STATISTICS:
        heapType = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
        break;
    case D3D12_QUERY_TYPE_TIMESTAMP:
        heapType = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        break;
    default:
        heapType = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
        break;
    }

    heapDesc.Type     = heapType;
    heapDesc.Count    = m_desc.NumQueries;
    heapDesc.NodeMask = 0;

    DX_CHECK_RESULT( m_context->D3DDevice->CreateQueryHeap( &heapDesc, IID_PPV_ARGS( m_queryHeap.put( ) ) ) );

    size_t resultSize = sizeof( uint64_t );
    if ( m_desc.Type == DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS )
    {
        resultSize = sizeof( D3D12_QUERY_DATA_PIPELINE_STATISTICS );
    }

    const CD3DX12_HEAP_PROPERTIES readbackHeapProps( D3D12_HEAP_TYPE_READBACK );
    const CD3DX12_RESOURCE_DESC   bufferDesc = CD3DX12_RESOURCE_DESC::Buffer( resultSize * m_desc.NumQueries );

    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommittedResource( &readbackHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                                                    IID_PPV_ARGS( m_readbackBuffer.put( ) ) ) );
}

DenOfIz_QueryType DX12QueryPool::GetType( ) const
{
    return m_desc.Type;
}

uint32_t DX12QueryPool::GetNumQueries( ) const
{
    return m_desc.NumQueries;
}

DenOfIz_QueryData DX12QueryPool::GetQueryData( uint32_t queryIndex )
{
    DenOfIz_QueryData result{ };
    result.Valid = true;

    const uint32_t queryCount = ( m_desc.Type == DENOFIZ_QUERY_TYPE_TIMESTAMP ? 2 : 1 );
    size_t         stride     = sizeof( uint64_t );
    if ( m_desc.Type == DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS )
    {
        stride = sizeof( D3D12_QUERY_DATA_PIPELINE_STATISTICS );
    }

    const CD3DX12_RANGE readRange( queryIndex * queryCount * stride, ( queryIndex + 1 ) * queryCount * stride );
    void               *mappedData = nullptr;
    HRESULT             hr         = m_readbackBuffer->Map( 0, &readRange, &mappedData );

    if ( FAILED( hr ) )
    {
        spdlog::error( "DX12QueryPool::GetQueryData: Failed to map readback buffer" );
        result.Valid = false;
        return result;
    }

    const auto queries = reinterpret_cast<uint64_t *>( static_cast<uint8_t *>( mappedData ) );
    switch ( m_desc.Type )
    {
    case DENOFIZ_QUERY_TYPE_TIMESTAMP:
        {
            result.BeginTimestamp = queries[ 0 ];
            result.EndTimestamp   = queries[ 1 ];
            break;
        }
    case DENOFIZ_QUERY_TYPE_OCCLUSION:
        {
            result.OcclusionCounts = queries[ 0 ];
            break;
        }
    case DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS:
        {
            const auto *dx12Stats                          = reinterpret_cast<D3D12_QUERY_DATA_PIPELINE_STATISTICS *>( queries );
            result.PipelineStats.InputAssemblyVertices     = dx12Stats->IAVertices;
            result.PipelineStats.InputAssemblyPrimitives   = dx12Stats->IAPrimitives;
            result.PipelineStats.VertexShaderInvocations   = dx12Stats->VSInvocations;
            result.PipelineStats.GeometryShaderInvocations = dx12Stats->GSInvocations;
            result.PipelineStats.GeometryShaderPrimitives  = dx12Stats->GSPrimitives;
            result.PipelineStats.ClippingInvocations       = dx12Stats->CInvocations;
            result.PipelineStats.ClippingPrimitives        = dx12Stats->CPrimitives;
            result.PipelineStats.PixelShaderInvocations    = dx12Stats->PSInvocations;
            result.PipelineStats.HullShaderInvocations     = dx12Stats->HSInvocations;
            result.PipelineStats.DomainShaderInvocations   = dx12Stats->DSInvocations;
            result.PipelineStats.ComputeShaderInvocations  = dx12Stats->CSInvocations;
            break;
        }
    default:
        result.Valid = false;
        break;
    }

    const CD3DX12_RANGE writeRange( 0, 0 );
    m_readbackBuffer->Unmap( 0, &writeRange );

    return result;
}

double DX12QueryPool::GetTimestampFrequency( )
{
    return static_cast<double>( m_timestampFrequency );
}

ID3D12QueryHeap *DX12QueryPool::GetQueryHeap( ) const
{
    return m_queryHeap.get( );
}

D3D12_QUERY_TYPE DX12QueryPool::GetD3D12QueryType( ) const
{
    return m_d3d12QueryType;
}

ID3D12Resource *DX12QueryPool::GetReadbackBuffer( ) const
{
    return m_readbackBuffer.get( );
}
