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

#include "DenOfIzGraphics/Assets/Serde/Physics/PhysicsAssetReader.h"
#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetReaderHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"

using namespace DenOfIz;

PhysicsAssetReader::PhysicsAssetReader( const PhysicsAssetReaderDesc &desc ) : m_reader( desc.Reader )
{
    if ( !m_reader )
    {
        spdlog::critical( "BinaryReader cannot be null for PhysicsAssetReader" );
    }
}

PhysicsAssetReader::~PhysicsAssetReader( ) = default;

PhysicsAsset* PhysicsAssetReader::Read( )
{
    m_physicsAsset       = new PhysicsAsset( );
    m_physicsAsset->Magic = m_reader->ReadUInt64( );
    if ( m_physicsAsset->Magic != PhysicsAsset{ }.Magic )
    {
        spdlog::critical( "Invalid PhysicsAsset magic number." );
    }

    m_physicsAsset->Version = m_reader->ReadUInt32( );
    if ( m_physicsAsset->Version > PhysicsAsset::Latest )
    {
        spdlog::warn( "PhysicsAsset version mismatch." );
    }

    m_physicsAsset->NumBytes = m_reader->ReadUInt64( );
    m_physicsAsset->Uri      = AssetUri::Parse( m_reader->ReadString( ) );
    m_physicsAsset->Name     = m_reader->ReadString( );

    const uint32_t numColliders = m_reader->ReadUInt32( );
    DZArenaArrayHelper<PhysicsColliderArray, PhysicsCollider>::AllocateAndConstructArray( m_physicsAsset->_Arena, m_physicsAsset->Colliders, numColliders );


    for ( uint32_t i = 0; i < numColliders; ++i )
    {
        PhysicsCollider &collider = m_physicsAsset->Colliders.Elements[ i ];

        collider.Type        = static_cast<PhysicsColliderType>( m_reader->ReadUInt32( ) );
        collider.Name        = m_reader->ReadString( );
        collider.Transform   = m_reader->ReadFloat_4x4( );
        collider.Friction    = m_reader->ReadFloat( );
        collider.Restitution = m_reader->ReadFloat( );
        collider.IsTrigger   = m_reader->ReadByte( ) != 0;

        switch ( collider.Type )
        {
        case PhysicsColliderType::Box:
            collider.Box.HalfExtents = m_reader->ReadFloat_3( );
            break;

        case PhysicsColliderType::Sphere:
            collider.Sphere.Radius = m_reader->ReadFloat( );
            break;

        case PhysicsColliderType::Capsule:
            collider.Capsule.Radius = m_reader->ReadFloat( );
            collider.Capsule.Height = m_reader->ReadFloat( );
            break;

        case PhysicsColliderType::ConvexHull:
        case PhysicsColliderType::TriangleMesh:
            collider.Mesh.VertexStream = AssetReaderHelpers::ReadAssetDataStream( m_reader );
            collider.Mesh.IndexStream  = AssetReaderHelpers::ReadAssetDataStream( m_reader );
            break;
        }
    }

    m_physicsAsset->UserProperties = AssetReaderHelpers::ReadUserProperties( &m_physicsAsset->_Arena, m_reader );
    return m_physicsAsset;
}
