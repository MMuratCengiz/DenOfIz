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
#include "DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetReader.h"
#include "DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"

using namespace DenOfIz;

class AnimationAssetSerdeTest : public testing::Test
{
    std::unique_ptr<AnimationAsset> m_asset;

protected:
    AnimationAsset *CreateSampleAnimationAsset( )
    {
        using namespace DenOfIz;
        m_asset              = std::make_unique<AnimationAsset>( );
        m_asset->Name        = "TestAnimation";
        m_asset->Uri         = AssetUri::Create( "test/TestAnimation.dzanim" );
        m_asset->SkeletonRef = AssetUri::Create( "test/TestSkeleton.dzskel" );

        m_asset->_Arena.EnsureCapacity( 8096 );
        DZArenaArrayHelper<AnimationClipArray, AnimationClip>::AllocateAndConstructArray( m_asset->_Arena, m_asset->Animations, 2 );

        AnimationClip &clip                 = m_asset->Animations.Elements[ 0 ];
        clip.Name                           = "Walk";
        constexpr double walkTicksPerSecond = 30.0;
        clip.Duration                       = 1.0f; // Duration in seconds

        DZArenaArrayHelper<JointAnimTrackArray, JointAnimTrack>::AllocateAndConstructArray( m_asset->_Arena, clip.Tracks, 2 );
        DZArenaArrayHelper<MorphAnimTrackArray, MorphAnimTrack>::AllocateAndConstructArray( m_asset->_Arena, clip.MorphTracks, 2 );

        JointAnimTrack &rootTrack = clip.Tracks.Elements[ 0 ];
        rootTrack.JointName       = "Root";

        DZArenaArrayHelper<PositionKeyArray, PositionKey>::AllocateAndConstructArray( m_asset->_Arena, rootTrack.PositionKeys, 2 );
        DZArenaArrayHelper<RotationKeyArray, RotationKey>::AllocateAndConstructArray( m_asset->_Arena, rootTrack.RotationKeys, 2 );
        DZArenaArrayHelper<ScaleKeyArray, ScaleKey>::AllocateAndConstructArray( m_asset->_Arena, rootTrack.ScaleKeys, 2 );

        rootTrack.PositionKeys.Elements[ 0 ] = { 0.0f / static_cast<float>( walkTicksPerSecond ), { 0.0f, 0.0f, 0.0f } };
        rootTrack.PositionKeys.Elements[ 1 ] = { 30.0f / static_cast<float>( walkTicksPerSecond ), { 1.0f, 0.0f, 0.0f } }; // Time = 1.0s

        rootTrack.RotationKeys.Elements[ 0 ] = { 0.0f / static_cast<float>( walkTicksPerSecond ), { 0.0f, 0.0f, 0.0f, 1.0f } };    // Identity quat
        rootTrack.RotationKeys.Elements[ 1 ] = { 30.0f / static_cast<float>( walkTicksPerSecond ), { 0.0f, 0.0f, 0.1f, 0.995f } }; // Time = 1.0s

        rootTrack.ScaleKeys.Elements[ 0 ] = { 0.0f / static_cast<float>( walkTicksPerSecond ), { 1.0f, 1.0f, 1.0f } };
        rootTrack.ScaleKeys.Elements[ 1 ] = { 30.0f / static_cast<float>( walkTicksPerSecond ), { 1.0f, 1.0f, 1.0f } }; // Time = 1.0s

        JointAnimTrack &legTrack = clip.Tracks.Elements[ 1 ];
        legTrack.JointName       = "LeftLeg";

        DZArenaArrayHelper<PositionKeyArray, PositionKey>::AllocateAndConstructArray( m_asset->_Arena, legTrack.PositionKeys, 2 );
        DZArenaArrayHelper<RotationKeyArray, RotationKey>::AllocateAndConstructArray( m_asset->_Arena, legTrack.RotationKeys, 2 );
        DZArenaArrayHelper<ScaleKeyArray, ScaleKey>::AllocateAndConstructArray( m_asset->_Arena, legTrack.ScaleKeys, 1 );

        legTrack.PositionKeys.Elements[ 0 ] = { 0.0f / static_cast<float>( walkTicksPerSecond ), { 0.0f, -0.5f, 0.0f } };
        legTrack.PositionKeys.Elements[ 1 ] = { 30.0f / static_cast<float>( walkTicksPerSecond ), { 0.0f, -0.5f, 0.5f } }; // Time = 1.0s

        legTrack.RotationKeys.Elements[ 0 ] = { 0.0f / static_cast<float>( walkTicksPerSecond ), { 0.0f, 0.0f, 0.0f, 1.0f } };    // Identity quat
        legTrack.RotationKeys.Elements[ 1 ] = { 30.0f / static_cast<float>( walkTicksPerSecond ), { 0.1f, 0.0f, 0.0f, 0.995f } }; // Time = 1.0s

        legTrack.ScaleKeys.Elements[ 0 ] = { 0.0f / static_cast<float>( walkTicksPerSecond ), { 1.0f, 1.0f, 1.0f } };

        MorphAnimTrack &morphTrack = clip.MorphTracks.Elements[ 0 ];
        morphTrack.Name            = "Smile";

        DZArenaArrayHelper<MorphKeyframeArray, MorphKeyframe>::AllocateAndConstructArray( m_asset->_Arena, morphTrack.Keyframes, 3 );

        morphTrack.Keyframes.Elements[ 0 ] = { 0.0f / static_cast<float>( walkTicksPerSecond ), 0.0f };
        morphTrack.Keyframes.Elements[ 1 ] = { 15.0f / static_cast<float>( walkTicksPerSecond ), 0.7f }; // Time = 0.5s
        morphTrack.Keyframes.Elements[ 2 ] = { 30.0f / static_cast<float>( walkTicksPerSecond ), 0.0f }; // Time = 1.0s

        AnimationClip &idleClip   = m_asset->Animations.Elements[ 1 ];
        idleClip.Name             = "Idle";
        idleClip.Duration         = 2.0f;
        JointAnimTrack &idleTrack = idleClip.Tracks.Elements[ 0 ];
        idleTrack.JointName       = "Root";

        DZArenaArrayHelper<PositionKeyArray, PositionKey>::AllocateAndConstructArray( m_asset->_Arena, idleTrack.PositionKeys, 1 );
        DZArenaArrayHelper<RotationKeyArray, RotationKey>::AllocateAndConstructArray( m_asset->_Arena, idleTrack.RotationKeys, 1 );
        DZArenaArrayHelper<ScaleKeyArray, ScaleKey>::AllocateAndConstructArray( m_asset->_Arena, idleTrack.ScaleKeys, 1 );
        DZArenaArrayHelper<JointAnimTrackArray, JointAnimTrack>::AllocateAndConstructArray( m_asset->_Arena, idleClip.Tracks, 1 );

        idleTrack.PositionKeys.Elements[ 0 ] = { 0.0f, { 0.0f, 0.0f, 0.0f } };
        idleTrack.RotationKeys.Elements[ 0 ] = { 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } };
        idleTrack.ScaleKeys.Elements[ 0 ]    = { 0.0f, { 1.0f, 1.0f, 1.0f } };
        idleClip.Tracks.Elements[ 0 ]        = idleTrack;
        return m_asset.get( );
    }
};

TEST_F( AnimationAssetSerdeTest, WriteAndReadBack )
{
    using namespace DenOfIz;

    BinaryContainer container;
    auto  sampleAsset = std::unique_ptr<AnimationAsset>( CreateSampleAnimationAsset( ) );

    {
        BinaryWriter         binaryWriter( container );
        AnimationAssetWriter writer( AnimationAssetWriterDesc{ &binaryWriter } );
        writer.Write( *sampleAsset );
    }
    BinaryReader         reader( container );
    AnimationAssetReader animReader( AnimationAssetReaderDesc{ &reader } );
    auto                 readAsset = std::unique_ptr<AnimationAsset>( animReader.Read( ) );

    ASSERT_EQ( readAsset->Magic, AnimationAsset{ }.Magic );
    ASSERT_EQ( readAsset->Version, AnimationAsset::Latest );
    ASSERT_STREQ( readAsset->Name.Get( ), sampleAsset->Name.Get( ) );
    ASSERT_STREQ( readAsset->Uri.ToInteropString( ).Get( ), sampleAsset->Uri.ToInteropString( ).Get( ) );
    ASSERT_STREQ( readAsset->SkeletonRef.ToInteropString( ).Get( ), sampleAsset->SkeletonRef.ToInteropString( ).Get( ) );

    ASSERT_EQ( readAsset->Animations.NumElements( ), sampleAsset->Animations.NumElements( ) );

    ASSERT_GE( readAsset->Animations.NumElements( ), 1 );
    ASSERT_GE( sampleAsset->Animations.NumElements( ), 1 );
    const AnimationClip &readClip   = readAsset->Animations.Elements[ 0 ];
    const AnimationClip &sampleClip = sampleAsset->Animations.Elements[ 0 ];

    ASSERT_STREQ( readClip.Name.Get( ), sampleClip.Name.Get( ) );
    ASSERT_FLOAT_EQ( readClip.Duration, sampleClip.Duration );

    ASSERT_EQ( readClip.Tracks.NumElements( ), sampleClip.Tracks.NumElements( ) );

    ASSERT_GE( readClip.Tracks.NumElements( ), 1 );
    ASSERT_GE( sampleClip.Tracks.NumElements( ), 1 );
    const JointAnimTrack &readRootTrack   = readClip.Tracks.Elements[ 0 ];
    const JointAnimTrack &sampleRootTrack = sampleClip.Tracks.Elements[ 0 ];

    ASSERT_STREQ( readRootTrack.JointName.Get( ), sampleRootTrack.JointName.Get( ) );

    ASSERT_EQ( readRootTrack.PositionKeys.NumElements( ), sampleRootTrack.PositionKeys.NumElements( ) );
    for ( size_t i = 0; i < readRootTrack.PositionKeys.NumElements( ); ++i )
    {
        const PositionKey &readKey   = readRootTrack.PositionKeys.Elements[ i ];
        const PositionKey &sampleKey = sampleRootTrack.PositionKeys.Elements[ i ];
        ASSERT_FLOAT_EQ( readKey.Timestamp, sampleKey.Timestamp );
        ASSERT_TRUE( Float3Equals( readKey.Value, sampleKey.Value ) ); // Assumes Float3Equals exists
    }

    ASSERT_EQ( readRootTrack.RotationKeys.NumElements( ), sampleRootTrack.RotationKeys.NumElements( ) );
    for ( size_t i = 0; i < readRootTrack.RotationKeys.NumElements( ); ++i )
    {
        const RotationKey &readKey   = readRootTrack.RotationKeys.Elements[ i ];
        const RotationKey &sampleKey = sampleRootTrack.RotationKeys.Elements[ i ];
        ASSERT_FLOAT_EQ( readKey.Timestamp, sampleKey.Timestamp );
        ASSERT_TRUE( Float4Equals( readKey.Value, sampleKey.Value ) ); // Assumes Float4Equals exists
    }

    ASSERT_EQ( readRootTrack.ScaleKeys.NumElements( ), sampleRootTrack.ScaleKeys.NumElements( ) );
    for ( size_t i = 0; i < readRootTrack.ScaleKeys.NumElements( ); ++i )
    {
        const ScaleKey &readKey   = readRootTrack.ScaleKeys.Elements[ i ];
        const ScaleKey &sampleKey = sampleRootTrack.ScaleKeys.Elements[ i ];
        ASSERT_FLOAT_EQ( readKey.Timestamp, sampleKey.Timestamp );
        ASSERT_TRUE( Float3Equals( readKey.Value, sampleKey.Value ) );
    }

    ASSERT_GE( readClip.Tracks.NumElements( ), 2 );
    ASSERT_GE( sampleClip.Tracks.NumElements( ), 2 );
    const JointAnimTrack &readLegTrack   = readClip.Tracks.Elements[ 1 ];
    const JointAnimTrack &sampleLegTrack = sampleClip.Tracks.Elements[ 1 ];

    ASSERT_STREQ( readLegTrack.JointName.Get( ), sampleLegTrack.JointName.Get( ) );
    ASSERT_EQ( readLegTrack.PositionKeys.NumElements( ), sampleLegTrack.PositionKeys.NumElements( ) );
    ASSERT_EQ( readLegTrack.RotationKeys.NumElements( ), sampleLegTrack.RotationKeys.NumElements( ) );
    ASSERT_EQ( readLegTrack.ScaleKeys.NumElements( ), sampleLegTrack.ScaleKeys.NumElements( ) );
    ASSERT_EQ( readClip.MorphTracks.NumElements( ), sampleClip.MorphTracks.NumElements( ) );
    ASSERT_GE( readClip.MorphTracks.NumElements( ), 1 );
    ASSERT_GE( sampleClip.MorphTracks.NumElements( ), 1 );
    const MorphAnimTrack &readMorphTrack   = readClip.MorphTracks.Elements[ 0 ];
    const MorphAnimTrack &sampleMorphTrack = sampleClip.MorphTracks.Elements[ 0 ];

    ASSERT_STREQ( readMorphTrack.Name.Get( ), sampleMorphTrack.Name.Get( ) );
    ASSERT_EQ( readMorphTrack.Keyframes.NumElements( ), sampleMorphTrack.Keyframes.NumElements( ) );
    for ( size_t i = 0; i < readMorphTrack.Keyframes.NumElements( ); ++i )
    {
        const MorphKeyframe &readKeyframe   = readMorphTrack.Keyframes.Elements[ i ];
        const MorphKeyframe &sampleKeyframe = sampleMorphTrack.Keyframes.Elements[ i ];
        ASSERT_FLOAT_EQ( readKeyframe.Timestamp, sampleKeyframe.Timestamp );
        ASSERT_FLOAT_EQ( readKeyframe.Weight, sampleKeyframe.Weight );
    }

    ASSERT_GE( readAsset->Animations.NumElements( ), 2 );
    ASSERT_GE( sampleAsset->Animations.NumElements( ), 2 );
    const AnimationClip &readIdleClip   = readAsset->Animations.Elements[ 1 ];
    const AnimationClip &sampleIdleClip = sampleAsset->Animations.Elements[ 1 ];

    ASSERT_STREQ( readIdleClip.Name.Get( ), sampleIdleClip.Name.Get( ) );
    ASSERT_FLOAT_EQ( readIdleClip.Duration, sampleIdleClip.Duration );

    ASSERT_EQ( readIdleClip.Tracks.NumElements( ), sampleIdleClip.Tracks.NumElements( ) );
    ASSERT_GE( readIdleClip.Tracks.NumElements( ), 1 );
    ASSERT_GE( sampleIdleClip.Tracks.NumElements( ), 1 );
    const JointAnimTrack &readIdleTrack   = readIdleClip.Tracks.Elements[ 0 ];
    const JointAnimTrack &sampleIdleTrack = sampleIdleClip.Tracks.Elements[ 0 ];

    ASSERT_STREQ( readIdleTrack.JointName.Get( ), sampleIdleTrack.JointName.Get( ) );
    ASSERT_EQ( readIdleTrack.PositionKeys.NumElements( ), sampleIdleTrack.PositionKeys.NumElements( ) );
    ASSERT_EQ( readIdleTrack.RotationKeys.NumElements( ), sampleIdleTrack.RotationKeys.NumElements( ) );
    ASSERT_EQ( readIdleTrack.ScaleKeys.NumElements( ), sampleIdleTrack.ScaleKeys.NumElements( ) );
    ASSERT_EQ( readIdleClip.MorphTracks.NumElements( ), sampleIdleClip.MorphTracks.NumElements( ) );
    ASSERT_EQ( readIdleClip.MorphTracks.NumElements( ), 0 ); // Expecting 0 morph tracks for idle clip
}
