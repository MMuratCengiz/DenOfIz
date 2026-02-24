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

#include "DenOfIzGraphics/UI/Clay.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"

namespace DenOfIz
{
    struct UIVertex
    {
        DenOfIz_Float3 Position;
        DenOfIz_Float2 TexCoord;
        DenOfIz_Float4 Color;
        uint32_t       TextureIndex;
    };

    struct UIVertexArray
    {
        UIVertex *Elements;
        uint64_t  NumElements;
    };

    class IRenderBatch
    {
    public:
        virtual ~IRenderBatch( )                                                                          = default;
        virtual void     AddVertices( const UIVertexArray &vertices, const DenOfIz_UInt32Array &indices ) = 0;
        virtual uint32_t GetCurrentVertexOffset( ) const                                                  = 0;
    };

    class IClayContext
    {
    public:
        virtual ~IClayContext( ) = default;

        virtual void OpenElement( const DenOfIz_ClayElementDeclaration *declaration ) const = 0;
        virtual void CloseElement( ) const                                                  = 0;

        virtual void Text( const char *, const DenOfIz_ClayTextDesc *desc ) const                   = 0;
        virtual void Text( const DenOfIz_StringView &text, const DenOfIz_ClayTextDesc *desc ) const = 0;

        virtual void                   Texture( ITexture *texture, float width, float height ) const                           = 0;
        virtual DenOfIz_ClayDimensions MeasureText( const char *text, uint16_t fontId, uint16_t fontSize ) const               = 0;
        virtual DenOfIz_ClayDimensions MeasureText( const DenOfIz_StringView &text, uint16_t fontId, uint16_t fontSize ) const = 0;

        virtual uint32_t                HashString( const char *str, uint32_t index = 0, uint32_t baseId = 0 ) const               = 0;
        virtual uint32_t                HashString( const DenOfIz_StringView &str, uint32_t index = 0, uint32_t baseId = 0 ) const = 0;
        virtual bool                    PointerOver( uint32_t id ) const                                                           = 0;
        virtual DenOfIz_ClayBoundingBox GetElementBoundingBox( uint32_t id ) const                                                 = 0;

        virtual DenOfIz_ClayDimensions GetViewportSize( ) const             = 0;
        virtual bool                   IsDebugModeEnabled( ) const          = 0;
        virtual float                  GetDpiScale( ) const                 = 0;
        virtual float                  PointsToPixels( float points ) const = 0;
        virtual float                  PixelsToPoints( float pixels ) const = 0;
    };
} // namespace DenOfIz
