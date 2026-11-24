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

#include <array>
#include <memory>
#include <string>
#include "DenOfIzExamples/IExample.h"
#include "DenOfIzGraphics/UI/UI.h"
#include "Spinning3DCubeWidget.h"

namespace DenOfIz
{
    /**
     * A small app built with the immediate mode UI: flexbox layout, widgets, a text field, a dropdown, a tooltip, a modal dialog,
     * a render target shown as an image and rounded clipping. Inactive pages stay declared with Display NONE, so they keep their
     * scroll positions.
     *
     * Keys: Tab/Shift+Tab move the focus, Enter/Space activate the focused widget, F11 toggles debug outlines,
     * Esc closes the dialog, the dropdown or the focus, then quits.
     */
    class UIExample final : public IExample
    {
        enum class Page : uint32_t
        {
            Controls,
            Text,
            Media,
            Count
        };

        DenOfIz_UI                            m_ui = DENOFIZ_NULL_HANDLE;
        std::unique_ptr<Spinning3DCubeWidget> m_cube;
        DenOfIz_SystemCursor                  m_cursor = DENOFIZ_SYSTEM_CURSOR_DEFAULT;
        std::array<char, 256>                 m_format{ };

        Page     m_page         = Page::Controls;
        float    m_deltaTime    = 0.0f;
        float    m_blinkTimer   = 0.0f;
        uint32_t m_focusId      = 0;
        bool     m_focusVisible = false;
        bool     m_keyActivate  = false;
        bool     m_textInput    = false;
        bool     m_debug        = false;

        uint32_t    m_clicks        = 0;
        bool        m_showTooltip   = false;
        bool        m_notifications = true;
        float       m_volume        = 0.65f;
        uint32_t    m_quality       = 1;
        bool        m_qualityOpen   = false;
        bool        m_dialogOpen    = false;
        const char *m_dialogResult  = "none";

        std::string m_text      = "Click to edit, drag to select";
        uint32_t    m_caret     = 0;
        uint32_t    m_anchor    = 0;
        bool        m_selecting = false;
        bool        m_reveal    = false;

        int32_t m_selectedRow = -1;

        uint32_t m_tabsId        = 0;
        uint32_t m_pagesId       = 0;
        uint32_t m_clickId       = 0;
        uint32_t m_qualityId     = 0;
        uint32_t m_fieldId       = 0;
        uint32_t m_fieldScrollId = 0;
        uint32_t m_fieldTextId   = 0;
        uint32_t m_listId        = 0;
        uint32_t m_dialogId      = 0;

    public:
        ~UIExample( ) override = default;
        void Init( ) override;
        void HandleEvent( DenOfIz_Event &event ) override;
        void Update( ) override;
        void Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        void Quit( ) override;

        ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc  = ExampleWindowDesc( );
            windowDesc.Title = "DenOfIz UI Example";
            return windowDesc;
        }

    private:
        void BuildUI( uint32_t frameIndex );
        void TopBar( );
        void Sidebar( );
        void ControlsPage( );
        void TextPage( );
        void MediaPage( uint32_t frameIndex );
        void QualityPopup( );
        void Tooltip( ) const;
        void Dialog( );
        void ToggleDebug( );

        bool Button( const char *label, uint32_t id, bool primary = true, bool disabled = false );
        bool Toggle( uint32_t id, bool &value, const char *label );
        void Slider( uint32_t id, float &value );
        void Dropdown( );
        void TextField( );

        bool HandleKeyDown( const DenOfIz_KeyboardEventData &key, bool editing );
        bool EditKey( const DenOfIz_KeyboardEventData &key );
        void Insert( DenOfIz_StringView text );
        bool DeleteSelection( );
        void MoveCaret( uint32_t offset, bool extend );

        uint32_t              Id( const char *name ) const;
        DenOfIz_UIInteraction Interaction( uint32_t id ) const;
        bool                  Activated( uint32_t id, const DenOfIz_UIInteraction &interaction ) const;
        void                  FocusRing( DenOfIz_UIElementDesc &desc ) const;
        void                  Open( const DenOfIz_UIElementDesc &desc ) const;
        void                  Close( ) const;
        void                  Label( DenOfIz_StringView text, float size, DenOfIz_UIColor color ) const;
        void                  Label( const char *text, float size, DenOfIz_UIColor color ) const;
        void                  BeginCard( const char *title, const char *description, float basis = 0.0f ) const;
        DenOfIz_StringView    Format( const char *format, ... );
    };
} // namespace DenOfIz
