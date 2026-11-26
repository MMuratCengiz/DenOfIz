/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/Interface/IPipelineCache.h"
#include "VulkanContext.h"

namespace DenOfIz
{
    class VulkanPipelineCache final : public IPipelineCache
    {
        VulkanContext  *m_context       = nullptr;
        VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;

    public:
        VulkanPipelineCache( VulkanContext *context, const DenOfIz_PipelineCacheDesc &desc );
        ~VulkanPipelineCache( ) override;

        size_t GetDataNumBytes( ) override;
        bool   GetData( DenOfIz_ByteArray &data ) override;

        VkPipelineCache GetCache( ) const;
    };

} // namespace DenOfIz
