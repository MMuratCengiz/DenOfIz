/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphics/Input/Clipboard.h"
#include <string>
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

namespace
{
    std::string s_clipboardText;
}

extern "C"
{

    void DenOfIz_Clipboard_SetText( const DenOfIz_StringView text )
    {
        if ( text.Chars && text.NumChars > 0 )
        {
            s_clipboardText.assign( text.Chars, text.Chars + text.NumChars );
        }
        else
        {
            s_clipboardText.clear( );
        }

        if ( SDL_SetClipboardText( s_clipboardText.c_str( ) ) != 0 )
        {
            spdlog::error( "Failed to set clipboard text: {}", SDL_GetError( ) );
        }
    }

    DenOfIz_StringView DenOfIz_Clipboard_GetText( )
    {
        char *clipboardText = SDL_GetClipboardText( );
        if ( clipboardText == nullptr )
        {
            spdlog::error( "Failed to get clipboard text: {}", SDL_GetError( ) );
            return DENOFIZ_STRING( "" );
        }

        s_clipboardText = std::string( clipboardText );
        SDL_free( clipboardText );
        return DENOFIZ_STRING( s_clipboardText.c_str( ) );
    }

    bool DenOfIz_Clipboard_HasText( )
    {
        return SDL_HasClipboardText( );
    }

    void DenOfIz_Clipboard_Clear( )
    {
        SDL_SetClipboardText( "" );
    }
}
