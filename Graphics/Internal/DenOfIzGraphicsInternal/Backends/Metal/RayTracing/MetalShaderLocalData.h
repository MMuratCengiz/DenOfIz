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

#include <memory>
#include <vector>
#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/IShaderLocalData.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalArgumentBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalContext.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalShaderLayout.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalLocalRootSignature.h"

namespace DenOfIz
{
    struct MetalLocalConstantBinding
    {
        uint32_t          Binding;
        std::vector<Byte> Data;
    };

    struct MetalLocalResourceBinding
    {
        DenOfIz_ResourceBindingType Type;
        uint32_t                    Binding;
        id<MTLBuffer>               Buffer;
        id<MTLTexture>              Texture;
        id<MTLSamplerState>         Sampler;
    };

    struct MetalLocalSpaceTables
    {
        MetalSpaceLayout                 Layout;
        std::unique_ptr<DescriptorTable> CbvSrvUavTable;
        std::unique_ptr<DescriptorTable> SamplerTable;
    };

    struct MetalLocalRecord
    {
        uint64_t                             LayoutHash = 0;
        uint32_t                             NumBytes   = 0;
        std::vector<MetalRootConstantLayout> RootConstants;
        std::vector<MetalLocalSpaceTables>   Spaces;
        std::vector<Byte>                    Data;
        std::vector<id<MTLResource>>         UsedResources;
    };

    class MetalShaderLocalData final : public IShaderLocalData
    {
        MetalContext                          *m_context;
        DenOfIz_ShaderLocalDataDesc            m_desc;
        MetalLocalRootSignature               *m_layout;
        std::vector<MetalLocalConstantBinding> m_constants;
        std::vector<MetalLocalResourceBinding> m_resources;
        std::vector<id<MTLResource>>           m_usedResources;

        std::vector<std::unique_ptr<MetalLocalRecord>> m_records;

    public:
        MetalShaderLocalData( MetalContext *context, const DenOfIz_ShaderLocalDataDesc &desc );
        void Begin( ) override;
        void Cbv( uint32_t binding, IBuffer *bufferResource ) override;
        void Cbv( uint32_t binding, const DenOfIz_ByteArrayView &data ) override;
        void Srv( uint32_t binding, const IBuffer *bufferResource ) override;
        void Srv( uint32_t binding, const ITexture *textureResource ) override;
        void Uav( uint32_t binding, const IBuffer *bufferResource ) override;
        void Uav( uint32_t binding, const ITexture *textureResource ) override;
        void Sampler( uint32_t binding, const ISampler *sampler ) override;
        void End( ) override;

        [[nodiscard]] const MetalLocalRecord *RecordFor( const MetalShaderLayout &layout );

    private:
        void EncodeRecord( MetalLocalRecord &record ) const;
        bool ValidateBinding( DenOfIz_ResourceBindingType type, uint32_t binding ) const;
    };
} // namespace DenOfIz
