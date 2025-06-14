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

#include "../../../../Internal/DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "../../TestComparators.h"
#include "DenOfIzGraphics/Assets/Serde/Mesh/MeshAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetReader.h"
#include "DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"

using namespace DenOfIz;

class MeshAssetSerdeTest : public testing::Test
{
    std::unique_ptr<MeshAsset> m_asset;

protected:
    MeshAsset *CreateSampleMeshAsset( )
    {
        using namespace DenOfIz;
        m_asset           = std::make_unique<MeshAsset>( );
        m_asset->Name     = "TestMesh";
        m_asset->Uri      = AssetUri::Create( "test/TestMesh.dzmesh" );
        m_asset->NumLODs  = 1;

        m_asset->EnabledAttributes.Position     = true;
        m_asset->EnabledAttributes.Normal       = true;
        m_asset->EnabledAttributes.UV           = true;
        m_asset->EnabledAttributes.Tangent      = false;
        m_asset->EnabledAttributes.Bitangent    = false;
        m_asset->EnabledAttributes.Color        = false;
        m_asset->EnabledAttributes.BlendIndices = false;
        m_asset->EnabledAttributes.BlendWeights = false;

        m_asset->AttributeConfig.NumPositionComponents = 3;
        m_asset->AttributeConfig.NumUVAttributes       = 1;

        m_asset->_Arena.EnsureCapacity( 8096 );
        DZArenaArrayHelper<SubMeshDataArray, SubMeshData>::AllocateAndConstructArray( m_asset->_Arena, m_asset->SubMeshes, 2 );
        DZArenaArrayHelper<MorphTargetArray, MorphTarget>::AllocateAndConstructArray( m_asset->_Arena, m_asset->MorphTargets, 1 );
        DZArenaArrayHelper<UserPropertyArray, UserProperty>::AllocateAndConstructArray( m_asset->_Arena, m_asset->UserProperties, 2 );

        SubMeshData &sm0 = m_asset->SubMeshes.Elements[ 0 ];
        sm0.Name        = "Quad";
        sm0.Topology    = PrimitiveTopology::Triangle;
        sm0.IndexType   = IndexType::Uint16;
        sm0.NumVertices = 4;
        sm0.NumIndices  = 6;
        sm0.MinBounds   = { -1.0f, -1.0f, 0.0f };
        sm0.MaxBounds   = { 1.0f, 1.0f, 0.0f };
        sm0.MaterialRef = AssetUri::Create( "materials/Default.dzmat" );

        DZArenaArrayHelper<BoundingVolumeArray, BoundingVolume>::AllocateAndConstructArray( m_asset->_Arena, sm0.BoundingVolumes, 1 );
        BoundingVolume &bv0 = sm0.BoundingVolumes.Elements[ 0 ];
        bv0.Name    = "BoxBV";
        bv0.Type    = BoundingVolumeType::Box;
        bv0.Box.Min = { -1.1f, -1.1f, -0.1f };
        bv0.Box.Max = { 1.1f, 1.1f, 0.1f };

        SubMeshData &sm1 = m_asset->SubMeshes.Elements[ 1 ];
        sm1.Name        = "Triangle";
        sm1.Topology    = PrimitiveTopology::Triangle;
        sm1.IndexType   = IndexType::Uint32;
        sm1.NumVertices = 3;
        sm1.NumIndices  = 3;
        sm1.MinBounds   = { -0.5f, -0.5f, 0.0f };
        sm1.MaxBounds   = { 0.5f, 0.5f, 0.0f };

        DZArenaArrayHelper<BoundingVolumeArray, BoundingVolume>::AllocateAndConstructArray( m_asset->_Arena, sm1.BoundingVolumes, 1 );
        BoundingVolume &bv1 = sm1.BoundingVolumes.Elements[ 0 ];
        bv1.Name = "HullBV";
        bv1.Type = BoundingVolumeType::ConvexHull;

        MorphTarget &mt0 = m_asset->MorphTargets.Elements[ 0 ];
        mt0.Name          = "Smile";
        mt0.DefaultWeight = 0.0f;

        UserProperty &up0 = m_asset->UserProperties.Elements[ 0 ];
        up0.Name         = "DesignerNote";
        up0.PropertyType = UserProperty::Type::String;
        up0.StringValue  = "This is a test mesh.";

        UserProperty &up1 = m_asset->UserProperties.Elements[ 1 ];
        up1.Name         = "ExportScale";
        up1.PropertyType = UserProperty::Type::Float;
        up1.FloatValue   = 100.0f;

        return m_asset.get( );
    }

    MeshVertex CreateMeshVertex( const float posX, const float posY, const float posZ, const float normalX, const float normalY, const float normalZ, const float uvX,
                                 const float uvY )
    {
        MeshVertex vertex;
        vertex.Position.X = posX;
        vertex.Position.Y = posY;
        vertex.Position.Z = posZ;
        vertex.Normal.X   = normalX;
        vertex.Normal.Y   = normalY;
        vertex.Normal.Z   = normalZ;
        DZArenaArrayHelper<Float_2Array, Float_2>::AllocateAndConstructArray( m_tempArena, vertex.UVs, 1 );
        vertex.UVs.Elements[ 0 ] = Float_2{ uvX, uvY };
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

    DZArena m_tempArena{ 1024 };
    std::vector<Byte> convexHullData;

    const std::vector<MorphTargetDelta> smileDeltas = {
        { { 0.0f, 0.1f, 0.0f, 0.0f } }, { { 0.0f, 0.0f, 0.0f, 0.0f } }, { { 0.0f, 0.0f, 0.0f, 0.0f } }, { { 0.0f, 0.1f, 0.0f, 0.0f } }
    };

    void SetUp( ) override
    {
        constexpr float hullVerts[] = { 0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f };
        convexHullData.resize( sizeof( hullVerts ) );
        std::memcpy( convexHullData.data( ), hullVerts, sizeof( hullVerts ) );
    }
};

TEST_F( MeshAssetSerdeTest, WriteAndReadBack )
{
    using namespace DenOfIz;

    BinaryContainer container;
    auto            sampleAsset = std::unique_ptr<MeshAsset>( CreateSampleMeshAsset( ) );

    {
        BinaryWriter    binaryWriter( container );
        MeshAssetWriter writer( MeshAssetWriterDesc{ &binaryWriter } );

        writer.Write( *sampleAsset );
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

        ASSERT_EQ( sampleAsset->SubMeshes.Elements[ 1 ].BoundingVolumes.Elements[ 0 ].Type, BoundingVolumeType::ConvexHull );
        writer.AddConvexHullData( 0, ByteArrayView( convexHullData.data( ), convexHullData.size( ) ) );
        for ( const auto &d : smileDeltas )
        {
            writer.AddMorphTargetDelta( d );
        }
        writer.FinalizeAsset( );
    }

    BinaryReader    reader( container );
    MeshAssetReader meshReader( MeshAssetReaderDesc{ &reader } );
    auto            readAsset = std::unique_ptr<MeshAsset>( meshReader.Read( ) );

    ASSERT_EQ( readAsset->Magic, MeshAsset{ }.Magic );
    ASSERT_EQ( readAsset->Version, MeshAsset::Latest );
    ASSERT_STREQ( readAsset->Name.Get( ), sampleAsset->Name.Get( ) );
    ASSERT_STREQ( readAsset->Uri.ToInteropString( ).Get( ), sampleAsset->Uri.ToInteropString( ).Get( ) );
    ASSERT_EQ( readAsset->NumLODs, sampleAsset->NumLODs );

    ASSERT_EQ( readAsset->EnabledAttributes.Position, sampleAsset->EnabledAttributes.Position );
    ASSERT_EQ( readAsset->EnabledAttributes.Normal, sampleAsset->EnabledAttributes.Normal );
    ASSERT_EQ( readAsset->EnabledAttributes.UV, sampleAsset->EnabledAttributes.UV );
    ASSERT_EQ( readAsset->EnabledAttributes.Tangent, sampleAsset->EnabledAttributes.Tangent );

    ASSERT_EQ( readAsset->AttributeConfig.NumPositionComponents, sampleAsset->AttributeConfig.NumPositionComponents );
    ASSERT_EQ( readAsset->AttributeConfig.NumUVAttributes, sampleAsset->AttributeConfig.NumUVAttributes );

    ASSERT_EQ( readAsset->SubMeshes.NumElements, sampleAsset->SubMeshes.NumElements );

    ASSERT_GE( readAsset->SubMeshes.NumElements, 1 );
    ASSERT_GE( sampleAsset->SubMeshes.NumElements, 1 );
    const SubMeshData &readSM0   = readAsset->SubMeshes.Elements[ 0 ];
    const SubMeshData &sampleSM0 = sampleAsset->SubMeshes.Elements[ 0 ];
    ASSERT_STREQ( readSM0.Name.Get( ), sampleSM0.Name.Get( ) );
    ASSERT_EQ( readSM0.Topology, sampleSM0.Topology );
    ASSERT_EQ( readSM0.IndexType, sampleSM0.IndexType );
    ASSERT_EQ( readSM0.NumVertices, quadVertices.size( ) );
    ASSERT_EQ( readSM0.NumIndices, quadIndices.size( ) );
    ASSERT_FLOAT_EQ( readSM0.MinBounds.X, sampleSM0.MinBounds.X );
    ASSERT_STREQ( readSM0.MaterialRef.ToInteropString( ).Get( ), sampleSM0.MaterialRef.ToInteropString( ).Get( ) );
    ASSERT_EQ( readSM0.LODLevel, sampleSM0.LODLevel );
    ASSERT_EQ( readSM0.BoundingVolumes.NumElements, sampleSM0.BoundingVolumes.NumElements );
    ASSERT_GE( readSM0.BoundingVolumes.NumElements, 1 );
    ASSERT_GE( sampleSM0.BoundingVolumes.NumElements, 1 );
    ASSERT_EQ( readSM0.BoundingVolumes.Elements[ 0 ].Type, BoundingVolumeType::Box );
    ASSERT_STREQ( readSM0.BoundingVolumes.Elements[ 0 ].Name.Get( ), sampleSM0.BoundingVolumes.Elements[ 0 ].Name.Get( ) );
    ASSERT_FLOAT_EQ( readSM0.BoundingVolumes.Elements[ 0 ].Box.Min.X, sampleSM0.BoundingVolumes.Elements[ 0 ].Box.Min.X );

    ASSERT_GE( readAsset->SubMeshes.NumElements, 2 );
    ASSERT_GE( sampleAsset->SubMeshes.NumElements, 2 );
    const SubMeshData &readSM1   = readAsset->SubMeshes.Elements[ 1 ];
    const SubMeshData &sampleSM1 = sampleAsset->SubMeshes.Elements[ 1 ];
    ASSERT_STREQ( readSM1.Name.Get( ), sampleSM1.Name.Get( ) );
    ASSERT_EQ( readSM1.IndexType, sampleSM1.IndexType );
    ASSERT_EQ( readSM1.NumVertices, triVertices.size( ) );
    ASSERT_EQ( readSM1.NumIndices, triIndices.size( ) );
    ASSERT_EQ( readSM1.BoundingVolumes.NumElements, sampleSM1.BoundingVolumes.NumElements );
    ASSERT_GE( readSM1.BoundingVolumes.NumElements, 1 );
    ASSERT_GE( sampleSM1.BoundingVolumes.NumElements, 1 );
    ASSERT_EQ( readSM1.BoundingVolumes.Elements[ 0 ].Type, BoundingVolumeType::ConvexHull );

    ASSERT_EQ( readAsset->MorphTargets.NumElements, sampleAsset->MorphTargets.NumElements );
    ASSERT_GE( readAsset->MorphTargets.NumElements, 1 );
    ASSERT_GE( sampleAsset->MorphTargets.NumElements, 1 );
    const MorphTarget &readMT0   = readAsset->MorphTargets.Elements[ 0 ];
    const MorphTarget &sampleMT0 = sampleAsset->MorphTargets.Elements[ 0 ];
    ASSERT_STREQ( readMT0.Name.Get( ), sampleMT0.Name.Get( ) );
    ASSERT_FLOAT_EQ( readMT0.DefaultWeight, sampleMT0.DefaultWeight );
    ASSERT_EQ( readMT0.VertexDeltaStream.NumBytes, smileDeltas.size( ) * meshReader.MorphDeltaEntryNumBytes( ) );

    ASSERT_EQ( readAsset->UserProperties.NumElements, sampleAsset->UserProperties.NumElements );
    ASSERT_GE( readAsset->UserProperties.NumElements, 1 );
    ASSERT_GE( sampleAsset->UserProperties.NumElements, 1 );
    const UserProperty &readUP0   = readAsset->UserProperties.Elements[ 0 ];
    const UserProperty &sampleUP0 = sampleAsset->UserProperties.Elements[ 0 ];
    ASSERT_EQ( readUP0.PropertyType, sampleUP0.PropertyType );
    ASSERT_STREQ( readUP0.Name.Get( ), sampleUP0.Name.Get( ) );
    ASSERT_STREQ( readUP0.StringValue.Get( ), sampleUP0.StringValue.Get( ) );

    ASSERT_GE( readAsset->UserProperties.NumElements, 2 );
    ASSERT_GE( sampleAsset->UserProperties.NumElements, 2 );
    const UserProperty &readUP1   = readAsset->UserProperties.Elements[ 1 ];
    const UserProperty &sampleUP1 = sampleAsset->UserProperties.Elements[ 1 ];
    ASSERT_EQ( readUP1.PropertyType, sampleUP1.PropertyType );
    ASSERT_STREQ( readUP1.Name.Get( ), sampleUP1.Name.Get( ) );
    ASSERT_FLOAT_EQ( readUP1.FloatValue, sampleUP1.FloatValue );

    MeshVertexArray readVerts0 = meshReader.ReadVertices( readSM0.VertexStream );
    ASSERT_EQ( readVerts0.NumElements, quadVertices.size( ) );
    for ( size_t i = 0; i < readVerts0.NumElements( ); ++i )
    {
        ASSERT_FLOAT_EQ( readVerts0.Elements[ i ].Position.X, quadVertices[ i ].Position.X );
        ASSERT_FLOAT_EQ( readVerts0.Elements[ i ].Position.Y, quadVertices[ i ].Position.Y );
        ASSERT_FLOAT_EQ( readVerts0.Elements[ i ].Position.Z, quadVertices[ i ].Position.Z );

        ASSERT_FLOAT_EQ( readVerts0.Elements[ i ].Normal.X, quadVertices[ i ].Normal.X );

        ASSERT_EQ( readVerts0.Elements[ i ].UVs.NumElements, 1 );
        ASSERT_FLOAT_EQ( readVerts0.Elements[ i ].UVs.Elements[ 0 ].X, quadVertices[ i ].UVs.Elements[ 0 ].X );
        ASSERT_FLOAT_EQ( readVerts0.Elements[ i ].UVs.Elements[ 0 ].Y, quadVertices[ i ].UVs.Elements[ 0 ].Y );
    }
    std::free( readVerts0.Elements );

    UInt16Array readIndices0 = meshReader.ReadIndices16( readSM0.IndexStream );
    ASSERT_EQ( readIndices0.NumElements, quadIndices.size( ) );
    for ( size_t i = 0; i < readIndices0.NumElements; ++i )
    {
        ASSERT_EQ( readIndices0.Elements[ i ], quadIndices[ i ] );
    }
    std::free( readIndices0.Elements );

    MeshVertexArray readVerts1 = meshReader.ReadVertices( readSM1.VertexStream );
    ASSERT_EQ( readVerts1.NumElements, triVertices.size( ) );

    UInt32Array readIndices1 = meshReader.ReadIndices32( readSM1.IndexStream );
    ASSERT_EQ( readIndices1.NumElements, triIndices.size( ) );
    for ( size_t i = 0; i < readIndices1.NumElements; ++i )
    {
        ASSERT_EQ( readIndices1.Elements[ i ], triIndices[ i ] );
    }
    std::free( readIndices1.Elements );

    ByteArray readHullData = meshReader.ReadConvexHullData( readSM1.BoundingVolumes.Elements[ 0 ].ConvexHull.VertexStream );
    AssertArrayEq( readHullData.Elements, convexHullData.data( ), readHullData.NumElements );

    MorphTargetDeltaArray readDeltas0 = meshReader.ReadMorphTargetDeltas( readMT0.VertexDeltaStream );
    ASSERT_EQ( readDeltas0.NumElements, smileDeltas.size( ) );
    for ( size_t i = 0; i < readDeltas0.NumElements( ); ++i )
    {
        ASSERT_FLOAT_EQ( readDeltas0.Elements[ i ].Position.X, smileDeltas[ i ].Position.X );
        ASSERT_FLOAT_EQ( readDeltas0.Elements[ i ].Position.Y, smileDeltas[ i ].Position.Y );
        ASSERT_FLOAT_EQ( readDeltas0.Elements[ i ].Position.Z, smileDeltas[ i ].Position.Z );
    }
}
