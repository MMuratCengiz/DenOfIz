/**/

#include "gtest/gtest.h"

#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetReader.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetWriter.h>
#include "../../TestComparators.h"

using namespace DenOfIz;

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
        rootJoint.LocalTranslation  = { 0.0f, 0.0f, 0.0f };
        rootJoint.LocalRotationQuat = { 0.0f, 0.0f, 0.0f, 1.0f };
        rootJoint.LocalScale        = { 1.0f, 1.0f, 1.0f };

        rootJoint.ChildIndices.AddElement( 1 );

        Joint spineJoint;
        spineJoint.Name        = "Spine";
        spineJoint.Index       = 1;
        spineJoint.ParentIndex = 0;

        spineJoint.InverseBindMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f };
        spineJoint.LocalTranslation  = { 0.0f, 1.0f, 0.0f };
        spineJoint.LocalRotationQuat = { 0.0f, 0.0f, 0.0f, 1.0f };
        spineJoint.LocalScale        = { 1.0f, 1.0f, 1.0f };

        spineJoint.ChildIndices.AddElement( 2 );

        Joint headJoint;
        headJoint.Name        = "Head";
        headJoint.Index       = 2;
        headJoint.ParentIndex = 1;

        headJoint.InverseBindMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -2.0f, 0.0f, 1.0f };
        headJoint.LocalTranslation  = { 0.0f, 1.0f, 0.0f };
        headJoint.LocalRotationQuat = { 0.0f, 0.0f, 0.0f, 1.0f };
        headJoint.LocalScale        = { 1.0f, 1.0f, 1.0f };

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
        writer.Write( sampleAsset );
    }

    BinaryReader        reader( container );
    SkeletonAssetReader skelReader( SkeletonAssetReaderDesc{ &reader } );

    SkeletonAsset readAsset = skelReader.Read( );

    ASSERT_EQ( readAsset.Magic, SkeletonAsset{ }.Magic );
    ASSERT_EQ( readAsset.Version, SkeletonAsset::Latest );
    ASSERT_STREQ( readAsset.Name.Get( ), sampleAsset.Name.Get( ) );
    ASSERT_STREQ( readAsset.Uri.ToInteropString( ).Get( ), sampleAsset.Uri.ToInteropString( ).Get( ) );

    ASSERT_EQ( readAsset.Joints.NumElements( ), sampleAsset.Joints.NumElements( ) );

    for ( size_t i = 0; i < readAsset.Joints.NumElements( ); ++i )
    {
        const Joint &readJoint   = readAsset.Joints.GetElement( i );
        const Joint &sampleJoint = sampleAsset.Joints.GetElement( i );

        ASSERT_STREQ( readJoint.Name.Get( ), sampleJoint.Name.Get( ) );
        ASSERT_EQ( readJoint.Index, sampleJoint.Index );
        ASSERT_EQ( readJoint.ParentIndex, sampleJoint.ParentIndex );

        ASSERT_TRUE( MatricesEqual( readJoint.InverseBindMatrix, sampleJoint.InverseBindMatrix ) );
        ASSERT_TRUE( Float3Equals( readJoint.LocalTranslation, sampleJoint.LocalTranslation ));
        ASSERT_TRUE( Float4Equals( readJoint.LocalRotationQuat, sampleJoint.LocalRotationQuat ));
        ASSERT_TRUE( Float3Equals( readJoint.LocalScale, sampleJoint.LocalScale ));

        ASSERT_EQ( readJoint.ChildIndices.NumElements( ), sampleJoint.ChildIndices.NumElements( ) );

        for ( size_t j = 0; j < readJoint.ChildIndices.NumElements( ); ++j )
        {
            ASSERT_EQ( readJoint.ChildIndices.GetElement( j ), sampleJoint.ChildIndices.GetElement( j ) );
        }
    }
}
