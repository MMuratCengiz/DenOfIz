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

#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetReader.h>
#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetWriter.h>
#include <DenOfIzGraphics/Assets/Stream/BinaryContainer.h>
#include "../../TestComparators.h"

using namespace DenOfIz;

class AnimationAssetSerdeTest : public testing::Test
{
protected:
    static AnimationAsset CreateSampleAnimationAsset( )
    {
        using namespace DenOfIz;
        AnimationAsset asset;
        asset.Name        = "TestAnimation";
        asset.Uri         = AssetUri::Create( "test/TestAnimation.dzanim" );
        asset.SkeletonRef = AssetUri::Create( "test/TestSkeleton.dzskel" );

        AnimationClip clip;
        clip.Name                           = "Walk";
        constexpr double walkTicksPerSecond = 30.0;
        clip.Duration                       = 1.0f; // Duration in seconds

        JointAnimTrack rootTrack;
        rootTrack.JointName = "Root";

        rootTrack.PositionKeys.AddElement( { 0.0f / static_cast<float>( walkTicksPerSecond ), { 0.0f, 0.0f, 0.0f } } );
        rootTrack.PositionKeys.AddElement( { 30.0f / static_cast<float>( walkTicksPerSecond ), { 1.0f, 0.0f, 0.0f } } ); // Time = 1.0s

        rootTrack.RotationKeys.AddElement( { 0.0f / static_cast<float>( walkTicksPerSecond ), { 0.0f, 0.0f, 0.0f, 1.0f } } );    // Identity quat
        rootTrack.RotationKeys.AddElement( { 30.0f / static_cast<float>( walkTicksPerSecond ), { 0.0f, 0.0f, 0.1f, 0.995f } } ); // Time = 1.0s

        rootTrack.ScaleKeys.AddElement( { 0.0f / static_cast<float>( walkTicksPerSecond ), { 1.0f, 1.0f, 1.0f } } );
        rootTrack.ScaleKeys.AddElement( { 30.0f / static_cast<float>( walkTicksPerSecond ), { 1.0f, 1.0f, 1.0f } } ); // Time = 1.0s

        clip.Tracks.AddElement( rootTrack );

        JointAnimTrack legTrack;
        legTrack.JointName = "LeftLeg";

        legTrack.PositionKeys.AddElement( { 0.0f / static_cast<float>( walkTicksPerSecond ), { 0.0f, -0.5f, 0.0f } } );
        legTrack.PositionKeys.AddElement( { 30.0f / static_cast<float>( walkTicksPerSecond ), { 0.0f, -0.5f, 0.5f } } ); // Time = 1.0s

        legTrack.RotationKeys.AddElement( { 0.0f / static_cast<float>( walkTicksPerSecond ), { 0.0f, 0.0f, 0.0f, 1.0f } } );    // Identity quat
        legTrack.RotationKeys.AddElement( { 30.0f / static_cast<float>( walkTicksPerSecond ), { 0.1f, 0.0f, 0.0f, 0.995f } } ); // Time = 1.0s

        legTrack.ScaleKeys.AddElement( { 0.0f / static_cast<float>( walkTicksPerSecond ), { 1.0f, 1.0f, 1.0f } } );

        clip.Tracks.AddElement( legTrack );

        MorphAnimTrack morphTrack;
        morphTrack.Name = "Smile";

        morphTrack.Keyframes.AddElement( { 0.0f / static_cast<float>( walkTicksPerSecond ), 0.0f } );
        morphTrack.Keyframes.AddElement( { 15.0f / static_cast<float>( walkTicksPerSecond ), 0.7f } ); // Time = 0.5s
        morphTrack.Keyframes.AddElement( { 30.0f / static_cast<float>( walkTicksPerSecond ), 0.0f } ); // Time = 1.0s

        clip.MorphTracks.AddElement( morphTrack );

        asset.Animations.AddElement( clip );

        AnimationClip idleClip;
        idleClip.Name     = "Idle";
        idleClip.Duration = 2.0f;
        JointAnimTrack idleTrack;
        idleTrack.JointName = "Root";

        idleTrack.PositionKeys.AddElement( { 0.0f, { 0.0f, 0.0f, 0.0f } } );
        idleTrack.RotationKeys.AddElement( { 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } } );
        idleTrack.ScaleKeys.AddElement( { 0.0f, { 1.0f, 1.0f, 1.0f } } );
        idleClip.Tracks.AddElement( idleTrack );
        asset.Animations.AddElement( idleClip );
        return asset;
    }
};

TEST_F( AnimationAssetSerdeTest, WriteAndReadBack )
{
    using namespace DenOfIz;

    BinaryContainer container;
    AnimationAsset  sampleAsset = CreateSampleAnimationAsset( );

    {
        BinaryWriter         binaryWriter( container );
        AnimationAssetWriter writer( AnimationAssetWriterDesc{ &binaryWriter } );
        writer.Write( sampleAsset );
    }
    BinaryReader         reader( container );
    AnimationAssetReader animReader( AnimationAssetReaderDesc{ &reader } );
    AnimationAsset       readAsset = animReader.Read( );

    ASSERT_EQ( readAsset.Magic, AnimationAsset{ }.Magic );
    ASSERT_EQ( readAsset.Version, AnimationAsset::Latest );
    ASSERT_STREQ( readAsset.Name.Get( ), sampleAsset.Name.Get( ) );
    ASSERT_STREQ( readAsset.Uri.ToString( ).Get( ), sampleAsset.Uri.ToString( ).Get( ) );
    ASSERT_STREQ( readAsset.SkeletonRef.ToString( ).Get( ), sampleAsset.SkeletonRef.ToString( ).Get( ) );

    ASSERT_EQ( readAsset.Animations.NumElements( ), sampleAsset.Animations.NumElements( ) );

    ASSERT_GE( readAsset.Animations.NumElements( ), 1 );
    ASSERT_GE( sampleAsset.Animations.NumElements( ), 1 );
    const AnimationClip &readClip   = readAsset.Animations.GetElement( 0 );
    const AnimationClip &sampleClip = sampleAsset.Animations.GetElement( 0 );

    ASSERT_STREQ( readClip.Name.Get( ), sampleClip.Name.Get( ) );
    ASSERT_FLOAT_EQ( readClip.Duration, sampleClip.Duration );

    ASSERT_EQ( readClip.Tracks.NumElements( ), sampleClip.Tracks.NumElements( ) );

    ASSERT_GE( readClip.Tracks.NumElements( ), 1 );
    ASSERT_GE( sampleClip.Tracks.NumElements( ), 1 );
    const JointAnimTrack &readRootTrack   = readClip.Tracks.GetElement( 0 );
    const JointAnimTrack &sampleRootTrack = sampleClip.Tracks.GetElement( 0 );

    ASSERT_STREQ( readRootTrack.JointName.Get( ), sampleRootTrack.JointName.Get( ) );

    ASSERT_EQ( readRootTrack.PositionKeys.NumElements( ), sampleRootTrack.PositionKeys.NumElements( ) );
    for ( size_t i = 0; i < readRootTrack.PositionKeys.NumElements( ); ++i )
    {
        const PositionKey &readKey   = readRootTrack.PositionKeys.GetElement( i );
        const PositionKey &sampleKey = sampleRootTrack.PositionKeys.GetElement( i );
        ASSERT_FLOAT_EQ( readKey.Timestamp, sampleKey.Timestamp );
        ASSERT_TRUE( Float3Equals( readKey.Value, sampleKey.Value ) ); // Assumes Float3Equals exists
    }

    ASSERT_EQ( readRootTrack.RotationKeys.NumElements( ), sampleRootTrack.RotationKeys.NumElements( ) );
    for ( size_t i = 0; i < readRootTrack.RotationKeys.NumElements( ); ++i )
    {
        const RotationKey &readKey   = readRootTrack.RotationKeys.GetElement( i );
        const RotationKey &sampleKey = sampleRootTrack.RotationKeys.GetElement( i );
        ASSERT_FLOAT_EQ( readKey.Timestamp, sampleKey.Timestamp );
        ASSERT_TRUE( Float4Equals( readKey.Value, sampleKey.Value ) ); // Assumes Float4Equals exists
    }

    ASSERT_EQ( readRootTrack.ScaleKeys.NumElements( ), sampleRootTrack.ScaleKeys.NumElements( ) );
    for ( size_t i = 0; i < readRootTrack.ScaleKeys.NumElements( ); ++i )
    {
        const ScaleKey &readKey   = readRootTrack.ScaleKeys.GetElement( i );
        const ScaleKey &sampleKey = sampleRootTrack.ScaleKeys.GetElement( i );
        ASSERT_FLOAT_EQ( readKey.Timestamp, sampleKey.Timestamp );
        ASSERT_TRUE( Float3Equals( readKey.Value, sampleKey.Value ) );
    }

    ASSERT_GE( readClip.Tracks.NumElements( ), 2 );
    ASSERT_GE( sampleClip.Tracks.NumElements( ), 2 );
    const JointAnimTrack &readLegTrack   = readClip.Tracks.GetElement( 1 );
    const JointAnimTrack &sampleLegTrack = sampleClip.Tracks.GetElement( 1 );

    ASSERT_STREQ( readLegTrack.JointName.Get( ), sampleLegTrack.JointName.Get( ) );
    ASSERT_EQ( readLegTrack.PositionKeys.NumElements( ), sampleLegTrack.PositionKeys.NumElements( ) );
    ASSERT_EQ( readLegTrack.RotationKeys.NumElements( ), sampleLegTrack.RotationKeys.NumElements( ) );
    ASSERT_EQ( readLegTrack.ScaleKeys.NumElements( ), sampleLegTrack.ScaleKeys.NumElements( ) );
    ASSERT_EQ( readClip.MorphTracks.NumElements( ), sampleClip.MorphTracks.NumElements( ) );
    ASSERT_GE( readClip.MorphTracks.NumElements( ), 1 );
    ASSERT_GE( sampleClip.MorphTracks.NumElements( ), 1 );
    const MorphAnimTrack &readMorphTrack   = readClip.MorphTracks.GetElement( 0 );
    const MorphAnimTrack &sampleMorphTrack = sampleClip.MorphTracks.GetElement( 0 );

    ASSERT_STREQ( readMorphTrack.Name.Get( ), sampleMorphTrack.Name.Get( ) );
    ASSERT_EQ( readMorphTrack.Keyframes.NumElements( ), sampleMorphTrack.Keyframes.NumElements( ) );
    for ( size_t i = 0; i < readMorphTrack.Keyframes.NumElements( ); ++i )
    {
        const MorphKeyframe &readKeyframe   = readMorphTrack.Keyframes.GetElement( i );
        const MorphKeyframe &sampleKeyframe = sampleMorphTrack.Keyframes.GetElement( i );
        ASSERT_FLOAT_EQ( readKeyframe.Timestamp, sampleKeyframe.Timestamp );
        ASSERT_FLOAT_EQ( readKeyframe.Weight, sampleKeyframe.Weight );
    }

    ASSERT_GE( readAsset.Animations.NumElements( ), 2 );
    ASSERT_GE( sampleAsset.Animations.NumElements( ), 2 );
    const AnimationClip &readIdleClip   = readAsset.Animations.GetElement( 1 );
    const AnimationClip &sampleIdleClip = sampleAsset.Animations.GetElement( 1 );

    ASSERT_STREQ( readIdleClip.Name.Get( ), sampleIdleClip.Name.Get( ) );
    ASSERT_FLOAT_EQ( readIdleClip.Duration, sampleIdleClip.Duration );

    ASSERT_EQ( readIdleClip.Tracks.NumElements( ), sampleIdleClip.Tracks.NumElements( ) );
    ASSERT_GE( readIdleClip.Tracks.NumElements( ), 1 );
    ASSERT_GE( sampleIdleClip.Tracks.NumElements( ), 1 );
    const JointAnimTrack &readIdleTrack   = readIdleClip.Tracks.GetElement( 0 );
    const JointAnimTrack &sampleIdleTrack = sampleIdleClip.Tracks.GetElement( 0 );

    ASSERT_STREQ( readIdleTrack.JointName.Get( ), sampleIdleTrack.JointName.Get( ) );
    ASSERT_EQ( readIdleTrack.PositionKeys.NumElements( ), sampleIdleTrack.PositionKeys.NumElements( ) );
    ASSERT_EQ( readIdleTrack.RotationKeys.NumElements( ), sampleIdleTrack.RotationKeys.NumElements( ) );
    ASSERT_EQ( readIdleTrack.ScaleKeys.NumElements( ), sampleIdleTrack.ScaleKeys.NumElements( ) );
    ASSERT_EQ( readIdleClip.MorphTracks.NumElements( ), sampleIdleClip.MorphTracks.NumElements( ) );
    ASSERT_EQ( readIdleClip.MorphTracks.NumElements( ), 0 ); // Expecting 0 morph tracks for idle clip
}
