/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
