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

namespace DenOfIz
{
    using SliderStyle = ClaySliderDesc;

    class SliderWidget : public Widget
    {
        float       m_value        = 0.0f;
        bool        m_isDragging   = false;
        bool        m_valueChanged = false;
        SliderStyle m_style;
        Float_2     m_lastMousePos;

    public:
        DZ_API SliderWidget( IClayContext *clayContext, uint32_t id, float initialValue = 0.5f, const SliderStyle &style = { } );

        DZ_API void Update( float deltaTime ) override;
        DZ_API void CreateLayoutElement( ) override;
        DZ_API void Render( const ClayBoundingBox &boundingBox, IRenderBatch *renderBatch ) override;
        DZ_API void HandleEvent( const Event &event ) override;

        DZ_API float GetValue( ) const;
        DZ_API void  SetValue( float value );
        DZ_API bool  WasValueChanged( ) const;
        DZ_API void  ClearValueChangedEvent( );
        DZ_API bool  IsDragging( ) const;

        DZ_API void               SetStyle( const SliderStyle &style );
        DZ_API const SliderStyle &GetStyle( ) const;

    private:
        void UpdateValueFromMouse( float mouseX );
    };

} // namespace DenOfIz
