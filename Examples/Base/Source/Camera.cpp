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

#include <SDL2/SDL.h>
#include "DenOfIzExamples/Camera.h"
#include <algorithm>

using namespace DirectX;

Camera::Camera( const float aspectRatio, const float fovY, const float nearZ, const float farZ ) :
    m_position( XMVectorSet( 0.0f, 0.0f, -5.0f, 1.0f ) ), m_rotationQuaternion( XMQuaternionIdentity( ) ), m_targetPosition( XMVectorSet( 0.0f, 0.0f, -5.0f, 1.0f ) ),
    m_targetRotationQuaternion( XMQuaternionIdentity( ) ), m_worldUp( XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) ), m_moveSpeed( DEFAULT_MOVE_SPEED ),
    m_rotateSpeed( DEFAULT_ROTATE_SPEED ), m_sensitivity( DEFAULT_SENSITIVITY ), m_smoothFactor( DEFAULT_SMOOTH_FACTOR ), m_yaw( -90.0f ), m_pitch( 0.0f ), m_targetYaw( -90.0f ),
    m_targetPitch( 0.0f ), m_firstMouse( true ), m_lastMouseX( 0 ), m_lastMouseY( 0 ), m_isOrbiting( false ), m_isDragging( false ), m_fovY( fovY ), m_nearZ( nearZ ),
    m_farZ( farZ )
{
    m_front = XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f );
    m_right = XMVector3Normalize( XMVector3Cross( m_worldUp, m_front ) );
    m_up    = XMVector3Cross( m_front, m_right );

    m_projectionMatrix = XMMatrixPerspectiveFovLH( fovY, aspectRatio, nearZ, farZ );
    UpdateViewMatrix( );
}

void Camera::Update( const float deltaTime )
{
    const Uint8 *keyState = SDL_GetKeyboardState( nullptr );
    float        velocity = m_moveSpeed;

    if ( keyState[ SDL_SCANCODE_LSHIFT ] )
    {
        velocity *= DEFAULT_SPRINT_MULTIPLIER;
    }
    velocity *= deltaTime;

    XMVECTOR moveDirection = XMVectorZero( );

    if ( keyState[ SDL_SCANCODE_W ] )
    {
        moveDirection = XMVectorAdd( moveDirection, m_front );
    }
    if ( keyState[ SDL_SCANCODE_S ] )
    {
        moveDirection = XMVectorSubtract( moveDirection, m_front );
    }
    if ( keyState[ SDL_SCANCODE_A ] )
    {
        moveDirection = XMVectorSubtract( moveDirection, m_right );
    }
    if ( keyState[ SDL_SCANCODE_D ] )
    {
        moveDirection = XMVectorAdd( moveDirection, m_right );
    }
    if ( keyState[ SDL_SCANCODE_E ] || keyState[ SDL_SCANCODE_SPACE ] )
    {
        moveDirection = XMVectorAdd( moveDirection, m_worldUp );
    }
    if ( keyState[ SDL_SCANCODE_Q ] || keyState[ SDL_SCANCODE_LCTRL ] )
    {
        moveDirection = XMVectorSubtract( moveDirection, m_worldUp );
    }

    if ( !XMVector3Equal( moveDirection, XMVectorZero( ) ) )
    {
        moveDirection    = XMVector3Normalize( moveDirection );
        m_targetPosition = XMVectorAdd( m_targetPosition, XMVectorScale( moveDirection, velocity ) );
    }

    m_position = XMVectorLerp( m_position, m_targetPosition, m_smoothFactor );

    m_yaw   = m_yaw + ( m_targetYaw - m_yaw ) * m_smoothFactor;
    m_pitch = m_pitch + ( m_targetPitch - m_pitch ) * m_smoothFactor;

    const XMVECTOR newFront = XMVectorSet( cos( XMConvertToRadians( m_yaw ) ) * cos( XMConvertToRadians( m_pitch ) ), sin( XMConvertToRadians( m_pitch ) ),
                                           sin( XMConvertToRadians( m_yaw ) ) * cos( XMConvertToRadians( m_pitch ) ), 0.0f );

    m_front = XMVector3Normalize( newFront );
    UpdateViewMatrix( );
}

void Camera::HandleEvent( const DenOfIz::Event &event )
{
    switch ( event.Type )
    {
    case DenOfIz::EventType::MouseMotion:
        {
            const int mouseX = event.Motion.X;
            const int mouseY = event.Motion.Y;

            if ( m_firstMouse )
            {
                m_lastMouseX  = mouseX;
                m_lastMouseY  = mouseY;
                m_firstMouse  = false;
                m_targetYaw   = m_yaw;
                m_targetPitch = m_pitch;
                return;
            }

            if ( SDL_GetMouseState( nullptr, nullptr ) & SDL_BUTTON( SDL_BUTTON_RIGHT ) || m_isOrbiting )
            {
                float xOffset = static_cast<float>( mouseX - m_lastMouseX );
                float yOffset = static_cast<float>( m_lastMouseY - mouseY );

                if ( abs( xOffset ) < 0.1f && abs( yOffset ) < 0.1f )
                {
                    m_lastMouseX = mouseX;
                    m_lastMouseY = mouseY;
                    return;
                }

                xOffset *= m_sensitivity;
                yOffset *= m_sensitivity;

                xOffset = -xOffset;

                m_targetYaw += xOffset * m_rotateSpeed;
                m_targetPitch += yOffset * m_rotateSpeed;

                m_targetPitch = std::clamp( m_targetPitch, MIN_PITCH, MAX_PITCH );
            }

            if ( SDL_GetMouseState( nullptr, nullptr ) & SDL_BUTTON( SDL_BUTTON_MIDDLE ) )
            {
                const float panX = static_cast<float>( mouseX - m_lastMouseX ) * m_sensitivity * 0.05f;
                const float panY = static_cast<float>( mouseY - m_lastMouseY ) * m_sensitivity * 0.05f;

                m_targetPosition = XMVectorAdd( m_targetPosition, XMVectorScale( m_right, -panX ) );
                m_targetPosition = XMVectorAdd( m_targetPosition, XMVectorScale( m_up, panY ) );
            }

            m_lastMouseX = mouseX;
            m_lastMouseY = mouseY;
            break;
        }

    case DenOfIz::EventType::MouseWheel:
        {
            const float zoomAmount = event.Wheel.Y * SCROLL_SENSITIVITY;
            m_targetPosition       = XMVectorAdd( m_targetPosition, XMVectorScale( m_front, zoomAmount * m_moveSpeed ) );
            break;
        }

    case DenOfIz::EventType::MouseButtonDown:
        {
            if ( event.Button.Button == DenOfIz::MouseButton::Right )
            {
                SDL_SetRelativeMouseMode( SDL_TRUE );
                m_lastMouseX = event.Button.X;
                m_lastMouseY = event.Button.Y;
                m_firstMouse = true;
            }
            break;
        }

    case DenOfIz::EventType::MouseButtonUp:
        {
            if ( event.Button.Button == DenOfIz::MouseButton::Right )
            {
                SDL_SetRelativeMouseMode( SDL_FALSE );
                m_yaw   = m_targetYaw;
                m_pitch = m_targetPitch;
            }
            break;
        }

    case DenOfIz::EventType::KeyDown:
        {
            switch ( event.Key.Keycode )
            {
            case DenOfIz::KeyCode::F:
                if ( event.Key.Mod.IsSet( DenOfIz::KeyMod::Ctrl ) )
                {
                    ResetCamera( );
                }
                break;
            case DenOfIz::KeyCode::Space:
                if ( event.Key.Mod.IsSet( DenOfIz::KeyMod::Alt ) )
                {
                    m_isOrbiting = !m_isOrbiting;
                }
                break;
            default:
                break;
            }
            break;
        }
    default:
        break;
    }
}

void Camera::SetPosition( const XMVECTOR &position )
{
    m_position        = position;
    m_targetPosition  = position;
    m_defaultPosition = position;
    UpdateViewMatrix( );
}

void Camera::SetFront( const XMVECTOR &front )
{
    m_front = XMVector3Normalize( front );

    XMFLOAT3 frontFloat;
    XMStoreFloat3( &frontFloat, m_front );

    m_yaw   = XMConvertToDegrees( atan2f( frontFloat.z, frontFloat.x ) );
    m_pitch = XMConvertToDegrees( asinf( frontFloat.y ) );

    m_targetYaw   = m_yaw;
    m_targetPitch = m_pitch;

    m_defaultYaw   = m_yaw;
    m_defaultPitch = m_pitch;

    UpdateViewMatrix( );
}

void Camera::UpdateViewMatrix( )
{
    m_right = XMVector3Normalize( XMVector3Cross( m_worldUp, m_front ) );
    m_up    = XMVector3Cross( m_front, m_right );

    const XMVECTOR target = XMVectorAdd( m_position, m_front );
    m_viewMatrix          = XMMatrixLookAtLH( m_position, target, m_up );
}

void Camera::ResetCamera( )
{
    m_targetPosition = m_defaultPosition;
    m_targetYaw      = m_defaultYaw;
    m_targetPitch    = m_defaultPitch;
}

XMVECTOR Camera::Position( ) const
{
    return m_position;
}

XMMATRIX Camera::ViewProjectionMatrix( ) const
{
    return XMMatrixMultiply( m_viewMatrix, m_projectionMatrix );
}

void Camera::SetAspectRatio( const float aspectRatio )
{
    m_projectionMatrix = XMMatrixPerspectiveFovLH( m_fovY, aspectRatio, m_nearZ, m_farZ );
}
