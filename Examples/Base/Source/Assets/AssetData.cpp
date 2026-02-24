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

#include "DenOfIzExamples/Assets/AssetData.h"
#include <vector>

using namespace DenOfIz;

AssetData::AssetData( const AssetDataDesc &desc ) : m_materialData( nullptr )
{
    uint32_t vertexCount = 0;
    uint32_t indexCount  = 0;
    DenOfIz_GeometryData_GetVertexCount( desc.GeometryData, &vertexCount );
    DenOfIz_GeometryData_GetIndexCount( desc.GeometryData, &indexCount );

    m_vertexBuffer = DenOfIz_BatchResourceCopy_CreateGeometryVertexBuffer( desc.BatchCopy, &desc.GeometryData );
    m_indexBuffer  = DenOfIz_BatchResourceCopy_CreateGeometryIndexBuffer( desc.BatchCopy, &desc.GeometryData );
    m_numVertices  = vertexCount;
    m_numIndices   = indexCount;
}

AssetData::~AssetData( )
{
    if ( DENOFIZ_HANDLE_IS_VALID( m_vertexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_vertexBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_indexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_indexBuffer );
    }
}

void AssetData::UpdateMaterialData( MaterialData *materialData )
{
    m_materialData = materialData;
}

DenOfIz_Buffer AssetData::VertexBuffer( ) const
{
    return m_vertexBuffer;
}

DenOfIz_Buffer AssetData::IndexBuffer( ) const
{
    return m_indexBuffer;
}

MaterialData *AssetData::Material( ) const
{
    return m_materialData;
}

uint32_t AssetData::NumVertices( ) const
{
    return m_numVertices;
}
uint32_t AssetData::NumIndices( ) const
{
    return m_numIndices;
}
