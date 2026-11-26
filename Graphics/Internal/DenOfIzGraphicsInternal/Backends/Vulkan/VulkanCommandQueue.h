/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/ICommandQueue.h>
#include "VulkanCommandList.h"
#include "VulkanContext.h"

namespace DenOfIz
{
    class VulkanCommandQueue final : public ICommandQueue
    {
        VulkanContext           *m_context;
        DenOfIz_CommandQueueDesc m_desc;
        VkQueue                  m_queue;
        uint32_t                 m_queueFamilyIndex;
        uint32_t                 m_queueIndex;

    public:
        VulkanCommandQueue( VulkanContext *context, const DenOfIz_CommandQueueDesc &desc );
        ~VulkanCommandQueue( ) override;

        void WaitIdle( ) override;
        void ExecuteCommandLists( const DenOfIz_ExecuteCommandListsDesc &executeCommandListsDesc ) override;

        [[nodiscard]] uint32_t          GetQueueFamilyIndex( ) const;
        [[nodiscard]] VkQueue           GetQueue( ) const;
        [[nodiscard]] DenOfIz_QueueType GetQueueType( ) const;

    private:
        void FindQueueFamilyIndex( VkQueueFlags requiredFlags );
    };
} // namespace DenOfIz
