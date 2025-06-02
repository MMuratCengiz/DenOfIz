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

#include "Widget.h"

using namespace DenOfIz;

namespace DenOfIz
{
    using ColorPickerStyle = ClayColorPickerDesc;

    class ColorPickerWidget : public Widget
    {
        Float_3          m_hsv                = Float_3{ 0.0f, 1.0f, 1.0f };
        Float_3          m_rgb                = Float_3{ 1.0f, 0.0f, 0.0f };
        bool             m_isExpanded         = false;
        bool             m_colorChanged       = false;
        bool             m_isDraggingWheel    = false;
        bool             m_isDraggingValueBar = false;
        ColorPickerStyle m_style;
        Float_2          m_lastMousePos;

    public:
        DZ_API ColorPickerWidget( IClayContext *clayContext, uint32_t id, const Float_3 &initialRgb = { }, const ColorPickerStyle &style = { } );

        DZ_API void Update( float deltaTime ) override;
        DZ_API void CreateLayoutElement( ) override;
        DZ_API void Render( const ClayBoundingBox &boundingBox, IRenderBatch *renderBatch ) override;
        DZ_API void HandleEvent( const Event &event ) override;

        DZ_API Float_3 GetRGB( ) const;
        DZ_API Float_3 GetHSV( ) const;

        DZ_API void SetRGB( const Float_3 &rgb );
        DZ_API void SetHSV( const Float_3 &hsv );

        DZ_API bool WasColorChanged( ) const;
        DZ_API void ClearColorChangedEvent( );
        DZ_API bool IsExpanded( ) const;
        DZ_API void SetExpanded( bool expanded );

        DZ_API void                    SetStyle( const ColorPickerStyle &style );
        DZ_API const ColorPickerStyle &GetStyle( ) const;

    private:
        void    UpdateFromMouseWheel( float mouseX, float mouseY );
        void    UpdateFromMouseValueBar( float mouseY );
        Float_3 HSVToRGB( const Float_3 &hsv ) const;
        Float_3 RGBToHSV( const Float_3 &rgb ) const;
    };

} // namespace DenOfIz
