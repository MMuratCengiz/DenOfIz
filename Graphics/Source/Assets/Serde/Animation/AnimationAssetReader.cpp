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

#include "DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetReader.h"
#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetReaderHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

AnimationAssetReader::AnimationAssetReader( const AnimationAssetReaderDesc &desc ) : m_reader( desc.Reader )
{
    if ( !m_reader )
    {
        spdlog::critical( "BinaryReader cannot be null for AnimationAssetReader" );
    }
}

AnimationAssetReader::~AnimationAssetReader( ) = default;

void AnimationAssetReader::ReadAnimationClip( AnimationClip &animationClip ) const
{
    animationClip.Name     = m_reader->ReadString( );
    animationClip.Duration = m_reader->ReadFloat( );

    const uint32_t numJointTracks = m_reader->ReadUInt32( );
    DZArenaArrayHelper<JointAnimTrackArray, JointAnimTrack>::AllocateAndConstructArray( m_animationAsset->_Arena, animationClip.Tracks, numJointTracks );
    for ( uint32_t i = 0; i < numJointTracks; ++i )
    {
        JointAnimTrack &track = animationClip.Tracks.Elements[ i ];
        track.JointName       = m_reader->ReadString( );

        const uint32_t numPosKeys = m_reader->ReadUInt32( );
        DZArenaArrayHelper<PositionKeyArray, PositionKey>::AllocateAndConstructArray( m_animationAsset->_Arena, track.PositionKeys, numPosKeys );
        for ( uint32_t j = 0; j < numPosKeys; ++j )
        {
            PositionKey &key = track.PositionKeys.Elements[ j ];
            key.Timestamp    = m_reader->ReadFloat( ); // Already in seconds
            key.Value        = m_reader->ReadFloat_3( );
        }

        const uint32_t numRotKeys = m_reader->ReadUInt32( );
        DZArenaArrayHelper<RotationKeyArray, RotationKey>::AllocateAndConstructArray( m_animationAsset->_Arena, track.RotationKeys, numRotKeys );
        for ( uint32_t j = 0; j < numRotKeys; ++j )
        {
            RotationKey &key = track.RotationKeys.Elements[ j ];
            key.Timestamp    = m_reader->ReadFloat( );   // Already in seconds
            key.Value        = m_reader->ReadFloat_4( ); // Quaternion
        }

        const uint32_t numScaleKeys = m_reader->ReadUInt32( );
        DZArenaArrayHelper<ScaleKeyArray, ScaleKey>::AllocateAndConstructArray( m_animationAsset->_Arena, track.ScaleKeys, numScaleKeys );
        for ( uint32_t j = 0; j < numScaleKeys; ++j )
        {
            ScaleKey &key = track.ScaleKeys.Elements[ j ];
            key.Timestamp = m_reader->ReadFloat( ); // Already in seconds
            key.Value     = m_reader->ReadFloat_3( );
        }
    }

    const uint32_t numMorphTracks = m_reader->ReadUInt32( );
    DZArenaArrayHelper<MorphAnimTrackArray, MorphAnimTrack>::AllocateAndConstructArray( m_animationAsset->_Arena, animationClip.MorphTracks, numMorphTracks );
    for ( uint32_t i = 0; i < numMorphTracks; ++i )
    {
        MorphAnimTrack &track = animationClip.MorphTracks.Elements[ i ];
        track.Name            = m_reader->ReadString( );

        const uint32_t numKeyframes = m_reader->ReadUInt32( );
        DZArenaArrayHelper<MorphKeyframeArray, MorphKeyframe>::AllocateAndConstructArray( m_animationAsset->_Arena, track.Keyframes, numKeyframes );
        for ( uint32_t j = 0; j < numKeyframes; ++j )
        {
            MorphKeyframe &keyframe = track.Keyframes.Elements[ j ];
            keyframe.Timestamp      = m_reader->ReadFloat( ); // Already in seconds
            keyframe.Weight         = m_reader->ReadFloat( );
        }
    }
}

AnimationAsset *AnimationAssetReader::Read( )
{
    m_animationAsset        = new AnimationAsset( );
    m_animationAsset->Magic = m_reader->ReadUInt64( );
    if ( m_animationAsset->Magic != AnimationAsset{ }.Magic )
    {
        spdlog::critical( "Invalid AnimationAsset magic number." );
    }

    m_animationAsset->Version = m_reader->ReadUInt32( );
    if ( m_animationAsset->Version > AnimationAsset::Latest )
    {
        spdlog::warn( "AnimationAsset version mismatch (File: {} , Expected: {} ). Attempting to read...", m_animationAsset->Version, AnimationAsset::Latest );
    }

    m_animationAsset->NumBytes    = m_reader->ReadUInt64( );
    m_animationAsset->Uri         = AssetUri::Parse( m_reader->ReadString( ) );
    m_animationAsset->Name        = m_reader->ReadString( );
    m_animationAsset->SkeletonRef = AssetUri::Parse( m_reader->ReadString( ) );

    const uint32_t numAnimations = m_reader->ReadUInt32( );
    DZArenaArrayHelper<AnimationClipArray, AnimationClip>::AllocateAndConstructArray( m_animationAsset->_Arena, m_animationAsset->Animations, numAnimations );

    for ( uint32_t i = 0; i < numAnimations; ++i )
    {
        ReadAnimationClip( m_animationAsset->Animations.Elements[ i ] );
    }

    return m_animationAsset;
}
