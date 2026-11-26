/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <harfbuzz/hb.h>
#include "DenOfIzGraphics/Assets/Font/Font.h"

namespace DenOfIz
{
    class Font
    {
        hb_blob_t          *m_blob;
        hb_face_t          *m_face;
        hb_font_t          *m_font;
        uint32_t            m_id;
        uint32_t            m_upem;
        bool                m_colour;
        DenOfIz_FontMetrics m_metrics{};

    public:
        Font( hb_blob_t *blob, uint32_t faceIndex );
        ~Font( );
        Font( const Font & )            = delete;
        Font &operator=( const Font & ) = delete;

        bool                       IsValid( ) const;
        uint32_t                   Id( ) const;
        uint32_t                   Upem( ) const;
        hb_font_t                 *HbFont( ) const;
        const DenOfIz_FontMetrics &Metrics( ) const;
        bool                       IsColour( ) const;
        bool                       HasCodePoint( uint32_t codePoint ) const;
        bool                       Covers( const char *text, uint32_t length ) const;

        static Font *Default( );
    };
} // namespace DenOfIz

#define FONT_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::Font, handle )
