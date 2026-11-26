/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <array>
#include "DenOfIzGraphics/Backends/Interface/BindGroup.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphicsInternal/Assets/Font/ColourGlyphStore.h"
#include "DenOfIzGraphicsInternal/Assets/Font/SlugGlyphStore.h"

namespace DenOfIz
{
    // The glyph stores mirrored into persistently mapped structured buffers: Slug curves, bands and glyph records, then colour glyph pixels,
    // bound in that order.
    class GlyphBuffers
    {
        struct Stream
        {
            DenOfIz_Buffer Buffer   = DENOFIZ_NULL_HANDLE;
            uint8_t       *Data     = nullptr;
            uint32_t       Capacity = 0;
        };

        DenOfIz_LogicalDevice m_device;
        std::array<Stream, 4> m_streams{ };
        uint32_t              m_generation       = 1;
        uint32_t              m_colourGeneration = 0;

    public:
        explicit GlyphBuffers( DenOfIz_LogicalDevice device );
        ~GlyphBuffers( );
        GlyphBuffers( const GlyphBuffers & )            = delete;
        GlyphBuffers &operator=( const GlyphBuffers & ) = delete;

        void     Write( SlugGlyphStore &glyphs, ColourGlyphStore &colourGlyphs );
        uint32_t Generation( ) const;
        void     Bind( DenOfIz_BindGroup bindGroup, uint32_t firstBinding ) const;

    private:
        void Create( uint32_t stream, uint32_t capacity );
        void Destroy( uint32_t stream );
    };
} // namespace DenOfIz
