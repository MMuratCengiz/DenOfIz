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
        Filter             MagFilter;
        Filter             MinFilter;
        SamplerAddressMode AddressModeU;
        SamplerAddressMode AddressModeV;
        SamplerAddressMode AddressModeW;
        bool               AnisotropyEnable = true;
        float              MaxAnisotropy    = 16.0f;
        bool               CompareEnable    = false;
        CompareOp          CompareOp        = CompareOp::Always;
        MipmapMode         MipmapMode;
        float              MipLodBias;
        float              MinLod;
        float              MaxLod;

        uint32_t Width;
        uint32_t Height;
        Format   Format;
    };

    struct TextureDesc
    {
        TextureAspect              Aspect = TextureAspect::Color;
        Format                     Format;
        BitSet<ResourceDescriptor> Descriptor;

        HeapType              HeapType        = HeapType::GPU;
        MSAASampleCount       MSAASampleCount = MSAASampleCount::_0;
        BitSet<ResourceState> InitialState;
        SamplerDesc           Sampler; // Requires `| Descriptor::Sampler`

        uint32_t Width;
        // if Height is > 1, it is a 2D texture
        uint32_t Height;
        // if Depth is > 1, it is a 3D texture
        uint32_t Depth     = 1;
        uint32_t ArraySize = 1;
        uint32_t MipLevels = 1;
    };

    class ITextureResource
    {

    protected:
        uint32_t    m_width;
        uint32_t    m_height;
        uint32_t    m_depth;
        const void *m_data;

        void InitDimensions(const TextureDesc &desc)
        {
            m_width  = desc.Width;
            m_height = desc.Height;
            m_depth  = desc.Depth;
        }
    public:
        std::string Name;
        virtual ~ITextureResource() = default;

        virtual void Deallocate() = 0;

        inline uint32_t GetWidth() const
        {
            return m_width;
        }
        inline uint32_t GetHeight() const
        {
            return m_height;
        }
        inline uint32_t GetDepth() const
        {
            return m_depth;
        }

    protected:
        virtual void Allocate(const void *data) = 0;
    };

    class ISampler
    {
    public:
        std::string Name;
        virtual ~ISampler() = default;
    };
} // namespace DenOfIz
