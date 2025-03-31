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

#include <DenOfIzGraphics/Assets/Serde/Common/AssetWriterHelpers.h>
#include <DenOfIzGraphics/Assets/Serde/Physics/PhysicsAssetWriter.h>

using namespace DenOfIz;

PhysicsAssetWriter::PhysicsAssetWriter( const PhysicsAssetWriterDesc &desc ) : m_writer( desc.Writer )
{
    if ( !m_writer )
    {
        LOG( FATAL ) << "BinaryWriter cannot be null for PhysicsAssetWriter";
    }
}

PhysicsAssetWriter::~PhysicsAssetWriter( ) = default;

void PhysicsAssetWriter::WritePhysicsAsset( const PhysicsAsset &physicsAsset ) const
{
    m_writer->WriteUInt64( physicsAsset.Magic );
    m_writer->WriteUInt32( physicsAsset.Version );
    m_writer->WriteUInt64( physicsAsset.NumBytes );
    m_writer->WriteString( physicsAsset.Uri.ToString( ) );
    m_writer->WriteString( physicsAsset.Name );
    m_writer->WriteUInt32( physicsAsset.Colliders.NumElements( ) );

    for ( size_t i = 0; i < physicsAsset.Colliders.NumElements( ); ++i )
    {
        const PhysicsCollider &collider = physicsAsset.Colliders.GetElement( i );

        m_writer->WriteUInt32( static_cast<uint32_t>( collider.Type ) );
        m_writer->WriteString( collider.Name );
        m_writer->WriteFloat_4x4( collider.Transform );
        m_writer->WriteFloat( collider.Friction );
        m_writer->WriteFloat( collider.Restitution );
        m_writer->WriteByte( collider.IsTrigger ? 1 : 0 );

        switch ( collider.Type )
        {
        case PhysicsColliderType::Box:
            m_writer->WriteFloat_3( collider.Box.HalfExtents );
            break;

        case PhysicsColliderType::Sphere:
            m_writer->WriteFloat( collider.Sphere.Radius );
            break;

        case PhysicsColliderType::Capsule:
            m_writer->WriteFloat( collider.Capsule.Radius );
            m_writer->WriteFloat( collider.Capsule.Height );
            break;

        case PhysicsColliderType::ConvexHull:
        case PhysicsColliderType::TriangleMesh:
            AssetWriterHelpers::WriteAssetDataStream( m_writer, collider.Mesh.VertexStream );
            AssetWriterHelpers::WriteAssetDataStream( m_writer, collider.Mesh.IndexStream );
            break;
        }
    }

    m_writer->WriteUInt32( physicsAsset.UserProperties.NumElements( ) );

    for ( size_t i = 0; i < physicsAsset.UserProperties.NumElements( ); ++i )
    {
        const UserProperty &prop = physicsAsset.UserProperties.GetElement( i );
        m_writer->WriteUInt32( static_cast<uint32_t>( prop.PropertyType ) );
        m_writer->WriteString( prop.Name );

        // Write property value based on type
        switch ( prop.PropertyType )
        {
        case UserProperty::Type::String:
            m_writer->WriteString( prop.StringValue );
            break;
        case UserProperty::Type::Int:
            m_writer->WriteInt32( prop.IntValue );
            break;
        case UserProperty::Type::Float:
            m_writer->WriteFloat( prop.FloatValue );
            break;
        case UserProperty::Type::Bool:
            m_writer->WriteByte( prop.BoolValue ? 1 : 0 );
            break;
        case UserProperty::Type::Float2:
            m_writer->WriteFloat_2( prop.Vector2Value );
            break;
        case UserProperty::Type::Float3:
            m_writer->WriteFloat_3( prop.Vector3Value );
            break;
        case UserProperty::Type::Float4:
            m_writer->WriteFloat_4( prop.Vector4Value );
            break;
        case UserProperty::Type::Color:
            m_writer->WriteFloat_4( prop.ColorValue );
            break;
        case UserProperty::Type::Float4x4:
            m_writer->WriteFloat_4x4( prop.TransformValue );
            break;
        }
    }

    m_writer->Flush( );
}
