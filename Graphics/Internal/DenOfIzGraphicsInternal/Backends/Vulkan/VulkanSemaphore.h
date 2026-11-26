/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/Interface/ISemaphore.h"
#include "VulkanContext.h"

namespace DenOfIz
{

    class VulkanSemaphore final : public ISemaphore
    {
        VulkanContext *m_context;
        VkSemaphore    m_timelineSemaphore{ };
        uint64_t       m_timelineValue = 0;

    public:
        explicit VulkanSemaphore( VulkanContext *context );
        ~VulkanSemaphore( ) override;
        void Wait( ) const;
        void Notify( ) override;
        bool IsCompleted( ) const override;

        [[nodiscard]] VkSemaphore GetSemaphore( ) const;
        [[nodiscard]] uint64_t    GetTimelineValue( ) const;
    };

} // namespace DenOfIz
