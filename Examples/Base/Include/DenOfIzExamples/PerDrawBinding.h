/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DirectXMath.h>
#include "DenOfIzExamples/Assets/AssetData.h"
#include "DenOfIzGraphics/Backends/Interface/BindGroup.h"
#include "DenOfIzGraphics/Backends/Interface/BindGroupLayout.h"
#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "Interop.h"

using namespace DirectX;

namespace DenOfIz
{
    class PerDrawBinding final
    {
        void             *m_modelMatrixMappedData = nullptr;
        DenOfIz_Buffer    m_modelMatrixBuffer     = DENOFIZ_NULL_HANDLE;
        DenOfIz_BindGroup m_bindGroup             = DENOFIZ_NULL_HANDLE;

    public:
        DZ_EXAMPLES_API PerDrawBinding( DenOfIz_LogicalDevice device, DenOfIz_BindGroupLayout layout );
        DZ_EXAMPLES_API ~PerDrawBinding( );
        DZ_EXAMPLES_API void                            Update( const XMFLOAT4X4 &modelMatrix ) const;
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_BindGroup BindGroup( ) const;
    };
} // namespace DenOfIz
