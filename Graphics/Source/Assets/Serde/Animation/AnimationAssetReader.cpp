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

using namespace DenOfIz;

void AnimationAssetReader::ReadAnimationData( AnimationClip &animationClip ) const
{
    animationClip.Name           = m_reader->ReadString( );
    animationClip.Duration       = m_reader->ReadFloat( );
    animationClip.TicksPerSecond = m_reader->ReadFloat( );

    // Read animation tracks
    const uint32_t trackCount = m_reader->ReadUInt32( );
    animationClip.Tracks      = InteropArray<JointAnimTrack>( trackCount );

    for ( uint32_t i = 0; i < trackCount; i++ )
    {
        JointAnimTrack &track = animationClip.Tracks.GetElement( i );
        track.JointName       = m_reader->ReadString( );

        // Read keyframes
        const uint32_t keyframeCount = m_reader->ReadUInt32( );
        track.Keyframes              = InteropArray<JointKeyframe>( keyframeCount );

        for ( uint32_t j = 0; j < keyframeCount; j++ )
        {
            JointKeyframe &keyframe = track.Keyframes.GetElement( j );
            keyframe.Timestamp      = m_reader->ReadFloat( );

            // Read position
            keyframe.Pose.Position.X = m_reader->ReadFloat( );
            keyframe.Pose.Position.Y = m_reader->ReadFloat( );
            keyframe.Pose.Position.Z = m_reader->ReadFloat( );
            keyframe.Pose.Position.W = m_reader->ReadFloat( );

            // Read rotation
            keyframe.Pose.Rotation.X = m_reader->ReadFloat( );
            keyframe.Pose.Rotation.Y = m_reader->ReadFloat( );
            keyframe.Pose.Rotation.Z = m_reader->ReadFloat( );
            keyframe.Pose.Rotation.W = m_reader->ReadFloat( );

            // Read scale
            keyframe.Pose.Scale.X = m_reader->ReadFloat( );
            keyframe.Pose.Scale.Y = m_reader->ReadFloat( );
            keyframe.Pose.Scale.Z = m_reader->ReadFloat( );
            keyframe.Pose.Scale.W = m_reader->ReadFloat( );
        }
    }
}