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

#pragma once

#include <SDL3_mixer/SDL_mixer.h>
#include <string>

namespace DenOfIz
{
    class AudioMixer
    {
    public:
        MIX_Mixer *m_mixer;

        explicit AudioMixer( const SDL_AudioSpec *spec );
        ~AudioMixer( );

        void  SetGain( float gain );
        float GetGain( ) const;
        void  StopAll( int32_t fadeOutMs );
        void  PauseAll( );
        void  ResumeAll( );
    };

    class AudioSource
    {
    public:
        MIX_Audio *m_audio;

        AudioSource( MIX_Mixer *mixer, const char *filePath, bool predecode );
        ~AudioSource( );

        int64_t GetDuration( ) const;
        bool    GetFormat( SDL_AudioSpec *spec ) const;

        const char *GetTitle( ) const;
        const char *GetArtist( ) const;
        const char *GetAlbum( ) const;
    };

    class AudioTrack
    {
    public:
        MIX_Track *m_track;

        explicit AudioTrack( MIX_Mixer *mixer );
        ~AudioTrack( );

        bool SetAudio( MIX_Audio *audio );

        bool Play( int32_t loops, int32_t fadeInMs, int64_t startMs, int64_t maxDurationMs );
        bool Stop( int32_t fadeOutMs );
        bool Pause( );
        bool Resume( );

        bool IsPlaying( ) const;
        bool IsPaused( ) const;

        void  SetGain( float gain );
        float GetGain( ) const;

        void    SetLoops( int32_t loops );
        int32_t GetLoops( ) const;

        bool    SetPositionMs( int64_t ms );
        int64_t GetPositionMs( ) const;
        int64_t GetRemainingMs( ) const;

        bool Tag( const char *tag );
        void Untag( const char *tag );
    };
} // namespace DenOfIz
