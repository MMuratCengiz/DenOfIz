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

#include "DenOfIzGraphics/Input/AudioTrack.h"
#include "DenOfIzGraphicsInternal/Input/AudioClasses.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include <string>

#define TRACK_IMPL( handle )  DENOFIZ_FROM_HANDLE( DenOfIz::AudioTrack, handle )
#define MIXER_IMPL( handle )  DENOFIZ_FROM_HANDLE( DenOfIz::AudioMixer, handle )
#define SOURCE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::AudioSource, handle )

namespace DenOfIz
{
    AudioTrack::AudioTrack( MIX_Mixer *mixer )
    {
        m_track = MIX_CreateTrack( mixer );
        if ( !m_track )
        {
            spdlog::error( "Failed to create audio track: {}", SDL_GetError( ) );
        }
    }

    AudioTrack::~AudioTrack( )
    {
        MIX_DestroyTrack( m_track );
    }

    bool AudioTrack::SetAudio( MIX_Audio *audio )
    {
        return MIX_SetTrackAudio( m_track, audio );
    }

    bool AudioTrack::Play( const int32_t loops, const int32_t fadeInMs, const int64_t startMs, const int64_t maxDurationMs )
    {
        const bool hasOptions = loops != 0 || fadeInMs > 0 || startMs > 0 || maxDurationMs > 0;
        if ( !hasOptions )
        {
            return MIX_PlayTrack( m_track, 0 );
        }

        const SDL_PropertiesID props = SDL_CreateProperties( );

        if ( loops != 0 )
        {
            SDL_SetNumberProperty( props, MIX_PROP_PLAY_LOOPS_NUMBER, loops );
        }
        if ( fadeInMs > 0 )
        {
            SDL_SetNumberProperty( props, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, fadeInMs );
        }
        if ( startMs > 0 )
        {
            SDL_SetNumberProperty( props, MIX_PROP_PLAY_START_MILLISECOND_NUMBER, startMs );
        }
        if ( maxDurationMs > 0 )
        {
            SDL_SetNumberProperty( props, MIX_PROP_PLAY_MAX_MILLISECONDS_NUMBER, maxDurationMs );
        }

        const bool result = MIX_PlayTrack( m_track, props );
        SDL_DestroyProperties( props );
        return result;
    }

    bool AudioTrack::Stop( const int32_t fadeOutMs )
    {
        const Sint64 fadeFrames = fadeOutMs > 0 ? MIX_TrackMSToFrames( m_track, fadeOutMs ) : 0;
        return MIX_StopTrack( m_track, fadeFrames );
    }

    bool AudioTrack::Pause( )
    {
        return MIX_PauseTrack( m_track );
    }

    bool AudioTrack::Resume( )
    {
        return MIX_ResumeTrack( m_track );
    }

    bool AudioTrack::IsPlaying( ) const
    {
        return MIX_TrackPlaying( m_track );
    }

    bool AudioTrack::IsPaused( ) const
    {
        return MIX_TrackPaused( m_track );
    }

    void AudioTrack::SetGain( const float gain )
    {
        MIX_SetTrackGain( m_track, gain );
    }

    float AudioTrack::GetGain( ) const
    {
        return MIX_GetTrackGain( m_track );
    }

    void AudioTrack::SetLoops( const int32_t loops )
    {
        MIX_SetTrackLoops( m_track, loops );
    }

    int32_t AudioTrack::GetLoops( ) const
    {
        return MIX_GetTrackLoops( m_track );
    }

    bool AudioTrack::SetPositionMs( const int64_t ms )
    {
        const Sint64 frames = MIX_TrackMSToFrames( m_track, ms );
        return MIX_SetTrackPlaybackPosition( m_track, frames );
    }

    int64_t AudioTrack::GetPositionMs( ) const
    {
        const Sint64 frames = MIX_GetTrackPlaybackPosition( m_track );
        return static_cast<int64_t>( MIX_TrackFramesToMS( m_track, frames ) );
    }

    int64_t AudioTrack::GetRemainingMs( ) const
    {
        const Sint64 frames = MIX_GetTrackRemaining( m_track );
        return static_cast<int64_t>( MIX_TrackFramesToMS( m_track, frames ) );
    }

    bool AudioTrack::Tag( const char *tag )
    {
        return MIX_TagTrack( m_track, tag );
    }

    void AudioTrack::Untag( const char *tag )
    {
        MIX_UntagTrack( m_track, tag );
    }
} // namespace DenOfIz

using namespace DenOfIz;

extern "C"
{

    DenOfIz_AudioTrack DenOfIz_AudioTrack_Create( DenOfIz_AudioMixer mixer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( mixer ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        auto *mixerImpl = MIXER_IMPL( mixer );
        auto *track     = new AudioTrack( mixerImpl->m_mixer );
        if ( !track->m_track )
        {
            delete track;
            return DENOFIZ_NULL_HANDLE;
        }
        return DENOFIZ_TO_HANDLE( track );
    }

    void DenOfIz_AudioTrack_Destroy( DenOfIz_AudioTrack track )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) )
        {
            return;
        }
        delete TRACK_IMPL( track );
    }

    bool DenOfIz_AudioTrack_SetAudio( DenOfIz_AudioTrack track, DenOfIz_AudioSource source )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) || !DENOFIZ_HANDLE_IS_VALID( source ) )
        {
            return false;
        }
        return TRACK_IMPL( track )->SetAudio( SOURCE_IMPL( source )->m_audio );
    }

    bool DenOfIz_AudioTrack_Play( DenOfIz_AudioTrack track, const DenOfIz_AudioPlayDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) )
        {
            return false;
        }
        if ( desc )
        {
            return TRACK_IMPL( track )->Play( desc->Loops, desc->FadeInMs, desc->StartMs, desc->MaxDurationMs );
        }
        return TRACK_IMPL( track )->Play( 0, 0, 0, 0 );
    }

    bool DenOfIz_AudioTrack_Stop( DenOfIz_AudioTrack track, int32_t fadeOutMs )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) )
        {
            return false;
        }
        return TRACK_IMPL( track )->Stop( fadeOutMs );
    }

    bool DenOfIz_AudioTrack_Pause( DenOfIz_AudioTrack track )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) )
        {
            return false;
        }
        return TRACK_IMPL( track )->Pause( );
    }

    bool DenOfIz_AudioTrack_Resume( DenOfIz_AudioTrack track )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) )
        {
            return false;
        }
        return TRACK_IMPL( track )->Resume( );
    }

    bool DenOfIz_AudioTrack_IsPlaying( DenOfIz_AudioTrack track )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) )
        {
            return false;
        }
        return TRACK_IMPL( track )->IsPlaying( );
    }

    bool DenOfIz_AudioTrack_IsPaused( DenOfIz_AudioTrack track )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) )
        {
            return false;
        }
        return TRACK_IMPL( track )->IsPaused( );
    }

    void DenOfIz_AudioTrack_SetGain( DenOfIz_AudioTrack track, float gain )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) )
        {
            return;
        }
        TRACK_IMPL( track )->SetGain( gain );
    }

    float DenOfIz_AudioTrack_GetGain( DenOfIz_AudioTrack track )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) )
        {
            return 1.0f;
        }
        return TRACK_IMPL( track )->GetGain( );
    }

    void DenOfIz_AudioTrack_SetLoops( DenOfIz_AudioTrack track, int32_t loops )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) )
        {
            return;
        }
        TRACK_IMPL( track )->SetLoops( loops );
    }

    int32_t DenOfIz_AudioTrack_GetLoops( DenOfIz_AudioTrack track )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) )
        {
            return 0;
        }
        return TRACK_IMPL( track )->GetLoops( );
    }

    bool DenOfIz_AudioTrack_SetPositionMs( DenOfIz_AudioTrack track, int64_t ms )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) )
        {
            return false;
        }
        return TRACK_IMPL( track )->SetPositionMs( ms );
    }

    int64_t DenOfIz_AudioTrack_GetPositionMs( DenOfIz_AudioTrack track )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) )
        {
            return 0;
        }
        return TRACK_IMPL( track )->GetPositionMs( );
    }

    int64_t DenOfIz_AudioTrack_GetRemainingMs( DenOfIz_AudioTrack track )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) )
        {
            return 0;
        }
        return TRACK_IMPL( track )->GetRemainingMs( );
    }

    bool DenOfIz_AudioTrack_Tag( DenOfIz_AudioTrack track, DenOfIz_StringView tag )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) || !tag.Chars || tag.NumChars == 0 )
        {
            return false;
        }
        std::string tagStr( tag.Chars, tag.NumChars );
        return TRACK_IMPL( track )->Tag( tagStr.c_str( ) );
    }

    void DenOfIz_AudioTrack_Untag( DenOfIz_AudioTrack track, DenOfIz_StringView tag )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( track ) || !tag.Chars || tag.NumChars == 0 )
        {
            return;
        }
        std::string tagStr( tag.Chars, tag.NumChars );
        TRACK_IMPL( track )->Untag( tagStr.c_str( ) );
    }
}
