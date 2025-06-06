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

#include "DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetWriter.h"
#include "DenOfIzGraphics/Assets/Serde/Common/AssetWriterHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

AnimationAssetWriter::AnimationAssetWriter( const AnimationAssetWriterDesc &desc ) : m_writer( desc.Writer ), m_streamStartOffset( 0 )
{
    if ( !m_writer )
    {
        spdlog::critical("BinaryWriter cannot be null for AnimationAssetWriter");
    }
}

AnimationAssetWriter::~AnimationAssetWriter( ) = default;

void AnimationAssetWriter::Write( const AnimationAsset &animationAsset )
{
    m_streamStartOffset = m_writer->Position( );
    m_writer->WriteUInt64( animationAsset.Magic );
    m_writer->WriteUInt32( animationAsset.Version );
    m_writer->WriteUInt64( animationAsset.NumBytes );
    m_writer->WriteString( animationAsset.Uri.ToInteropString( ) );

    m_writer->WriteString( animationAsset.Name );
    m_writer->WriteString( animationAsset.SkeletonRef.ToInteropString( ) );

    m_writer->WriteUInt32( animationAsset.Animations.NumElements( ) );
    for ( size_t i = 0; i < animationAsset.Animations.NumElements( ); ++i )
    {
        const AnimationClip &clip = animationAsset.Animations.GetElement( i );

        m_writer->WriteString( clip.Name );
        m_writer->WriteFloat( clip.Duration );

        m_writer->WriteUInt32( clip.Tracks.NumElements( ) );
        for ( size_t j = 0; j < clip.Tracks.NumElements( ); ++j )
        {
            const JointAnimTrack &track = clip.Tracks.GetElement( j );
            m_writer->WriteString( track.JointName );

            m_writer->WriteUInt32( track.PositionKeys.NumElements( ) );
            for ( size_t k = 0; k < track.PositionKeys.NumElements( ); ++k )
            {
                const PositionKey &key = track.PositionKeys.GetElement( k );
                m_writer->WriteFloat( key.Timestamp );
                m_writer->WriteFloat_3( key.Value );
            }

            m_writer->WriteUInt32( track.RotationKeys.NumElements( ) );
            for ( size_t k = 0; k < track.RotationKeys.NumElements( ); ++k )
            {
                const RotationKey &key = track.RotationKeys.GetElement( k );
                m_writer->WriteFloat( key.Timestamp );
                m_writer->WriteFloat_4( key.Value );
            }

            m_writer->WriteUInt32( track.ScaleKeys.NumElements( ) );
            for ( size_t k = 0; k < track.ScaleKeys.NumElements( ); ++k )
            {
                const ScaleKey &key = track.ScaleKeys.GetElement( k );
                m_writer->WriteFloat( key.Timestamp );
                m_writer->WriteFloat_3( key.Value );
            }
        }

        m_writer->WriteUInt32( clip.MorphTracks.NumElements( ) );
        for ( size_t j = 0; j < clip.MorphTracks.NumElements( ); ++j )
        {
            const MorphAnimTrack &track = clip.MorphTracks.GetElement( j );
            m_writer->WriteString( track.Name );

            m_writer->WriteUInt32( track.Keyframes.NumElements( ) );
            for ( size_t k = 0; k < track.Keyframes.NumElements( ); ++k )
            {
                const MorphKeyframe &keyframe = track.Keyframes.GetElement( k );
                m_writer->WriteFloat( keyframe.Timestamp );
                m_writer->WriteFloat( keyframe.Weight );
            }
        }
    }

    const auto currentPos = m_writer->Position( );
    m_writer->Seek( m_streamStartOffset + sizeof( uint64_t ) + sizeof( uint32_t ) );
    m_writer->WriteUInt64( currentPos - m_streamStartOffset );
    m_writer->Seek( currentPos );

    m_writer->Flush( );
}
