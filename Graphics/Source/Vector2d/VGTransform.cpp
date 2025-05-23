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
    // Match FrameDebugRenderer coordinate system: Y flipped so (0,0) is bottom-left, Y increases upward
    const XMMATRIX projection = XMMatrixOrthographicOffCenterLH( 0.0f, static_cast<float>( width ), static_cast<float>( height ), 0.0f, 0.0f, 1.0f );
    XMStoreFloat4x4( &m_projection, projection );
}

void VGTransform::SetTransform( const Float_4x4 &transform )
{
    m_transform = XMLoadFloat4x4( reinterpret_cast<const XMFLOAT4X4 *>( &transform ) );
}

void VGTransform::ResetTransform( )
{
    m_transform = XMMatrixIdentity( );
}

void VGTransform::Transform( const Float_4x4 &matrix )
{
    const auto newTransform = XMLoadFloat4x4( reinterpret_cast<const XMFLOAT4X4 *>( &matrix ) );
    ApplyTransform( newTransform );
}

void VGTransform::Translate( const Float_2 &offset )
{
    const auto translation = XMMatrixTranslation( offset.X, offset.Y, 0.0f );
    ApplyTransform( translation );
}

void VGTransform::Scale( const Float_2 &scale )
{
    const auto scaling = XMMatrixScaling( scale.X, scale.Y, 1.0f );
    ApplyTransform( scaling );
}

void VGTransform::Scale( const float scale )
{
    Scale( { scale, scale } );
}

void VGTransform::Rotate( const float angleRadians )
{
    const auto rotation = XMMatrixRotationZ( angleRadians );
    ApplyTransform( rotation );
}

void VGTransform::Rotate( const float angleRadians, const Float_2 &center )
{
    // Translate to origin, rotate, translate back
    const auto translate1 = XMMatrixTranslation( -center.X, -center.Y, 0.0f );
    const auto rotation   = XMMatrixRotationZ( angleRadians );
    const auto translate2 = XMMatrixTranslation( center.X, center.Y, 0.0f );
    const auto transform  = XMMatrixMultiply( XMMatrixMultiply( translate1, rotation ), translate2 );
    ApplyTransform( transform );
}

void VGTransform::Skew( const Float_2 &skew )
{
    // Create skew matrix manually
    const XMFLOAT4X4 skewMatrix = { 1.0f, skew.Y, 0.0f, 0.0f, skew.X, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    const auto transform = XMLoadFloat4x4( &skewMatrix );
    ApplyTransform( transform );
}

void VGTransform::PushTransform( )
{
    m_transformStack.push_back( m_transform );
}

void VGTransform::PushTransform( const Float_4x4 &transform )
{
    PushTransform( );
    Transform( transform );
}

void VGTransform::PopTransform( )
{
    if ( !m_transformStack.empty( ) )
    {
        m_transform = m_transformStack.back( );
        m_transformStack.pop_back( );
    }
}

Float_4x4 VGTransform::GetMatrix( ) const
{
    return InteropMathConverter::Float_4X4FromXMMATRIX( m_transform );
}

Float_4x4 VGTransform::GetProjectionMatrix( ) const
{
    const auto projMatrix = XMLoadFloat4x4( &m_projection );
    return InteropMathConverter::Float_4X4FromXMMATRIX( projMatrix );
}

Float_4x4 VGTransform::GetCombinedMatrix( ) const
{
    const auto projMatrix = XMLoadFloat4x4( &m_projection );
    const auto combined = XMMatrixMultiply( m_transform, projMatrix );
    return InteropMathConverter::Float_4X4FromXMMATRIX( combined );
}

void VGTransform::ApplyTransform( const XMMATRIX &transform )
{
    m_transform = XMMatrixMultiply( transform, m_transform );
}