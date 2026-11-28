/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <string>
#include "DenOfIzGraphicsInternal/Backends/Interface/IBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanContext.h"

namespace DenOfIz
{
    class VulkanBuffer final : public IBuffer
    {
        DenOfIz_BufferDesc m_desc{ };
        std::string        m_debugName;
        VulkanContext     *m_context = nullptr;
        VmaAllocation      m_allocation{ };
        VkBuffer           m_instance{ };
        size_t             m_offset{ };
        size_t             m_numBytes{ };
        size_t             m_bufferSize{ };
        const void        *m_data         = nullptr;
        void              *m_mappedMemory = nullptr;
        VkDeviceAddress    m_deviceAddress{ };
        uint32_t           m_state;

    public:
        void *MapMemory( ) override;
        void  UnmapMemory( ) override;

        explicit VulkanBuffer( VulkanContext *context, DenOfIz_BufferDesc desc );
        ~VulkanBuffer( ) override;

        [[nodiscard]] size_t      NumBytes( ) const override;
        [[nodiscard]] const void *Data( ) const override;

        [[nodiscard]] size_t                 Offset( ) const;
        [[nodiscard]] size_t                 BufferSize( ) const;
        [[nodiscard]] const VkBuffer        &Instance( ) const;
        [[nodiscard]] const VkDeviceAddress &DeviceAddress( ) const;

        [[nodiscard]] DenOfIz_ByteArray GetData( ) const override;
        void                            SetData( const DenOfIz_ByteArrayView &data, bool keepMapped ) override;
        void                            WriteData( const DenOfIz_ByteArrayView &data, uint32_t bufferOffset ) override;
    };

} // namespace DenOfIz
