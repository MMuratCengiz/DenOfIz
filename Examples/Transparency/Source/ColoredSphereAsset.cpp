/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include <DenOfIzExamples/ColoredSphereAsset.h>

using namespace DenOfIz;

ColoredSphereAsset::ColoredSphereAsset( DenOfIz_LogicalDevice device, DenOfIz_BatchResourceCopy batchResourceCopy, const XMFLOAT4 color ) : m_color( color )
{
    DenOfIz_SphereDesc sphereDesc{ };
    sphereDesc.BuildDesc    = 0;
    sphereDesc.Diameter     = 1.0f;
    sphereDesc.Tessellation = 64;

    DenOfIz_GeometryData sphere = DENOFIZ_NULL_HANDLE;
    DenOfIz_Geometry_BuildSphere( &sphereDesc, &sphere );

    AssetDataDesc assetDataDesc{ };
    assetDataDesc.Device       = device;
    assetDataDesc.BatchCopy    = batchResourceCopy;
    assetDataDesc.GeometryData = sphere;
    m_assetData                = new AssetData( assetDataDesc );

    DenOfIz_GeometryData_Destroy( sphere );

    XMStoreFloat4x4( &m_modelMatrix, XMMatrixIdentity( ) );
}

ColoredSphereAsset::~ColoredSphereAsset( )
{
    delete m_assetData;
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
    return m_assetData;
}

XMFLOAT4X4 ColoredSphereAsset::ModelMatrix( ) const
{
    return m_modelMatrix;
}
