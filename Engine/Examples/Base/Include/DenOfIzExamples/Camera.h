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

#include <DirectXMath.h>
#include <SDl2/SDL.h>

class Camera
{
public:
    explicit Camera( float aspectRatio, float fovY = DirectX::XM_PIDIV4, float nearZ = 0.1f, float farZ = 100.0f );

    void                            Update( float deltaTime );
    [[nodiscard]] DirectX::XMMATRIX ViewMatrix( ) const;
    [[nodiscard]] DirectX::XMMATRIX ProjectionMatrix( ) const;
    [[nodiscard]] DirectX::XMMATRIX ViewProjectionMatrix( ) const;

    void HandleEvent( const SDL_Event &event );

    // Set camera position and rotation
    void SetPosition( const DirectX::XMVECTOR &position );
    void SetRotation( const DirectX::XMVECTOR &rotation );

private:
    DirectX::XMVECTOR m_position;
    DirectX::XMVECTOR m_rotation;

    DirectX::XMMATRIX m_viewMatrix{};
    DirectX::XMMATRIX m_projectionMatrix{};

    float m_moveSpeed;
    float m_rotateSpeed;

    float m_yaw;
    float m_pitch;

    void UpdateViewMatrix( );
};
