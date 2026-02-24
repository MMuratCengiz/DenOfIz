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

#include "DenOfIzGraphicsInternal/Backends/Common/ShaderBindingTableDebugData.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/IShaderBindingTable.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalPipeline.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalShaderLocalData.h"

namespace DenOfIz
{
    class MetalShaderBindingTable : public IShaderBindingTable
    {
    private:
        MetalContext                  *m_context;
        MetalPipeline                 *m_pipeline;
        DenOfIz_ShaderBindingTableDesc m_desc;
        size_t                         m_numBufferBytes;
        id<MTLBuffer>                  m_buffer;
        std::vector<id<MTLResource>>   m_usedResources;
        Byte                          *m_mappedMemory;

        IRVirtualAddressRange          m_rayGenerationShaderRange;
        IRVirtualAddressRangeAndStride m_hitGroupShaderRange;
        IRVirtualAddressRangeAndStride m_missShaderRange;

        uint32_t m_missGroupOffset  = 0;
        uint32_t m_hitGroupOffset   = 0;
        uint32_t m_rayGenNumBytes   = 0;
        uint32_t m_hitGroupNumBytes = 0;
        uint32_t m_missNumBytes     = 0;

        ShaderBindingTableDebugData m_debugData;

    public:
        MetalShaderBindingTable( MetalContext *context, const DenOfIz_ShaderBindingTableDesc &desc );
        void Resize( const DenOfIz_SBTSizeDesc & ) override;
        void BindRayGenerationShader( const DenOfIz_RayGenerationBindingDesc &desc ) override;
        void BindHitGroup( const DenOfIz_HitGroupBindingDesc &desc ) override;
        void BindMissShader( const DenOfIz_MissBindingDesc &desc ) override;
        void Build( ) override;
        ~MetalShaderBindingTable( ) override = default;

        const std::vector<id<MTLResource>>   &UsedResources( ) const;
        const id<MTLBuffer>                   GetMetalBuffer( ) const;
        const IRVirtualAddressRange          &RayGenerationShaderRange( ) const;
        const IRVirtualAddressRangeAndStride &HitGroupShaderRange( ) const;
        const IRVirtualAddressRangeAndStride &MissShaderRange( ) const;

    private:
        IRShaderIdentifier EncodeShaderIndex( uint32_t offset, uint32_t shaderIndex, int customIntersectionIndex = -1 );
        uint32_t           EncodeData( uint32_t offset, const MetalShaderLocalData *data );
        uint32_t           AlignRecord( const uint32_t &size );
    };
} // namespace DenOfIz
