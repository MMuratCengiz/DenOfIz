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

#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include <DenOfIzGraphics/Utilities/ContainerUtilities.h>
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
        /*
         * Each binding in the same slot could be of 3 types, each of which should have unique location, TODO(the
         * order should incrementally go like Cbv < Srv < Uav, this same order should be enforced in DxilToMsl to ensure
         * the expected locations match)
         */
        struct MetalResourceBindingIndex
        {
            int             Cbv = -1;
            MTLRenderStages CbvStages;
            int             Srv = -1;
            MTLRenderStages SrvStages;
            int             Uav = -1;
            MTLRenderStages UavStages;
        };
        struct MetalDescriptorOffsets
        {
            uint32_t                               CbvSrvUavTableOffset = 0; // Index of the CbvSrvUav table in the top level argument buffer
            uint32_t                               SamplerTableOffset   = 0; // Index of the Sampler table in the top level argument buffer
            std::unordered_map<uint32_t, uint32_t> UniqueTLABIndex;          // Unique indexes within the TopLevelArgumentBuffer for Root Level Cbvs
            std::vector<MetalResourceBindingIndex> CbvSrvUavResourceIndices; // Indexes of each binding within the CbvUavSrv table
            std::vector<uint32_t>                  SamplerResourceIndices;   // Indexes of each binding within the Sampler table
            std::vector<MTLRenderStages>           SamplerResourceStages;    // Indexes of each binding within the Sampler table
        };

        MetalContext     *m_context;
        RootSignatureDesc m_desc;

        uint32_t                                       m_numTLABAddresses     = 0;
        uint32_t                                       m_numRootConstantBytes = 0;
        std::vector<MetalRootConstant>                 m_rootConstants;
        std::vector<MetalDescriptorOffsets>            m_descriptorOffsets;

    public:
        MetalRootSignature( MetalContext *context, const RootSignatureDesc &desc );
        const uint32_t                                      NumTLABAddresses( ) const;
        [[nodiscard]] const uint32_t                       &NumRootConstantBytes( ) const;
        [[nodiscard]] const std::vector<MetalRootConstant> &RootConstants( ) const;
        const uint32_t                                      CbvSrvUavTableOffset( uint32_t registerSpace ) const;
        const uint32_t                                      CbvSrvUavResourceIndex( const ResourceBindingSlot &slot ) const;
        const MTLRenderStages                               CbvSrvUavResourceShaderStages( const ResourceBindingSlot &slot ) const;
        const uint32_t                                      SamplerTableOffset( uint32_t registerSpace ) const;
        const uint32_t                                      SamplerResourceIndex( const ResourceBindingSlot &slot ) const;
        const MTLRenderStages                               SamplerResourceShaderStages( const ResourceBindingSlot &slot ) const;
        const uint32_t                                      TLABOffset( const ResourceBindingSlot &slot ) const;
        ~MetalRootSignature( ) override;

    private:
        static IRRootParameterType GetRootParameterType( ResourceBindingType type );
    };

} // namespace DenOfIz
