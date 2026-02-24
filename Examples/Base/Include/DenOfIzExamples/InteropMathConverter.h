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

#include "DenOfIzGraphics/Utilities/InteropMath.h"

#include <DirectXMath.h>

namespace DenOfIz
{
    class InteropMathConverter
    {
    public:
        static DirectX::XMFLOAT4X4 Float_4x4ToXMFLOAT4X4( const DenOfIz_Float4x4 &matrix );
        static DenOfIz_Float4x4    Float_4x4FromXMFLOAT4X4( const DirectX::XMFLOAT4X4 &matrix );
        static DenOfIz_Float4x4    Float_4X4FromXMMATRIX( const DirectX::XMMATRIX &matrix );
        static DirectX::XMMATRIX   Float_4X4ToXMMATRIX( const DenOfIz_Float4x4 &matrix );
        static DenOfIz_Float4      Float_4FromXMVECTOR( const DirectX::XMVECTOR &vector );
        static DenOfIz_Float4      Float_4FromXMFLOAT4( const DirectX::XMFLOAT4 &vector );
        static DirectX::XMVECTOR   Float_4ToXMVECTOR( const DenOfIz_Float4 &vector );
        static DenOfIz_Float3      Float_3FromXMVECTOR( const DirectX::XMVECTOR &vector );
        static DenOfIz_Float3      Float_3FromXMFLOAT3( const DirectX::XMFLOAT3 &vector );
        static DirectX::XMVECTOR   Float_3ToXMVECTOR( const DenOfIz_Float3 &vector );
        static DenOfIz_Float2      Float_2FromXMVECTOR( const DirectX::XMVECTOR &vector );
        static DenOfIz_Float2      Float_2FromXMFLOAT2( const DirectX::XMFLOAT2 &vector );
        static DirectX::XMVECTOR   Float_2ToXMVECTOR( const DenOfIz_Float2 &vector );
    };
} // namespace DenOfIz
