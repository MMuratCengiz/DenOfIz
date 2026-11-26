/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <string>
#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"
#include "VulkanContext.h"

namespace DenOfIz
{

    class VulkanTexture final : public ITexture
    {
        DenOfIz_Format m_format = DENOFIZ_FORMAT_UNDEFINED;
        uint32_t       m_width  = 1;
        uint32_t       m_height = 1;
        uint32_t       m_depth  = 1;

        VulkanContext      *m_context = nullptr;
        DenOfIz_TextureDesc m_desc{ };
        std::string         m_debugName;
        bool                m_isExternal = false;

        VkImage                  m_image{ };
        std::vector<VkImageView> m_imageViews{ };
        VkFormat                 m_vkFormat{ };
        VkSampler                m_sampler{ };
        VkImageAspectFlags       m_aspect{ };
        VmaAllocation            m_allocation{ };
        uint32_t                 m_mipLevels{ };
        mutable VkImageLayout    m_layout{ };

    public:
        VulkanTexture( VulkanContext *context, const DenOfIz_TextureDesc &desc );

        VulkanTexture( const VkImage &image, const VkImageView &imageView, const VkFormat format, const VkImageAspectFlags imageAspect, const DenOfIz_TextureDesc &desc );
        void NotifyLayoutChange( const VkImageLayout newLayout ) const;
        void UpdateExternal( const VkImage &image, const VkImageView &imageView, const DenOfIz_TextureDesc &desc );

        ~VulkanTexture( ) override;
        [[nodiscard]] VkImage            Image( ) const;
        [[nodiscard]] VkImageView        ImageView( uint32_t mipLevel = 0 ) const;
        [[nodiscard]] VkImageLayout      Layout( ) const;
        [[nodiscard]] VkImageAspectFlags Aspect( ) const;

        [[nodiscard]] uint32_t       GetWidth( ) const;
        [[nodiscard]] uint32_t       GetHeight( ) const;
        [[nodiscard]] uint32_t       GetDepth( ) const;
        [[nodiscard]] DenOfIz_Format GetFormat( ) const override;
        [[nodiscard]] uint32_t       GetMipLevels( ) const override;
        [[nodiscard]] const DenOfIz_TextureDesc &GetDesc( ) const override
        {
            return m_desc;
        }

    private:
        void TransitionToInitialLayout( ) const;
        void CreateImageView( );
    };

    class VulkanSampler final : public ISampler
    {
        VulkanContext      *m_context;
        DenOfIz_SamplerDesc m_desc;
        std::string         m_debugName;
        VkSampler           m_sampler{ };

    public:
        VulkanSampler( VulkanContext *context, const DenOfIz_SamplerDesc &desc );
        ~VulkanSampler( ) override;

        [[nodiscard]] VkSampler Instance( ) const;
    };
} // namespace DenOfIz
