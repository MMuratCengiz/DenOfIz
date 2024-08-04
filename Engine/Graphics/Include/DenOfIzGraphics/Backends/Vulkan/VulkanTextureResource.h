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
        VulkanContext *m_context = nullptr;
        TextureDesc    m_desc{ };
        bool           m_isExternal = false;

        VkImage                  m_image{ };
        std::vector<VkImageView> m_imageViews{ };
        VkFormat                 m_format{ };
        VkSampler                m_sampler{ };
        VkImageAspectFlags       m_aspect{ };
        VmaAllocation            m_allocation{ };
        uint32_t                 m_mipLevels{ };
        // Mutable because this value is already changed elsewhere, NotifyLayoutChange is simply letting state tracker know.
        mutable VkImageLayout m_layout{ };

    public:
        VulkanTextureResource( VulkanContext *context, const TextureDesc &desc );

        // Use as render target
        VulkanTextureResource( const VkImage &image, const VkImageView &imageView, const VkFormat format, const VkImageAspectFlags imageAspect ) :
            m_image( image ), m_imageViews( { imageView } ), m_format( format ), m_aspect( imageAspect )
        {
            m_isExternal = true;
        }

        void NotifyLayoutChange( const VkImageLayout newLayout ) const
        {
            m_layout = newLayout;
        }

        ~VulkanTextureResource( ) override;
        [[nodiscard]] VkImage Image( ) const
        {
            return m_image;
        }
        [[nodiscard]] VkImageView ImageView( uint32_t mipLevel = 0 ) const
        {
            return m_imageViews[ mipLevel ];
        }
        [[nodiscard]] VkImageLayout Layout( ) const
        {
            return m_layout;
        }
        [[nodiscard]] VkImageAspectFlags Aspect( ) const
        {
            return m_aspect;
        }

    private:
        void TransitionToInitialLayout( ) const;
    };

    class VulkanSampler final : public ISampler
    {
        VulkanContext *m_context;
        SamplerDesc    m_desc;
        VkSampler      m_sampler{ };

    public:
        VulkanSampler( VulkanContext *context, const SamplerDesc &desc );
        ~VulkanSampler( ) override;

        [[nodiscard]] VkSampler Instance( ) const
        {
            return m_sampler;
        }
    };
} // namespace DenOfIz
