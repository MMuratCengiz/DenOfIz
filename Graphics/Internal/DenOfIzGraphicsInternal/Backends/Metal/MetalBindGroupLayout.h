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
#include "DenOfIzGraphicsInternal/Backends/Interface/IBindGroupLayout.h"
#include "MetalContext.h"

namespace DenOfIz
{

    struct MetalResourceBindingIndex
    {
        int             Cbv = -1;
        MTLRenderStages CbvStages;
        int             Srv = -1;
        MTLRenderStages SrvStages;
        int             Uav = -1;
        MTLRenderStages UavStages;
    };

    class MetalBindGroupLayout final : public IBindGroupLayout
    {
    private:
        MetalContext               *m_context;
        DenOfIz_BindGroupLayoutDesc m_desc;

        uint32_t                               m_cbvSrvUavTableOffset   = UINT_MAX;
        uint32_t                               m_samplerTableOffset     = UINT_MAX;
        uint32_t                               m_cbvSrvUavResourceCount = 0;
        std::unordered_map<uint32_t, uint32_t> m_uniqueTLABIndex;
        std::vector<MetalResourceBindingIndex> m_cbvSrvUavResourceIndices;
        std::vector<uint32_t>                  m_samplerResourceIndices;
        std::vector<MTLRenderStages>           m_samplerResourceStages;

        bool     m_hasCbvSrvUav         = false;
        bool     m_hasSamplers          = false;
        bool     m_hasBindless          = false;
        uint32_t m_numRootLevelBindings = 0;

    public:
        MetalBindGroupLayout( MetalContext *context, const DenOfIz_BindGroupLayoutDesc &desc );
        ~MetalBindGroupLayout( ) override;

        [[nodiscard]] const DenOfIz_BindGroupLayoutDesc &Desc( ) const;
        [[nodiscard]] uint32_t                           RegisterSpace( ) const;

        void SetCbvSrvUavTableOffset( uint32_t offset );
        void SetSamplerTableOffset( uint32_t offset );
        void SetTLABIndex( uint32_t hash, uint32_t index );

        [[nodiscard]] uint32_t        CbvSrvUavTableOffset( ) const;
        [[nodiscard]] uint32_t        CbvSrvUavTableSize( ) const;
        [[nodiscard]] uint32_t        CbvSrvUavResourceIndex( const DenOfIz_ResourceBindingSlot &slot ) const;
        [[nodiscard]] MTLRenderStages CbvSrvUavResourceShaderStages( const DenOfIz_ResourceBindingSlot &slot ) const;
        [[nodiscard]] uint32_t        SamplerTableOffset( ) const;
        [[nodiscard]] uint32_t        SamplerResourceIndex( const DenOfIz_ResourceBindingSlot &slot ) const;
        [[nodiscard]] MTLRenderStages SamplerResourceShaderStages( const DenOfIz_ResourceBindingSlot &slot ) const;
        [[nodiscard]] uint32_t        TLABOffset( const DenOfIz_ResourceBindingSlot &slot ) const;

        [[nodiscard]] bool     HasCbvSrvUav( ) const;
        [[nodiscard]] bool     HasSamplers( ) const;
        [[nodiscard]] bool     HasBindless( ) const;
        [[nodiscard]] uint32_t NumRootLevelBindings( ) const;

        static IRRootParameterType GetRootParameterType( DenOfIz_ResourceBindingType type );
    };

} // namespace DenOfIz
