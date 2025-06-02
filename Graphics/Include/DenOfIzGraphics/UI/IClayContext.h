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

#include <DenOfIzGraphics/UI/ClayData.h>
#include <DenOfIzGraphics/Utilities/InteropMath.h>

namespace DenOfIz
{
    struct UIVertex
    {
        Float_3  Position; // Z used for depth test
        Float_2  TexCoord;
        Float_4  Color;
        uint32_t TextureIndex; // For bindless textures
    };

    class IRenderBatch
    {
    public:
        virtual ~IRenderBatch( )                                                                                      = default;
        virtual void     AddVertices( const InteropArray<UIVertex> &vertices, const InteropArray<uint32_t> &indices ) = 0;
        virtual uint32_t GetCurrentVertexOffset( ) const                                                              = 0;
    };

    class DZ_API IClayContext
    {
    public:
        virtual ~IClayContext( ) = default;

        virtual void OpenElement( const ClayElementDeclaration &declaration ) const = 0;
        virtual void CloseElement( ) const                                          = 0;

        virtual void           Text( const InteropString &text, const ClayTextDesc &desc ) const                            = 0;
        virtual ClayDimensions MeasureText( const InteropString &text, uint16_t fontId, uint16_t fontSize ) const           = 0;

        virtual uint32_t        HashString( const InteropString &str, uint32_t index = 0, uint32_t baseId = 0 ) const      = 0;
        virtual bool            PointerOver( uint32_t id ) const                                                            = 0;
        virtual ClayBoundingBox GetElementBoundingBox( uint32_t id ) const                                                 = 0;

        virtual ClayDimensions GetViewportSize( ) const    = 0;
        virtual bool           IsDebugModeEnabled( ) const = 0;
    };
} // namespace DenOfIz