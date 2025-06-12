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

#include "DenOfIzGraphics/Assets/Serde/Font/FontAsset.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"

namespace DenOfIz
{
    struct DZ_API FontAssetWriterDesc
    {
        BinaryWriter *Writer;
    };

    class FontAssetWriter
    {
        BinaryWriter    *m_writer;
        uint64_t         m_dataOffset          = 0;
        uint64_t         m_streamStartLocation = 0;
        FontAsset const *m_fontAsset;

    public:
        DZ_API explicit FontAssetWriter( const FontAssetWriterDesc &desc );
        DZ_API ~FontAssetWriter( );

        DZ_API void Write( const FontAsset &fontAsset );
        DZ_API void End( ) const;

    private:
        void WriteHeader( uint64_t totalNumBytes ) const;
        void WriteMetadata( const FontAsset &fontAsset ) const;
        void WriteGlyph( const FontAsset &fontAsset ) const;
    };
} // namespace DenOfIz
