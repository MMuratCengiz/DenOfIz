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

#include "DenOfIzGraphics/Input/AudioMixer.h"
#include "DenOfIzGraphicsInternal/Input/AudioClasses.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include <atomic>

#define MIXER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::AudioMixer, handle )

namespace
{
    std::atomic<int> g_mixerRefCount{ 0 };

    SDL_AudioFormat ToSDLAudioFormat( DenOfIz_AudioFormat format )
    {
        switch ( format )
        {
        case DENOFIZ_AUDIO_FORMAT_U8:
            return SDL_AUDIO_U8;
        case DENOFIZ_AUDIO_FORMAT_S8:
            return SDL_AUDIO_S8;
        case DENOFIZ_AUDIO_FORMAT_S16:
            return SDL_AUDIO_S16;
        case DENOFIZ_AUDIO_FORMAT_S32:
            return SDL_AUDIO_S32;
        case DENOFIZ_AUDIO_FORMAT_F32:
            return SDL_AUDIO_F32;
        default:
            return SDL_AUDIO_F32;
        }
    }
} // anonymous namespace

namespace DenOfIz
{
    AudioMixer::AudioMixer( const SDL_AudioSpec *spec )
    {
        if ( g_mixerRefCount.fetch_add( 1 ) == 0 )
        {
            if ( !MIX_Init( ) )
            {
                spdlog::critical( "Failed to initialize SDL3_mixer: {}", SDL_GetError( ) );
            }
        }

        m_mixer = MIX_CreateMixerDevice( SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, spec );
        if ( !m_mixer )
        {
            spdlog::critical( "Failed to create audio mixer: {}", SDL_GetError( ) );
        }
    }

    AudioMixer::~AudioMixer( )
    {
        MIX_DestroyMixer( m_mixer );

        if ( g_mixerRefCount.fetch_sub( 1 ) == 1 )
        {
            MIX_Quit( );
        }
    }

    void AudioMixer::SetGain( const float gain )
    {
        MIX_SetMixerGain( m_mixer, gain );
    }

    float AudioMixer::GetGain( ) const
    {
        return MIX_GetMixerGain( m_mixer );
    }

    void AudioMixer::StopAll( const int32_t fadeOutMs )
    {
        MIX_StopAllTracks( m_mixer, static_cast<Sint64>( fadeOutMs ) );
    }

    void AudioMixer::PauseAll( )
    {
        MIX_PauseAllTracks( m_mixer );
    }

    void AudioMixer::ResumeAll( )
    {
        MIX_ResumeAllTracks( m_mixer );
    }
} // namespace DenOfIz

using namespace DenOfIz;

extern "C"
{

    DenOfIz_AudioMixer DenOfIz_AudioMixer_Create( const DenOfIz_AudioSpec *spec )
    {
        SDL_AudioSpec sdlSpec = { };
        if ( spec )
        {
            sdlSpec.format   = ToSDLAudioFormat( spec->Format );
            sdlSpec.channels = spec->Channels;
            sdlSpec.freq     = spec->Freq;
        }
        else
        {
            sdlSpec.format   = SDL_AUDIO_F32;
            sdlSpec.channels = 2;
            sdlSpec.freq     = 44100;
        }

        auto *mixer = new AudioMixer( &sdlSpec );
        if ( !mixer->m_mixer )
        {
            delete mixer;
            return DENOFIZ_NULL_HANDLE;
        }
        return DENOFIZ_TO_HANDLE( mixer );
    }

    void DenOfIz_AudioMixer_Destroy( DenOfIz_AudioMixer mixer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( mixer ) )
        {
            return;
        }
        delete MIXER_IMPL( mixer );
    }

    void DenOfIz_AudioMixer_SetGain( DenOfIz_AudioMixer mixer, float gain )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( mixer ) )
        {
            return;
        }
        MIXER_IMPL( mixer )->SetGain( gain );
    }

    float DenOfIz_AudioMixer_GetGain( DenOfIz_AudioMixer mixer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( mixer ) )
        {
            return 1.0f;
        }
        return MIXER_IMPL( mixer )->GetGain( );
    }

    void DenOfIz_AudioMixer_StopAll( DenOfIz_AudioMixer mixer, int32_t fadeOutMs )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( mixer ) )
        {
            return;
        }
        MIXER_IMPL( mixer )->StopAll( fadeOutMs );
    }

    void DenOfIz_AudioMixer_PauseAll( DenOfIz_AudioMixer mixer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( mixer ) )
        {
            return;
        }
        MIXER_IMPL( mixer )->PauseAll( );
    }

    void DenOfIz_AudioMixer_ResumeAll( DenOfIz_AudioMixer mixer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( mixer ) )
        {
            return;
        }
        MIXER_IMPL( mixer )->ResumeAll( );
    }

    int32_t DenOfIz_AudioMixer_GetNumDecoders( )
    {
        return MIX_GetNumAudioDecoders( );
    }

    DenOfIz_StringView DenOfIz_AudioMixer_GetDecoder( int32_t index )
    {
        const char *name = MIX_GetAudioDecoder( index );
        if ( name )
        {
            return { name, strlen( name ) };
        }
        return { };
    }
}
