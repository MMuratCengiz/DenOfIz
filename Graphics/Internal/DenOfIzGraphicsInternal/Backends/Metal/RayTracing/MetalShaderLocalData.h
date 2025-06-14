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

#include "DenOfIzGraphics/Backends/Interface/RayTracing/IShaderLocalData.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalArgumentBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalContext.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalLocalRootSignature.h"

namespace DenOfIz
{
    class MetalShaderLocalData final : public IShaderLocalData
    {
        MetalContext                        *m_context;
        ShaderLocalDataDesc                  m_desc;
        MetalLocalRootSignature             *m_layout;
        std::unique_ptr<DescriptorTable>     m_srvUavTable;
        std::unique_ptr<DescriptorTable>     m_samplerTable;
        std::vector<id<MTLResource>>         m_usedResources;
        std::vector<Byte>                    m_data;

    public:
        MetalShaderLocalData( MetalContext *context, const ShaderLocalDataDesc &desc );
        void Begin( ) override;
        void Cbv( uint32_t binding, IBufferResource *bufferResource ) override;
        void Cbv( uint32_t binding, const ByteArrayView &data ) override;
        void Srv( uint32_t binding, const IBufferResource *bufferResource ) override;
        void Srv( uint32_t binding, const ITextureResource *textureResource ) override;
        void Uav( uint32_t binding, const IBufferResource *bufferResource ) override;
        void Uav( uint32_t binding, const ITextureResource *textureResource ) override;
        void Sampler( uint32_t binding, const ISampler *sampler ) override;
        void End( ) override;

        [[nodiscard]] const DescriptorTable *SrvUavTable( ) const;
        [[nodiscard]] const DescriptorTable *SamplerTable( ) const;
        [[nodiscard]] const std::vector<id<MTLResource>> &UsedResources( ) const;
        [[nodiscard]] uint32_t    DataNumBytes( ) const;
        [[nodiscard]] const Byte *Data( ) const;
    private:
        void EnsureSrvUavTable( );
        void EnsureSamplerTable( );
    };
} // namespace DenOfIz
