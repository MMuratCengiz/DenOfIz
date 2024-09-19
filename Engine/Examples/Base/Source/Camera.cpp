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

using namespace DirectX;

Camera::Camera( const float aspectRatio, const float fovY, const float nearZ, const float farZ ) :
    m_position( XMVectorSet( 0.0f, 0.0f, -5.0f, 1.0f ) ), m_rotation( XMVectorZero( ) ), m_moveSpeed( 5.0f ), m_rotateSpeed( 2.5f ), m_yaw( 0.0f ), m_pitch( 0.0f )
{
    m_projectionMatrix = XMMatrixPerspectiveFovLH( fovY, aspectRatio, nearZ, farZ );
    UpdateViewMatrix( );
}

void Camera::Update( const float deltaTime )
{
    // Keyboard input for moving the camera
    const Uint8 *keyState = SDL_GetKeyboardState( nullptr );

    const XMVECTOR forward = XMVector3Normalize( XMVectorSet( sinf( m_yaw ), 0.0f, cosf( m_yaw ), 0.0f ) );
    const XMVECTOR right   = XMVector3Normalize( XMVectorSet( cosf( m_yaw ), 0.0f, -sinf( m_yaw ), 0.0f ) );
    const XMVECTOR up      = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

    if ( keyState[ SDL_SCANCODE_W ] )
    {
        m_position = XMVectorAdd( m_position, forward * m_moveSpeed * deltaTime );
    }
    if ( keyState[ SDL_SCANCODE_S ] )
    {
        m_position = XMVectorSubtract( m_position, forward * m_moveSpeed * deltaTime );
    }
    if ( keyState[ SDL_SCANCODE_A ] )
    {
        m_position = XMVectorSubtract( m_position, right * m_moveSpeed * deltaTime );
    }
    if ( keyState[ SDL_SCANCODE_D ] )
    {
        m_position = XMVectorAdd( m_position, right * m_moveSpeed * deltaTime );
    }
    if ( keyState[ SDL_SCANCODE_Q ] )
    {
        m_position = XMVectorSubtract( m_position, up * m_moveSpeed * deltaTime );
    }
    if ( keyState[ SDL_SCANCODE_E ] )
    {
        m_position = XMVectorAdd( m_position, up * m_moveSpeed * deltaTime );
    }
    UpdateViewMatrix( );
}

void Camera::HandleEvent( const SDL_Event &event )
{
    if ( event.type == SDL_MOUSEMOTION )
    {
        const auto mouseX = static_cast<float>( event.motion.xrel );
        const auto mouseY = static_cast<float>( event.motion.yrel );

        m_yaw += mouseX * m_rotateSpeed * 0.01f;
        m_pitch += mouseY * m_rotateSpeed * 0.01f;

        m_pitch = std::clamp( m_pitch, -XM_PIDIV2, XM_PIDIV2 ); // Prevent flipping
        UpdateViewMatrix( );
    }
}

void Camera::UpdateViewMatrix( )
{
    const XMVECTOR forward = XMVectorSet( cosf( m_pitch ) * sinf( m_yaw ), sinf( m_pitch ), cosf( m_pitch ) * cosf( m_yaw ), 0.0f );

    const XMVECTOR up     = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    const XMVECTOR target = XMVectorAdd( m_position, forward );

    m_viewMatrix = XMMatrixLookAtLH( m_position, target, up );
}

XMMATRIX Camera::ViewMatrix( ) const
{
    return m_viewMatrix;
}

XMMATRIX Camera::ProjectionMatrix( ) const
{
    return m_projectionMatrix;
}

XMMATRIX Camera::ViewProjectionMatrix( ) const
{
    return XMMatrixMultiply( m_viewMatrix, m_projectionMatrix );
}

void Camera::SetPosition( const XMVECTOR &position )
{
    m_position = position;
    UpdateViewMatrix( );
}

void Camera::SetRotation( const XMVECTOR &rotation )
{
    m_rotation = rotation;
    m_yaw      = XMVectorGetX( rotation );
    m_pitch    = XMVectorGetY( rotation );
    UpdateViewMatrix( );
}
