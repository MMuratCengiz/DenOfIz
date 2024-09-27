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
#include <DenOfIzExamples/Camera.h>
#include <DirectXMath.h>
#include <algorithm>
#include "DenOfIzCore/Common_Macro.h"

using namespace DirectX;

Camera::Camera( const float aspectRatio, const float fovY, const float nearZ, const float farZ ) :
    m_position( XMVectorSet( 0.0f, 0.0f, -5.0f, 1.0f ) ), m_moveSpeed( 5.0f ), m_rotateSpeed( 2.5f ), m_yaw( -90.0f ), m_pitch( 0.0f )
{
    m_worldUp          = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    m_front            = XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f );
    m_right            = XMVector3Cross( m_worldUp, m_front );
    m_up               = XMVector3Cross( m_right, m_front );
    m_projectionMatrix = XMMatrixPerspectiveFovLH( fovY, aspectRatio, nearZ, farZ );
    m_viewMatrix       = XMMatrixIdentity( );
    UpdateViewMatrix( );
}

void Camera::Update( const float deltaTime )
{
    const Uint8 *keyState    = SDL_GetKeyboardState( nullptr );
    float        sensitivity = deltaTime * m_moveSpeed;

    if ( keyState[ SDL_SCANCODE_W ] )
    {
        m_position = XMVectorAdd( m_position, XMVectorScale( m_front, sensitivity ) );
    }
    if ( keyState[ SDL_SCANCODE_S ] )
    {
        m_position = XMVectorSubtract( m_position, XMVectorScale( m_front, sensitivity ) );
    }
    if ( keyState[ SDL_SCANCODE_A ] )
    {
        m_position = XMVectorSubtract( m_position, XMVectorScale( m_right, sensitivity ) );
    }
    if ( keyState[ SDL_SCANCODE_D ] )
    {
        m_position = XMVectorAdd( m_position, XMVectorScale( m_right, sensitivity ) );
    }
    if ( keyState[ SDL_SCANCODE_R ] )
    {
        m_position = XMVectorAdd( m_position, XMVectorScale( m_up, sensitivity ) );
    }
    if ( keyState[ SDL_SCANCODE_F ] )
    {
        m_position = XMVectorSubtract( m_position, XMVectorScale( m_up, sensitivity ) );
    }
    UpdateViewMatrix( );
}

void Camera::HandleEvent( const SDL_Event &event )
{
    return;
    if ( event.type == SDL_MOUSEMOTION )
    {
        int mouseX = event.motion.xrel;
        int mouseY = event.motion.yrel;
        DZ_RETURN_IF( mouseX == 0 && mouseY == 0 );
        // mouseX = std::clamp( mouseX, -1, 1 );
        // mouseY = std::clamp( mouseY, -1, 1 );
        mouseX *= -1;
        mouseY *= -1;

        auto xOffset = static_cast<float>( mouseX );
        auto yOffset = static_cast<float>( mouseY );

        xOffset *= m_rotateSpeed;
        yOffset *= m_rotateSpeed;

        m_yaw += xOffset;
        m_pitch += yOffset;

        m_pitch                 = std::clamp( m_pitch, -89.0f, 89.0f );
        const XMVECTOR newFront = XMVectorSet( cos( XMConvertToRadians( m_yaw ) ) * cos( XMConvertToRadians( m_pitch ) ), sin( XMConvertToRadians( m_pitch ) ),
                                               sin( XMConvertToRadians( m_yaw ) ) * cos( XMConvertToRadians( m_pitch ) ), 0.0f );

        m_front = XMVector3Normalize( newFront );
        UpdateViewMatrix( );
    }
}

void Camera::UpdateViewMatrix( )
{
    m_right      = XMVector3Normalize( XMVector3Cross( m_worldUp, m_front ) );
    m_up         = XMVector3Cross( m_front, m_right );
    m_viewMatrix = XMMatrixLookAtLH( m_position, XMVectorAdd( m_position, m_front ), m_up );
}

XMMATRIX Camera::ViewProjectionMatrix( ) const
{
    return XMMatrixMultiplyTranspose( m_viewMatrix, m_projectionMatrix );
    //            return XMMatrixMultiplyTranspose( m_projectionMatrix, m_viewMatrix );
}
