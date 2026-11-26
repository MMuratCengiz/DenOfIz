/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <unordered_map>
#include <vector>
#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/ILocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalContext.h"

namespace DenOfIz
{
    class MetalLocalRootSignature final : public ILocalRootSignature
    {
        MetalContext                                              *m_context;
        DenOfIz_LocalRootSignatureDesc                             m_desc;
        std::vector<DenOfIz_LocalResourceBindingDesc>              m_bindingsCopy;
        std::unordered_map<uint64_t, const DenOfIz_LocalResourceBindingDesc *> m_bindings;

    public:
        MetalLocalRootSignature( MetalContext *context, const DenOfIz_LocalRootSignatureDesc &desc );

        [[nodiscard]] const DenOfIz_LocalRootSignatureDesc     &Desc( ) const;
        [[nodiscard]] const DenOfIz_LocalResourceBindingDesc *FindBinding( DenOfIz_ResourceBindingType type, uint32_t binding ) const;
    };
} // namespace DenOfIz
