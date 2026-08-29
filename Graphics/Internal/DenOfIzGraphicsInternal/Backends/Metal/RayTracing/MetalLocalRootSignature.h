/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
