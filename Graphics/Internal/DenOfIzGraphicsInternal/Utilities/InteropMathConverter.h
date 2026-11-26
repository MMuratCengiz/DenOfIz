/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DirectXMath.h>
#include "DenOfIzGraphics/Utilities/InteropMath.h"

namespace DenOfIz
{
    class InteropMathConverter
    {
    public:
        static DirectX::XMFLOAT4X4 Float_4x4ToXMFLOAT4X4( const DenOfIz_Float4x4 &matrix );
        static DenOfIz_Float4x4    Float_4x4FromXMFLOAT4X4( const DirectX::XMFLOAT4X4 &matrix );
        static DenOfIz_Float4x4    Float_4X4FromXMMATRIX( const DirectX::XMMATRIX &matrix );
        static DirectX::XMMATRIX   Float_4X4ToXMMATRIX( const DenOfIz_Float4x4 &matrix );
    };
} // namespace DenOfIz
