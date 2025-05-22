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

#include <DenOfIzGraphics/Vector2d/VGTransform.h>

#include "DenOfIzGraphics/Utilities/InteropMathConverter.h"

using namespace DenOfIz;
using namespace DirectX;

VGTransform::VGTransform( const uint32_t width, const uint32_t height )
{
    m_transform = XMMatrixIdentity( );
    SetSize( width, height );
}

void VGTransform::SetSize( const uint32_t width, const uint32_t height )
{
    // Set up 2D screen coordinates: (0,0) at top-left, Y increases downward
    const XMMATRIX projection = XMMatrixOrthographicOffCenterLH( 0.0f, static_cast<float>( width ), 0.0f, static_cast<float>( height ), 0.0f, 1.0f );
    XMStoreFloat4x4( &m_projection, projection );
}

void VGTransform::Scale( const Float_2 scale )
{
    m_transform = m_transform * XMMatrixScaling( scale.X, scale.Y, 1.0f );
}

void VGTransform::Translate( const Float_2 translation )
{
    m_transform = m_transform * XMMatrixTranslation( translation.X, translation.Y, 0.0f );
}

void VGTransform::Rotate( const Float_2 quatXY )
{
    // Does this make sense?
    m_transform = m_transform * XMMatrixRotationZ( quatXY.X ) * XMMatrixRotationX( quatXY.Y );
}

Float_4x4 VGTransform::GetMatrix( ) const
{
    return InteropMathConverter::Float_4X4FromXMMATRIX( m_transform );
}

Float_4x4 VGTransform::GetProjectionMatrix( ) const
{
    auto projMatrix = XMLoadFloat4x4( &m_projection );
    return InteropMathConverter::Float_4X4FromXMMATRIX( projMatrix );
}

Float_4x4 VGTransform::GetCombinedMatrix( ) const
{
    // Combine view transform and projection: Projection × View
    auto projMatrix = XMLoadFloat4x4( &m_projection );
    auto combined = XMMatrixMultiply( m_transform, projMatrix );
    return InteropMathConverter::Float_4X4FromXMMATRIX( combined );
}
