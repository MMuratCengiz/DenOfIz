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

#include <DenOfIzGraphics/Animation/AnimationUtils.h>
#include <DenOfIzGraphics/Utilities/InteropMathConverter.h>

namespace DenOfIz::AnimationUtils
{
    void ApplyCoordinateSystemCorrection( InteropArray<Float_4x4> &matrices )
    {
        using namespace DirectX;
        static const XMMATRIX correctionMatrix = XMMatrixRotationX( XM_PIDIV2 );

        for ( size_t i = 0; i < matrices.NumElements( ); ++i )
        {
            Float_4x4 &matrix   = matrices.GetElement( i );
            XMMATRIX   xmMatrix = InteropMathConverter::Float_4X4ToXMMATRIX( matrix );
            xmMatrix            = XMMatrixMultiply( xmMatrix, correctionMatrix );
            matrix              = InteropMathConverter::Float_4X4FromXMMATRIX( xmMatrix );
        }
    }
} // namespace DenOfIz::AnimationUtils
