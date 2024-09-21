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

#include <DenOfIzExamples/SphereAsset.h>
#include <DenOfIzGraphics/Data/Geometry.h>

using namespace DenOfIz;

SphereAsset::SphereAsset( ILogicalDevice *device, BatchResourceCopy *batchResourceCopy )
{
    m_materialData              = std::make_unique<MaterialData>( );
    std::string baseTexturePath = "Assets/Textures/Bricks_005/Stylized_Bricks_005_";

    m_materialData->AttachSampler( device->CreateSampler( SamplerDesc{ } ) );
    m_materialData->AttachAlbedoData( batchResourceCopy->CreateAndLoadTexture( baseTexturePath + "basecolor.png" ) );
    m_materialData->AttachNormalData( batchResourceCopy->CreateAndLoadTexture( baseTexturePath + "normal.png" ) );
    m_materialData->AttachHeightData( batchResourceCopy->CreateAndLoadTexture( baseTexturePath + "height.png" ) );
    m_materialData->AttachRoughnessData( batchResourceCopy->CreateAndLoadTexture( baseTexturePath + "roughness.png" ) );
    m_materialData->AttachAoData( batchResourceCopy->CreateAndLoadTexture( baseTexturePath + "ambientOcclusion.png" ) );

    std::unique_ptr<IBufferResource> vertexBuffer;
    std::unique_ptr<IBufferResource> indexBuffer;

    GeometryData sphere = Geometry::BuildSphere( { .Diameter = 1.0f, .Tessellation = 64 } );

    m_assetData = batchResourceCopy->CreateGeometryAssetData( sphere );
    m_assetData->UpdateMaterialData( m_materialData.get( ) );

    XMStoreFloat4x4( &m_modelMatrix, XMMatrixIdentity( ) );
}

AssetData *SphereAsset::Data( ) const
{
    return m_assetData.get( );
}

XMFLOAT4X4 SphereAsset::ModelMatrix( ) const
{
    return m_modelMatrix;
}
