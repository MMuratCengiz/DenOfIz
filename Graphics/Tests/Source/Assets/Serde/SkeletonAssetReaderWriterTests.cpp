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
#include "DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetReader.h"
#include "DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"

using namespace DenOfIz;

class SkeletonAssetSerdeTest : public testing::Test
{
    std::unique_ptr<SkeletonAsset> m_asset;

protected:
    SkeletonAsset *CreateSampleSkeletonAsset( )
    {
        using namespace DenOfIz;
        m_asset       = std::make_unique<SkeletonAsset>( );
        m_asset->Name = "TestSkeleton";
        m_asset->Uri  = AssetUri::Create( "test/TestSkeleton.dzskel" );

        m_asset->_Arena.EnsureCapacity( 4096 );
        DZArenaArrayHelper<JointArray, Joint>::AllocateAndConstructArray( m_asset->_Arena, m_asset->Joints, 3 );

        Joint &rootJoint      = m_asset->Joints.Elements[ 0 ];
        rootJoint.Name        = "Root";
        rootJoint.Index       = 0;
        rootJoint.ParentIndex = -1;

        rootJoint.InverseBindMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
        rootJoint.LocalTranslation  = { 0.0f, 0.0f, 0.0f };
        rootJoint.LocalRotationQuat = { 0.0f, 0.0f, 0.0f, 1.0f };
        rootJoint.LocalScale        = { 1.0f, 1.0f, 1.0f };

        DZArenaArrayHelper<UInt32Array, uint32_t>::AllocateAndConstructArray( m_asset->_Arena, rootJoint.ChildIndices, 1 );
        rootJoint.ChildIndices.Elements[ 0 ] = 1;

        Joint &spineJoint      = m_asset->Joints.Elements[ 1 ];
        spineJoint.Name        = "Spine";
        spineJoint.Index       = 1;
        spineJoint.ParentIndex = 0;

        spineJoint.InverseBindMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f };
        spineJoint.LocalTranslation  = { 0.0f, 1.0f, 0.0f };
        spineJoint.LocalRotationQuat = { 0.0f, 0.0f, 0.0f, 1.0f };
        spineJoint.LocalScale        = { 1.0f, 1.0f, 1.0f };

        DZArenaArrayHelper<UInt32Array, uint32_t>::AllocateAndConstructArray( m_asset->_Arena, spineJoint.ChildIndices, 1 );
        spineJoint.ChildIndices.Elements[ 0 ] = 2;

        Joint &headJoint      = m_asset->Joints.Elements[ 2 ];
        headJoint.Name        = "Head";
        headJoint.Index       = 2;
        headJoint.ParentIndex = 1;

        headJoint.InverseBindMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -2.0f, 0.0f, 1.0f };
        headJoint.LocalTranslation  = { 0.0f, 1.0f, 0.0f };
        headJoint.LocalRotationQuat = { 0.0f, 0.0f, 0.0f, 1.0f };
        headJoint.LocalScale        = { 1.0f, 1.0f, 1.0f };

        DZArenaArrayHelper<UInt32Array, uint32_t>::AllocateAndConstructArray( m_asset->_Arena, headJoint.ChildIndices, 0 );

        return m_asset.get( );
    }
};

TEST_F( SkeletonAssetSerdeTest, WriteAndReadBack )
{
    using namespace DenOfIz;

    BinaryContainer container;
    auto            sampleAsset = std::unique_ptr<SkeletonAsset>( CreateSampleSkeletonAsset( ) );

    {
        BinaryWriter        binaryWriter( container );
        SkeletonAssetWriter writer( SkeletonAssetWriterDesc{ &binaryWriter } );
        writer.Write( *sampleAsset );
    }
    BinaryReader        reader( container );
    SkeletonAssetReader skelReader( SkeletonAssetReaderDesc{ &reader } );
    auto                readAsset = std::unique_ptr<SkeletonAsset>( skelReader.Read( ) );

    ASSERT_EQ( readAsset->Magic, SkeletonAsset{ }.Magic );
    ASSERT_EQ( readAsset->Version, SkeletonAsset::Latest );
    ASSERT_STREQ( readAsset->Name.Get( ), sampleAsset->Name.Get( ) );
    ASSERT_STREQ( readAsset->Uri.ToInteropString( ).Get( ), sampleAsset->Uri.ToInteropString( ).Get( ) );

    ASSERT_EQ( readAsset->Joints.NumElements, sampleAsset->Joints.NumElements );

    for ( size_t i = 0; i < readAsset->Joints.NumElements; ++i )
    {
        const Joint &readJoint   = readAsset->Joints.Elements[ i ];
        const Joint &sampleJoint = sampleAsset->Joints.Elements[ i ];

        ASSERT_STREQ( readJoint.Name.Get( ), sampleJoint.Name.Get( ) );
        ASSERT_EQ( readJoint.Index, sampleJoint.Index );
        ASSERT_EQ( readJoint.ParentIndex, sampleJoint.ParentIndex );

        ASSERT_TRUE( MatricesEqual( readJoint.InverseBindMatrix, sampleJoint.InverseBindMatrix ) );
        ASSERT_TRUE( Float3Equals( readJoint.LocalTranslation, sampleJoint.LocalTranslation ) );
        ASSERT_TRUE( Float4Equals( readJoint.LocalRotationQuat, sampleJoint.LocalRotationQuat ) );
        ASSERT_TRUE( Float3Equals( readJoint.LocalScale, sampleJoint.LocalScale ) );

        ASSERT_EQ( readJoint.ChildIndices.NumElements, sampleJoint.ChildIndices.NumElements );

        for ( size_t j = 0; j < readJoint.ChildIndices.NumElements; ++j )
        {
            ASSERT_EQ( readJoint.ChildIndices.Elements[ j ], sampleJoint.ChildIndices.Elements[ j ] );
        }
    }
}
