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

#include <DenOfIzGraphics/Assets/Font/Font.h>
#include <DenOfIzGraphics/Assets/Font/TextLayoutCache.h>
#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>
#include <DenOfIzGraphics/UI/ClayData.h>
#include <DenOfIzGraphics/UI/UITextVertexCache.h>
#include <memory>
#include <unordered_map>

namespace DenOfIz
{
    struct ClayTextCacheDesc
    {
        ILogicalDevice *LogicalDevice = nullptr;
        uint32_t        MaxTextures   = 128;
    };

    struct ClayTextFontData
    {
        Font                             *FontPtr = nullptr;
        std::unique_ptr<ITextureResource> Atlas;
        uint32_t                          TextureIndex  = 0;
        uint32_t                          LastUsedFrame = 0;
    };

    class DZ_API ClayTextCache
    {
        ILogicalDevice                                *m_logicalDevice = nullptr;
        std::unordered_map<uint16_t, ClayTextFontData> m_fonts;
        mutable TextLayoutCache                        m_textLayoutCache;
        mutable UITextVertexCache                      m_textVertexCache;
        mutable uint32_t                               m_currentFrame = 0;
        uint32_t                                       m_maxTextures  = 128;
        float                                          m_dpiScale     = 1.0f;

        std::vector<ITextureResource *> m_textures;
        std::vector<bool>               m_textureFontFlags;
        uint32_t                        m_nextTextureIndex = 1; // 0 is reserved for null texture

    public:
        explicit ClayTextCache( const ClayTextCacheDesc &desc );
        ~ClayTextCache( ) = default;

        void  AddFont( uint16_t fontId, Font *font );
        void  RemoveFont( uint16_t fontId );
        Font *GetFont( uint16_t fontId ) const;

        ClayDimensions MeasureText( const InteropString &text, const Clay_TextElementConfig &desc ) const;
        ClayDimensions MeasureTextDirect( const char *text, size_t length, uint16_t fontId, uint16_t fontSize ) const;

        TextLayout         *GetOrCreateShapedText( const Clay_RenderCommand *command, Font *font ) const;
        TextLayout         *GetOrCreateShapedTextDirect( const char *text, size_t length, uint16_t fontId, uint32_t fontSize, Font *font ) const;
        CachedTextVertices *GetOrCreateTextVertices( const TextVertexCacheKey &key ) const;

        uint32_t          GetFontTextureIndex( uint16_t fontId ) const;
        ITextureResource *GetFontTexture( uint16_t fontId ) const;
        void              GetAllFontTextures( std::vector<ITextureResource *> &outTextures ) const;

        void UpdateFrame( uint32_t currentFrame ) const;
        void CleanupCaches( uint32_t maxLayoutAge = 3000, uint32_t maxVertexAge = 6000 ) const;
        void ClearCaches( ) const;

        void  SetDpiScale( float dpiScale );
        float GetDpiScale( ) const;

        static uint64_t HashString( const char *str, size_t length );

    private:
        void     InitializeFontAtlas( ClayTextFontData *fontData );
        uint32_t RegisterTexture( ITextureResource *texture );
    };

} // namespace DenOfIz
