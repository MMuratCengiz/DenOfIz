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

#include <DenOfIzGraphics/Backends/Interface/RayTracing/IShaderBindingTable.h>
#include <DenOfIzGraphics/Backends/Metal/MetalBufferResource.h>
#include <DenOfIzGraphics/Backends/Metal/MetalPipeline.h>

namespace DenOfIz
{
    class MetalShaderBindingTable : public IShaderBindingTable
    {
    private:
        MetalContext          *m_context;
        MetalPipeline         *m_pipeline;
        ShaderBindingTableDesc m_desc;
        size_t                 m_numBufferBytes;
        id<MTLBuffer>          m_buffer;
        IRShaderIdentifier    *m_mappedMemory;

        IRVirtualAddressRange          m_rayGenerationShaderRange;
        IRVirtualAddressRangeAndStride m_hitGroupShaderRange;
        IRVirtualAddressRangeAndStride m_missShaderRange;

        uint32_t m_missGroupOffset = 0;
        uint32_t m_hitGroupOffset  = 0;

    public:
        MetalShaderBindingTable( MetalContext *context, const ShaderBindingTableDesc &desc );
        void                                   Resize( const SBTSizeDesc                                   &) override;
        void                                   BindRayGenerationShader( const RayGenerationBindingDesc &desc ) override;
        void                                   BindHitGroup( const HitGroupBindingDesc &desc ) override;
        void                                   BindMissShader( const MissBindingDesc &desc ) override;
        void                                   Build( ) override;
        [[nodiscard]] virtual IBufferResource *Buffer( ) const override;
        ~MetalShaderBindingTable( ) override = default;

        const id<MTLBuffer>                   MetalBuffer( ) const;
        const IRVirtualAddressRange          &RayGenerationShaderRange( ) const;
        const IRVirtualAddressRangeAndStride &HitGroupShaderRange( ) const;
        const IRVirtualAddressRangeAndStride &MissShaderRange( ) const;

    private:
        bool BindHitGroupRecursive( const HitGroupBindingDesc &desc );
    };
} // namespace DenOfIz
