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

#include "Interop.h"
#include <DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h>
#include <DirectXMath.h>

class Camera
{
public:
    DZ_EXAMPLES_API explicit Camera( float aspectRatio, float fovY = DirectX::XM_PIDIV4, float nearZ = 0.1f, float farZ = 100.0f );

    DZ_EXAMPLES_API void          Update( float deltaTime );
    [[nodiscard]] DZ_EXAMPLES_API DirectX::XMMATRIX ViewProjectionMatrix( ) const;

    DZ_EXAMPLES_API void HandleEvent( const SDL_Event &event );

private:
    DirectX::XMVECTOR m_position{ };
    DirectX::XMMATRIX m_viewMatrix{ };
    DirectX::XMMATRIX m_projectionMatrix{ };
    DirectX::XMVECTOR m_rotationQuaternion = DirectX::XMQuaternionIdentity( );
    DirectX::XMVECTOR m_front;
    DirectX::XMVECTOR m_right;
    DirectX::XMVECTOR m_up;
    DirectX::XMVECTOR m_worldUp;

    float m_moveSpeed;
    float m_rotateSpeed;

    float m_yaw;
    float m_pitch;

    void UpdateViewMatrix( );
};
