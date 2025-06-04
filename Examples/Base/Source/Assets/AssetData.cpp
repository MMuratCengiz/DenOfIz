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

using namespace DenOfIz;

AssetData::AssetData( const AssetDataDesc &desc ) : m_materialData( nullptr )
{
    m_vertexBuffer = std::unique_ptr<IBufferResource>( desc.BatchCopy->CreateGeometryVertexBuffer( desc.GeometryData ) );
    m_indexBuffer  = std::unique_ptr<IBufferResource>( desc.BatchCopy->CreateGeometryIndexBuffer( desc.GeometryData ) );
    m_numVertices  = desc.GeometryData.Vertices.NumElements( );
    m_numIndices   = desc.GeometryData.Indices.NumElements( );
}

void AssetData::UpdateMaterialData( MaterialData *materialData )
{
    m_materialData = materialData;
}

IBufferResource *AssetData::VertexBuffer( ) const
{
    return m_vertexBuffer.get( );
}

IBufferResource *AssetData::IndexBuffer( ) const
{
    return m_indexBuffer.get( );
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
