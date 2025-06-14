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

#include "DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h"
#include "DenOfIzGraphicsInternal/Utilities/Storage.h"
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
        void                SetRootConstantsData( uint32_t binding, const ByteArrayView &data ) override;
        void                SetRootConstants( uint32_t binding, void *data ) override;
        IResourceBindGroup *BeginUpdate( ) override;
        IResourceBindGroup *Cbv( const uint32_t binding, IBufferResource *resource ) override;
        IResourceBindGroup *Cbv( const BindBufferDesc& desc ) override;
        IResourceBindGroup *Srv( const uint32_t binding, IBufferResource *resource ) override;
        IResourceBindGroup *Srv( const BindBufferDesc& desc ) override;
        IResourceBindGroup *Srv( const uint32_t binding, ITextureResource *resource ) override;
        IResourceBindGroup *SrvArray( const uint32_t binding, const TextureResourceArray &resources ) override;
        IResourceBindGroup *SrvArrayIndex( const uint32_t binding, uint32_t arrayIndex, ITextureResource *resource ) override;
        IResourceBindGroup *Srv( const uint32_t binding, ITopLevelAS *accelerationStructure ) override;
        IResourceBindGroup *Uav( const uint32_t binding, IBufferResource *resource ) override;
        IResourceBindGroup *Uav( const BindBufferDesc& desc ) override;
        IResourceBindGroup *Uav( const uint32_t binding, ITextureResource *resource ) override;
        IResourceBindGroup *Sampler( const uint32_t binding, ISampler *sampler ) override;
        void                EndUpdate( ) override;

        [[nodiscard]] const std::vector<VulkanRootConstantBinding> &RootConstants( ) const;
        [[nodiscard]] bool                                          HasDescriptorSet( ) const;
        [[nodiscard]] const VkDescriptorSet                        &GetDescriptorSet( ) const;
        [[nodiscard]] VulkanRootSignature                          *RootSignature( ) const;
        [[nodiscard]] uint32_t                                      RegisterSpace( ) const;

    private:
        void                  BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource );
        VkWriteDescriptorSet &BindBuffer( const ResourceBindingSlot &name, IBufferResource *resource );
        void                  BindSampler( const ResourceBindingSlot &name, ISampler *sampler );
        VkWriteDescriptorSet &CreateWriteDescriptor( const ResourceBindingSlot &slot );
        ResourceBindingSlot   GetSlot( uint32_t binding, const ResourceBindingType &type ) const;
    };

} // namespace DenOfIz
