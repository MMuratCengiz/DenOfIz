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

#include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
#include "VulkanContext.h"
#include "VulkanRootSignature.h"

namespace DenOfIz
{

    class VulkanResourceBindGroup final : public IResourceBindGroup
    {
        VulkanContext       *m_context;
        VulkanRootSignature *m_rootSignature;

        std::vector<VkDescriptorSet>      m_descriptorSets;
        std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;

    public:
             VulkanResourceBindGroup( VulkanContext *context, ResourceBindGroupDesc desc );
        void Update( UpdateDesc desc ) override;

        const std::vector<VkWriteDescriptorSet> &GetWriteDescriptorSets( ) const
        {
            return m_writeDescriptorSets;
        }

        VkWriteDescriptorSet &CreateWriteDescriptor( std::string &name );

    protected:
        void BindTexture( const std::string &name, ITextureResource *resource ) override;
        void BindBuffer( const std::string &name, IBufferResource *resource ) override;
        void BindSampler( const std::string &name, ISampler *sampler ) override;
    };

} // namespace DenOfIz
