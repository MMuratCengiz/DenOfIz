/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
