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

#include <DenOfIzGraphics/UI/ClayData.h>
#include <DenOfIzGraphics/UI/Widgets/ColorPickerWidget.h>
#include <algorithm>
#include <cmath>

using namespace DenOfIz;

ColorPickerWidget::ColorPickerWidget( ClayContext *clayContext, const uint32_t id, const Float_3 &initialRgb, const ColorPickerStyle &style ) :
    Widget( clayContext, id ), m_rgb( initialRgb ), m_style( style )
{
    m_hsv = RGBToHSV( m_rgb );
}

void ColorPickerWidget::Update( float deltaTime )
{
    UpdateHoverState( );
}

void ColorPickerWidget::CreateLayoutElement( )
{

    ClayElementDeclaration decl;
    decl.Id                   = m_id;
    decl.Layout.Sizing.Width  = ClaySizingAxis::Fixed( m_isExpanded ? m_style.Size : m_style.CompactSize );
    decl.Layout.Sizing.Height = ClaySizingAxis::Fixed( m_isExpanded ? m_style.Size : m_style.CompactSize );
    decl.Custom.CustomData    = this;

    m_clayContext->OpenElement( decl );
    m_clayContext->CloseElement( );
}

void ColorPickerWidget::Render( const Clay_RenderCommand *command, IRenderBatch *renderBatch )
{
    const auto &bounds = command->boundingBox;

    ClayBoundingBox colorBounds;
    colorBounds.X      = bounds.x;
    colorBounds.Y      = bounds.y;
    colorBounds.Width  = bounds.width;
    colorBounds.Height = bounds.height;

    ClayColor currentColor;
    currentColor.R = static_cast<uint8_t>( m_rgb.X * 255 );
    currentColor.G = static_cast<uint8_t>( m_rgb.Y * 255 );
    currentColor.B = static_cast<uint8_t>( m_rgb.Z * 255 );
    currentColor.A = 255;

    AddRectangle( renderBatch, colorBounds, currentColor );

    ClayBorderWidth borderWidth( 1 );
    ClayColor       borderColor( 128, 128, 128, 255 );
    AddBorder( renderBatch, colorBounds, borderColor, borderWidth );

    // TODO: Add color picker interface (color wheel, value bar, etc.)
}

void ColorPickerWidget::HandleEvent( const Event &event )
{
    m_colorChanged = false;
    if ( event.Type == EventType::MouseButtonDown && event.Button.Button == MouseButton::Left )
    {
        if ( m_isHovered )
        {
            const auto bounds = GetBoundingBox( );

            if ( !m_isExpanded )
            {
                m_isExpanded = !m_isExpanded;
            }
            else
            {
                const float relativeX = event.Button.X - bounds.X;
                const float relativeY = event.Button.Y - bounds.Y;
                if ( relativeX > m_style.Size - m_style.ValueBarWidth - 10 )
                {
                    m_isDraggingValueBar = true;
                    UpdateFromMouseValueBar( relativeY );
                }
                else if ( relativeX < m_style.Size )
                {
                    m_isDraggingWheel = true;
                    UpdateFromMouseWheel( relativeX, relativeY );
                }
            }
            m_lastMousePos = Float_2{ static_cast<float>( event.Button.X ), static_cast<float>( event.Button.Y ) };
        }
        else if ( m_isExpanded )
        {
            m_isExpanded = false;
        }
    }
    else if ( event.Type == EventType::MouseButtonUp && event.Button.Button == MouseButton::Left )
    {
        m_isDraggingWheel    = false;
        m_isDraggingValueBar = false;
    }
    else if ( event.Type == EventType::MouseMotion )
    {
        if ( m_isDraggingWheel || m_isDraggingValueBar )
        {
            const auto  bounds    = GetBoundingBox( );
            const float relativeX = event.Motion.X - bounds.X;
            const float relativeY = event.Motion.Y - bounds.Y;

            if ( m_isDraggingValueBar )
            {
                UpdateFromMouseValueBar( relativeY );
            }
            else if ( m_isDraggingWheel )
            {
                UpdateFromMouseWheel( relativeX, relativeY );
            }
            m_lastMousePos = Float_2{ static_cast<float>( event.Motion.X ), static_cast<float>( event.Motion.Y ) };
        }
    }
}

Float_3 ColorPickerWidget::GetRGB( ) const
{
    return m_rgb;
}

Float_3 ColorPickerWidget::GetHSV( ) const
{
    return m_hsv;
}

void ColorPickerWidget::SetRGB( const Float_3 &rgb )
{
    m_rgb          = rgb;
    m_hsv          = RGBToHSV( rgb );
    m_colorChanged = true;
}

void ColorPickerWidget::SetHSV( const Float_3 &hsv )
{
    m_hsv          = hsv;
    m_rgb          = HSVToRGB( hsv );
    m_colorChanged = true;
}

bool ColorPickerWidget::WasColorChanged( ) const
{
    return m_colorChanged;
}

void ColorPickerWidget::ClearColorChangedEvent( )
{
    m_colorChanged = false;
}

bool ColorPickerWidget::IsExpanded( ) const
{
    return m_isExpanded;
}

void ColorPickerWidget::SetExpanded( const bool expanded )
{
    m_isExpanded = expanded;
}

void ColorPickerWidget::SetStyle( const ColorPickerStyle &style )
{
    m_style = style;
}

const ColorPickerStyle &ColorPickerWidget::GetStyle( ) const
{
    return m_style;
}

void ColorPickerWidget::UpdateFromMouseWheel( const float mouseX, const float mouseY )
{
    const float centerX  = m_style.Size / 2.0f;
    const float centerY  = m_style.Size / 2.0f;
    const float dx       = mouseX - centerX;
    const float dy       = mouseY - centerY;
    const float distance = std::sqrt( dx * dx + dy * dy );
    const float radius   = m_style.Size / 2.0f;

    if ( distance <= radius )
    {
        const float angle = std::atan2( dy, dx ) + XM_PI;
        const float hue   = angle / ( 2.0f * 3.14159f ) * 360.0f;

        const float saturation = std::min( distance / radius, 1.0f );

        m_hsv.X        = hue;
        m_hsv.Y        = saturation;
        m_rgb          = HSVToRGB( m_hsv );
        m_colorChanged = true;
    }
}

void ColorPickerWidget::UpdateFromMouseValueBar( const float mouseY )
{
    const float value = 1.0f - std::clamp( mouseY / m_style.Size, 0.0f, 1.0f );
    m_hsv.Z           = value;
    m_rgb             = HSVToRGB( m_hsv );
    m_colorChanged    = true;
}

Float_3 ColorPickerWidget::HSVToRGB( const Float_3 &hsv ) const
{
    const float h = hsv.X / 60.0f;
    const float s = hsv.Y;
    const float v = hsv.Z;

    const float c = v * s;
    const float x = c * ( 1.0f - std::abs( std::fmod( h, 2.0f ) - 1.0f ) );
    const float m = v - c;

    Float_3 rgb;
    if ( h < 1.0f )
    {
        rgb = Float_3{ c, x, 0 };
    }
    else if ( h < 2.0f )
    {
        rgb = Float_3{ x, c, 0 };
    }
    else if ( h < 3.0f )
    {
        rgb = Float_3{ 0, c, x };
    }
    else if ( h < 4.0f )
    {
        rgb = Float_3{ 0, x, c };
    }
    else if ( h < 5.0f )
    {
        rgb = Float_3{ x, 0, c };
    }
    else
    {
        rgb = Float_3{ c, 0, x };
    }

    return Float_3{ rgb.X + m, rgb.Y + m, rgb.Z + m };
}

Float_3 ColorPickerWidget::RGBToHSV( const Float_3 &rgb ) const
{
    float r = rgb.X;
    float g = rgb.Y;
    float b = rgb.Z;

    const float cmax  = std::max( { r, g, b } );
    const float cmin  = std::min( { r, g, b } );
    const float delta = cmax - cmin;

    Float_3 hsv;

    if ( delta == 0 )
    {
        hsv.X = 0;
    }
    else if ( cmax == r )
    {
        hsv.X = 60.0f * std::fmod( ( g - b ) / delta, 6.0f );
    }
    else if ( cmax == g )
    {
        hsv.X = 60.0f * ( ( b - r ) / delta + 2.0f );
    }
    else
    {
        hsv.X = 60.0f * ( ( r - g ) / delta + 4.0f );
    }

    if ( hsv.X < 0 )
    {
        hsv.X += 360.0f;
    }

    hsv.Y = cmax == 0 ? 0 : delta / cmax;
    hsv.Z = cmax;
    return hsv;
}
