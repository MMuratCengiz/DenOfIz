/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DirectXMath.h>
#include "../Interop.h"
#include "DenOfIzExamples/Assets/AssetData.h"
#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"

namespace DenOfIz
{
    using namespace DirectX;

    class SphereAsset final
    {
        XMFLOAT4X4                    m_modelMatrix = { };
        DenOfIz_Buffer                m_modelBuffer = DENOFIZ_NULL_HANDLE;
        std::unique_ptr<AssetData>    m_assetData;
        std::unique_ptr<MaterialData> m_materialData;

    public:
        DZ_EXAMPLES_API            SphereAsset( DenOfIz_LogicalDevice device, DenOfIz_BatchResourceCopy batchResourceCopy );
        DZ_EXAMPLES_API void       Translate( XMFLOAT4 translation );
        DZ_EXAMPLES_API void       Rotate( XMFLOAT4 rotation );
        DZ_EXAMPLES_API void       Scale( XMFLOAT4 scale );
        DZ_EXAMPLES_API AssetData *Data( ) const;
        DZ_EXAMPLES_API XMFLOAT4X4 ModelMatrix( ) const;
    };
} // namespace DenOfIz
