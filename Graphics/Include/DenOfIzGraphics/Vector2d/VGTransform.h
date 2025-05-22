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

#include <DenOfIzGraphics/Utilities/InteropMath.h>
#include <DirectXMath.h>

namespace DenOfIz
{
    class VGTransform
    {
        DirectX::XMFLOAT4X4 m_projection{ };
        DirectX::XMMATRIX m_transform{ };

    public:
        DZ_API VGTransform( uint32_t width, uint32_t height );
        DZ_API void      SetSize( uint32_t width, uint32_t height );
        DZ_API void      Scale( Float_2 scale );
        DZ_API void      Translate( Float_2 translation );
        DZ_API void      Rotate( Float_2 quatXY );
        DZ_API Float_4x4 GetMatrix( ) const;
        DZ_API Float_4x4 GetProjectionMatrix( ) const;
        DZ_API Float_4x4 GetCombinedMatrix( ) const; // projection * transform
    };
} // namespace DenOfIz
