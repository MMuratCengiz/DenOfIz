/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/
#pragma once

#include <DirectXMath.h>
#include "DenOfIzExamples/Assets/AssetData.h"
#include "DenOfIzExamples/Interop.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "DenOfIzGraphics/Data/Geometry.h"

namespace DenOfIz
{
    using namespace DirectX;

    class ColoredSphereAsset final
    {
        XMFLOAT4X4 m_modelMatrix = { };
        AssetData *m_assetData   = nullptr;
        XMFLOAT4   m_color;

    public:
        DZ_EXAMPLES_API ColoredSphereAsset( DenOfIz_LogicalDevice device, DenOfIz_BatchResourceCopy batchResourceCopy, XMFLOAT4 color );
        DZ_EXAMPLES_API ~ColoredSphereAsset( );
        DZ_EXAMPLES_API void       Translate( XMFLOAT3 translation );
        DZ_EXAMPLES_API void       Rotate( XMFLOAT3 rotation );
        DZ_EXAMPLES_API void       Scale( XMFLOAT3 scale );
        DZ_EXAMPLES_API void       SetTransform( const XMMATRIX &transform );
        DZ_EXAMPLES_API XMFLOAT4   GetColor( ) const;
        DZ_EXAMPLES_API AssetData *Data( ) const;
        DZ_EXAMPLES_API XMFLOAT4X4 ModelMatrix( ) const;
    };
} // namespace DenOfIz
