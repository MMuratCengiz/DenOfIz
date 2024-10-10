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

#include <DenOfIzGraphics/Utilities/Storage.h>
#include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
#include "VulkanContext.h"
#include "VulkanDescriptorPoolManager.h"
#include "VulkanRootSignature.h"

namespace DenOfIz
{
    struct VulkanRootConstantBinding
    {
        VkPipelineLayout   PipelineLayout;
        VkShaderStageFlags ShaderStage;
        uint32_t           Binding;
        uint32_t           Offset;
        uint32_t           Size;
        void              *Data;
    };

    class VulkanResourceBindGroup final : public IResourceBindGroup
    {
        VulkanContext        *m_context;
        VulkanRootSignature  *m_rootSignature;
        ResourceBindGroupDesc m_desc;

        VkDescriptorSetLayout                  m_emptyLayout{ };
        VkDescriptorSet                        m_descriptorSet{ };
        std::vector<VkWriteDescriptorSet>      m_writeDescriptorSets;
        Storage                                m_storage;
        std::vector<VulkanRootConstantBinding> m_rootConstants;

    public:
        VulkanResourceBindGroup( VulkanContext *context, const ResourceBindGroupDesc &desc );
        ~VulkanResourceBindGroup( ) override;
        void SetRootConstants( uint32_t binding, void *data ) override;
        void Update( const UpdateDesc &desc ) override;

        [[nodiscard]] const std::vector<VulkanRootConstantBinding> &RootConstants( ) const;
        [[nodiscard]] bool                                          HasDescriptorSet( ) const;
        [[nodiscard]] const VkDescriptorSet                        &GetDescriptorSet( ) const;
        [[nodiscard]] VulkanRootSignature                          *RootSignature( ) const;
        [[nodiscard]] uint32_t                                      RegisterSpace( ) const;

    protected:
        void BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource ) override;
        void BindBuffer( const ResourceBindingSlot &name, IBufferResource *resource ) override;
        void BindSampler( const ResourceBindingSlot &name, ISampler *sampler ) override;

    private:
        VkWriteDescriptorSet &CreateWriteDescriptor( const ResourceBindingSlot &slot );
    };

} // namespace DenOfIz
