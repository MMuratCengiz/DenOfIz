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
    using CheckboxStyle = ClayCheckboxDesc;

    class CheckboxWidget : public Widget
    {
        bool          m_isChecked  = false;
        bool          m_wasClicked = false;
        CheckboxStyle m_style;

    public:
        DZ_API CheckboxWidget( IClayContext *clayContext, uint32_t id, bool initialChecked = false, const CheckboxStyle &style = { } );

        DZ_API void Update( float deltaTime ) override;
        DZ_API void CreateLayoutElement( ) override;
        DZ_API void Render( const ClayBoundingBox &boundingBox, IRenderBatch *renderBatch ) override;
        DZ_API void HandleEvent( const Event &event ) override;

        DZ_API bool IsChecked( ) const;
        DZ_API void SetChecked( bool checked );
        DZ_API bool WasClicked( ) const;
        DZ_API void ClearClickEvent( );

        DZ_API void                 SetStyle( const CheckboxStyle &style );
        DZ_API const CheckboxStyle &GetStyle( ) const;
    };

} // namespace DenOfIz
