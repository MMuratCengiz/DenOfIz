/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/ICommandListPool.h>
#include "VulkanCommandList.h"
#include "VulkanCommandQueue.h"

namespace DenOfIz
{
    class VulkanCommandPool final : public ICommandListPool
    {
        VulkanContext                                  *m_context;
        std::vector<std::unique_ptr<VulkanCommandList>> m_commandLists;
        std::vector<VulkanCommandList *>                m_commandListPtrs;
        DenOfIz_CommandListPoolDesc                     m_desc;
        std::vector<VkCommandPool>                      m_commandPools;
        VulkanCommandQueue                             *m_commandQueue;

    public:
        VulkanCommandPool( VulkanContext *context, const DenOfIz_CommandListPoolDesc &desc );
        DenOfIz_CommandListArray        GetCommandLists( ) override;
        [[nodiscard]] DenOfIz_QueueType GetQueueType( ) const;
        ~VulkanCommandPool( ) override;
    };
} // namespace DenOfIz
