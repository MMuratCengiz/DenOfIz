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

#include "gtest/gtest.h"

#include <DenOfIzGraphics/Assets/Serde/Physics/PhysicsAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Physics/PhysicsAssetReader.h>
#include <DenOfIzGraphics/Assets/Serde/Physics/PhysicsAssetWriter.h>
#include "../../TestComparators.h"

using namespace DenOfIz;

class PhysicsAssetSerdeTest : public testing::Test
{
protected:
    static PhysicsAsset CreateSamplePhysicsAsset( )
    {
        using namespace DenOfIz;
        PhysicsAsset asset;
        asset.Name = "TestPhysicsAsset";
        asset.Uri  = AssetUri::Create( "test/TestPhysics.dzphys" );

        PhysicsCollider boxCollider;
        boxCollider.Type            = PhysicsColliderType::Box;
        boxCollider.Name            = "BoxCollider";
        boxCollider.Transform       = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
        boxCollider.Friction        = 0.5f;
        boxCollider.Restitution     = 0.3f;
        boxCollider.IsTrigger       = false;
        boxCollider.Box.HalfExtents = { 1.0f, 1.0f, 1.0f };

        PhysicsCollider sphereCollider;
        sphereCollider.Type          = PhysicsColliderType::Sphere;
        sphereCollider.Name          = "SphereCollider";
        sphereCollider.Transform     = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f };
        sphereCollider.Friction      = 0.2f;
        sphereCollider.Restitution   = 0.8f;
        sphereCollider.IsTrigger     = false;
        sphereCollider.Sphere.Radius = 0.5f;

        // Create capsule collider
        PhysicsCollider capsuleCollider;
        capsuleCollider.Type           = PhysicsColliderType::Capsule;
        capsuleCollider.Name           = "CapsuleCollider";
        capsuleCollider.Transform      = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f };
        capsuleCollider.Friction       = 0.1f;
        capsuleCollider.Restitution    = 0.5f;
        capsuleCollider.IsTrigger      = true;
        capsuleCollider.Capsule.Radius = 0.3f;
        capsuleCollider.Capsule.Height = 1.0f;

        // Add colliders to asset
        asset.Colliders.AddElement( boxCollider );
        asset.Colliders.AddElement( sphereCollider );
        asset.Colliders.AddElement( capsuleCollider );

        // Add some user properties
        UserProperty prop1;
        prop1.Name         = "Mass";
        prop1.PropertyType = UserProperty::Type::Float;
        prop1.FloatValue   = 10.0f;
        asset.UserProperties.AddElement( prop1 );

        UserProperty prop2;
        prop2.Name         = "IsDynamic";
        prop2.PropertyType = UserProperty::Type::Bool;
        prop2.BoolValue    = true;
        asset.UserProperties.AddElement( prop2 );

        return asset;
    }
};

TEST_F( PhysicsAssetSerdeTest, WriteAndReadBack )
{
    using namespace DenOfIz;

    BinaryContainer container;
    BinaryWriter    binaryWriter( container );
    PhysicsAsset    sampleAsset = CreateSamplePhysicsAsset( );

    {
        PhysicsAssetWriter writer( PhysicsAssetWriterDesc{ &binaryWriter } );
        writer.Write( sampleAsset );
    }

    BinaryReader       reader( container );
    PhysicsAssetReader physReader( PhysicsAssetReaderDesc{ &reader } );

    PhysicsAsset readAsset = physReader.Read( );

    ASSERT_EQ( readAsset.Magic, PhysicsAsset{ }.Magic );
    ASSERT_EQ( readAsset.Version, PhysicsAsset::Latest );
    ASSERT_STREQ( readAsset.Name.Get( ), sampleAsset.Name.Get( ) );
    ASSERT_STREQ( readAsset.Uri.ToInteropString( ).Get( ), sampleAsset.Uri.ToInteropString( ).Get( ) );

    ASSERT_EQ( readAsset.Colliders.NumElements( ), sampleAsset.Colliders.NumElements( ) );

    const PhysicsCollider &readBoxCollider   = readAsset.Colliders.GetElement( 0 );
    const PhysicsCollider &sampleBoxCollider = sampleAsset.Colliders.GetElement( 0 );

    ASSERT_EQ( readBoxCollider.Type, PhysicsColliderType::Box );
    ASSERT_STREQ( readBoxCollider.Name.Get( ), sampleBoxCollider.Name.Get( ) );
    ASSERT_TRUE( MatricesEqual( readBoxCollider.Transform, sampleBoxCollider.Transform ) );
    ASSERT_FLOAT_EQ( readBoxCollider.Friction, sampleBoxCollider.Friction );
    ASSERT_FLOAT_EQ( readBoxCollider.Restitution, sampleBoxCollider.Restitution );
    ASSERT_EQ( readBoxCollider.IsTrigger, sampleBoxCollider.IsTrigger );
    ASSERT_TRUE( Vector3Equal( readBoxCollider.Box.HalfExtents, sampleBoxCollider.Box.HalfExtents ) );

    const PhysicsCollider &readSphereCollider   = readAsset.Colliders.GetElement( 1 );
    const PhysicsCollider &sampleSphereCollider = sampleAsset.Colliders.GetElement( 1 );

    ASSERT_EQ( readSphereCollider.Type, PhysicsColliderType::Sphere );
    ASSERT_STREQ( readSphereCollider.Name.Get( ), sampleSphereCollider.Name.Get( ) );
    ASSERT_TRUE( MatricesEqual( readSphereCollider.Transform, sampleSphereCollider.Transform ) );
    ASSERT_FLOAT_EQ( readSphereCollider.Friction, sampleSphereCollider.Friction );
    ASSERT_FLOAT_EQ( readSphereCollider.Restitution, sampleSphereCollider.Restitution );
    ASSERT_EQ( readSphereCollider.IsTrigger, sampleSphereCollider.IsTrigger );
    ASSERT_FLOAT_EQ( readSphereCollider.Sphere.Radius, sampleSphereCollider.Sphere.Radius );

    const PhysicsCollider &readCapsuleCollider   = readAsset.Colliders.GetElement( 2 );
    const PhysicsCollider &sampleCapsuleCollider = sampleAsset.Colliders.GetElement( 2 );

    ASSERT_EQ( readCapsuleCollider.Type, PhysicsColliderType::Capsule );
    ASSERT_STREQ( readCapsuleCollider.Name.Get( ), sampleCapsuleCollider.Name.Get( ) );
    ASSERT_TRUE( MatricesEqual( readCapsuleCollider.Transform, sampleCapsuleCollider.Transform ) );
    ASSERT_FLOAT_EQ( readCapsuleCollider.Friction, sampleCapsuleCollider.Friction );
    ASSERT_FLOAT_EQ( readCapsuleCollider.Restitution, sampleCapsuleCollider.Restitution );
    ASSERT_EQ( readCapsuleCollider.IsTrigger, sampleCapsuleCollider.IsTrigger );
    ASSERT_FLOAT_EQ( readCapsuleCollider.Capsule.Radius, sampleCapsuleCollider.Capsule.Radius );
    ASSERT_FLOAT_EQ( readCapsuleCollider.Capsule.Height, sampleCapsuleCollider.Capsule.Height );

    ASSERT_EQ( readAsset.UserProperties.NumElements( ), sampleAsset.UserProperties.NumElements( ) );

    const UserProperty &readProp1   = readAsset.UserProperties.GetElement( 0 );
    const UserProperty &sampleProp1 = sampleAsset.UserProperties.GetElement( 0 );

    ASSERT_EQ( readProp1.PropertyType, sampleProp1.PropertyType );
    ASSERT_STREQ( readProp1.Name.Get( ), sampleProp1.Name.Get( ) );
    ASSERT_FLOAT_EQ( readProp1.FloatValue, sampleProp1.FloatValue );

    const UserProperty &readProp2   = readAsset.UserProperties.GetElement( 1 );
    const UserProperty &sampleProp2 = sampleAsset.UserProperties.GetElement( 1 );

    ASSERT_EQ( readProp2.PropertyType, sampleProp2.PropertyType );
    ASSERT_STREQ( readProp2.Name.Get( ), sampleProp2.Name.Get( ) );
    ASSERT_EQ( readProp2.BoolValue, sampleProp2.BoolValue );
}
