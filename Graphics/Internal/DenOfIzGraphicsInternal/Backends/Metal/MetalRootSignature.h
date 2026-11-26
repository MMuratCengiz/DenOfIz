/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/Interface/IRootSignature.h"
#include "MetalBindGroupLayout.h"

namespace DenOfIz
{
    class MetalRootSignature final : public IRootSignature
    {
        MetalContext             *m_context;
        DenOfIz_RootSignatureDesc m_desc;

        std::vector<MetalBindGroupLayout *> m_bindGroupLayouts;

    public:
        MetalRootSignature( MetalContext *context, const DenOfIz_RootSignatureDesc &desc );
        [[nodiscard]] const std::vector<MetalBindGroupLayout *> &BindGroupLayouts( ) const;
        [[nodiscard]] MetalBindGroupLayout                      *GetBindGroupLayout( uint32_t registerSpace ) const;
        ~MetalRootSignature( ) override;
    };
} // namespace DenOfIz
