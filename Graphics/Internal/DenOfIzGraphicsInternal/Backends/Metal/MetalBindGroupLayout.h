/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <unordered_map>
#include "DenOfIzGraphicsInternal/Backends/Interface/IBindGroupLayout.h"
#include "MetalContext.h"

namespace DenOfIz
{
    class MetalBindGroupLayout final : public IBindGroupLayout
    {
        MetalContext                                                *m_context;
        std::unordered_map<uint64_t, const DenOfIz_BindingDesc *>    m_bindings;

    public:
        MetalBindGroupLayout( MetalContext *context, const DenOfIz_BindGroupLayoutDesc &desc );
        ~MetalBindGroupLayout( ) override;

        [[nodiscard]] uint32_t                   RegisterSpace( ) const;
        [[nodiscard]] const DenOfIz_BindingDesc *FindBinding( DenOfIz_ResourceBindingType type, uint32_t binding ) const;
        [[nodiscard]] MTLRenderStages            ShaderStages( DenOfIz_ResourceBindingType type, uint32_t binding ) const;
    };

} // namespace DenOfIz
