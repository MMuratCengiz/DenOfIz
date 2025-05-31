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

#include <DenOfIzGraphics/UI/Clay.h>
#include <DenOfIzGraphics/UI/ClayData.h>
#include <DenOfIzGraphics/UI/Widgets/TextFieldWidget.h>
#include <algorithm>

#include "DenOfIzGraphics/UI/ClayClipboard.h"

using namespace DenOfIz;

TextFieldWidget::TextFieldWidget( Clay *clay, uint32_t id, const TextFieldStyle &style ) : Widget( clay, id ), m_style( style )
{
    m_textFieldState.Text            = m_text;
    m_textFieldState.CursorPosition  = m_cursorPosition;
    m_textFieldState.SelectionStart  = m_selectionStart;
    m_textFieldState.SelectionEnd    = m_selectionEnd;
    m_textFieldState.HasSelection    = m_hasSelection;
    m_textFieldState.IsFocused       = false;
    m_textFieldState.CursorVisible   = m_cursorVisible;
    m_textFieldState.CursorBlinkTime = m_cursorBlinkTime;
    m_textFieldState.SelectionAnchor = m_selectionAnchor;
}

void TextFieldWidget::Update( const float deltaTime )
{
    UpdateHoverState( );
    UpdateCursorBlink( deltaTime );
}

void TextFieldWidget::CreateLayoutElement( )
{
    ClayElementDeclaration decl;
    decl.Id                   = m_id;
    decl.Layout.Sizing.Width  = ClaySizingAxis::Grow( );
    decl.Layout.Sizing.Height = ClaySizingAxis::Fixed( m_style.Height );
    decl.Custom.CustomData    = &m_widgetData;

    m_clay->OpenElement( decl );
    m_clay->CloseElement( );
}

void TextFieldWidget::Render( )
{
    m_textFieldState.Text            = m_text;
    m_textFieldState.CursorPosition  = m_cursorPosition;
    m_textFieldState.SelectionStart  = m_selectionStart;
    m_textFieldState.SelectionEnd    = m_selectionEnd;
    m_textFieldState.HasSelection    = m_hasSelection;
    m_textFieldState.IsFocused       = m_isFocused;
    m_textFieldState.CursorVisible   = m_cursorVisible;
    m_textFieldState.CursorBlinkTime = m_cursorBlinkTime;
    m_textFieldState.SelectionAnchor = m_selectionAnchor;

    m_renderData.State     = &m_textFieldState;
    m_renderData.Desc      = m_style;
    m_renderData.ElementId = m_id;

    m_widgetData.Type = ClayCustomWidgetType::TextField;
    m_widgetData.Data = &m_renderData;
}

void TextFieldWidget::HandleEvent( const Event &event )
{
    m_textChanged = false;

    if ( event.Type == EventType::MouseButtonDown && event.Button.Button == MouseButton::Left )
    {
        if ( m_isHovered )
        {
            m_isFocused = true;

            const size_t clickPos = GetCharacterIndexAtPosition( event.Button.X );
            m_cursorPosition      = clickPos;
            m_selectionAnchor     = clickPos;
            m_dragStartPos        = clickPos;
            m_isSelecting         = true;
            ClearSelection( );
        }
        else
        {
            m_isFocused = false;
        }
    }
    else if ( event.Type == EventType::MouseButtonUp && event.Button.Button == MouseButton::Left )
    {
        m_isSelecting = false;
    }
    else if ( event.Type == EventType::MouseMotion && m_isSelecting )
    {
        // Update selection
        const size_t dragPos = GetCharacterIndexAtPosition( event.Motion.X );
        if ( dragPos != m_dragStartPos )
        {
            m_hasSelection   = true;
            m_selectionStart = std::min( m_selectionAnchor, dragPos );
            m_selectionEnd   = std::max( m_selectionAnchor, dragPos );
            m_cursorPosition = dragPos;
        }
    }
    else if ( m_isFocused )
    {
        if ( event.Type == EventType::KeyDown )
        {
            HandleKeyPress( event );
        }
        else if ( event.Type == EventType::TextInput )
        {
            HandleTextInput( event );
        }
    }
}
InteropString TextFieldWidget::GetText( ) const
{
    return m_text;
}

void TextFieldWidget::SetText( const InteropString &text )
{
    if ( m_style.MaxLength > 0 && text.NumChars( ) > m_style.MaxLength )
    {
        m_text = InteropString( text.Get( ), m_style.MaxLength );
    }
    else
    {
        m_text = text;
    }
    m_cursorPosition = m_text.NumChars( );
    ClearSelection( );
    m_textChanged = true;
}
bool TextFieldWidget::WasTextChanged( ) const
{
    return m_textChanged;
}
void TextFieldWidget::ClearTextChangedEvent( )
{
    m_textChanged = false;
}

InteropString TextFieldWidget::GetSelectedText( ) const
{
    if ( !m_hasSelection )
    {
        return InteropString( "" );
    }

    const char  *textData = m_text.Get( );
    const size_t length   = m_selectionEnd - m_selectionStart;
    return InteropString( textData + m_selectionStart, length );
}

void TextFieldWidget::ClearSelection( )
{
    m_hasSelection   = false;
    m_selectionStart = 0;
    m_selectionEnd   = 0;
}

void TextFieldWidget::DeleteSelection( )
{
    if ( !m_hasSelection || m_style.ReadOnly )
    {
        return;
    }

    std::string newText( m_text.Get( ) );
    newText.erase( m_selectionStart, m_selectionEnd - m_selectionStart );
    m_text           = InteropString( newText.c_str( ) );
    m_cursorPosition = m_selectionStart;
    ClearSelection( );
    m_textChanged = true;
}

void TextFieldWidget::SelectAll( )
{
    if ( m_text.NumChars( ) > 0 )
    {
        m_hasSelection   = true;
        m_selectionStart = 0;
        m_selectionEnd   = m_text.NumChars( );
        m_cursorPosition = m_selectionEnd;
    }
}

size_t TextFieldWidget::GetCursorPosition( ) const
{
    return m_cursorPosition;
}

void TextFieldWidget::SetCursorPosition( const size_t pos )
{
    m_cursorPosition = std::min( pos, m_text.NumChars( ) );
    ClearSelection( );
}

void TextFieldWidget::SetStyle( const TextFieldStyle &style )
{
    m_style = style;
}

const TextFieldStyle &TextFieldWidget::GetStyle( ) const
{
    return m_style;
}

void TextFieldWidget::InsertText( const InteropString &text )
{
    if ( m_style.ReadOnly )
    {
        return;
    }

    if ( m_hasSelection )
    {
        DeleteSelection( );
    }

    if ( m_style.MaxLength > 0 && m_text.NumChars( ) + text.NumChars( ) > m_style.MaxLength )
    {
        return;
    }

    std::string currentText( m_text.Get( ) );
    currentText.insert( m_cursorPosition, text.Get( ) );
    m_text = InteropString( currentText.c_str( ) );
    m_cursorPosition += text.NumChars( );
    m_textChanged = true;
}

void TextFieldWidget::HandleKeyPress( const Event &event )
{
    const bool isCtrlDown  = event.Key.Mod.IsSet( KeyMod::Ctrl ) || event.Key.Mod.IsSet( KeyMod::LCtrl ) || event.Key.Mod.IsSet( KeyMod::RCtrl );
    const bool isShiftDown = event.Key.Mod.IsSet( KeyMod::Shift ) || event.Key.Mod.IsSet( KeyMod::LShift ) || event.Key.Mod.IsSet( KeyMod::RShift );
    const bool isCmdDown   = event.Key.Mod.IsSet( KeyMod::Gui ) || event.Key.Mod.IsSet( KeyMod::LGui ) || event.Key.Mod.IsSet( KeyMod::RGui ); // Command key on macOS

    if ( isCtrlDown || isCmdDown )
    {
        switch ( event.Key.Keycode )
        {
        case KeyCode::A: // Select All
            SelectAll( );
            break;

        case KeyCode::C: // Copy
            if ( m_hasSelection )
            {
                ClayClipboard::SetText( GetSelectedText( ) );
            }
            break;

        case KeyCode::V: // Paste
            if ( !m_style.ReadOnly )
            {
                const InteropString pasteText = ClayClipboard::GetText( );
                if ( pasteText.NumChars( ) > 0 )
                {
                    if ( m_hasSelection )
                    {
                        DeleteSelection( );
                    }
                    InsertText( pasteText );
                    m_cursorBlinkTime = 0.0f;
                    m_cursorVisible   = true;
                }
            }
            break;

        case KeyCode::X: // Cut
            if ( m_hasSelection && !m_style.ReadOnly )
            {
                ClayClipboard::SetText( GetSelectedText( ) );
                DeleteSelection( );
                m_cursorBlinkTime = 0.0f;
                m_cursorVisible   = true;
            }
            break;

        default:
            break;
        }
        return;
    }

    switch ( event.Key.Keycode )
    {
    case KeyCode::Left:
        if ( isShiftDown )
        {
            // Extend selection left
            if ( m_cursorPosition > 0 )
            {
                if ( !m_hasSelection )
                {
                    m_selectionAnchor = m_cursorPosition;
                }
                m_cursorPosition--;
                m_hasSelection   = true;
                m_selectionStart = std::min( m_selectionAnchor, m_cursorPosition );
                m_selectionEnd   = std::max( m_selectionAnchor, m_cursorPosition );
            }
        }
        else
        {
            if ( m_hasSelection )
            {
                m_cursorPosition = m_selectionStart;
                ClearSelection( );
            }
            else if ( m_cursorPosition > 0 )
            {
                m_cursorPosition--;
            }
        }
        m_cursorBlinkTime = 0.0f;
        m_cursorVisible   = true;
        break;

    case KeyCode::Right:
        if ( isShiftDown )
        {
            // Extend selection right
            if ( m_cursorPosition < m_text.NumChars( ) )
            {
                if ( !m_hasSelection )
                {
                    m_selectionAnchor = m_cursorPosition;
                }
                m_cursorPosition++;
                m_hasSelection   = true;
                m_selectionStart = std::min( m_selectionAnchor, m_cursorPosition );
                m_selectionEnd   = std::max( m_selectionAnchor, m_cursorPosition );
            }
        }
        else
        {
            if ( m_hasSelection )
            {
                m_cursorPosition = m_selectionEnd;
                ClearSelection( );
            }
            else if ( m_cursorPosition < m_text.NumChars( ) )
            {
                m_cursorPosition++;
            }
        }
        m_cursorBlinkTime = 0.0f;
        m_cursorVisible   = true;
        break;

    case KeyCode::Home:
        if ( isShiftDown )
        {
            if ( !m_hasSelection )
            {
                m_selectionAnchor = m_cursorPosition;
            }
            m_cursorPosition = 0;
            m_hasSelection   = true;
            m_selectionStart = 0;
            m_selectionEnd   = m_selectionAnchor;
        }
        else
        {
            m_cursorPosition = 0;
            ClearSelection( );
        }
        m_cursorBlinkTime = 0.0f;
        m_cursorVisible   = true;
        break;

    case KeyCode::End:
        if ( isShiftDown )
        {
            if ( !m_hasSelection )
            {
                m_selectionAnchor = m_cursorPosition;
            }
            m_cursorPosition = m_text.NumChars( );
            m_hasSelection   = true;
            m_selectionStart = m_selectionAnchor;
            m_selectionEnd   = m_text.NumChars( );
        }
        else
        {
            m_cursorPosition = m_text.NumChars( );
            ClearSelection( );
        }
        m_cursorBlinkTime = 0.0f;
        m_cursorVisible   = true;
        break;

    case KeyCode::Backspace:
        if ( !m_style.ReadOnly )
        {
            if ( m_hasSelection )
            {
                DeleteSelection( );
            }
            else if ( m_cursorPosition > 0 )
            {
                std::string text( m_text.Get( ) );
                text.erase( m_cursorPosition - 1, 1 );
                m_text = InteropString( text.c_str( ) );
                m_cursorPosition--;
                m_textChanged = true;
            }
        }
        m_cursorBlinkTime = 0.0f;
        m_cursorVisible   = true;
        break;

    case KeyCode::Delete:
        if ( !m_style.ReadOnly )
        {
            if ( m_hasSelection )
            {
                DeleteSelection( );
            }
            else if ( m_cursorPosition < m_text.NumChars( ) )
            {
                std::string text( m_text.Get( ) );
                text.erase( m_cursorPosition, 1 );
                m_text        = InteropString( text.c_str( ) );
                m_textChanged = true;
            }
        }
        m_cursorBlinkTime = 0.0f;
        m_cursorVisible   = true;
        break;

    case KeyCode::Return:
        m_isFocused = false;
        break;

    case KeyCode::Escape:
        m_isFocused = false;
        ClearSelection( );
        break;

    default:
        break;
    }
}

void TextFieldWidget::HandleTextInput( const Event &event )
{
    if ( !m_style.ReadOnly && event.Text.Text[ 0 ] != 0 )
    {
        InsertText( InteropString( event.Text.Text ) );
    }
}

void TextFieldWidget::UpdateCursorBlink( const float deltaTime )
{
    if ( m_isFocused )
    {
        m_cursorBlinkTime += deltaTime;
        if ( m_cursorBlinkTime >= 0.5f )
        {
            m_cursorVisible   = !m_cursorVisible;
            m_cursorBlinkTime = 0.0f;
        }
    }
    else
    {
        m_cursorVisible   = false;
        m_cursorBlinkTime = 0.0f;
    }
}

size_t TextFieldWidget::GetCharacterIndexAtPosition( const float x ) const
{
    // TODO: Use Text layout, must be input somehow
    const auto  bounds    = GetBoundingBox( );
    const float relativeX = x - bounds.X - m_style.Padding.Left;

    if ( relativeX <= 0 )
    {
        return 0;
    }

    const float  avgCharWidth = m_style.FontSize * 0.6f; // Rough estimate
    const size_t index        = relativeX / avgCharWidth;
    return std::min( index, m_text.NumChars( ) );
}
