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

namespace DenOfIz
{

    enum class Format
    {
        Undefined,
        R32G32B32A32Float,
        R32G32B32A32Uint,
        R32G32B32A32Sint,
        R32G32B32Float,
        R32G32B32Uint,
        R32G32B32Sint,
        R16G16B16A16Float,
        R16G16B16A16Unorm,
        R16G16B16A16Uint,
        R16G16B16A16Snorm,
        R16G16B16A16Sint,
        R32G32Float,
        R32G32Uint,
        R32G32Sint,
        R10G10B10A2Unorm,
        R10G10B10A2Uint,
        R8G8B8A8Unorm,
        R8G8B8A8UnormSrgb,
        R8G8B8A8Uint,
        R8G8B8A8Snorm,
        R8G8B8A8Sint,
        R16G16Float,
        R16G16Unorm,
        R16G16Uint,
        R16G16Snorm,
        R16G16Sint,
        D32Float,
        R32Float,
        R32Uint,
        R32Sint,
        D24UnormS8Uint,
        R8G8Unorm,
        R8G8Uint,
        R8G8Snorm,
        R8G8Sint,
        R16Float,
        D16Unorm,
        R16Unorm,
        R16Uint,
        R16Snorm,
        R16Sint,
        R8Unorm,
        R8Uint,
        R8Snorm,
        R8Sint,
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

    static uint32_t GetImageFormatSize(const Format &format)
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
        case Format::BC2Unorm:
        case Format::BC2UnormSrgb:
        case Format::BC3Unorm:
        case Format::BC3UnormSrgb:
        case Format::BC4Unorm:
        case Format::BC4Snorm:
        case Format::BC5Unorm:
        case Format::BC5Snorm:
            return 1;
        case Format::B8G8R8A8Unorm:
            return 4;
        case Format::BC6HUfloat16:
        case Format::BC6HSfloat16:
            return 2;
        case Format::BC7Unorm:
            return 1;
        case Format::BC7UnormSrgb:
            return 1;
        default:
            return 0;
        }
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

    enum ImageLayout
    {
        Undefined,
        General,
        ColorAttachmentOptimal,
        DepthStencilAttachmentOptimal,
        DepthStencilReadOnlyOptimal,
        ShaderReadOnlyOptimal,
        TransferSrcOptimal,
        TransferDstOptimal,
        PreInitialized,
        DepthReadOnlyStencilAttachmentOptimal,
        DepthAttachmentStencilReadOnlyOptimal,
        DepthAttachmentOptimal,
        DepthReadOnlyOptimal,
        StencilAttachmentOptimal,
        StencilReadOnlyOptimal,
        ReadOnlyOptimal,
        AttachmentOptimal,
        PresentSrc,
        VideoDecodeDst,
        VideoDecodeSrc,
        VideoDecodeDpb,
        SharedPresent,
        FragmentShadingRateAttachmentOptimal,
        RenderingLocalRead,
        VideoEncodeDst,
        VideoEncodeSrc,
        VideoEncodeDpb,
    };

    enum class HeapType
    {
        Auto,
        GPU,
        CPU,
        CPU_GPU,
        GPU_CPU
    };

    struct BufferUsage
    {
        uint32_t Map : 1;
        uint32_t CopySrc : 1;
        uint32_t CopyDst : 1;
        uint32_t IndexBuffer : 1;
        uint32_t VertexBuffer : 1;
        uint32_t UniformBuffer : 1;
        uint32_t Storage : 1;
        uint32_t Indirect : 1;
        uint32_t QueryResolve : 1;
        uint32_t AccelerationStructureScratch : 1;
        uint32_t BottomLevelAccelerationStructureInput : 1;
        uint32_t TopLevelAccelerationStructureInput : 1;
        uint32_t Exclusive : 1;
        uint32_t Ordered : 1;
        // = 0 Assumed to be read only
        uint32_t ReadWrite : 1;
    };

    enum class TextureMemoryUsage
    {
        TransferSrc,
        TransferDst,
        Sampled,
        Storage,
        ColorAttachment,
        DepthStencilAttachment,
        TransientAttachment,
        InputAttachment,
        VideoDecodeDst,
        VideoDecodeSrc,
        VideoDecodeDpb,
        FragmentDensityMap,
        FragmentShadingRateAttachment,
        HostTransferEXT,
        VideoEncodeDst,
        VideoEncodeSrc,
        VideoEncodeDpb,
        AttachmentFeedbackLoop,
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
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder,
        MirrorClampToEdge,
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
        CubicIMG,
        CubicEXT,
    };

    enum class CompareOp
    {
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

    struct ResourceState
    {
        unsigned int Undefined : 1;
        unsigned int VertexAndConstantBuffer : 1;
        unsigned int IndexBuffer : 1;
        unsigned int RenderTarget : 1;
        unsigned int UnorderedAccess : 1;
        unsigned int DepthWrite : 1;
        unsigned int DepthRead : 1;
        unsigned int ShaderResource : 1;
        unsigned int StreamOut : 1;
        unsigned int IndirectArgument : 1;
        unsigned int CopyDst : 1;
        unsigned int CopySource : 1;
        unsigned int GenericRead : 1;
        unsigned int Present : 1;
        unsigned int Common : 1;
        unsigned int AccelerationStructureRead : 1;
        unsigned int AccelerationStructureWrite : 1;
    };

    enum class ResourceType
    {
        Texture,
        CubeMap,
        Buffer
    };

    enum class ResourceBindingType
    {
        Sampler,
        Texture,
        TextureReadWrite,
        Buffer,
        BufferReadWrite,
        BufferDynamic,
        Storage,
        StorageImage,
        StorageDynamic,
        AccelerationStructure,
    };

    enum class LoadOp
    {
        Clear,
        Load,
        Unidentified
    };

    enum class StoreOp
    {
        Store,
        None,
        Unidentified
    };

    enum QueueType
    {
        Graphics,
        Compute,
        Copy,
        Presentation
    };

    struct PhysicalDeviceCapabilities
    {
        bool DedicatedTransferQueue;
        bool RayTracing;
        bool ComputeShaders;
        bool Tearing;
        bool Tessellation;
        bool GeometryShaders;
        bool HDR;
    };

    struct PhysicalDeviceProperties
    {
        bool IsDedicated;
        unsigned int MemoryAvailableInMb;
    };

    struct PhysicalDevice
    {
        long Id;
        std::string Name;
        PhysicalDeviceProperties Properties;
        PhysicalDeviceCapabilities Capabilities;
    };

} // namespace DenOfIz
