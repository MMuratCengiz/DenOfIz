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
#include <DenOfIzGraphics/UI/Widgets/CheckboxWidget.h>

using namespace DenOfIz;

CheckboxWidget::CheckboxWidget( Clay *clay, const uint32_t id, const bool initialChecked, const CheckboxStyle &style ) : Widget( clay, id ), m_isChecked( initialChecked ), m_style( style )
{
    m_checkboxState.Checked = initialChecked;
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
    decl.Custom.CustomData    = &m_widgetData;

    m_clay->OpenElement( decl );
    m_clay->CloseElement( );
}

void CheckboxWidget::Render( )
{
    m_checkboxState.Checked = m_isChecked;

    m_renderData.State     = &m_checkboxState;
    m_renderData.Desc      = m_style;
    m_renderData.ElementId = m_id;

    m_widgetData.Type = ClayCustomWidgetType::Checkbox;
    m_widgetData.Data = &m_renderData;
}

void CheckboxWidget::HandleEvent( const Event &event )
{
    m_wasClicked = false;

    if ( event.Type == EventType::MouseButtonDown && event.Button.Button == MouseButton::Left )
    {
        if ( m_isHovered )
        {
            m_isChecked             = !m_isChecked;
            m_checkboxState.Checked = m_isChecked;
            m_wasClicked            = true;
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
        m_isChecked             = checked;
        m_checkboxState.Checked = checked;
        m_wasClicked            = true;
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
