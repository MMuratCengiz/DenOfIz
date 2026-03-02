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

#include "DenOfIzGraphics/Input/AudioSource.h"
#include "DenOfIzGraphicsInternal/Input/AudioClasses.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include <string>

#define SOURCE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::AudioSource, handle )
#define MIXER_IMPL( handle )  DENOFIZ_FROM_HANDLE( DenOfIz::AudioMixer, handle )

namespace
{
    DenOfIz_AudioFormat FromSDLAudioFormat( SDL_AudioFormat format )
    {
        switch ( format )
        {
        case SDL_AUDIO_U8:
            return DENOFIZ_AUDIO_FORMAT_U8;
        case SDL_AUDIO_S8:
            return DENOFIZ_AUDIO_FORMAT_S8;
        case SDL_AUDIO_S16:
            return DENOFIZ_AUDIO_FORMAT_S16;
        case SDL_AUDIO_S32:
            return DENOFIZ_AUDIO_FORMAT_S32;
        case SDL_AUDIO_F32:
            return DENOFIZ_AUDIO_FORMAT_F32;
        default:
            return DENOFIZ_AUDIO_FORMAT_UNKNOWN;
        }
    }
} // anonymous namespace

namespace DenOfIz
{
    AudioSource::AudioSource( MIX_Mixer *mixer, const char *filePath, const bool predecode )
    {
        m_audio = MIX_LoadAudio( mixer, filePath, predecode );
        if ( !m_audio )
        {
            spdlog::error( "Failed to load audio '{}': {}", filePath, SDL_GetError( ) );
        }
    }

    AudioSource::~AudioSource( )
    {
        MIX_DestroyAudio( m_audio );
    }

    int64_t AudioSource::GetDuration( ) const
    {
        const Sint64 frames = MIX_GetAudioDuration( m_audio );
        return static_cast<int64_t>( MIX_AudioFramesToMS( m_audio, frames ) );
    }

    bool AudioSource::GetFormat( SDL_AudioSpec *spec ) const
    {
        return MIX_GetAudioFormat( m_audio, spec );
    }

    const char *AudioSource::GetTitle( ) const
    {
        const SDL_PropertiesID props = MIX_GetAudioProperties( m_audio );
        return SDL_GetStringProperty( props, MIX_PROP_METADATA_TITLE_STRING, "" );
    }

    const char *AudioSource::GetArtist( ) const
    {
        const SDL_PropertiesID props = MIX_GetAudioProperties( m_audio );
        return SDL_GetStringProperty( props, MIX_PROP_METADATA_ARTIST_STRING, "" );
    }

    const char *AudioSource::GetAlbum( ) const
    {
        const SDL_PropertiesID props = MIX_GetAudioProperties( m_audio );
        return SDL_GetStringProperty( props, MIX_PROP_METADATA_ALBUM_STRING, "" );
    }
} // namespace DenOfIz

using namespace DenOfIz;

extern "C"
{

    DenOfIz_AudioSource DenOfIz_AudioSource_LoadFromFile( DenOfIz_AudioMixer mixer,
                                                          DenOfIz_StringView filePath,
                                                          bool predecode )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( mixer ) || !filePath.Chars || filePath.NumChars == 0 )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        auto *mixerImpl = MIXER_IMPL( mixer );
        std::string path( filePath.Chars, filePath.NumChars );

        auto *source = new AudioSource( mixerImpl->m_mixer, path.c_str( ), predecode );
        if ( !source->m_audio )
        {
            delete source;
            return DENOFIZ_NULL_HANDLE;
        }
        return DENOFIZ_TO_HANDLE( source );
    }

    void DenOfIz_AudioSource_Destroy( DenOfIz_AudioSource source )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( source ) )
        {
            return;
        }
        delete SOURCE_IMPL( source );
    }

    int64_t DenOfIz_AudioSource_GetDuration( DenOfIz_AudioSource source )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( source ) )
        {
            return 0;
        }
        return SOURCE_IMPL( source )->GetDuration( );
    }

    DenOfIz_AudioSpec DenOfIz_AudioSource_GetFormat( DenOfIz_AudioSource source )
    {
        DenOfIz_AudioSpec result = { DENOFIZ_AUDIO_FORMAT_UNKNOWN, 0, 0 };
        if ( !DENOFIZ_HANDLE_IS_VALID( source ) )
        {
            return result;
        }

        SDL_AudioSpec sdlSpec = { };
        if ( SOURCE_IMPL( source )->GetFormat( &sdlSpec ) )
        {
            result.Format   = FromSDLAudioFormat( sdlSpec.format );
            result.Channels = sdlSpec.channels;
            result.Freq     = sdlSpec.freq;
        }
        return result;
    }

    DenOfIz_StringView DenOfIz_AudioSource_GetTitle( DenOfIz_AudioSource source )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( source ) )
        {
            return { };
        }
        const char *title = SOURCE_IMPL( source )->GetTitle( );
        return { title, strlen( title ) };
    }

    DenOfIz_StringView DenOfIz_AudioSource_GetArtist( DenOfIz_AudioSource source )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( source ) )
        {
            return { };
        }
        const char *artist = SOURCE_IMPL( source )->GetArtist( );
        return { artist, strlen( artist ) };
    }

    DenOfIz_StringView DenOfIz_AudioSource_GetAlbum( DenOfIz_AudioSource source )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( source ) )
        {
            return { };
        }
        const char *album = SOURCE_IMPL( source )->GetAlbum( );
        return { album, strlen( album ) };
    }
}
