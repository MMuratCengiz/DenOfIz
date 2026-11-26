/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/BindGroup.h"
#include "DenOfIzGraphics/Backends/Interface/BindGroupLayout.h"
#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "FreeCamera.h"

namespace DenOfIz
{
    class PerFrameBinding final
    {
        DenOfIz_Buffer    m_deltaTimeBuffer          = DENOFIZ_NULL_HANDLE;
        void             *m_deltaTimeMappedData      = nullptr;
        DenOfIz_Buffer    m_viewProjectionBuffer     = DENOFIZ_NULL_HANDLE;
        void             *m_viewProjectionMappedData = nullptr;
        DenOfIz_BindGroup m_bindGroup                = DENOFIZ_NULL_HANDLE;

    public:
        DZ_EXAMPLES_API PerFrameBinding( DenOfIz_LogicalDevice device, DenOfIz_BindGroupLayout layout );
        DZ_EXAMPLES_API ~PerFrameBinding( );
        DZ_EXAMPLES_API void                            Update( const FreeCamera *camera, float deltaTime ) const;
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_BindGroup BindGroup( ) const;
    };
} // namespace DenOfIz
