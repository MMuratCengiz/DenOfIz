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

#include "DenOfIzGraphicsInternal/Backends/Interface/IQueryPool.h"
#include "WebGPUContext.h"

namespace DenOfIz
{
    class WebGPUQueryPool final : public IQueryPool
    {
        WebGPUContext        *m_context;
        DenOfIz_QueryPoolDesc m_desc;
        WGPUQuerySet          m_querySet;
        WGPUBuffer            m_readbackBuffer;

    public:
        WebGPUQueryPool( WebGPUContext *context, const DenOfIz_QueryPoolDesc &desc );
        ~WebGPUQueryPool( ) override;

        DenOfIz_QueryType GetType( ) const override;
        uint32_t          GetNumQueries( ) const override;
        DenOfIz_QueryData GetQueryData( uint32_t queryIndex ) override;
        double            GetTimestampFrequency( ) override;

        WGPUQuerySet GetQuerySet( ) const
        {
            return m_querySet;
        }
        WGPUBuffer GetReadbackBuffer( ) const
        {
            return m_readbackBuffer;
        }
    };
} // namespace DenOfIz
