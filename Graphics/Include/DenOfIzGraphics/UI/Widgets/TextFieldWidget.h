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
    enum class TextFieldType : uint8_t
    {
        SingleLine,
        MultiLine,
        Password
    };

    using TextFieldStyle = ClayTextFieldDesc;

    class DZ_API TextFieldWidget : public Widget
    {
        InteropString  m_text;
        size_t         m_cursorPosition  = 0;
        size_t         m_selectionStart  = 0;
        size_t         m_selectionEnd    = 0;
        bool           m_hasSelection    = false;
        bool           m_textChanged     = false;
        float          m_cursorBlinkTime = 0.0f;
        bool           m_cursorVisible   = true;
        bool           m_isSelecting     = false;
        size_t         m_dragStartPos    = 0;
        size_t         m_selectionAnchor = 0;
        TextFieldStyle m_style;

    public:
        DZ_API TextFieldWidget( ClayContext *clayContext, uint32_t id, const TextFieldStyle &style = { } );

        DZ_API void Update( float deltaTime ) override;
        DZ_API void CreateLayoutElement( ) override;
        DZ_API void Render( const Clay_RenderCommand *command, IRenderBatch *renderBatch ) override;
        DZ_API void HandleEvent( const Event &event ) override;

        DZ_API InteropString GetText( ) const;
        DZ_API void          SetText( const InteropString &text );
        DZ_API bool          WasTextChanged( ) const;
        DZ_API void          ClearTextChangedEvent( );

        DZ_API InteropString GetSelectedText( ) const;
        DZ_API void          ClearSelection( );
        DZ_API void          DeleteSelection( );
        DZ_API void          SelectAll( );

        DZ_API size_t GetCursorPosition( ) const;
        DZ_API void   SetCursorPosition( size_t pos );

        DZ_API void                  SetStyle( const TextFieldStyle &style );
        DZ_API const TextFieldStyle &GetStyle( ) const;

    private:
        void   InsertText( const InteropString &text );
        void   HandleKeyPress( const Event &event );
        void   HandleTextInput( const Event &event );
        void   UpdateCursorBlink( float deltaTime );
        size_t GetCharacterIndexAtPosition( float x, float y = 0 ) const;
    };

} // namespace DenOfIz
