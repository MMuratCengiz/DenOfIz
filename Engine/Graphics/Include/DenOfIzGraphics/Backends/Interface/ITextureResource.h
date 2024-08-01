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

#include "CommonData.h"

namespace DenOfIz
{
    struct SubresourceDesc
    {
        uint32_t MipSlice   = 0;
        uint32_t ArraySlice = 0;
        uint32_t PlaneSlice = 0;
    };

    struct SamplerDesc
    {
        Filter             MagFilter     = Filter::Linear;
        Filter             MinFilter     = Filter::Linear;
        SamplerAddressMode AddressModeU  = SamplerAddressMode::Repeat;
        SamplerAddressMode AddressModeV  = SamplerAddressMode::Repeat;
        SamplerAddressMode AddressModeW  = SamplerAddressMode::Repeat;
        float              MaxAnisotropy = 0.0f;
        CompareOp          CompareOp     = CompareOp::Always;
        MipmapMode         MipmapMode    = MipmapMode::Linear;
        float              MipLodBias    = 0.0f;
        float              MinLod        = 0.0f;
        float              MaxLod        = 1.0f;
    };

    struct TextureDesc
    {
        TextureAspect              Aspect = TextureAspect::Color;
        Format                     Format = Format::Undefined;
        BitSet<ResourceDescriptor> Descriptor;

        HeapType              HeapType        = HeapType::GPU;
        MSAASampleCount       MSAASampleCount = MSAASampleCount::_0;
        BitSet<ResourceState> InitialState;
        SamplerDesc           Sampler; // Requires `| Descriptor::Sampler`

        uint32_t Width = 1;
        // if Height is > 1, it is a 2D texture
        uint32_t Height = 1;
        // if Depth is > 1, it is a 3D texture
        uint32_t Depth     = 1;
        uint32_t ArraySize = 1;
        uint32_t MipLevels = 1;
    };

    class ITextureResource
    {

    protected:
        Format      m_format = Format::Undefined;
        uint32_t    m_width  = 1;
        uint32_t    m_height = 1;
        uint32_t    m_depth  = 1;
        const void *m_data   = nullptr;

        void InitFields( const TextureDesc &desc )
        {
            m_width  = desc.Width;
            m_height = desc.Height;
            m_depth  = desc.Depth;
            m_format = desc.Format;
        }

    public:
        std::string Name;
        virtual ~   ITextureResource( ) = default;

        [[nodiscard]] uint32_t GetWidth( ) const
        {
            return m_width;
        }
        [[nodiscard]] uint32_t GetHeight( ) const
        {
            return m_height;
        }
        [[nodiscard]] uint32_t GetDepth( ) const
        {
            return m_depth;
        }
        [[nodiscard]] Format GetFormat( ) const
        {
            return m_format;
        }
    };

    class ISampler
    {
    public:
        std::string Name;
        virtual ~   ISampler( ) = default;
    };
} // namespace DenOfIz
