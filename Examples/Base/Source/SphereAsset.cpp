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

#include "DenOfIzExamples/Assets/SphereAsset.h"
#include "DenOfIzGraphics/Data/Geometry.h"

using namespace DenOfIz;

SphereAsset::SphereAsset( ILogicalDevice *device, BatchResourceCopy *batchResourceCopy )
{
    const std::string baseTexturePath = "Assets/Textures/Bricks_005/Stylized_Bricks_005_";
    MaterialDesc      materialDesc{ };
    materialDesc.Device           = device;
    materialDesc.BatchCopy        = batchResourceCopy;
    materialDesc.AlbedoTexture    = ( baseTexturePath + "basecolor.png" ).c_str( );
    materialDesc.NormalTexture    = ( baseTexturePath + "normal.png" ).c_str( );
    materialDesc.HeightTexture    = ( baseTexturePath + "height.png" ).c_str( );
    materialDesc.RoughnessTexture = ( baseTexturePath + "roughness.png" ).c_str( );
    materialDesc.AoTexture        = ( baseTexturePath + "ambientOcclusion.png" ).c_str( );
    m_materialData                = std::make_unique<MaterialData>( materialDesc );

    const GeometryData sphere = Geometry::BuildSphere( { .Diameter = 1.0f, .Tessellation = 64 } );

    AssetDataDesc assetDataDesc{ };
    assetDataDesc.Device       = device;
    assetDataDesc.BatchCopy    = batchResourceCopy;
    assetDataDesc.GeometryData = sphere;
    m_assetData                = std::make_unique<AssetData>( assetDataDesc );
    m_assetData->UpdateMaterialData( m_materialData.get( ) );

    XMStoreFloat4x4( &m_modelMatrix, XMMatrixIdentity( ) );
}

void SphereAsset::Translate( XMFLOAT4 translation )
{
    XMMATRIX translationMatrix = XMMatrixTranslation( translation.x, translation.y, translation.z );
    XMMATRIX currentMatrix     = XMLoadFloat4x4( &m_modelMatrix );
    XMStoreFloat4x4( &m_modelMatrix, currentMatrix * translationMatrix );
}

void SphereAsset::Rotate( XMFLOAT4 rotation )
{
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw( rotation.x, rotation.y, rotation.z );
    XMMATRIX currentMatrix  = XMLoadFloat4x4( &m_modelMatrix );
    XMStoreFloat4x4( &m_modelMatrix, currentMatrix * rotationMatrix );
}

void SphereAsset::Scale( XMFLOAT4 scale )
{
    XMMATRIX scaleMatrix   = XMMatrixScaling( scale.x, scale.y, scale.z );
    XMMATRIX currentMatrix = XMLoadFloat4x4( &m_modelMatrix );
    XMStoreFloat4x4( &m_modelMatrix, currentMatrix * scaleMatrix );
}

AssetData *SphereAsset::Data( ) const
{
    return m_assetData.get( );
}

XMFLOAT4X4 SphereAsset::ModelMatrix( ) const
{
    return m_modelMatrix;
}
