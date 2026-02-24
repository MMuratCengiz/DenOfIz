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

#include "DX12Context.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/IQueryPool.h"

namespace DenOfIz
{
    class DX12QueryPool final : public IQueryPool
    {
        DX12Context                  *m_context;
        DenOfIz_QueryPoolDesc         m_desc;
        wil::com_ptr<ID3D12QueryHeap> m_queryHeap;
        wil::com_ptr<ID3D12Resource>  m_readbackBuffer;
        D3D12_QUERY_TYPE              m_d3d12QueryType;
        uint64_t                      m_timestampFrequency;

    public:
        DX12QueryPool( DX12Context *context, const DenOfIz_QueryPoolDesc &desc );
        ~DX12QueryPool( ) override = default;

        DenOfIz_QueryType GetType( ) const override;
        uint32_t          GetNumQueries( ) const override;
        double            GetTimestampFrequency( ) override;
        DenOfIz_QueryData GetQueryData( uint32_t queryIndex ) override;

        ID3D12QueryHeap *GetQueryHeap( ) const;
        D3D12_QUERY_TYPE GetD3D12QueryType( ) const;
        ID3D12Resource  *GetReadbackBuffer( ) const;
    };
} // namespace DenOfIz
