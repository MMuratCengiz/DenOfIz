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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12PipelineCache.h"
#include <cstring>
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12PipelineCache::DX12PipelineCache( DX12Context *context, const DenOfIz_PipelineCacheDesc &desc ) : m_context( context )
{
    DZ_NOT_NULL( context );

    ID3D12Device1 *device1 = nullptr;
    HRESULT        hr      = m_context->D3DDevice->QueryInterface( IID_PPV_ARGS( &device1 ) );
    if ( FAILED( hr ) )
    {
        spdlog::error( "Failed to get ID3D12Device1 interface for pipeline library support" );
        return;
    }

    if ( desc.Data != nullptr && desc.DataSize > 0 )
    {
        hr = device1->CreatePipelineLibrary( desc.Data, desc.DataSize, IID_PPV_ARGS( &m_pipelineLibrary ) );
        if ( FAILED( hr ) )
        {
            spdlog::warn( "Failed to create pipeline library from cache data, creating empty library" );
            hr = device1->CreatePipelineLibrary( nullptr, 0, IID_PPV_ARGS( &m_pipelineLibrary ) );
        }
    }
    else
    {
        hr = device1->CreatePipelineLibrary( nullptr, 0, IID_PPV_ARGS( &m_pipelineLibrary ) );
    }

    device1->Release( );

    if ( FAILED( hr ) )
    {
        spdlog::error( "Failed to create pipeline library" );
    }
}

DX12PipelineCache::~DX12PipelineCache( )
{
    m_pipelineLibrary.reset( );
}

size_t DX12PipelineCache::GetDataNumBytes( )
{
    if ( !m_pipelineLibrary )
    {
        return 0;
    }

    return m_pipelineLibrary->GetSerializedSize( );
}

bool DX12PipelineCache::GetData( DenOfIz_ByteArray &data )
{
    if ( !m_pipelineLibrary || !data.Elements )
    {
        return false;
    }

    const SIZE_T size = m_pipelineLibrary->GetSerializedSize( );
    if ( size == 0 || data.NumElements < size )
    {
        return false;
    }

    const HRESULT hr = m_pipelineLibrary->Serialize( data.Elements, size );
    if ( FAILED( hr ) )
    {
        spdlog::error( "Failed to serialize pipeline library" );
        return false;
    }

    data.NumElements = size;
    return true;
}

ID3D12PipelineLibrary *DX12PipelineCache::GetLibrary( ) const
{
    return m_pipelineLibrary.get( );
}
