/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzExamples/Assets/MaterialData.h"
#include "DenOfIzGraphics/Backends/Interface/BindGroup.h"
#include "DenOfIzGraphics/Backends/Interface/BindGroupLayout.h"
#include "NullTexture.h"

namespace DenOfIz
{
    class PerMaterialBinding final
    {
        DenOfIz_BindGroup            m_bindGroup = DENOFIZ_NULL_HANDLE;
        std::unique_ptr<NullTexture> m_nullTexture;

    public:
        DZ_EXAMPLES_API PerMaterialBinding( DenOfIz_LogicalDevice device, DenOfIz_BindGroupLayout layout );
        DZ_EXAMPLES_API ~PerMaterialBinding( );
        DZ_EXAMPLES_API void                            Update( const MaterialData *materialData ) const;
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_BindGroup BindGroup( ) const;

    private:
        [[nodiscard]] DenOfIz_Texture OrNull( DenOfIz_Texture texture ) const;
    };
} // namespace DenOfIz
