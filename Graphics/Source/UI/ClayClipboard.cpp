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

#include "DenOfIzGraphics/UI/ClayClipboard.h"
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

namespace DenOfIz
{
    void ClayClipboard::SetText( const InteropString &text )
    {
        if ( SDL_SetClipboardText( text.Get( ) ) != 0 )
        {
            spdlog::error( "Failed to set clipboard text: {}", SDL_GetError( ) );
        }
    }

    InteropString ClayClipboard::GetText( )
    {
        char *clipboardText = SDL_GetClipboardText( );
        if ( clipboardText == nullptr )
        {
            spdlog::error( "Failed to get clipboard text: {}", SDL_GetError( ) );
            return InteropString( "" );
        }

        InteropString result( clipboardText );
        SDL_free( clipboardText );
        return result;
    }

    bool ClayClipboard::HasText( )
    {
        return SDL_HasClipboardText( ) == SDL_TRUE;
    }

    void ClayClipboard::Clear( )
    {
        SDL_SetClipboardText( "" );
    }
} // namespace DenOfIz
