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

#include "DenOfIzGraphics/Backends/Interface/RayTracing/ILocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalContext.h"

namespace DenOfIz
{
    struct MetalLocalBindingDesc
    {
        uint32_t            DescriptorTableIndex;
        size_t              NumBytes;
        ResourceBindingType Type;
    };

    class MetalLocalRootSignature final : public ILocalRootSignature
    {
        constexpr static MetalLocalBindingDesc empty = { };

        MetalContext          *m_context;
        LocalRootSignatureDesc m_desc;

        std::vector<MetalLocalBindingDesc> m_uavBindings;
        std::vector<MetalLocalBindingDesc> m_srvBindings;
        std::vector<MetalLocalBindingDesc> m_samplerBindings;
        std::vector<uint32_t>              m_inlineDataOffsets;
        std::vector<uint32_t>              m_inlineDataNumBytes;
        uint32_t                           m_totalInlineDataBytes = 0;

    public:
        MetalLocalRootSignature( MetalContext *context, const LocalRootSignatureDesc &desc );
        uint32_t NumInlineBytes( ) const;
        uint32_t NumSrvUavs( ) const;
        uint32_t NumSamplers( ) const;

        const uint32_t               InlineDataOffset( uint32_t binding ) const;
        const uint32_t               InlineNumBytes( uint32_t binding ) const;
        const MetalLocalBindingDesc &SrvBinding( uint32_t binding ) const;
        const MetalLocalBindingDesc &UavBinding( uint32_t binding ) const;
        const MetalLocalBindingDesc &SamplerBinding( uint32_t binding ) const;

    private:
        bool EnsureSize( uint32_t binding, const std::vector<MetalLocalBindingDesc> &bindings ) const;
    };
} // namespace DenOfIz
