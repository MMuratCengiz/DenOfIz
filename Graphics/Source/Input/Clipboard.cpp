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
