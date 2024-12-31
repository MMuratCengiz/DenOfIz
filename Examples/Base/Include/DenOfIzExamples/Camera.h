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
#pragma once

#include <DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h>
#include <DirectXMath.h>
#include "Interop.h"

class Camera
{
public:
    const float DEFAULT_MOVE_SPEED        = 5.0f;
    const float DEFAULT_SPRINT_MULTIPLIER = 2.0f;
    const float DEFAULT_ROTATE_SPEED      = 1.25f;
    const float DEFAULT_SMOOTH_FACTOR     = 0.25f;
    const float DEFAULT_SENSITIVITY       = 0.3f;
    const float MIN_PITCH                 = -89.0f;
    const float MAX_PITCH                 = 89.0f;
    const float SCROLL_SENSITIVITY        = 0.1f;

    DZ_EXAMPLES_API explicit Camera( float aspectRatio, float fovY = DirectX::XM_PIDIV4, float nearZ = 0.01f, float farZ = 500.0f );

    DZ_EXAMPLES_API void          Update( float deltaTime );
    [[nodiscard]] DZ_EXAMPLES_API DirectX::XMVECTOR Position( ) const;
    [[nodiscard]] DZ_EXAMPLES_API DirectX::XMMATRIX ViewProjectionMatrix( ) const;

    DZ_EXAMPLES_API void HandleEvent( const SDL_Event &event );
    void                 SetPosition( const DirectX::XMVECTOR &position );
    void                 SetFront( const DirectX::XMVECTOR &front );

private:
    void UpdateViewMatrix( );
    void ResetCamera( );

    DirectX::XMVECTOR m_position;
    DirectX::XMVECTOR m_rotationQuaternion;
    DirectX::XMMATRIX m_viewMatrix;
    DirectX::XMMATRIX m_projectionMatrix;

    DirectX::XMVECTOR m_defaultPosition;
    DirectX::XMVECTOR m_targetPosition;
    DirectX::XMVECTOR m_targetRotationQuaternion;

    DirectX::XMVECTOR m_front;
    DirectX::XMVECTOR m_right;
    DirectX::XMVECTOR m_up;
    DirectX::XMVECTOR m_worldUp;

    float m_moveSpeed;
    float m_rotateSpeed;
    float m_sensitivity;
    float m_smoothFactor;

    float m_yaw;
    float m_pitch;
    float m_defaultYaw = -90.0f;
    float m_defaultPitch;
    float m_targetYaw;
    float m_targetPitch;

    bool m_firstMouse;
    int  m_lastMouseX;
    int  m_lastMouseY;

    bool m_isOrbiting;
    bool m_isDragging;
};
