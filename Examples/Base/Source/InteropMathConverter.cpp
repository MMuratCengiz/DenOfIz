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

#include "DenOfIzExamples/InteropMathConverter.h"

using namespace DenOfIz;

#ifndef __EMSCRIPTEN__
DirectX::XMFLOAT4X4 InteropMathConverter::Float_4x4ToXMFLOAT4X4( const DenOfIz_Float4x4 &matrix )
{
    DirectX::XMFLOAT4X4 result;
    result._11 = matrix._11;
    result._12 = matrix._12;
    result._13 = matrix._13;
    result._14 = matrix._14;
    result._21 = matrix._21;
    result._22 = matrix._22;
    result._23 = matrix._23;
    result._24 = matrix._24;
    result._31 = matrix._31;
    result._32 = matrix._32;
    result._33 = matrix._33;
    result._34 = matrix._34;
    result._41 = matrix._41;
    result._42 = matrix._42;
    result._43 = matrix._43;
    result._44 = matrix._44;
    return result;
}

DenOfIz_Float4x4 InteropMathConverter::Float_4x4FromXMFLOAT4X4( const DirectX::XMFLOAT4X4 &matrix )
{
    DenOfIz_Float4x4 result;
    result._11 = matrix._11;
    result._12 = matrix._12;
    result._13 = matrix._13;
    result._14 = matrix._14;
    result._21 = matrix._21;
    result._22 = matrix._22;
    result._23 = matrix._23;
    result._24 = matrix._24;
    result._31 = matrix._31;
    result._32 = matrix._32;
    result._33 = matrix._33;
    result._34 = matrix._34;
    result._41 = matrix._41;
    result._42 = matrix._42;
    result._43 = matrix._43;
    result._44 = matrix._44;
    return result;
}

DenOfIz_Float4x4 InteropMathConverter::Float_4X4FromXMMATRIX( const DirectX::XMMATRIX &matrix )
{
    DenOfIz_Float4x4 result;
    result._11 = DirectX::XMVectorGetX( matrix.r[ 0 ] );
    result._12 = DirectX::XMVectorGetY( matrix.r[ 0 ] );
    result._13 = DirectX::XMVectorGetZ( matrix.r[ 0 ] );
    result._14 = DirectX::XMVectorGetW( matrix.r[ 0 ] );
    result._21 = DirectX::XMVectorGetX( matrix.r[ 1 ] );
    result._22 = DirectX::XMVectorGetY( matrix.r[ 1 ] );
    result._23 = DirectX::XMVectorGetZ( matrix.r[ 1 ] );
    result._24 = DirectX::XMVectorGetW( matrix.r[ 1 ] );
    result._31 = DirectX::XMVectorGetX( matrix.r[ 2 ] );
    result._32 = DirectX::XMVectorGetY( matrix.r[ 2 ] );
    result._33 = DirectX::XMVectorGetZ( matrix.r[ 2 ] );
    result._34 = DirectX::XMVectorGetW( matrix.r[ 2 ] );
    result._41 = DirectX::XMVectorGetX( matrix.r[ 3 ] );
    result._42 = DirectX::XMVectorGetY( matrix.r[ 3 ] );
    result._43 = DirectX::XMVectorGetZ( matrix.r[ 3 ] );
    result._44 = DirectX::XMVectorGetW( matrix.r[ 3 ] );
    return result;
}

DirectX::XMMATRIX InteropMathConverter::Float_4X4ToXMMATRIX( const DenOfIz_Float4x4 &matrix )
{
    // clang-format off
        return DirectX::XMMatrixSet(
            matrix._11, matrix._12, matrix._13, matrix._14,
            matrix._21, matrix._22, matrix._23, matrix._24,
            matrix._31, matrix._32, matrix._33, matrix._34,
            matrix._41, matrix._42, matrix._43, matrix._44
        );
    // clang-format on
}
#endif

DenOfIz_Float4 InteropMathConverter::Float_4FromXMVECTOR( const DirectX::XMVECTOR &vector )
{
    return DenOfIz_Float4{ DirectX::XMVectorGetX( vector ), DirectX::XMVectorGetY( vector ), DirectX::XMVectorGetZ( vector ), DirectX::XMVectorGetW( vector ) };
}

DenOfIz_Float4 InteropMathConverter::Float_4FromXMFLOAT4( const DirectX::XMFLOAT4 &vector )
{
    return DenOfIz_Float4{ vector.x, vector.y, vector.z, vector.w };
}

DirectX::XMVECTOR InteropMathConverter::Float_4ToXMVECTOR( const DenOfIz_Float4 &vector )
{
    return DirectX::XMVectorSet( vector.X, vector.Y, vector.Z, vector.W );
}

DenOfIz_Float3 InteropMathConverter::Float_3FromXMVECTOR( const DirectX::XMVECTOR &vector )
{
    return DenOfIz_Float3{ DirectX::XMVectorGetX( vector ), DirectX::XMVectorGetY( vector ), DirectX::XMVectorGetZ( vector ) };
}

DenOfIz_Float3 InteropMathConverter::Float_3FromXMFLOAT3( const DirectX::XMFLOAT3 &vector )
{
    return DenOfIz_Float3{ vector.x, vector.y, vector.z };
}

DirectX::XMVECTOR InteropMathConverter::Float_3ToXMVECTOR( const DenOfIz_Float3 &vector )
{
    return DirectX::XMVectorSet( vector.X, vector.Y, vector.Z, 0.0f );
}

DenOfIz_Float2 InteropMathConverter::Float_2FromXMVECTOR( const DirectX::XMVECTOR &vector )
{
    return DenOfIz_Float2{ DirectX::XMVectorGetX( vector ), DirectX::XMVectorGetY( vector ) };
}

DenOfIz_Float2 InteropMathConverter::Float_2FromXMFLOAT2( const DirectX::XMFLOAT2 &vector )
{
    return DenOfIz_Float2{ vector.x, vector.y };
}

DirectX::XMVECTOR InteropMathConverter::Float_2ToXMVECTOR( const DenOfIz_Float2 &vector )
{
    return DirectX::XMVectorSet( vector.X, vector.Y, 0.0f, 0.0f );
}
