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
#include <DenOfIzGraphics/UI/Widgets/SliderWidget.h>
#include <algorithm>
#include <cmath>

using namespace DenOfIz;

SliderWidget::SliderWidget( Clay *clay, const uint32_t id, const float initialValue, const SliderStyle &style ) :
    Widget( clay, id ), m_value( std::clamp( initialValue, style.MinValue, style.MaxValue ) ), m_style( style ), m_lastMousePos( { } )
{
    m_sliderState.Value          = m_value;
    m_sliderState.IsBeingDragged = false;
}

void SliderWidget::Update( float deltaTime )
{
    UpdateHoverState( );
}

void SliderWidget::CreateLayoutElement( )
{
    ClayElementDeclaration decl;
    decl.Id                   = m_id;
    decl.Layout.Sizing.Width  = ClaySizingAxis::Grow( );
    decl.Layout.Sizing.Height = ClaySizingAxis::Fixed( m_style.KnobSize );
    decl.Custom.CustomData    = &m_widgetData;

    m_clay->OpenElement( decl );
    m_clay->CloseElement( );
}

void SliderWidget::Render( )
{
    m_sliderState.Value          = m_value;
    m_sliderState.IsBeingDragged = m_isDragging;

    m_renderData.State     = &m_sliderState;
    m_renderData.Desc      = m_style;
    m_renderData.ElementId = m_id;

    m_widgetData.Type = ClayCustomWidgetType::Slider;
    m_widgetData.Data = &m_renderData;
}

void SliderWidget::HandleEvent( const Event &event )
{
    m_valueChanged = false;

    if ( event.Type == EventType::MouseButtonDown && event.Button.Button == MouseButton::Left )
    {
        if ( m_isHovered )
        {
            m_isDragging                 = true;
            m_sliderState.IsBeingDragged = true;
            m_lastMousePos               = Float_2{ static_cast<float>( event.Button.X ), static_cast<float>( event.Button.Y ) };
            UpdateValueFromMouse( static_cast<float>( event.Button.X ) );
        }
    }
    else if ( event.Type == EventType::MouseButtonUp && event.Button.Button == MouseButton::Left )
    {
        m_isDragging                 = false;
        m_sliderState.IsBeingDragged = false;
    }
    else if ( event.Type == EventType::MouseMotion && m_isDragging )
    {
        m_lastMousePos = Float_2{ static_cast<float>( event.Motion.X ), static_cast<float>( event.Motion.Y ) };
        UpdateValueFromMouse( static_cast<float>( event.Motion.X ) );
    }
}

float SliderWidget::GetValue( ) const
{
    return m_value;
}

void SliderWidget::SetValue( const float value )
{
    const float newValue = std::clamp( value, m_style.MinValue, m_style.MaxValue );
    if ( m_value != newValue )
    {
        m_value             = newValue;
        m_sliderState.Value = newValue;
        m_valueChanged      = true;
    }
}

bool SliderWidget::WasValueChanged( ) const
{
    return m_valueChanged;
}

void SliderWidget::ClearValueChangedEvent( )
{
    m_valueChanged = false;
}

bool SliderWidget::IsDragging( ) const
{
    return m_isDragging;
}

void SliderWidget::SetStyle( const SliderStyle &style )
{
    m_style = style;
}

const SliderStyle &SliderWidget::GetStyle( ) const
{
    return m_style;
}

void SliderWidget::UpdateValueFromMouse( const float mouseX )
{
    const auto  bounds          = GetBoundingBox( );
    const float relativeX       = mouseX - bounds.X;
    const float normalizedValue = std::clamp( relativeX / bounds.Width, 0.0f, 1.0f );

    float newValue = m_style.MinValue + normalizedValue * ( m_style.MaxValue - m_style.MinValue );

    if ( m_style.Step > 0 )
    {
        newValue = std::round( newValue / m_style.Step ) * m_style.Step;
    }
    SetValue( newValue );
}
