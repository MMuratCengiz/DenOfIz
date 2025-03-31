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
        clip.Name           = "Walk";
        clip.Duration       = 1.0f;
        clip.TicksPerSecond = 30.0;

        JointAnimTrack rootTrack;
        rootTrack.JointName = "Root";

        JointKeyframe rootKey1{ };
        rootKey1.Timestamp     = 0.0f;
        rootKey1.Pose.Position = { 0.0f, 0.0f, 0.0f, 1.0f };
        rootKey1.Pose.Rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
        rootKey1.Pose.Scale    = { 1.0f, 1.0f, 1.0f, 0.0f };
        rootTrack.Keyframes.AddElement( rootKey1 );

        JointKeyframe rootKey2{ };
        rootKey2.Timestamp     = 1.0f;
        rootKey2.Pose.Position = { 1.0f, 0.0f, 0.0f, 1.0f };
        rootKey2.Pose.Rotation = { 0.0f, 0.0f, 0.1f, 0.995f };
        rootKey2.Pose.Scale    = { 1.0f, 1.0f, 1.0f, 0.0f };
        rootTrack.Keyframes.AddElement( rootKey2 );

        clip.Tracks.AddElement( rootTrack );

        JointAnimTrack legTrack;
        legTrack.JointName = "LeftLeg";

        JointKeyframe legKey1{ };
        legKey1.Timestamp     = 0.0f;
        legKey1.Pose.Position = { 0.0f, -0.5f, 0.0f, 1.0f };
        legKey1.Pose.Rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
        legKey1.Pose.Scale    = { 1.0f, 1.0f, 1.0f, 0.0f };
        legTrack.Keyframes.AddElement( legKey1 );

        JointKeyframe legKey2{ };
        legKey2.Timestamp     = 1.0f;
        legKey2.Pose.Position = { 0.0f, -0.5f, 0.5f, 1.0f };
        legKey2.Pose.Rotation = { 0.1f, 0.0f, 0.0f, 0.995f };
        legKey2.Pose.Scale    = { 1.0f, 1.0f, 1.0f, 0.0f };
        legTrack.Keyframes.AddElement( legKey2 );

        clip.Tracks.AddElement( legTrack );

        MorphAnimTrack morphTrack;
        morphTrack.Name = "Smile";

        MorphKeyframe morphKey1{ };
        morphKey1.Timestamp = 0.0f;
        morphKey1.Weight    = 0.0f;
        morphTrack.Keyframes.AddElement( morphKey1 );

        MorphKeyframe morphKey2{ };
        morphKey2.Timestamp = 0.5f;
        morphKey2.Weight    = 0.7f;
        morphTrack.Keyframes.AddElement( morphKey2 );

        MorphKeyframe morphKey3{ };
        morphKey3.Timestamp = 1.0f;
        morphKey3.Weight    = 0.0f;
        morphTrack.Keyframes.AddElement( morphKey3 );

        clip.MorphTracks.AddElement( morphTrack );

        asset.Animations.AddElement( clip );

        AnimationClip idleClip;
        idleClip.Name           = "Idle";
        idleClip.Duration       = 2.0f;
        idleClip.TicksPerSecond = 15.0;

        JointAnimTrack idleTrack;
        idleTrack.JointName = "Root";

        JointKeyframe idleKey{ };
        idleKey.Timestamp     = 0.0f;
        idleKey.Pose.Position = { 0.0f, 0.0f, 0.0f, 1.0f };
        idleKey.Pose.Rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
        idleKey.Pose.Scale    = { 1.0f, 1.0f, 1.0f, 0.0f };
        idleTrack.Keyframes.AddElement( idleKey );

        idleClip.Tracks.AddElement( idleTrack );

        asset.Animations.AddElement( idleClip );

        return asset;
    }
};

TEST_F( AnimationAssetSerdeTest, WriteAndReadBack )
{
    using namespace DenOfIz;

    BinaryContainer container;
    BinaryWriter    binaryWriter( container );
    AnimationAsset  sampleAsset = CreateSampleAnimationAsset( );

    {
        AnimationAssetWriter writer( AnimationAssetWriterDesc{ &binaryWriter } );
        writer.Write( sampleAsset );
    }

    BinaryReader         reader( container );
    AnimationAssetReader animReader( AnimationAssetReaderDesc{ &reader } );

    AnimationAsset readAsset = animReader.Read( );

    ASSERT_EQ( readAsset.Magic, AnimationAsset{ }.Magic );
    ASSERT_EQ( readAsset.Version, AnimationAsset::Latest );
    ASSERT_STREQ( readAsset.Name.Get( ), sampleAsset.Name.Get( ) );
    ASSERT_STREQ( readAsset.Uri.ToString( ).Get( ), sampleAsset.Uri.ToString( ).Get( ) );
    ASSERT_STREQ( readAsset.SkeletonRef.ToString( ).Get( ), sampleAsset.SkeletonRef.ToString( ).Get( ) );

    ASSERT_EQ( readAsset.Animations.NumElements( ), sampleAsset.Animations.NumElements( ) );

    const AnimationClip &readClip   = readAsset.Animations.GetElement( 0 );
    const AnimationClip &sampleClip = sampleAsset.Animations.GetElement( 0 );

    ASSERT_STREQ( readClip.Name.Get( ), sampleClip.Name.Get( ) );
    ASSERT_FLOAT_EQ( readClip.Duration, sampleClip.Duration );
    ASSERT_DOUBLE_EQ( readClip.TicksPerSecond, sampleClip.TicksPerSecond );

    ASSERT_EQ( readClip.Tracks.NumElements( ), sampleClip.Tracks.NumElements( ) );

    const JointAnimTrack &readRootTrack   = readClip.Tracks.GetElement( 0 );
    const JointAnimTrack &sampleRootTrack = sampleClip.Tracks.GetElement( 0 );

    ASSERT_STREQ( readRootTrack.JointName.Get( ), sampleRootTrack.JointName.Get( ) );
    ASSERT_EQ( readRootTrack.Keyframes.NumElements( ), sampleRootTrack.Keyframes.NumElements( ) );

    for ( size_t i = 0; i < readRootTrack.Keyframes.NumElements( ); ++i )
    {
        const JointKeyframe &readKeyframe   = readRootTrack.Keyframes.GetElement( i );
        const JointKeyframe &sampleKeyframe = sampleRootTrack.Keyframes.GetElement( i );

        ASSERT_FLOAT_EQ( readKeyframe.Timestamp, sampleKeyframe.Timestamp );
        ASSERT_TRUE( Float4Equals( readKeyframe.Pose.Position, sampleKeyframe.Pose.Position ) );
        ASSERT_TRUE( Float4Equals( readKeyframe.Pose.Rotation, sampleKeyframe.Pose.Rotation ) );
        ASSERT_TRUE( Float4Equals( readKeyframe.Pose.Scale, sampleKeyframe.Pose.Scale ) );
    }

    const JointAnimTrack &readLegTrack   = readClip.Tracks.GetElement( 1 );
    const JointAnimTrack &sampleLegTrack = sampleClip.Tracks.GetElement( 1 );

    ASSERT_STREQ( readLegTrack.JointName.Get( ), sampleLegTrack.JointName.Get( ) );
    ASSERT_EQ( readLegTrack.Keyframes.NumElements( ), sampleLegTrack.Keyframes.NumElements( ) );

    ASSERT_EQ( readClip.MorphTracks.NumElements( ), sampleClip.MorphTracks.NumElements( ) );

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

    const AnimationClip &readIdleClip   = readAsset.Animations.GetElement( 1 );
    const AnimationClip &sampleIdleClip = sampleAsset.Animations.GetElement( 1 );

    ASSERT_STREQ( readIdleClip.Name.Get( ), sampleIdleClip.Name.Get( ) );
    ASSERT_FLOAT_EQ( readIdleClip.Duration, sampleIdleClip.Duration );
    ASSERT_DOUBLE_EQ( readIdleClip.TicksPerSecond, sampleIdleClip.TicksPerSecond );

    ASSERT_EQ( readIdleClip.Tracks.NumElements( ), sampleIdleClip.Tracks.NumElements( ) );

    const JointAnimTrack &readIdleTrack   = readIdleClip.Tracks.GetElement( 0 );
    const JointAnimTrack &sampleIdleTrack = sampleIdleClip.Tracks.GetElement( 0 );

    ASSERT_STREQ( readIdleTrack.JointName.Get( ), sampleIdleTrack.JointName.Get( ) );
    ASSERT_EQ( readIdleTrack.Keyframes.NumElements( ), sampleIdleTrack.Keyframes.NumElements( ) );
}
