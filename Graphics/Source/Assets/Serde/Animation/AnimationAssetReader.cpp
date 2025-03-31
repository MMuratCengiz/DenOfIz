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

#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetReader.h>
#include <DenOfIzGraphics/Assets/Serde/Common/AssetReaderHelpers.h>

using namespace DenOfIz;

AnimationAssetReader::AnimationAssetReader( const AnimationAssetReaderDesc &desc ) : m_reader( desc.Reader )
{
    if ( !m_reader )
    {
        LOG( FATAL ) << "BinaryReader cannot be null for AnimationAssetReader";
    }
}

AnimationAssetReader::~AnimationAssetReader( ) = default;

void AnimationAssetReader::Read( AnimationClip &animationClip ) const
{
    animationClip.Name           = m_reader->ReadString( );
    animationClip.Duration       = m_reader->ReadFloat( );
    animationClip.TicksPerSecond = static_cast<double>( m_reader->ReadFloat( ) );

    const uint32_t numJointTracks = m_reader->ReadUInt32( );
    animationClip.Tracks.Resize( numJointTracks );

    for ( uint32_t i = 0; i < numJointTracks; ++i )
    {
        JointAnimTrack &track = animationClip.Tracks.GetElement( i );
        track.JointName       = m_reader->ReadString( );

        const uint32_t numKeyframes = m_reader->ReadUInt32( );
        track.Keyframes.Resize( numKeyframes );

        for ( uint32_t j = 0; j < numKeyframes; ++j )
        {
            JointKeyframe &keyframe = track.Keyframes.GetElement( j );
            keyframe.Timestamp      = m_reader->ReadFloat( );
            keyframe.Pose.Position  = m_reader->ReadFloat_4( );
            keyframe.Pose.Rotation  = m_reader->ReadFloat_4( );
            keyframe.Pose.Scale     = m_reader->ReadFloat_4( );
        }
    }

    const uint32_t numMorphTracks = m_reader->ReadUInt32( );
    animationClip.MorphTracks.Resize( numMorphTracks );

    for ( uint32_t i = 0; i < numMorphTracks; ++i )
    {
        MorphAnimTrack &track = animationClip.MorphTracks.GetElement( i );
        track.Name            = m_reader->ReadString( );

        const uint32_t numKeyframes = m_reader->ReadUInt32( );
        track.Keyframes.Resize( numKeyframes );

        for ( uint32_t j = 0; j < numKeyframes; ++j )
        {
            MorphKeyframe &keyframe = track.Keyframes.GetElement( j );
            keyframe.Timestamp      = m_reader->ReadFloat( );
            keyframe.Weight         = m_reader->ReadFloat( );
        }
    }
}

AnimationAsset AnimationAssetReader::Read( )
{
    m_animationAsset       = { };
    m_animationAsset.Magic = m_reader->ReadUInt64( );
    if ( m_animationAsset.Magic != AnimationAsset{ }.Magic )
    {
        LOG( FATAL ) << "Invalid AnimationAsset magic number.";
    }

    m_animationAsset.Version = m_reader->ReadUInt32( );
    if ( m_animationAsset.Version > AnimationAsset::Latest )
    {
        LOG( WARNING ) << "AnimationAsset version mismatch.";
    }

    m_animationAsset.NumBytes = m_reader->ReadUInt64( );
    m_animationAsset.Uri      = AssetUri::Parse( m_reader->ReadString( ) );

    m_animationAsset.Name        = m_reader->ReadString( );
    m_animationAsset.SkeletonRef = AssetUri::Parse( m_reader->ReadString( ) );

    const uint32_t numAnimations = m_reader->ReadUInt32( );
    m_animationAsset.Animations.Resize( numAnimations );

    for ( uint32_t i = 0; i < numAnimations; ++i )
    {
        AnimationClip &clip = m_animationAsset.Animations.GetElement( i );
        Read( clip );
    }

    return m_animationAsset;
}
