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

#include <DenOfIzExamples/ColoredSphereAsset.h>

using namespace DenOfIz;

ColoredSphereAsset::ColoredSphereAsset( ILogicalDevice *device, BatchResourceCopy *batchResourceCopy, const XMFLOAT4 color ) : m_color( color )
{
    const GeometryData sphere = Geometry::BuildSphere( { .Diameter = 1.0f, .Tessellation = 64 } );

    AssetDataDesc assetDataDesc{ };
    assetDataDesc.Device       = device;
    assetDataDesc.BatchCopy    = batchResourceCopy;
    assetDataDesc.GeometryData = sphere;
    m_assetData                = std::make_unique<AssetData>( assetDataDesc );

    XMStoreFloat4x4( &m_modelMatrix, XMMatrixIdentity( ) );
}

void ColoredSphereAsset::Translate( const XMFLOAT3 translation )
{
    const XMMATRIX translationMatrix = XMMatrixTranslation( translation.x, translation.y, translation.z );
    const XMMATRIX currentMatrix     = XMLoadFloat4x4( &m_modelMatrix );
    XMStoreFloat4x4( &m_modelMatrix, currentMatrix * translationMatrix );
}

void ColoredSphereAsset::Rotate( const XMFLOAT3 rotation )
{
    const XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw( rotation.x, rotation.y, rotation.z );
    const XMMATRIX currentMatrix  = XMLoadFloat4x4( &m_modelMatrix );
    XMStoreFloat4x4( &m_modelMatrix, currentMatrix * rotationMatrix );
}

void ColoredSphereAsset::Scale( const XMFLOAT3 scale )
{
    const XMMATRIX scaleMatrix   = XMMatrixScaling( scale.x, scale.y, scale.z );
    const XMMATRIX currentMatrix = XMLoadFloat4x4( &m_modelMatrix );
    XMStoreFloat4x4( &m_modelMatrix, currentMatrix * scaleMatrix );
}

void ColoredSphereAsset::SetTransform( const XMMATRIX &transform )
{
    XMStoreFloat4x4( &m_modelMatrix, transform );
}

XMFLOAT4 ColoredSphereAsset::GetColor( ) const
{
    return m_color;
}

AssetData *ColoredSphereAsset::Data( ) const
{
    return m_assetData.get( );
}

XMFLOAT4X4 ColoredSphereAsset::ModelMatrix( ) const
{
    return m_modelMatrix;
}
