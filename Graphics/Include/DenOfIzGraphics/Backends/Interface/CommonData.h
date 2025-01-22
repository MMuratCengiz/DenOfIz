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

#include <DenOfIzGraphics/Utilities/BitSet.h>
#include <DenOfIzGraphics/Utilities/Common_Macro.h>
#include <DenOfIzGraphics/Utilities/Interop.h>
#include <string>
#include <vector>

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

    DZ_API uint32_t      FormatNumBytes( const Format &format );
    DZ_API Format        FormatToTypeless( const Format &format );
    DZ_API FormatSubType GetFormatSubType( const Format &format );
    DZ_API bool          IsFormatBC( const Format &format );
    DZ_API uint32_t      FormatBlockSize( const Format &format );

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

    DZ_API int MSAASampleCountToNumSamples( const MSAASampleCount &sampleCount );

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
        StructuredBuffer      = 1 << 15,
    };

    enum class ResourceUsage
    {
        Undefined                     = 1 << 1,
        VertexAndConstantBuffer       = 1 << 2,
        IndexBuffer                   = 1 << 3,
        RenderTarget                  = 1 << 4,
        UnorderedAccess               = 1 << 5,
        DepthWrite                    = 1 << 6,
        DepthRead                     = 1 << 7,
        ShaderResource                = 1 << 8,
        PixelShaderResource           = 1 << 9,
        StreamOut                     = 1 << 10,
        IndirectArgument              = 1 << 11,
        CopyDst                       = 1 << 12,
        CopySrc                       = 1 << 13,
        GenericRead                   = 1 << 14,
        Present                       = 1 << 15,
        Common                        = 1 << 16,
        AccelerationStructureRead     = 1 << 17,
        AccelerationStructureWrite    = 1 << 18,
        AccelerationStructureGeometry = 1 << 19,
        ShaderBindingTable            = 1 << 20,
    };

    enum class ResourceBindingType
    {
        ConstantBuffer,
        ShaderResource,
        UnorderedAccess,
        Sampler
    };
    DZ_API ResourceBindingType ResourceDescriptorBindingType( const BitSet<ResourceDescriptor> &descriptor );

    struct DZ_API ResourceBindingSlot
    {
        ResourceBindingType         Type          = ResourceBindingType::ConstantBuffer;
        uint32_t                    Binding       = 0;
        uint32_t                    RegisterSpace = 0;

        // To simplify having a really odd looking vector of ResourceBindingSlots
        [[nodiscard]] uint32_t      Key( ) const;
        [[nodiscard]] InteropString ToString( ) const;
    };
    template class DZ_API InteropArray<ResourceBindingSlot>;

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
        Copy,
        RayTracing
    };

    struct DZ_API DeviceConstants
    {
        uint32_t StorageBufferAlignment;
        uint32_t ConstantBufferAlignment;
        uint32_t BufferTextureAlignment;
        uint32_t BufferTextureRowAlignment;
    };

    struct DZ_API PhysicalDeviceCapabilities
    {
        bool DedicatedCopyQueue;
        bool RayTracing;
        bool ComputeShaders;
        bool Tearing;
        bool Tessellation;
        bool GeometryShaders;
        bool HDR;
    };

    struct DZ_API PhysicalDeviceProperties
    {
        bool         IsDedicated;
        unsigned int MemoryAvailableInMb;
    };

    struct DZ_API PhysicalDevice
    {
        long                       Id;
        InteropString              Name;
        PhysicalDeviceProperties   Properties;
        PhysicalDeviceCapabilities Capabilities;
        DeviceConstants            Constants;
    };
    template class DZ_API InteropArray<PhysicalDevice>;

    /**
     * Modifiable configuration to configure backend specific settings.
     * This class is not thread safe and should be configured before any backend is initialized to avoid any undefined behavior.
     */
    struct DZ_API DZConfiguration
    {
        /**
         *  NOTE: Maximum register space for Vulkan is 32(number of sets). For DirectX12/Metal this is the maximum number of root level buffers.
         *  Therefore constants should be set to 32 or lower.
         */
        /**
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

    template class DZ_API BitSet<ResourceDescriptor>;
    template class DZ_API BitSet<ResourceUsage>;
} // namespace DenOfIz
