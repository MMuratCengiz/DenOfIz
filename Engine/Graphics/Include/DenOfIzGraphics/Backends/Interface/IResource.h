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

#include <string>
#include <vector>
#include "CommonData.h"

namespace DenOfIz
{

    class IResource
    {
    public:
        virtual ~IResource() = default;
        std::string Name;

        virtual const ResourceType Type() = 0;
    };

    struct BufferView
    {
        uint64_t Offset;
        uint64_t Stride;
    };

    struct BufferCreateInfo
    {
        bool KeepMemoryMapped = false;

        BufferView BufferView; // For Structured Buffers
        ImageFormat Format = ImageFormat::Undefined;
        BufferUsage Usage;
        HeapType HeapType;
    };

    class IBufferResource : public IResource
    {
    protected:
        uint32_t m_size;
        const void *m_data;

        void *m_mappedMemory = nullptr;

    public:
        void Allocate(const void *data, uint32_t size)
        {
            m_size = size;
            m_data = data;
            Allocate(data);
        }

        virtual void Deallocate() = 0;

        inline uint32_t GetSize() const { return m_size; }
        inline const void *GetData() const { return m_data; }

    protected:
        virtual void Allocate(const void *data) = 0;

    private:
        const ResourceType Type() override { return ResourceType::Buffer; }
    };

    struct ImageCreateInfo
    {
        ImageAspect Aspect = ImageAspect::Color;
        ImageFormat Format;
        ImageMemoryUsage ImageUsage;
        HeapType HeapType = HeapType::Auto;
        MSAASampleCount MSAASampleCount = MSAASampleCount::_0;
    };

    struct SamplerCreateInfo
    {
        Filter MagFilter;
        Filter MinFilter;
        SamplerAddressMode AddressModeU;
        SamplerAddressMode AddressModeV;
        SamplerAddressMode AddressModeW;
        bool AnisotropyEnable = true;
        float MaxAnisotropy = 16.0f;
        //	borderColor, unnormalizedCoordinates; Todo, Maybe?
        bool CompareEnable = false;
        CompareOp CompareOp = CompareOp::Always;
        MipmapMode MipmapMode;
        float MipLodBias;
        float MinLod;
        float MaxLod;

        uint32_t Width;
        uint32_t Height;
        ImageFormat Format;
    };

    class ITextureResource : public IResource
    {

    protected:
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_depth;
        const void *m_data;

    public:
        virtual void Allocate(const void *data, uint32_t width, uint32_t height, uint32_t depth = 0)
        {
            m_width = width;
            m_height = height;
            m_depth = depth;
            m_data = data;
            Allocate(data);
        }

        virtual void Deallocate() = 0;

        virtual void AttachSampler(SamplerCreateInfo &) = 0;

        inline uint32_t GetWidth() const { return m_width; }
        inline uint32_t GetHeight() const { return m_height; }
        inline uint32_t GetDepth() const { return m_depth; }

        const ResourceType Type() override { return ResourceType::Texture; };

    protected:
        virtual void Allocate(const void *data) = 0;
    };

} // namespace DenOfIz
