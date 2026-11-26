/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
