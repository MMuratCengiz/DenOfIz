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
    using DropdownStyle = ClayDropdownDesc;

    class DZ_API DropdownWidget : public Widget
    {
        InteropArray<InteropString> m_options;
        int32_t                     m_selectedIndex    = -1;
        bool                        m_isOpen           = false;
        bool                        m_selectionChanged = false;
        float                       m_scrollOffset     = 0.0f;
        DropdownStyle               m_style;
        uint32_t                    m_dropdownListId;
        ClayDropdownState           m_dropdownState;
        ClayDropdownRenderData      m_renderData;
        ClayCustomWidgetData        m_widgetData;

    public:
        DZ_API DropdownWidget( Clay *clay, uint32_t id, const InteropArray<InteropString> &options, const DropdownStyle &style = { } );

        DZ_API void Update( float deltaTime ) override;
        DZ_API void CreateLayoutElement( ) override;
        DZ_API void Render( ) override;
        DZ_API void HandleEvent( const Event &event ) override;

        DZ_API int32_t       GetSelectedIndex( ) const;
        DZ_API void          SetSelectedIndex( int32_t index );
        DZ_API InteropString GetSelectedText( ) const;
        DZ_API bool          WasSelectionChanged( ) const;
        DZ_API void          ClearSelectionChangedEvent( );
        DZ_API bool          IsOpen( ) const;
        DZ_API void          SetOpen( bool open );

        DZ_API void  SetOptions( const InteropArray<InteropString> &options );
        DZ_API const InteropArray<InteropString> &GetOptions( ) const;

        DZ_API void                 SetStyle( const DropdownStyle &style );
        DZ_API const DropdownStyle &GetStyle( ) const;

        DZ_API void RenderDropdownList( );
    };

} // namespace DenOfIz
