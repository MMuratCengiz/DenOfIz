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

#include <DenOfIzGraphics/UI/Widgets/SliderWidget.h>
#include <algorithm>
#include <cmath>

using namespace DenOfIz;

SliderWidget::SliderWidget( IClayContext *clayContext, const uint32_t id, const float initialValue, const SliderStyle &style ) :
    Widget( clayContext, id ), m_value( std::clamp( initialValue, style.MinValue, style.MaxValue ) ), m_style( style ), m_lastMousePos( { } )
{
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
    decl.Custom.CustomData    = this;

    m_clayContext->OpenElement( decl );
    m_clayContext->CloseElement( );
}

void SliderWidget::Render( const Clay_RenderCommand *command, IRenderBatch *renderBatch )
{
    const auto &bounds = command->boundingBox;

    const float trackY       = bounds.y + ( bounds.height - m_style.Height ) * 0.5f;
    const float trackPadding = m_style.KnobSize * 0.5f;
    const float trackWidth   = bounds.width - trackPadding * 2.0f;

    ClayBoundingBox trackBounds;
    trackBounds.X      = bounds.x + trackPadding;
    trackBounds.Y      = trackY;
    trackBounds.Width  = trackWidth;
    trackBounds.Height = m_style.Height;

    AddRectangle( renderBatch, trackBounds, m_style.BackgroundColor, ClayCornerRadius( m_style.CornerRadius ) );

    const float normalizedValue = ( m_value - m_style.MinValue ) / ( m_style.MaxValue - m_style.MinValue );
    const float fillWidth       = trackWidth * normalizedValue;

    if ( fillWidth > 0 )
    {
        ClayBoundingBox fillBounds = trackBounds;
        fillBounds.Width           = fillWidth;
        AddRectangle( renderBatch, fillBounds, m_style.FillColor, ClayCornerRadius( m_style.CornerRadius ) );
    }

    const float knobX = bounds.x + trackPadding + normalizedValue * trackWidth - m_style.KnobSize * 0.5f;
    const float knobY = bounds.y + ( bounds.height - m_style.KnobSize ) * 0.5f;

    ClayBoundingBox knobBounds;
    knobBounds.X      = knobX;
    knobBounds.Y      = knobY;
    knobBounds.Width  = m_style.KnobSize;
    knobBounds.Height = m_style.KnobSize;

    AddRectangle( renderBatch, knobBounds, m_style.KnobColor, ClayCornerRadius( m_style.KnobSize * 0.5f ) );

    ClayBorderWidth knobBorderWidth( 1 );
    AddBorder( renderBatch, knobBounds, m_style.KnobBorderColor, knobBorderWidth, ClayCornerRadius( m_style.KnobSize * 0.5f ) );
}

void SliderWidget::HandleEvent( const Event &event )
{
    m_valueChanged = false;
    if ( event.Type == EventType::MouseButtonDown && event.Button.Button == MouseButton::Left )
    {
        if ( m_isHovered )
        {
            m_isDragging   = true;
            m_lastMousePos = Float_2{ static_cast<float>( event.Button.X ), static_cast<float>( event.Button.Y ) };
            UpdateValueFromMouse( static_cast<float>( event.Button.X ) );
        }
    }
    else if ( event.Type == EventType::MouseButtonUp && event.Button.Button == MouseButton::Left )
    {
        m_isDragging = false;
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
        m_value        = newValue;
        m_valueChanged = true;
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
