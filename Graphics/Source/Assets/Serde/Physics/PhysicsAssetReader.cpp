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

#include <DenOfIzGraphics/Assets/Serde/Common/AssetReaderHelpers.h>
#include <DenOfIzGraphics/Assets/Serde/Physics/PhysicsAssetReader.h>

using namespace DenOfIz;

PhysicsAssetReader::PhysicsAssetReader( const PhysicsAssetReaderDesc &desc ) : m_reader( desc.Reader )
{
    if ( !m_reader )
    {
        LOG( FATAL ) << "BinaryReader cannot be null for PhysicsAssetReader";
    }
}

PhysicsAssetReader::~PhysicsAssetReader( ) = default;

PhysicsAsset PhysicsAssetReader::ReadPhysicsAsset( )
{
    m_physicsAsset       = PhysicsAsset( );
    m_physicsAsset.Magic = m_reader->ReadUInt64( );
    if ( m_physicsAsset.Magic != PhysicsAsset{ }.Magic )
    {
        LOG( FATAL ) << "Invalid PhysicsAsset magic number.";
    }

    m_physicsAsset.Version = m_reader->ReadUInt32( );
    if ( m_physicsAsset.Version > PhysicsAsset::Latest )
    {
        LOG( WARNING ) << "PhysicsAsset version mismatch.";
    }

    m_physicsAsset.NumBytes = m_reader->ReadUInt64( );
    m_physicsAsset.Uri      = AssetUri::Parse( m_reader->ReadString( ) );
    m_physicsAsset.Name     = m_reader->ReadString( );

    const uint32_t numColliders = m_reader->ReadUInt32( );
    m_physicsAsset.Colliders.Resize( numColliders );

    for ( uint32_t i = 0; i < numColliders; ++i )
    {
        PhysicsCollider &collider = m_physicsAsset.Colliders.GetElement( i );

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

    const uint32_t numUserProps = m_reader->ReadUInt32( );
    m_physicsAsset.UserProperties.Resize( numUserProps );

    for ( uint32_t i = 0; i < numUserProps; ++i )
    {
        UserProperty &prop = m_physicsAsset.UserProperties.GetElement( i );
        prop.PropertyType  = static_cast<UserProperty::Type>( m_reader->ReadUInt32( ) );
        prop.Name          = m_reader->ReadString( );
        switch ( prop.PropertyType )
        {
        case UserProperty::Type::String:
            prop.StringValue = m_reader->ReadString( );
            break;
        case UserProperty::Type::Int:
            prop.IntValue = m_reader->ReadInt32( );
            break;
        case UserProperty::Type::Float:
            prop.FloatValue = m_reader->ReadFloat( );
            break;
        case UserProperty::Type::Bool:
            prop.BoolValue = m_reader->ReadByte( ) != 0;
            break;
        case UserProperty::Type::Float2:
            prop.Vector2Value = m_reader->ReadFloat_2( );
            break;
        case UserProperty::Type::Float3:
            prop.Vector3Value = m_reader->ReadFloat_3( );
            break;
        case UserProperty::Type::Float4:
            prop.Vector4Value = m_reader->ReadFloat_4( );
            break;
        case UserProperty::Type::Color:
            prop.ColorValue = m_reader->ReadFloat_4( );
            break;
        case UserProperty::Type::Float4x4:
            prop.TransformValue = m_reader->ReadFloat_4x4( );
            break;
        }
    }

    return m_physicsAsset;
}
