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

#include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetReader.h>
#include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetWriter.h>
#include "../../TestComparators.h"

using namespace DenOfIz;

class MeshAssetSerdeTest : public testing::Test
{
protected:
    static MeshAsset CreateSampleMeshAsset( )
    {
        using namespace DenOfIz;
        MeshAsset asset;
        asset.Name    = "TestMesh";
        asset.Uri     = AssetUri::Create( "test/TestMesh.dzmesh" );
        asset.NumLODs = 1;

        asset.EnabledAttributes.Position     = true;
        asset.EnabledAttributes.Normal       = true;
        asset.EnabledAttributes.UV           = true;
        asset.EnabledAttributes.Tangent      = false;
        asset.EnabledAttributes.Bitangent    = false;
        asset.EnabledAttributes.Color        = false;
        asset.EnabledAttributes.BlendIndices = false;
        asset.EnabledAttributes.BlendWeights = false;

        asset.AttributeConfig.NumPositionComponents = 3;
        asset.AttributeConfig.NumUVAttributes       = 1;

        SubMeshData sm0;
        sm0.Name        = "Quad";
        sm0.Topology    = PrimitiveTopology::Triangle;
        sm0.IndexType   = IndexType::Uint16;
        sm0.NumVertices = 4;
        sm0.NumIndices  = 6;
        sm0.MinBounds   = { -1.0f, -1.0f, 0.0f };
        sm0.MaxBounds   = { 1.0f, 1.0f, 0.0f };
        sm0.MaterialRef = AssetUri::Create( "materials/Default.dzmat" );

        BoundingVolume bv0;
        bv0.Name    = "BoxBV";
        bv0.Type    = BoundingVolumeType::Box;
        bv0.Box.Min = { -1.1f, -1.1f, -0.1f };
        bv0.Box.Max = { 1.1f, 1.1f, 0.1f };
        sm0.BoundingVolumes.AddElement( bv0 );
        asset.SubMeshes.AddElement( sm0 );

        SubMeshData sm1;
        sm1.Name        = "Triangle";
        sm1.Topology    = PrimitiveTopology::Triangle;
        sm1.IndexType   = IndexType::Uint32;
        sm1.NumVertices = 3;
        sm1.NumIndices  = 3;
        sm1.MinBounds   = { -0.5f, -0.5f, 0.0f };
        sm1.MaxBounds   = { 0.5f, 0.5f, 0.0f };

        BoundingVolume bv1;
        bv1.Name = "HullBV";
        bv1.Type = BoundingVolumeType::ConvexHull;

        sm1.BoundingVolumes.AddElement( bv1 );
        asset.SubMeshes.AddElement( sm1 );

        MorphTarget mt0;
        mt0.Name          = "Smile";
        mt0.DefaultWeight = 0.0f;
        asset.MorphTargets.AddElement( mt0 );

        UserProperty up0;
        up0.Name         = "DesignerNote";
        up0.PropertyType = UserProperty::Type::String;
        up0.StringValue  = "This is a test mesh.";
        asset.UserProperties.AddElement( up0 );

        UserProperty up1;
        up1.Name         = "ExportScale";
        up1.PropertyType = UserProperty::Type::Float;
        up1.FloatValue   = 100.0f;
        asset.UserProperties.AddElement( up1 );

        return asset;
    }

    static MeshVertex CreateMeshVertex( const float posX, const float posY, const float posZ, const float normalX, const float normalY, const float normalZ, const float uvX,
                                        const float uvY )
    {
        MeshVertex vertex;
        vertex.Position.X = posX;
        vertex.Position.Y = posY;
        vertex.Position.Z = posZ;
        vertex.Normal.X   = normalX;
        vertex.Normal.Y   = normalY;
        vertex.Normal.Z   = normalZ;
        vertex.UVs.AddElement( Float_2{ uvX, uvY } );
        return vertex;
    }

    const std::vector<MeshVertex> quadVertices = { CreateMeshVertex( -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f ),
                                                   CreateMeshVertex( 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f ),
                                                   CreateMeshVertex( 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f ),
                                                   CreateMeshVertex( -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f ) };
    const std::vector<uint16_t>   quadIndices  = { 0, 1, 2, 0, 2, 3 };

    const std::vector<MeshVertex> triVertices = { CreateMeshVertex( 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f ),
                                                  CreateMeshVertex( -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f ),
                                                  CreateMeshVertex( 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f ) };

    const std::vector<uint32_t> triIndices = { 0, 1, 2 };

    InteropArray<Byte> convexHullData;

    const std::vector<MorphTargetDelta> smileDeltas = {
        { { 0.0f, 0.1f, 0.0f, 0.0f } }, { { 0.0f, 0.0f, 0.0f, 0.0f } }, { { 0.0f, 0.0f, 0.0f, 0.0f } }, { { 0.0f, 0.1f, 0.0f, 0.0f } }
    };

    void SetUp( ) override
    {

        constexpr float hullVerts[] = { 0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f };
        convexHullData.MemCpy( hullVerts, sizeof( hullVerts ) );
    }
};

TEST_F( MeshAssetSerdeTest, WriteAndReadBack )
{
    using namespace DenOfIz;

    BinaryContainer container;
    BinaryWriter    binaryWriter( container );
    MeshAsset       sampleAsset = CreateSampleMeshAsset( );

    {
        MeshAssetWriter writer( MeshAssetWriterDesc{ &binaryWriter } );

        writer.Write( sampleAsset );
        for ( const auto &v : quadVertices )
        {
            writer.AddVertex( v );
        }
        for ( const auto &i : quadIndices )
        {
            writer.AddIndex16( i );
        }

        for ( const auto &v : triVertices )
        {
            writer.AddVertex( v );
        }
        for ( const auto &i : triIndices )
        {
            writer.AddIndex32( i );
        }

        ASSERT_EQ( sampleAsset.SubMeshes.GetElement( 1 ).BoundingVolumes.GetElement( 0 ).Type, BoundingVolumeType::ConvexHull );
        writer.AddConvexHullData( 0, convexHullData );
        for ( const auto &d : smileDeltas )
        {
            writer.AddMorphTargetDelta( d );
        }
        writer.FinalizeAsset( );
    }

    BinaryReader    reader( container );
    MeshAssetReader meshReader( MeshAssetReaderDesc{ &reader } );

    MeshAsset readAsset = meshReader.Read( );

    ASSERT_EQ( readAsset.Magic, MeshAsset{ }.Magic );
    ASSERT_EQ( readAsset.Version, MeshAsset::Latest );
    ASSERT_STREQ( readAsset.Name.Get( ), sampleAsset.Name.Get( ) );
    ASSERT_STREQ( readAsset.Uri.ToInteropString( ).Get( ), sampleAsset.Uri.ToInteropString( ).Get( ) );
    ASSERT_EQ( readAsset.NumLODs, sampleAsset.NumLODs );

    ASSERT_EQ( readAsset.EnabledAttributes.Position, sampleAsset.EnabledAttributes.Position );
    ASSERT_EQ( readAsset.EnabledAttributes.Normal, sampleAsset.EnabledAttributes.Normal );
    ASSERT_EQ( readAsset.EnabledAttributes.UV, sampleAsset.EnabledAttributes.UV );
    ASSERT_EQ( readAsset.EnabledAttributes.Tangent, sampleAsset.EnabledAttributes.Tangent );

    ASSERT_EQ( readAsset.AttributeConfig.NumPositionComponents, sampleAsset.AttributeConfig.NumPositionComponents );
    ASSERT_EQ( readAsset.AttributeConfig.NumUVAttributes, sampleAsset.AttributeConfig.NumUVAttributes );

    ASSERT_EQ( readAsset.SubMeshes.NumElements( ), sampleAsset.SubMeshes.NumElements( ) );

    const SubMeshData &readSM0   = readAsset.SubMeshes.GetElement( 0 );
    const SubMeshData &sampleSM0 = sampleAsset.SubMeshes.GetElement( 0 );
    ASSERT_STREQ( readSM0.Name.Get( ), sampleSM0.Name.Get( ) );
    ASSERT_EQ( readSM0.Topology, sampleSM0.Topology );
    ASSERT_EQ( readSM0.IndexType, sampleSM0.IndexType );
    ASSERT_EQ( readSM0.NumVertices, quadVertices.size( ) );
    ASSERT_EQ( readSM0.NumIndices, quadIndices.size( ) );
    ASSERT_FLOAT_EQ( readSM0.MinBounds.X, sampleSM0.MinBounds.X );
    ASSERT_STREQ( readSM0.MaterialRef.ToInteropString( ).Get( ), sampleSM0.MaterialRef.ToInteropString( ).Get( ) );
    ASSERT_EQ( readSM0.LODLevel, sampleSM0.LODLevel );
    ASSERT_EQ( readSM0.BoundingVolumes.NumElements( ), sampleSM0.BoundingVolumes.NumElements( ) );
    ASSERT_EQ( readSM0.BoundingVolumes.GetElement( 0 ).Type, BoundingVolumeType::Box );
    ASSERT_STREQ( readSM0.BoundingVolumes.GetElement( 0 ).Name.Get( ), sampleSM0.BoundingVolumes.GetElement( 0 ).Name.Get( ) );
    ASSERT_FLOAT_EQ( readSM0.BoundingVolumes.GetElement( 0 ).Box.Min.X, sampleSM0.BoundingVolumes.GetElement( 0 ).Box.Min.X );

    const SubMeshData &readSM1   = readAsset.SubMeshes.GetElement( 1 );
    const SubMeshData &sampleSM1 = sampleAsset.SubMeshes.GetElement( 1 );
    ASSERT_STREQ( readSM1.Name.Get( ), sampleSM1.Name.Get( ) );
    ASSERT_EQ( readSM1.IndexType, sampleSM1.IndexType );
    ASSERT_EQ( readSM1.NumVertices, triVertices.size( ) );
    ASSERT_EQ( readSM1.NumIndices, triIndices.size( ) );
    ASSERT_EQ( readSM1.BoundingVolumes.NumElements( ), sampleSM1.BoundingVolumes.NumElements( ) );
    ASSERT_EQ( readSM1.BoundingVolumes.GetElement( 0 ).Type, BoundingVolumeType::ConvexHull );

    ASSERT_EQ( readAsset.MorphTargets.NumElements( ), sampleAsset.MorphTargets.NumElements( ) );
    const MorphTarget &readMT0   = readAsset.MorphTargets.GetElement( 0 );
    const MorphTarget &sampleMT0 = sampleAsset.MorphTargets.GetElement( 0 );
    ASSERT_STREQ( readMT0.Name.Get( ), sampleMT0.Name.Get( ) );
    ASSERT_FLOAT_EQ( readMT0.DefaultWeight, sampleMT0.DefaultWeight );
    ASSERT_EQ( readMT0.VertexDeltaStream.NumBytes, smileDeltas.size( ) * meshReader.MorphDeltaEntryNumBytes( ) );

    ASSERT_EQ( readAsset.UserProperties.NumElements( ), sampleAsset.UserProperties.NumElements( ) );
    const UserProperty &readUP0   = readAsset.UserProperties.GetElement( 0 );
    const UserProperty &sampleUP0 = sampleAsset.UserProperties.GetElement( 0 );
    ASSERT_EQ( readUP0.PropertyType, sampleUP0.PropertyType );
    ASSERT_STREQ( readUP0.Name.Get( ), sampleUP0.Name.Get( ) );
    ASSERT_STREQ( readUP0.StringValue.Get( ), sampleUP0.StringValue.Get( ) );

    const UserProperty &readUP1   = readAsset.UserProperties.GetElement( 1 );
    const UserProperty &sampleUP1 = sampleAsset.UserProperties.GetElement( 1 );
    ASSERT_EQ( readUP1.PropertyType, sampleUP1.PropertyType );
    ASSERT_STREQ( readUP1.Name.Get( ), sampleUP1.Name.Get( ) );
    ASSERT_FLOAT_EQ( readUP1.FloatValue, sampleUP1.FloatValue );

    InteropArray<MeshVertex> readVerts0 = meshReader.ReadVertices( readSM0.VertexStream );
    ASSERT_EQ( readVerts0.NumElements( ), quadVertices.size( ) );
    for ( size_t i = 0; i < readVerts0.NumElements( ); ++i )
    {
        ASSERT_FLOAT_EQ( readVerts0.GetElement( i ).Position.X, quadVertices[ i ].Position.X );
        ASSERT_FLOAT_EQ( readVerts0.GetElement( i ).Position.Y, quadVertices[ i ].Position.Y );
        ASSERT_FLOAT_EQ( readVerts0.GetElement( i ).Position.Z, quadVertices[ i ].Position.Z );

        ASSERT_FLOAT_EQ( readVerts0.GetElement( i ).Normal.X, quadVertices[ i ].Normal.X );

        ASSERT_EQ( readVerts0.GetElement( i ).UVs.NumElements( ), 1 );
        ASSERT_FLOAT_EQ( readVerts0.GetElement( i ).UVs.GetElement( 0 ).X, quadVertices[ i ].UVs.GetElement( 0 ).X );
        ASSERT_FLOAT_EQ( readVerts0.GetElement( i ).UVs.GetElement( 0 ).Y, quadVertices[ i ].UVs.GetElement( 0 ).Y );
    }

    InteropArray<uint16_t> readIndices0 = meshReader.ReadIndices16( readSM0.IndexStream );
    ASSERT_EQ( readIndices0.NumElements( ), quadIndices.size( ) );
    for ( size_t i = 0; i < readIndices0.NumElements( ); ++i )
    {
        ASSERT_EQ( readIndices0.GetElement( i ), quadIndices[ i ] );
    }

    InteropArray<MeshVertex> readVerts1 = meshReader.ReadVertices( readSM1.VertexStream );
    ASSERT_EQ( readVerts1.NumElements( ), triVertices.size( ) );

    InteropArray<uint32_t> readIndices1 = meshReader.ReadIndices32( readSM1.IndexStream );
    ASSERT_EQ( readIndices1.NumElements( ), triIndices.size( ) );
    for ( size_t i = 0; i < readIndices1.NumElements( ); ++i )
    {
        ASSERT_EQ( readIndices1.GetElement( i ), triIndices[ i ] );
    }

    InteropArray<Byte> readHullData = meshReader.ReadConvexHullData( readSM1.BoundingVolumes.GetElement( 0 ).ConvexHull.VertexStream );
    AssertInteropArrayEq( readHullData, convexHullData );

    InteropArray<MorphTargetDelta> readDeltas0 = meshReader.ReadMorphTargetDeltas( readMT0.VertexDeltaStream );
    ASSERT_EQ( readDeltas0.NumElements( ), smileDeltas.size( ) );
    for ( size_t i = 0; i < readDeltas0.NumElements( ); ++i )
    {

        ASSERT_FLOAT_EQ( readDeltas0.GetElement( i ).Position.X, smileDeltas[ i ].Position.X );
        ASSERT_FLOAT_EQ( readDeltas0.GetElement( i ).Position.Y, smileDeltas[ i ].Position.Y );
        ASSERT_FLOAT_EQ( readDeltas0.GetElement( i ).Position.Z, smileDeltas[ i ].Position.Z );
    }
}
