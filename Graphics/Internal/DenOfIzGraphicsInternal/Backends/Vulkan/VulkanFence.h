/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/Interface/IFence.h"
#include "VulkanContext.h"

namespace DenOfIz
{

    class VulkanFence final : public IFence
    {
        VulkanContext *m_context;
        VkFence        m_fence{ };

    public:
        explicit VulkanFence( VulkanContext *context );
        ~VulkanFence( ) override;
        void                  Wait( ) override;
        void                  Reset( ) override;
        [[nodiscard]] VkFence GetFence( ) const;
    };

} // namespace DenOfIz
