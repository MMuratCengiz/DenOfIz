/**/

#include "gtest/gtest.h"

#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetReader.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetWriter.h>

using namespace DenOfIz;

template <typename T>
void AssertInteropArrayEq( const InteropArray<T> &arr1, const InteropArray<T> &arr2 )
{
    ASSERT_EQ( arr1.NumElements( ), arr2.NumElements( ) );
    for ( size_t i = 0; i < arr1.NumElements( ); ++i )
    {
        ASSERT_EQ( arr1.GetElement( i ), arr2.GetElement( i ) );
    }
}

bool MatricesEqual( const Float_4x4 &a, const Float_4x4 &b, const float epsilon = 0.00001f )
{
    for ( int i = 0; i < 4; ++i )
    {
        for ( int j = 0; j < 4; ++j )
        {
            if ( std::abs( a.GetElement( i, j ) - b.GetElement( i, j ) ) > epsilon )
            {
                return false;
            }
        }
    }
    return true;
}

class SkeletonAssetSerdeTest : public testing::Test
{
protected:
    static SkeletonAsset CreateSampleSkeletonAsset( )
    {
        using namespace DenOfIz;
        SkeletonAsset asset;
        asset.Name = "TestSkeleton";
        asset.Uri  = AssetUri::Create( "test/TestSkeleton.dzskel" );

        Joint rootJoint;
        rootJoint.Name        = "Root";
        rootJoint.Index       = 0;
        rootJoint.ParentIndex = -1;

        rootJoint.InverseBindMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
        rootJoint.LocalTransform    = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
        rootJoint.GlobalTransform   = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

        rootJoint.ChildIndices.AddElement( 1 );

        Joint spineJoint;
        spineJoint.Name        = "Spine";
        spineJoint.Index       = 1;
        spineJoint.ParentIndex = 0;

        spineJoint.InverseBindMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f };
        spineJoint.LocalTransform    = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f };
        spineJoint.GlobalTransform   = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f };

        spineJoint.ChildIndices.AddElement( 2 );

        Joint headJoint;
        headJoint.Name        = "Head";
        headJoint.Index       = 2;
        headJoint.ParentIndex = 1;

        headJoint.InverseBindMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -2.0f, 0.0f, 1.0f };
        headJoint.LocalTransform    = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f };
        headJoint.GlobalTransform   = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f };

        asset.Joints.AddElement( rootJoint );
        asset.Joints.AddElement( spineJoint );
        asset.Joints.AddElement( headJoint );

        return asset;
    }
};

TEST_F( SkeletonAssetSerdeTest, WriteAndReadBack )
{
    using namespace DenOfIz;

    BinaryContainer container;
    BinaryWriter    binaryWriter( container );
    SkeletonAsset   sampleAsset = CreateSampleSkeletonAsset( );

    {
        SkeletonAssetWriter writer( SkeletonAssetWriterDesc{ &binaryWriter } );
        writer.WriteSkeletonAsset( sampleAsset );
    }

    BinaryReader        reader( container );
    SkeletonAssetReader skelReader( SkeletonAssetReaderDesc{ &reader } );

    SkeletonAsset readAsset = skelReader.ReadSkeletonAsset( );

    ASSERT_EQ( readAsset.Magic, SkeletonAsset{ }.Magic );
    ASSERT_EQ( readAsset.Version, SkeletonAsset::Latest );
    ASSERT_STREQ( readAsset.Name.Get( ), sampleAsset.Name.Get( ) );
    ASSERT_STREQ( readAsset.Uri.ToString( ).Get( ), sampleAsset.Uri.ToString( ).Get( ) );

    ASSERT_EQ( readAsset.Joints.NumElements( ), sampleAsset.Joints.NumElements( ) );

    for ( size_t i = 0; i < readAsset.Joints.NumElements( ); ++i )
    {
        const Joint &readJoint   = readAsset.Joints.GetElement( i );
        const Joint &sampleJoint = sampleAsset.Joints.GetElement( i );

        ASSERT_STREQ( readJoint.Name.Get( ), sampleJoint.Name.Get( ) );
        ASSERT_EQ( readJoint.Index, sampleJoint.Index );
        ASSERT_EQ( readJoint.ParentIndex, sampleJoint.ParentIndex );

        ASSERT_TRUE( MatricesEqual( readJoint.InverseBindMatrix, sampleJoint.InverseBindMatrix ) );
        ASSERT_TRUE( MatricesEqual( readJoint.LocalTransform, sampleJoint.LocalTransform ) );
        ASSERT_TRUE( MatricesEqual( readJoint.GlobalTransform, sampleJoint.GlobalTransform ) );

        ASSERT_EQ( readJoint.ChildIndices.NumElements( ), sampleJoint.ChildIndices.NumElements( ) );

        for ( size_t j = 0; j < readJoint.ChildIndices.NumElements( ); ++j )
        {
            ASSERT_EQ( readJoint.ChildIndices.GetElement( j ), sampleJoint.ChildIndices.GetElement( j ) );
        }
    }
}
