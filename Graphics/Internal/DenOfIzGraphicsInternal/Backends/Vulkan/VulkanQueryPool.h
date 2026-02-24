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
#include "VulkanContext.h"

namespace DenOfIz
{
    class VulkanQueryPool final : public IQueryPool
    {
        VulkanContext        *m_context;
        DenOfIz_QueryPoolDesc m_desc;
        VkQueryPool           m_queryPool;
        double                m_timestampFrequency;

    public:
        VulkanQueryPool( VulkanContext *context, const DenOfIz_QueryPoolDesc &desc );
        ~VulkanQueryPool( ) override;

        DenOfIz_QueryType GetType( ) const override;
        uint32_t          GetNumQueries( ) const override;
        double            GetTimestampFrequency( ) override;
        DenOfIz_QueryData GetQueryData( uint32_t queryIndex ) override;
        VkQueryPool       GetVkQueryPool( ) const;
    };
} // namespace DenOfIz
