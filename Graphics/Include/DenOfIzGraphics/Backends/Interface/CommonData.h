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
#include "DenOfIzGraphics/Utilities/BitSet.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

namespace DenOfIz
{
    enum class FormatSubType
    {
        Undefined,
        Float,
        Uint,
        Sint,
        Unorm,
        Snorm,
        Typeless,
        Srgb
    };

    enum class Format
    {
        Undefined,
        R32G32B32A32Float,
        R32G32B32A32Sint,
        R32G32B32A32Typeless,
        R32G32B32A32Uint,
        R32G32B32Float,
        R32G32B32Uint,
        R32G32B32Sint,
        R16G16B16A16Float,
        R16G16B16A16Snorm,
        R16G16B16A16Sint,
        R16G16B16A16Typeless,
        R16G16B16A16Unorm,
        R16G16B16A16Uint,
        R32G32Float,
        R32G32Sint,
        R32G32Typeless,
        R32G32Uint,
        R10G10B10A2Typeless,
        R10G10B10A2Uint,
        R10G10B10A2Unorm,
        R8G8B8A8Uint,
        R8G8B8A8Sint,
        R8G8B8A8Snorm,
        R8G8B8A8Typeless,
        R8G8B8A8Unorm,
        R8G8B8A8UnormSrgb,
        R16G16Float,
        R16G16Sint,
        R16G16Snorm,
        R16G16Typeless,
        R16G16Uint,
        R16G16Unorm,
        D32Float,
        R32Float,
        R32Sint,
        R32Typeless,
        R32Uint,
        D24UnormS8Uint,
        R8G8Snorm,
        R8G8Typeless,
        R8G8Uint,
        R8G8Unorm,
        R8G8Sint,
        D16Unorm,
        R16Float,
        R16Sint,
        R16Snorm,
        R16Typeless,
        R16Uint,
        R16Unorm,
        R8Sint,
        R8Snorm,
        R8Typeless,
        R8Uint,
        R8Unorm,
        BC1Unorm,
        BC1UnormSrgb,
        BC2Unorm,
        BC2UnormSrgb,
        BC3Unorm,
        BC3UnormSrgb,
        BC4Unorm,
        BC4Snorm,
        BC5Unorm,
        BC5Snorm,
        B8G8R8A8Unorm,
        BC6HUfloat16,
        BC6HSfloat16,
        BC7Unorm,
        BC7UnormSrgb
    };

    static uint32_t FormatNumBytes( const Format &format )
    {
        switch ( format )
        {
        case Format::R32G32B32A32Float:
        case Format::R32G32B32A32Uint:
        case Format::R32G32B32A32Sint:
            return 16;
        case Format::R32G32B32Float:
        case Format::R32G32B32Uint:
        case Format::R32G32B32Sint:
            return 12;
        case Format::R16G16B16A16Float:
        case Format::R16G16B16A16Unorm:
        case Format::R16G16B16A16Uint:
        case Format::R16G16B16A16Snorm:
        case Format::R16G16B16A16Sint:
        case Format::R32G32Float:
        case Format::R32G32Uint:
        case Format::R32G32Sint:
            return 8;
        case Format::R10G10B10A2Unorm:
        case Format::R10G10B10A2Uint:
        case Format::R8G8B8A8Unorm:
        case Format::R8G8B8A8UnormSrgb:
        case Format::R8G8B8A8Uint:
        case Format::R8G8B8A8Snorm:
        case Format::R8G8B8A8Sint:
        case Format::R16G16Float:
        case Format::R16G16Unorm:
        case Format::R16G16Uint:
        case Format::R16G16Snorm:
        case Format::R16G16Sint:
        case Format::D32Float:
        case Format::R32Float:
        case Format::R32Uint:
        case Format::R32Sint:
        case Format::D24UnormS8Uint:
            return 4;
        case Format::R8G8Unorm:
        case Format::R8G8Uint:
        case Format::R8G8Snorm:
        case Format::R8G8Sint:
        case Format::R16Float:
        case Format::D16Unorm:
        case Format::R16Unorm:
        case Format::R16Uint:
            return 2;
        case Format::R16Snorm:
        case Format::R16Sint:
        case Format::R8Unorm:
        case Format::R8Uint:
        case Format::R8Snorm:
        case Format::R8Sint:
            return 1;
        // Recheck what the below are and what the expected sizes are.
        case Format::BC1Unorm:
        case Format::BC1UnormSrgb:
            return 8;
        case Format::BC2Unorm:
        case Format::BC2UnormSrgb:
        case Format::BC3Unorm:
        case Format::BC3UnormSrgb:
            return 16;
        case Format::BC4Unorm:
        case Format::BC4Snorm:
            return 8;
        case Format::BC5Unorm:
        case Format::BC5Snorm:
            return 16;
        case Format::B8G8R8A8Unorm:
            return 4;
        case Format::BC6HUfloat16:
        case Format::BC6HSfloat16:
        case Format::BC7Unorm:
        case Format::BC7UnormSrgb:
            return 16;
        default:
            return 1;
        }
    }

    static Format FormatToTypeless( const Format &format )
    {
        switch ( format )
        {
        case Format::R32G32B32A32Float:
        case Format::R32G32B32A32Uint:
        case Format::R32G32B32A32Sint:
            return Format::R32G32B32A32Typeless;

        case Format::R16G16B16A16Float:
        case Format::R16G16B16A16Unorm:
        case Format::R16G16B16A16Uint:
        case Format::R16G16B16A16Snorm:
        case Format::R16G16B16A16Sint:
            return Format::R16G16B16A16Typeless;

        case Format::R32G32Float:
        case Format::R32G32Uint:
        case Format::R32G32Sint:
            return Format::R32G32Typeless;

        case Format::R10G10B10A2Unorm:
        case Format::R10G10B10A2Uint:
            return Format::R10G10B10A2Typeless;

        case Format::R8G8B8A8Unorm:
        case Format::R8G8B8A8Uint:
        case Format::R8G8B8A8Snorm:
        case Format::R8G8B8A8Sint:
            return Format::R8G8B8A8Typeless;

        case Format::R16G16Float:
        case Format::R16G16Unorm:
        case Format::R16G16Uint:
        case Format::R16G16Snorm:
        case Format::R16G16Sint:
            return Format::R16G16Typeless;

        case Format::R32Float:
        case Format::R32Uint:
        case Format::R32Sint:
            return Format::R32Typeless;

        case Format::R8G8Unorm:
        case Format::R8G8Uint:
        case Format::R8G8Snorm:
        case Format::R8G8Sint:
            return Format::R8G8Typeless;

        case Format::R16Float:
        case Format::R16Unorm:
        case Format::R16Uint:
        case Format::R16Snorm:
        case Format::R16Sint:
            return Format::R16Typeless;

        case Format::R8Unorm:
        case Format::R8Uint:
        case Format::R8Snorm:
        case Format::R8Sint:
            return Format::R8Typeless;

        default:
            return format;
        }
    }

    static FormatSubType GetFormatSubType( const Format &format )
    {
        switch ( format )
        {
        case Format::Undefined:
            return FormatSubType::Undefined;
        case Format::R32G32B32A32Float:
        case Format::R32G32B32Float:
        case Format::R16G16B16A16Float:
        case Format::R32G32Float:
        case Format::R16G16Float:
        case Format::D32Float:
        case Format::R32Float:
        case Format::R16Float:
        case Format::BC6HUfloat16:
        case Format::BC6HSfloat16:
            return FormatSubType::Float;
        case Format::R32G32B32A32Sint:
        case Format::R32G32B32Sint:
        case Format::R16G16B16A16Sint:
        case Format::R32G32Sint:
        case Format::R8G8B8A8Sint:
        case Format::R16G16Sint:
        case Format::R32Sint:
        case Format::R8G8Sint:
        case Format::R16Sint:
        case Format::R8Sint:
            return FormatSubType::Sint;
        case Format::R32G32B32A32Typeless:
        case Format::R16G16B16A16Typeless:
        case Format::R32G32Typeless:
        case Format::R10G10B10A2Typeless:
        case Format::R8G8B8A8Typeless:
        case Format::R16G16Typeless:
        case Format::R32Typeless:
        case Format::R8G8Typeless:
        case Format::R16Typeless:
        case Format::R8Typeless:
            return FormatSubType::Typeless;
        case Format::R32G32B32A32Uint:
        case Format::R32G32B32Uint:
        case Format::R16G16B16A16Uint:
        case Format::R32G32Uint:
        case Format::R10G10B10A2Uint:
        case Format::R8G8B8A8Uint:
        case Format::R16G16Uint:
        case Format::R32Uint:
        case Format::D24UnormS8Uint:
        case Format::R8G8Uint:
        case Format::R16Uint:
        case Format::R8Uint:
            return FormatSubType::Uint;
        case Format::R16G16B16A16Snorm:
        case Format::R8G8B8A8Snorm:
        case Format::R16G16Snorm:
        case Format::R8G8Snorm:
        case Format::R16Snorm:
        case Format::R8Snorm:
        case Format::BC4Snorm:
        case Format::BC5Snorm:
            return FormatSubType::Snorm;
        case Format::R16G16B16A16Unorm:
        case Format::R10G10B10A2Unorm:
        case Format::R8G8B8A8Unorm:
        case Format::R16G16Unorm:
        case Format::R8G8Unorm:
        case Format::D16Unorm:
        case Format::R16Unorm:
        case Format::R8Unorm:
        case Format::BC1Unorm:
        case Format::BC2Unorm:
        case Format::BC3Unorm:
        case Format::BC4Unorm:
        case Format::BC5Unorm:
        case Format::B8G8R8A8Unorm:
        case Format::BC7Unorm:
        case Format::R8G8B8A8UnormSrgb:
        case Format::BC1UnormSrgb:
        case Format::BC2UnormSrgb:
        case Format::BC3UnormSrgb:
        case Format::BC7UnormSrgb:
            return FormatSubType::Unorm;
        }

        return FormatSubType::Undefined;
    }

    static bool IsFormatBC( const Format &format )
    {
        switch ( format )
        {
        case Format::BC1Unorm:
        case Format::BC1UnormSrgb:
        case Format::BC2Unorm:
        case Format::BC2UnormSrgb:
        case Format::BC3Unorm:
        case Format::BC3UnormSrgb:
        case Format::BC4Unorm:
        case Format::BC4Snorm:
        case Format::BC5Unorm:
        case Format::BC5Snorm:
        case Format::BC6HUfloat16:
        case Format::BC6HSfloat16:
        case Format::BC7Unorm:
        case Format::BC7UnormSrgb:
            return true;
        default:
            return false;
        }
    }

    static uint32_t FormatBlockSize( const Format &format )
    {
        return IsFormatBC( format ) ? 4 : 1;
    }

    enum class PrimitiveTopology
    {
        Point,
        Line,
        Triangle,
        Patch
    };

    enum class IndexType
    {
        Uint16,
        Uint32
    };

    enum class MSAASampleCount
    {
        _0, // Disabled
        _1,
        _2,
        _4,
        _8,
        _16,
        _32,
        _64,
    };

    static int MSAASampleCountToNumSamples( const MSAASampleCount &sampleCount )
    {
        switch ( sampleCount )
        {
        case MSAASampleCount::_1:
            return 1;
        case MSAASampleCount::_2:
            return 2;
        case MSAASampleCount::_4:
            return 4;
        case MSAASampleCount::_8:
            return 8;
        case MSAASampleCount::_16:
            return 16;
        case MSAASampleCount::_32:
            return 32;
        case MSAASampleCount::_64:
            return 64;
        default:
            return 1;
        }
    }

    enum class HeapType
    {
        GPU,
        CPU,
        CPU_GPU,
        GPU_CPU
    };

    enum class TextureAspect
    {
        Color,
        Depth,
        Stencil,
        Metadata,
        Plane0,
        Plane1,
        Plane2,
        None
    };

    enum class SamplerAddressMode
    {
        Repeat,
        Mirror,
        ClampToEdge,
        ClampToBorder,
    };

    enum class MipmapMode
    {
        Nearest,
        Linear
    };

    enum class Filter
    {
        Nearest,
        Linear,
    };

    enum class CompareOp
    {
        Never,
        Equal,
        NotEqual,
        Always,
        Less,
        LessOrEqual,
        Greater,
        GreaterOrEqual
    };

    enum class StencilOp
    {
        Keep,
        Zero,
        Replace,
        IncrementAndClamp,
        DecrementAndClamp,
        Invert,
        IncrementAndWrap,
        DecrementAndWrap
    };

    enum class ResourceDescriptor
    {
        Buffer                = 1 << 1,
        RWBuffer              = 1 << 2,
        Texture               = 1 << 3,
        RWTexture             = 1 << 4,
        RenderTarget          = 1 << 5,
        DepthStencil          = 1 << 6,
        Sampler               = 1 << 7,
        UniformBuffer         = 1 << 8,
        RootConstant          = 1 << 9,
        IndexBuffer           = 1 << 10,
        VertexBuffer          = 1 << 11,
        IndirectBuffer        = 1 << 12,
        TextureCube           = 1 << 13,
        AccelerationStructure = 1 << 14,
    };

    enum class ResourceState
    {
        Undefined                  = 1 << 1,
        VertexAndConstantBuffer    = 1 << 2,
        IndexBuffer                = 1 << 3,
        RenderTarget               = 1 << 4,
        UnorderedAccess            = 1 << 5,
        DepthWrite                 = 1 << 6,
        DepthRead                  = 1 << 7,
        ShaderResource             = 1 << 8,
        PixelShaderResource        = 1 << 9,
        StreamOut                  = 1 << 10,
        IndirectArgument           = 1 << 11,
        CopyDst                    = 1 << 12,
        CopySrc                    = 1 << 13,
        GenericRead                = 1 << 14,
        Present                    = 1 << 15,
        Common                     = 1 << 16,
        AccelerationStructureRead  = 1 << 17,
        AccelerationStructureWrite = 1 << 18
    };

    enum class DescriptorBufferBindingType
    {
        ConstantBuffer,
        ShaderResource,
        UnorderedAccess,
        Sampler
    };

    static DescriptorBufferBindingType ResourceDescriptorBindingType( const BitSet<ResourceDescriptor> &descriptor )
    {
        if ( descriptor.Any( { ResourceDescriptor::RWTexture, ResourceDescriptor::RWBuffer } ) )
        {
            return DescriptorBufferBindingType::UnorderedAccess;
        }
        if ( descriptor.IsSet( ResourceDescriptor::Sampler ) )
        {
            return DescriptorBufferBindingType::Sampler;
        }
        if ( descriptor.IsSet( ResourceDescriptor::UniformBuffer ) )
        {
            return DescriptorBufferBindingType::ConstantBuffer;
        }

        return DescriptorBufferBindingType::ShaderResource;
    }

    enum class LoadOp
    {
        Clear,
        Load,
        DontCare
    };

    enum class StoreOp
    {
        Store,
        None,
        DontCare
    };

    enum class QueueType
    {
        Graphics,
        Compute,
        Copy
    };

    struct DeviceConstants
    {
        uint32_t ConstantBufferAlignment;
        uint32_t BufferTextureAlignment;
        uint32_t BufferTextureRowAlignment;
    };

    struct PhysicalDeviceCapabilities
    {
        bool DedicatedCopyQueue;
        bool RayTracing;
        bool ComputeShaders;
        bool Tearing;
        bool Tessellation;
        bool GeometryShaders;
        bool HDR;
    };

    struct PhysicalDeviceProperties
    {
        bool         IsDedicated;
        unsigned int MemoryAvailableInMb;
    };

    struct PhysicalDevice
    {
        long                       Id;
        std::string                Name;
        PhysicalDeviceProperties   Properties;
        PhysicalDeviceCapabilities Capabilities;
        DeviceConstants            Constants;
    };

    struct DZConfiguration
    {
        /**
         *  NOTE: Maximum register space for Vulkan is 32(number of sets). Therefore the register spaces take the last two up.
         *
         *  Specify the register space where all bindings are stored as root level buffer.
         *  for best performance results only include buffers and not textures. As Textures and Samplers usually are bound on a descriptor table.
         *  - For (D3D12/Metal) this will use direct buffers/root buffers instead of descriptor tables.
         *  - For (Vulkan) this doesn't have any effect.
         */
        uint32_t RootLevelBufferRegisterSpace = 30;
        /**
         * Specify the register space where all bindings are stored as root constants(in DirectX12) or push constants(in Vulkan). In Metal these behave the same as root level
         * buffers.
         */
        uint32_t RootConstantRegisterSpace = 31;

        static DZConfiguration Instance( )
        {
            static DZConfiguration instance;
            return instance;
        }
    };

    typedef unsigned char Byte;
} // namespace DenOfIz

