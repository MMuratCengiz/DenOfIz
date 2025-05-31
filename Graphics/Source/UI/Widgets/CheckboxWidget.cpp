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

#include <DenOfIzGraphics/UI/Widgets/CheckboxWidget.h>

using namespace DenOfIz;

CheckboxWidget::CheckboxWidget( ClayContext *clayContext, const uint32_t id, const bool initialChecked, const CheckboxStyle &style ) :
    Widget( clayContext, id ), m_isChecked( initialChecked ), m_style( style )
{
}

void CheckboxWidget::Update( float deltaTime )
{
    UpdateHoverState( );
}

void CheckboxWidget::CreateLayoutElement( )
{
    ClayElementDeclaration decl;
    decl.Id                   = m_id;
    decl.Layout.Sizing.Width  = ClaySizingAxis::Fixed( m_style.Size );
    decl.Layout.Sizing.Height = ClaySizingAxis::Fixed( m_style.Size );
    decl.Custom.CustomData    = this;

    m_clayContext->OpenElement( decl );
    m_clayContext->CloseElement( );
}

void CheckboxWidget::Render( const Clay_RenderCommand *command, IRenderBatch *renderBatch )
{
    const auto &bounds = command->boundingBox;

    ClayBoundingBox checkboxBounds;
    checkboxBounds.X      = bounds.x;
    checkboxBounds.Y      = bounds.y;
    checkboxBounds.Width  = bounds.width;
    checkboxBounds.Height = bounds.height;

    ClayColor backgroundColor = m_isHovered ? m_style.HoverBackgroundColor : m_style.BackgroundColor;
    ClayColor borderColor     = m_isHovered ? m_style.HoverBorderColor : m_style.BorderColor;

    AddRectangle( renderBatch, checkboxBounds, backgroundColor, ClayCornerRadius( m_style.CornerRadius ) );

    ClayBorderWidth borderWidth( static_cast<uint16_t>( m_style.BorderWidth ) );
    AddBorder( renderBatch, checkboxBounds, borderColor, borderWidth, ClayCornerRadius( m_style.CornerRadius ) );

    if ( m_isChecked )
    {
        const float checkSize    = m_style.Size * 0.6f;
        const float checkOffsetX = bounds.x + ( m_style.Size - checkSize ) * 0.5f;
        const float checkOffsetY = bounds.y + ( m_style.Size - checkSize ) * 0.5f;

        ClayBoundingBox checkBounds;
        checkBounds.X      = checkOffsetX;
        checkBounds.Y      = checkOffsetY;
        checkBounds.Width  = checkSize;
        checkBounds.Height = checkSize;

        AddRectangle( renderBatch, checkBounds, m_style.CheckColor );
    }
}

void CheckboxWidget::HandleEvent( const Event &event )
{
    m_wasClicked = false;

    if ( event.Type == EventType::MouseButtonDown && event.Button.Button == MouseButton::Left )
    {
        if ( m_isHovered )
        {
            m_isChecked  = !m_isChecked;
            m_wasClicked = true;
        }
    }
}

bool CheckboxWidget::IsChecked( ) const
{
    return m_isChecked;
}

void CheckboxWidget::SetChecked( const bool checked )
{
    if ( m_isChecked != checked )
    {
        m_isChecked  = checked;
        m_wasClicked = true;
    }
}

bool CheckboxWidget::WasClicked( ) const
{
    return m_wasClicked;
}

void CheckboxWidget::ClearClickEvent( )
{
    m_wasClicked = false;
}

void CheckboxWidget::SetStyle( const CheckboxStyle &style )
{
    m_style = style;
}

const CheckboxStyle &CheckboxWidget::GetStyle( ) const
{
    return m_style;
}
