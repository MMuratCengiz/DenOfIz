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

#include <DenOfIzCore/ContainerUtilities.h>
#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include <unordered_set>

namespace DenOfIz
{
    struct MetalBindingDesc
    {
        ResourceBindingDesc Parent;
        MTLRenderStages     Stages;
    };

    struct MetalRootConstant
    {
        uint32_t Offset;
        int      NumBytes;
    };

    class MetalRootSignature final : public IRootSignature
    {
        MetalContext     *m_context;
        RootSignatureDesc m_desc;

        std::unordered_map<uint32_t, MetalBindingDesc>  m_metalBindings;
        uint32_t                                        m_numTLABAddresses   = 0;
        uint32_t                                        m_numRootConstantBytes = 0;
        std::vector<MetalRootConstant>                  m_rootConstants;

    public:
        MetalRootSignature( MetalContext *context, const RootSignatureDesc &desc );
        const MetalBindingDesc                             &FindMetalBinding( const ResourceBindingSlot &slot ) const;
        const uint32_t                                      NumTLABAddresses( ) const;
        [[nodiscard]] const uint32_t                       &NumRootConstantBytes( ) const;
        [[nodiscard]] const std::vector<MetalRootConstant> &RootConstants( ) const;
        ~MetalRootSignature( ) override;
    };

} // namespace DenOfIz
