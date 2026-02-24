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

#include <clay.h>

#include <unordered_map>
#include <vector>
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/Texture.h"
#include "DenOfIzGraphics/UI/Clay.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Assets/Font/TextLayoutCache.h"
#include "UITextVertexCache.h"

namespace DenOfIz
{
    struct ClayTextCacheDesc
    {
        DenOfIz_LogicalDevice LogicalDevice = DENOFIZ_NULL_HANDLE;
        uint32_t              MaxTextures   = 128;
    };

    struct ClayTextFontData
    {
        Font           *FontPtr       = nullptr;
        DenOfIz_Texture Atlas         = DENOFIZ_NULL_HANDLE;
        uint32_t        TextureIndex  = 0;
        uint32_t        LastUsedFrame = 0;
    };

    class ClayTextCache
    {
        DenOfIz_LogicalDevice                          m_logicalDevice = DENOFIZ_NULL_HANDLE;
        std::unordered_map<uint16_t, ClayTextFontData> m_fonts;
        mutable TextLayoutCache                        m_textLayoutCache;
        mutable UITextVertexCache                      m_textVertexCache;
        mutable uint32_t                               m_currentFrame = 0;
        uint32_t                                       m_maxTextures  = 128;
        float                                          m_dpiScale     = 1.0f;

        std::vector<DenOfIz_Texture> m_textures;
        std::vector<bool>            m_textureFontFlags;
        uint32_t                     m_nextTextureIndex = 1;

    public:
        explicit ClayTextCache( const ClayTextCacheDesc &desc );
        ~ClayTextCache( );

        void  AddFont( uint16_t fontId, Font *font );
        void  RemoveFont( uint16_t fontId );
        Font *GetFont( uint16_t fontId ) const;

        DenOfIz_ClayDimensions MeasureText( const DenOfIz_StringView &text, const Clay_TextElementConfig &desc ) const;

        TextLayout         *GetOrCreateShapedText( const Clay_RenderCommand *command, Font *font ) const;
        TextLayout         *GetOrCreateShapedText( const DenOfIz_StringView &text, uint16_t fontId, uint32_t fontSize, Font *font ) const;
        CachedTextVertices *GetOrCreateTextVertices( const TextVertexCacheKey &key ) const;

        uint32_t        GetFontTextureIndex( uint16_t fontId ) const;
        DenOfIz_Texture GetFontTexture( uint16_t fontId ) const;
        void            GetAllFontTextures( std::vector<DenOfIz_Texture> &outTextures ) const;

        void UpdateFrame( uint32_t currentFrame ) const;
        void ClearCaches( ) const;

        static uint64_t HashString( const DenOfIz_StringView &text );

    private:
        void     InitializeFontAtlas( ClayTextFontData *fontData );
        uint32_t RegisterTexture( DenOfIz_Texture texture );
    };

} // namespace DenOfIz
