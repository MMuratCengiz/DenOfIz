/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/Interface/IBindGroup.h"
#include "DenOfIzGraphicsInternal/Utilities/Storage.h"
#include "VulkanBindGroupLayout.h"
#include "VulkanContext.h"

namespace DenOfIz
{
    class VulkanBindGroup final : public IBindGroup
    {
        VulkanContext         *m_context;
        VulkanBindGroupLayout *m_bindGroupLayout;
        DenOfIz_BindGroupDesc  m_desc;

        VkDescriptorSetLayout             m_emptyLayout{ };
        VkDescriptorSet                   m_descriptorSet{ };
        VkDescriptorPool                  m_descriptorPool{ };
        std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;
        Storage                           m_storage;

    public:
        VulkanBindGroup( VulkanContext *context, const DenOfIz_BindGroupDesc &desc );
        ~VulkanBindGroup( ) override;
        IBindGroup *BeginUpdate( ) override;
        IBindGroup *Cbv( const uint32_t binding, IBuffer *resource ) override;
        IBindGroup *Cbv( const uint32_t binding, IBuffer *resource, size_t offset ) override;
        IBindGroup *Srv( const uint32_t binding, IBuffer *resource ) override;
        IBindGroup *Srv( const uint32_t binding, IBuffer *resource, size_t offset ) override;
        IBindGroup *Srv( const uint32_t binding, ITexture *resource, uint32_t mipLevel = DZ_ALL_MIP_LEVELS ) override;
        IBindGroup *SrvArray( const uint32_t binding, const DenOfIz_TextureArray &resources ) override;
        IBindGroup *SrvArrayIndex( const uint32_t binding, uint32_t arrayIndex, ITexture *resource ) override;
        IBindGroup *Srv( const uint32_t binding, ITopLevelAS *accelerationStructure ) override;
        IBindGroup *Uav( const uint32_t binding, IBuffer *resource ) override;
        IBindGroup *Uav( const uint32_t binding, IBuffer *resource, size_t offset ) override;
        IBindGroup *Uav( const uint32_t binding, ITexture *resource, uint32_t mipLevel = DZ_ALL_MIP_LEVELS ) override;
        IBindGroup *Sampler( const uint32_t binding, ISampler *sampler ) override;
        void        EndUpdate( ) override;

        [[nodiscard]] bool                   HasDescriptorSet( ) const;
        [[nodiscard]] const VkDescriptorSet &GetDescriptorSet( ) const;
        [[nodiscard]] VulkanBindGroupLayout *BindGroupLayout( ) const;
        [[nodiscard]] uint32_t               RegisterSpace( ) const;

    private:
        void                        BindTexture( const DenOfIz_ResourceBindingSlot &slot, ITexture *resource, uint32_t mipLevel = DZ_ALL_MIP_LEVELS );
        VkWriteDescriptorSet       &BindBuffer( const DenOfIz_ResourceBindingSlot &name, IBuffer *resource );
        void                        BindSampler( const DenOfIz_ResourceBindingSlot &name, ISampler *sampler );
        VkWriteDescriptorSet       &CreateWriteDescriptor( const DenOfIz_ResourceBindingSlot &slot );
        DenOfIz_ResourceBindingSlot GetSlot( uint32_t binding, const DenOfIz_ResourceBindingType &type ) const;
    };

} // namespace DenOfIz
