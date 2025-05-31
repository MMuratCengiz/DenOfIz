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

#include <DenOfIzGraphics/UI/ClayClipboard.h>
#include <DenOfIzGraphics/UI/Widgets/TextFieldWidget.h>
#include <algorithm>

using namespace DenOfIz;

TextFieldWidget::TextFieldWidget( ClayContext *clayContext, uint32_t id, const TextFieldStyle &style ) : Widget( clayContext, id ), m_style( style )
{
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
    decl.Layout.Padding       = m_style.Padding;

    if ( m_style.Type == ClayTextFieldType::MultiLine )
    {
        decl.Scroll.Vertical   = true;
        decl.Scroll.Horizontal = false;
    }

    decl.Custom.CustomData = this;
    decl.BackgroundColor   = m_style.BackgroundColor;
    decl.Border.Color      = m_isFocused ? m_style.FocusBorderColor : m_style.BorderColor;
    decl.Border.Width      = ClayBorderWidth( 1 );

    m_clayContext->OpenElement( decl );

    // Add text element if we have text to display
    if ( !m_text.IsEmpty( ) )
    {
        ClayTextDesc textDesc;
        textDesc.TextColor = m_style.TextColor;
        textDesc.FontId    = m_style.FontId;
        textDesc.FontSize  = m_style.FontSize;

        m_clayContext->Text( m_text, textDesc );
    }
    else if ( !m_style.PlaceholderText.IsEmpty( ) )
    {
        ClayTextDesc textDesc;
        textDesc.TextColor = m_style.PlaceholderColor;
        textDesc.FontId    = m_style.FontId;
        textDesc.FontSize  = m_style.FontSize;

        m_clayContext->Text( m_style.PlaceholderText, textDesc );
    }

    m_clayContext->CloseElement( );
}

void TextFieldWidget::Render( const Clay_RenderCommand *command, IRenderBatch *renderBatch )
{
    const auto &bounds = command->boundingBox;

    // Background, border, and text are handled by Clay elements in CreateLayoutElement
    // Only render cursor and selection here as these are interactive elements

    // Text bounds with padding (needed for cursor positioning)
    ClayBoundingBox textBounds;
    textBounds.X      = bounds.x + m_style.Padding.Left;
    textBounds.Y      = bounds.y + m_style.Padding.Top;
    textBounds.Width  = bounds.width - m_style.Padding.Left - m_style.Padding.Right;
    textBounds.Height = bounds.height - m_style.Padding.Top - m_style.Padding.Bottom;

    if ( m_hasSelection && m_selectionStart != m_selectionEnd )
    {
        const size_t selStart = std::min( m_selectionStart, m_selectionEnd );
        const size_t selEnd   = std::max( m_selectionStart, m_selectionEnd );

        if ( selStart < selEnd && selEnd <= m_text.NumChars( ) )
        {
            const ClayDimensions textSizeForSelection = m_clayContext->MeasureText( InteropString( "I" ), m_style.FontId, m_style.FontSize );
            const float          selectionHeight      = textSizeForSelection.Height;

            if ( m_style.Type == ClayTextFieldType::MultiLine )
            {
                const std::string textStr             = m_text.Get( );
                const std::string textBeforeSelection = textStr.substr( 0, selStart );
                const std::string selectedText        = textStr.substr( selStart, selEnd - selStart );

                size_t startLine              = 0;
                size_t lastNewlineBeforeStart = 0;
                for ( size_t i = 0; i < textBeforeSelection.length( ); ++i )
                {
                    if ( textBeforeSelection[ i ] == '\n' )
                    {
                        startLine++;
                        lastNewlineBeforeStart = i + 1;
                    }
                }

                const std::string    textOnStartLine = textBeforeSelection.substr( lastNewlineBeforeStart );
                const ClayDimensions startLineSize   = m_clayContext->MeasureText( InteropString( textOnStartLine.c_str( ) ), m_style.FontId, m_style.FontSize );

                float currentY = textBounds.Y + startLine * selectionHeight;
                float currentX = textBounds.X + startLineSize.Width;

                size_t currentPos = 0;
                while ( currentPos < selectedText.length( ) )
                {
                    size_t nextNewline = selectedText.find( '\n', currentPos );
                    if ( nextNewline == std::string::npos )
                    {
                        nextNewline = selectedText.length( );
                    }

                    const std::string    lineText = selectedText.substr( currentPos, nextNewline - currentPos );
                    const ClayDimensions lineSize = m_clayContext->MeasureText( InteropString( lineText.c_str( ) ), m_style.FontId, m_style.FontSize );

                    ClayBoundingBox selectionBounds;
                    selectionBounds.X      = currentX;
                    selectionBounds.Y      = currentY;
                    selectionBounds.Width  = lineSize.Width;
                    selectionBounds.Height = selectionHeight;

                    AddRectangle( renderBatch, selectionBounds, m_style.SelectionColor );

                    if ( nextNewline < selectedText.length( ) )
                    {
                        currentY += selectionHeight;
                        currentX   = textBounds.X;
                        currentPos = nextNewline + 1;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                float selectionStartX = textBounds.X;
                if ( selStart > 0 )
                {
                    const std::string    textBeforeSelection = std::string( m_text.Get( ) ).substr( 0, selStart );
                    const ClayDimensions beforeSize          = m_clayContext->MeasureText( InteropString( textBeforeSelection.c_str( ) ), m_style.FontId, m_style.FontSize );
                    selectionStartX += beforeSize.Width;
                }

                const std::string    selectedText = std::string( m_text.Get( ) ).substr( selStart, selEnd - selStart );
                const ClayDimensions selectedSize = m_clayContext->MeasureText( InteropString( selectedText.c_str( ) ), m_style.FontId, m_style.FontSize );

                ClayBoundingBox selectionBounds;
                selectionBounds.X      = selectionStartX;
                selectionBounds.Y      = textBounds.Y;
                selectionBounds.Width  = selectedSize.Width;
                selectionBounds.Height = selectionHeight;

                AddRectangle( renderBatch, selectionBounds, m_style.SelectionColor );
            }
        }
    }

    if ( m_isFocused && m_cursorVisible && !m_style.ReadOnly )
    {
        const ClayDimensions textSizeForCursor = m_clayContext->MeasureText( InteropString( "I" ), m_style.FontId, m_style.FontSize );
        const float          cursorHeight      = textSizeForCursor.Height;
        const float          lineHeight        = m_style.LineHeight > 0 ? m_style.LineHeight : cursorHeight * 1.2f;

        float cursorX = textBounds.X;
        float cursorY = textBounds.Y;

        if ( !m_text.IsEmpty( ) && m_cursorPosition > 0 )
        {
            const std::string textStr          = m_text.Get( );
            const std::string textBeforeCursor = textStr.substr( 0, std::min( m_cursorPosition, m_text.NumChars( ) ) );

            if ( m_style.Type == ClayTextFieldType::MultiLine )
            {
                size_t lineNumber     = 0;
                size_t lastNewlinePos = 0;
                for ( size_t i = 0; i < textBeforeCursor.length( ); ++i )
                {
                    if ( textBeforeCursor[ i ] == '\n' )
                    {
                        lineNumber++;
                        lastNewlinePos = i + 1;
                    }
                }

                const std::string    textOnCurrentLine = textBeforeCursor.substr( lastNewlinePos );
                const ClayDimensions textSize          = m_clayContext->MeasureText( InteropString( textOnCurrentLine.c_str( ) ), m_style.FontId, m_style.FontSize );

                cursorX += textSize.Width;
                cursorY += lineNumber * lineHeight;
            }
            else
            {
                const ClayDimensions textSize = m_clayContext->MeasureText( InteropString( textBeforeCursor.c_str( ) ), m_style.FontId, m_style.FontSize );
                cursorX += textSize.Width;
            }
        }

        ClayBoundingBox cursorBounds;
        cursorBounds.X      = cursorX;
        cursorBounds.Y      = cursorY;
        cursorBounds.Width  = m_style.CursorWidth;
        cursorBounds.Height = cursorHeight;

        AddRectangle( renderBatch, cursorBounds, m_style.CursorColor );
    }
}

void TextFieldWidget::HandleEvent( const Event &event )
{
    m_textChanged = false;
    if ( event.Type == EventType::MouseButtonDown && event.Button.Button == MouseButton::Left )
    {
        if ( m_isHovered )
        {
            m_isFocused = true;

            const size_t clickPos = GetCharacterIndexAtPosition( event.Button.X, event.Button.Y );
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
        if ( const size_t dragPos = GetCharacterIndexAtPosition( event.Motion.X, event.Motion.Y ); dragPos != m_dragStartPos )
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
                if ( const InteropString pasteText = ClayClipboard::GetText( ); pasteText.NumChars( ) > 0 )
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
        if ( m_style.Type == ClayTextFieldType::MultiLine && !m_style.ReadOnly )
        {
            InsertText( InteropString( "\n" ) );
            m_cursorBlinkTime = 0.0f;
            m_cursorVisible   = true;
        }
        else
        {
            m_isFocused = false;
        }
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

size_t TextFieldWidget::GetCharacterIndexAtPosition( const float x, const float y ) const
{
    const auto  bounds    = GetBoundingBox( );
    const float relativeX = x - bounds.X - m_style.Padding.Left;
    const float relativeY = y - bounds.Y - m_style.Padding.Top;

    if ( relativeX <= 0 && relativeY <= 0 )
    {
        return 0;
    }

    const std::string textStr( m_text.Get( ) );
    if ( m_style.Type == ClayTextFieldType::MultiLine )
    {
        const ClayDimensions lineTextSize = m_clayContext->MeasureText( InteropString( "I" ), m_style.FontId, m_style.FontSize );
        const float          lineHeight   = m_style.LineHeight > 0 ? m_style.LineHeight : lineTextSize.Height * 1.2f;

        size_t lineNumber = std::max( 0.0f, relativeY ) / lineHeight;

        std::vector<std::string> lines;
        size_t                   start = 0;
        size_t                   pos   = 0;
        while ( pos <= textStr.length( ) )
        {
            if ( pos == textStr.length( ) || textStr[ pos ] == '\n' )
            {
                lines.push_back( textStr.substr( start, pos - start ) );
                start = pos + 1;
            }
            pos++;
        }

        if ( lineNumber >= lines.size( ) )
        {
            lineNumber = lines.size( ) > 0 ? lines.size( ) - 1 : 0;
        }

        size_t charOffset = 0;
        for ( size_t i = 0; i < lineNumber && i < lines.size( ); ++i )
        {
            charOffset += lines[ i ].length( ) + 1; // +1 for newline
        }

        if ( lineNumber < lines.size( ) )
        {
            const std::string &line = lines[ lineNumber ];
            if ( relativeX <= 0 )
            {
                return charOffset;
            }

            size_t low  = 0;
            size_t high = line.length( );
            while ( low < high )
            {
                const size_t         mid    = ( low + high ) / 2;
                std::string          substr = line.substr( 0, mid );
                const ClayDimensions dims   = m_clayContext->MeasureText( InteropString( substr.c_str( ) ), m_style.FontId, m_style.FontSize );
                if ( dims.Width < relativeX )
                {
                    low = mid + 1;
                }
                else
                {
                    high = mid;
                }
            }

            return charOffset + low;
        }

        return charOffset;
    }
    if ( relativeX <= 0 )
    {
        return 0;
    }

    size_t low  = 0;
    size_t high = textStr.length( );
    while ( low < high )
    {
        const size_t mid    = ( low + high ) / 2;
        std::string  substr = textStr.substr( 0, mid );
        if ( const ClayDimensions dims = m_clayContext->MeasureText( InteropString( substr.c_str( ) ), m_style.FontId, m_style.FontSize ); dims.Width < relativeX )
        {
            low = mid + 1;
        }
        else
        {
            high = mid;
        }
    }

    return low;
}
