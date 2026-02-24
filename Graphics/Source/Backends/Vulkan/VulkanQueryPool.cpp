
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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanQueryPool.h"

#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

VulkanQueryPool::VulkanQueryPool( VulkanContext *context, const DenOfIz_QueryPoolDesc &desc ) : m_context( context ), m_desc( desc ), m_queryPool( VK_NULL_HANDLE )
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties( m_context->PhysicalDevice, &deviceProperties );
    m_timestampFrequency      = 1000000000.0 / deviceProperties.limits.timestampPeriod;
    uint32_t actualQueryCount = m_desc.NumQueries;
    if ( m_desc.Type == DENOFIZ_QUERY_TYPE_TIMESTAMP )
    {
        actualQueryCount = m_desc.NumQueries * 2;
    }

    VkQueryPoolCreateInfo createInfo{ };
    createInfo.sType      = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    createInfo.queryType  = DenOfIz_VulkanEnumConverter_ConvertQueryType( m_desc.Type );
    createInfo.queryCount = actualQueryCount;

    if ( m_desc.Type == DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS )
    {
        createInfo.pipelineStatistics = DenOfIz_VulkanEnumConverter_ConvertPipelineStageFlags( m_desc.PipelineStatisticFlags );
    }

    VK_CHECK_RESULT( vkCreateQueryPool( m_context->LogicalDevice, &createInfo, nullptr, &m_queryPool ) );
}

VulkanQueryPool::~VulkanQueryPool( )
{
    if ( m_queryPool != VK_NULL_HANDLE )
    {
        vkDestroyQueryPool( m_context->LogicalDevice, m_queryPool, nullptr );
    }
}

DenOfIz_QueryType VulkanQueryPool::GetType( ) const
{
    return m_desc.Type;
}

uint32_t VulkanQueryPool::GetNumQueries( ) const
{
    return m_desc.NumQueries;
}

DenOfIz_QueryData VulkanQueryPool::GetQueryData( uint32_t queryIndex )
{
    DenOfIz_QueryData result{ };

    switch ( m_desc.Type )
    {
    case DENOFIZ_QUERY_TYPE_TIMESTAMP:
        {
            uint64_t       queries[ 2 ] = { 0, 0 };
            const uint32_t actualIndex  = queryIndex * 2;
            const VkResult vkResult =
                vkGetQueryPoolResults( m_context->LogicalDevice, m_queryPool, actualIndex, 2, sizeof( queries ), queries, sizeof( uint64_t ), VK_QUERY_RESULT_64_BIT );

            if ( vkResult == VK_SUCCESS )
            {
                result.Valid          = true;
                result.BeginTimestamp = queries[ 0 ];
                result.EndTimestamp   = queries[ 1 ];
            }
            break;
        }
    case DENOFIZ_QUERY_TYPE_OCCLUSION:
        {
            uint64_t       occlusionCount = 0;
            const VkResult vkResult =
                vkGetQueryPoolResults( m_context->LogicalDevice, m_queryPool, queryIndex, 1, sizeof( uint64_t ), &occlusionCount, sizeof( uint64_t ), VK_QUERY_RESULT_64_BIT );

            if ( vkResult == VK_SUCCESS )
            {
                result.Valid           = true;
                result.OcclusionCounts = occlusionCount;
            }
            break;
        }
    case DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS:
        {
            uint32_t numStats = 0;
            if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_INPUT_ASSEMBLY_VERTICES_BIT )
            {
                numStats++;
            }
            if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_INPUT_ASSEMBLY_PRIMITIVES_BIT )
            {
                numStats++;
            }
            if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_VERTEX_SHADER_INVOCATIONS_BIT )
            {
                numStats++;
            }
            if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_GEOMETRY_SHADER_INVOCATIONS_BIT )
            {
                numStats++;
            }
            if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_GEOMETRY_SHADER_PRIMITIVES_BIT )
            {
                numStats++;
            }
            if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_CLIPPING_INVOCATIONS_BIT )
            {
                numStats++;
            }
            if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_CLIPPING_PRIMITIVES_BIT )
            {
                numStats++;
            }
            if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_CLIPPING_PRIMITIVES_BIT )
            {
                numStats++;
            }
            if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_HULL_SHADER_INVOCATIONS_BIT )
            {
                numStats++;
            }
            if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_DOMAIN_SHADER_INVOCATIONS_BIT )
            {
                numStats++;
            }
            if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_COMPUTE_SHADER_INVOCATIONS_BIT )
            {
                numStats++;
            }

            if ( numStats > 0 )
            {
                std::vector<uint64_t> queryResults( numStats );
                const VkResult        vkResult = vkGetQueryPoolResults( m_context->LogicalDevice, m_queryPool, queryIndex, 1, numStats * sizeof( uint64_t ), queryResults.data( ),
                                                                        sizeof( uint64_t ), VK_QUERY_RESULT_64_BIT );

                if ( vkResult == VK_SUCCESS )
                {
                    result.Valid = true;

                    uint32_t index = 0;
                    if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_INPUT_ASSEMBLY_VERTICES_BIT )
                    {
                        result.PipelineStats.InputAssemblyVertices = queryResults[ index++ ];
                    }
                    if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_INPUT_ASSEMBLY_PRIMITIVES_BIT )
                    {
                        result.PipelineStats.InputAssemblyPrimitives = queryResults[ index++ ];
                    }
                    if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_VERTEX_SHADER_INVOCATIONS_BIT )
                    {
                        result.PipelineStats.VertexShaderInvocations = queryResults[ index++ ];
                    }
                    if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_GEOMETRY_SHADER_INVOCATIONS_BIT )
                    {
                        result.PipelineStats.GeometryShaderInvocations = queryResults[ index++ ];
                    }
                    if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_GEOMETRY_SHADER_PRIMITIVES_BIT )
                    {
                        result.PipelineStats.GeometryShaderPrimitives = queryResults[ index++ ];
                    }
                    if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_CLIPPING_INVOCATIONS_BIT )
                    {
                        result.PipelineStats.ClippingInvocations = queryResults[ index++ ];
                    }
                    if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_CLIPPING_PRIMITIVES_BIT )
                    {
                        result.PipelineStats.ClippingPrimitives = queryResults[ index++ ];
                    }
                    if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_CLIPPING_PRIMITIVES_BIT )
                    {
                        result.PipelineStats.PixelShaderInvocations = queryResults[ index++ ];
                    }
                    if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_HULL_SHADER_INVOCATIONS_BIT )
                    {
                        result.PipelineStats.HullShaderInvocations = queryResults[ index++ ];
                    }
                    if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_DOMAIN_SHADER_INVOCATIONS_BIT )
                    {
                        result.PipelineStats.DomainShaderInvocations = queryResults[ index++ ];
                    }
                    if ( m_desc.PipelineStatisticFlags & DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_COMPUTE_SHADER_INVOCATIONS_BIT )
                    {
                        result.PipelineStats.ComputeShaderInvocations = queryResults[ index++ ];
                    }
                }
            }
            break;
        }
    }

    return result;
}

VkQueryPool VulkanQueryPool::GetVkQueryPool( ) const
{
    return m_queryPool;
}

double VulkanQueryPool::GetTimestampFrequency( )
{
    return m_timestampFrequency;
}
