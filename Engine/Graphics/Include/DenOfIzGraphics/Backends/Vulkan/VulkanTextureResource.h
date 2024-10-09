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

#include "../Interface/ITextureResource.h"
#include "VulkanContext.h"

namespace DenOfIz
{

    class VulkanTextureResource final : public ITextureResource
    {
        Format        m_format       = Format::Undefined;
        uint32_t      m_width        = 1;
        uint32_t      m_height       = 1;
        uint32_t      m_depth        = 1;
        ResourceState m_initialState = ResourceState::Undefined;

        VulkanContext *m_context = nullptr;
        TextureDesc    m_desc{ };
        bool           m_isExternal = false;

        VkImage                  m_image{ };
        std::vector<VkImageView> m_imageViews{ };
        VkFormat                 m_vkFormat{ };
        VkSampler                m_sampler{ };
        VkImageAspectFlags       m_aspect{ };
        VmaAllocation            m_allocation{ };
        uint32_t                 m_mipLevels{ };
        // Mutable because this value is already changed elsewhere, NotifyLayoutChange is simply letting state tracker know.
        mutable VkImageLayout m_layout{ };

    public:
        VulkanTextureResource( VulkanContext *context, const TextureDesc &desc );

        // Use as render target
        VulkanTextureResource( const VkImage &image, const VkImageView &imageView, const VkFormat format, const VkImageAspectFlags imageAspect, const TextureDesc &desc );
        void NotifyLayoutChange( const VkImageLayout newLayout ) const;

        ~VulkanTextureResource( ) override;
        [[nodiscard]] VkImage            Image( ) const;
        [[nodiscard]] VkImageView        ImageView( uint32_t mipLevel = 0 ) const;
        [[nodiscard]] VkImageLayout      Layout( ) const;
        [[nodiscard]] VkImageAspectFlags Aspect( ) const;

        [[nodiscard]] BitSet<ResourceState> InitialState( ) const override;
        [[nodiscard]] uint32_t              GetWidth( ) const;
        [[nodiscard]] uint32_t              GetHeight( ) const;
        [[nodiscard]] uint32_t              GetDepth( ) const;
        [[nodiscard]] Format                GetFormat( ) const override;

    private:
        void TransitionToInitialLayout( ) const;
        void CreateImageView( );
    };

    class VulkanSampler final : public ISampler
    {
        VulkanContext *m_context;
        SamplerDesc    m_desc;
        VkSampler      m_sampler{ };

    public:
        VulkanSampler( VulkanContext *context, const SamplerDesc &desc );
        ~VulkanSampler( ) override;

        [[nodiscard]] VkSampler Instance( ) const;
    };
} // namespace DenOfIz
