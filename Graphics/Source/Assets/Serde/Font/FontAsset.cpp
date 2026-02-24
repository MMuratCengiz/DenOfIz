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

#include "DenOfIzGraphics/Assets/Serde/Font/FontAsset.h"
#include "DenOfIzGraphicsInternal/Assets/Font/EmbeddedFonts.h"

#include <cstring>
#include <string>
#include <vector>

#define FONT_ASSET_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::FontAsset, handle )

namespace DenOfIz
{
    struct FontAsset
    {
        std::string                    Path;
        std::vector<Byte>              Data;
        std::vector<DenOfIz_FontGlyph> Glyphs;
        std::vector<Byte>              AtlasData;

        uint64_t                 Magic            = 0x544E4F465A44;
        uint32_t                 Version          = 2;
        uint64_t                 NumBytes         = 0;
        uint32_t                 InitialFontSize  = 36;
        DenOfIz_AntiAliasingMode AntiAliasingMode = DENOFIZ_ANTI_ALIASING_MODE_NONE;
        uint32_t                 AtlasWidth       = 512;
        uint32_t                 AtlasHeight      = 512;
        DenOfIz_FontMetrics      Metrics          = { };
    };
} // namespace DenOfIz

extern "C"
{
    DenOfIz_FontAsset DenOfIz_FontAsset_Create( )
    {
        auto *fontAsset = new DenOfIz::FontAsset( );
        return DENOFIZ_TO_HANDLE( fontAsset );
    }

    void DenOfIz_FontAsset_Destroy( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return;
        }
        delete FONT_ASSET_IMPL( fontAsset );
    }

    uint64_t DenOfIz_FontAsset_Magic( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return 0;
        }
        return FONT_ASSET_IMPL( fontAsset )->Magic;
    }

    uint32_t DenOfIz_FontAsset_Version( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return 0;
        }
        return FONT_ASSET_IMPL( fontAsset )->Version;
    }

    uint64_t DenOfIz_FontAsset_NumBytes( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return 0;
        }
        return FONT_ASSET_IMPL( fontAsset )->NumBytes;
    }

    DenOfIz_StringView DenOfIz_FontAsset_Path( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return { };
        }
        DenOfIz::FontAsset *impl = FONT_ASSET_IMPL( fontAsset );
        return { impl->Path.c_str( ), impl->Path.size( ) };
    }

    DenOfIz_ByteArray DenOfIz_FontAsset_Data( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return { };
        }
        DenOfIz::FontAsset *impl = FONT_ASSET_IMPL( fontAsset );
        return { impl->Data.data( ), impl->Data.size( ) };
    }

    uint32_t DenOfIz_FontAsset_InitialFontSize( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return 0;
        }
        return FONT_ASSET_IMPL( fontAsset )->InitialFontSize;
    }

    DenOfIz_AntiAliasingMode DenOfIz_FontAsset_GetAntiAliasingMode( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return DENOFIZ_ANTI_ALIASING_MODE_NONE;
        }
        return FONT_ASSET_IMPL( fontAsset )->AntiAliasingMode;
    }

    uint32_t DenOfIz_FontAsset_AtlasWidth( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return 0;
        }
        return FONT_ASSET_IMPL( fontAsset )->AtlasWidth;
    }

    uint32_t DenOfIz_FontAsset_AtlasHeight( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return 0;
        }
        return FONT_ASSET_IMPL( fontAsset )->AtlasHeight;
    }

    DenOfIz_FontMetrics DenOfIz_FontAsset_GetMetrics( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return { };
        }
        return FONT_ASSET_IMPL( fontAsset )->Metrics;
    }

    DenOfIz_FontGlyphArray DenOfIz_FontAsset_Glyphs( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return { };
        }
        DenOfIz::FontAsset *impl = FONT_ASSET_IMPL( fontAsset );
        return { impl->Glyphs.data( ), impl->Glyphs.size( ) };
    }

    size_t DenOfIz_FontAsset_NumGlyphs( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return 0;
        }
        return FONT_ASSET_IMPL( fontAsset )->Glyphs.size( );
    }

    DenOfIz_ByteArray DenOfIz_FontAsset_AtlasData( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return { };
        }
        DenOfIz::FontAsset *impl = FONT_ASSET_IMPL( fontAsset );
        return { impl->AtlasData.data( ), impl->AtlasData.size( ) };
    }

    void DenOfIz_FontAsset_SetVersion( DenOfIz_FontAsset fontAsset, uint32_t version )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return;
        }
        FONT_ASSET_IMPL( fontAsset )->Version = version;
    }

    void DenOfIz_FontAsset_SetNumBytes( DenOfIz_FontAsset fontAsset, uint64_t numBytes )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return;
        }
        FONT_ASSET_IMPL( fontAsset )->NumBytes = numBytes;
    }

    void DenOfIz_FontAsset_SetPath( DenOfIz_FontAsset fontAsset, DenOfIz_StringView path )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return;
        }
        FONT_ASSET_IMPL( fontAsset )->Path = std::string( path.Chars, path.NumChars );
    }

    void DenOfIz_FontAsset_SetData( DenOfIz_FontAsset fontAsset, const Byte *data, size_t numBytes )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return;
        }
        FONT_ASSET_IMPL( fontAsset )->Data.assign( data, data + numBytes );
    }

    void DenOfIz_FontAsset_SetInitialFontSize( DenOfIz_FontAsset fontAsset, uint32_t fontSize )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return;
        }
        FONT_ASSET_IMPL( fontAsset )->InitialFontSize = fontSize;
    }

    void DenOfIz_FontAsset_SetAntiAliasingMode( DenOfIz_FontAsset fontAsset, DenOfIz_AntiAliasingMode mode )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return;
        }
        FONT_ASSET_IMPL( fontAsset )->AntiAliasingMode = mode;
    }

    void DenOfIz_FontAsset_SetAtlasWidth( DenOfIz_FontAsset fontAsset, uint32_t width )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return;
        }
        FONT_ASSET_IMPL( fontAsset )->AtlasWidth = width;
    }

    void DenOfIz_FontAsset_SetAtlasHeight( DenOfIz_FontAsset fontAsset, uint32_t height )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return;
        }
        FONT_ASSET_IMPL( fontAsset )->AtlasHeight = height;
    }

    void DenOfIz_FontAsset_SetMetrics( DenOfIz_FontAsset fontAsset, const DenOfIz_FontMetrics *metrics )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) || metrics == NULL )
        {
            return;
        }
        FONT_ASSET_IMPL( fontAsset )->Metrics = *metrics;
    }

    void DenOfIz_FontAsset_SetAtlasData( DenOfIz_FontAsset fontAsset, const Byte *data, size_t numBytes )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return;
        }
        FONT_ASSET_IMPL( fontAsset )->AtlasData.assign( data, data + numBytes );
    }

    void DenOfIz_FontAsset_AddGlyph( DenOfIz_FontAsset fontAsset, const DenOfIz_FontGlyph *glyph )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) || glyph == NULL )
        {
            return;
        }
        FONT_ASSET_IMPL( fontAsset )->Glyphs.push_back( *glyph );
    }

    void DenOfIz_FontAsset_SetGlyphs( DenOfIz_FontAsset fontAsset, const DenOfIz_FontGlyph *glyphs, size_t count )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) || glyphs == NULL )
        {
            return;
        }
        FONT_ASSET_IMPL( fontAsset )->Glyphs.assign( glyphs, glyphs + count );
    }

    void DenOfIz_FontAsset_ReserveGlyphs( DenOfIz_FontAsset fontAsset, size_t capacity )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return;
        }
        FONT_ASSET_IMPL( fontAsset )->Glyphs.reserve( capacity );
    }

    void DenOfIz_FontAsset_ClearGlyphs( DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return;
        }
        FONT_ASSET_IMPL( fontAsset )->Glyphs.clear( );
    }

    DenOfIz_StringView DenOfIz_FontAsset_Extension( )
    {
        return DENOFIZ_STRING( "dzfont" );
    }

    DenOfIz_FontAsset DenOfIz_FontAsset_GetJetbrainsMono( )
    {
        return DenOfIz::EmbeddedFonts::GetJetbrainsMono( );
    }
}
