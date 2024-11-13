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

#include <DenOfIzGraphics/Backends/Interface/RayTracing/IShaderLocalData.h>
#include <DenOfIzGraphics/Backends/Vulkan/RayTracing/VulkanShaderLocalDataLayout.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanContext.h>
#include <DenOfIzGraphics/Utilities/Storage.h>

namespace DenOfIz
{
    class VulkanShaderLocalData final : public IShaderLocalData
    {
        VulkanContext                    *m_context;
        ShaderLocalDataDesc               m_desc;
        VulkanShaderLocalDataLayout      *m_layout;
        std::vector<Byte>                 m_data;
        Storage                           m_storage;
        std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;
        std::vector<uint8_t>              m_inlineData;
        VkDescriptorSet                   m_descriptorSet = nullptr;

    public:
        VulkanShaderLocalData( VulkanContext *context, const ShaderLocalDataDesc &desc );
        ~VulkanShaderLocalData( );

        void Begin( ) override;
        void Cbv( const uint32_t binding, IBufferResource *bufferResource ) override;
        void Cbv( const uint32_t binding, const InteropArray<Byte> &data ) override;
        void Srv( const uint32_t binding, const IBufferResource *bufferResource ) override;
        void Srv( const uint32_t binding, const ITextureResource *textureResource ) override;
        void Uav( const uint32_t binding, const IBufferResource *bufferResource ) override;
        void Uav( const uint32_t binding, const ITextureResource *textureResource ) override;
        void Sampler( uint32_t binding, const ISampler *sampler ) override;
        void End( ) override;

        [[nodiscard]] const VkDescriptorSet *DescriptorSet( ) const;
        [[nodiscard]] uint32_t               DataNumBytes( ) const;
        [[nodiscard]] const Byte            *Data( ) const;

    private:
        VkWriteDescriptorSet &CreateWriteDescriptor( uint32_t binding, VkDescriptorType type );
    };
} // namespace DenOfIz
