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

// Adaption of `vk::Format`
enum class ImageFormat
{
	Undefined,
	R4G4UnormPack8,
	R4G4B4A4UnormPack16,
	B4G4R4A4UnormPack16,
	R5G6B5UnormPack16,
	B5G6R5UnormPack16,
	R5G5B5A1UnormPack16,
	B5G5R5A1UnormPack16,
	A1R5G5B5UnormPack16,
	R8Unorm,
	R8Snorm,
	R8Uscaled,
	R8Sscaled,
	R8Uint,
	R8Sint,
	R8Srgb,
	R8G8Unorm,
	R8G8Snorm,
	R8G8Uscaled,
	R8G8Sscaled,
	R8G8Uint,
	R8G8Sint,
	R8G8Srgb,
	R8G8B8Unorm,
	R8G8B8Snorm,
	R8G8B8Uscaled,
	R8G8B8Sscaled,
	R8G8B8Uint,
	R8G8B8Sint,
	R8G8B8Srgb,
	B8G8R8Unorm,
	B8G8R8Snorm,
	B8G8R8Uscaled,
	B8G8R8Sscaled,
	B8G8R8Uint,
	B8G8R8Sint,
	B8G8R8Srgb,
	R8G8B8A8Unorm,
	R8G8B8A8Snorm,
	R8G8B8A8Uscaled,
	R8G8B8A8Sscaled,
	R8G8B8A8Uint,
	R8G8B8A8Sint,
	R8G8B8A8Srgb,
	B8G8R8A8Unorm,
	B8G8R8A8Snorm,
	B8G8R8A8Uscaled,
	B8G8R8A8Sscaled,
	B8G8R8A8Uint,
	B8G8R8A8Sint,
	B8G8R8A8Srgb,
	A8B8G8R8UnormPack32,
	A8B8G8R8SnormPack32,
	A8B8G8R8UscaledPack32,
	A8B8G8R8SscaledPack32,
	A8B8G8R8UintPack32,
	A8B8G8R8SintPack32,
	A8B8G8R8SrgbPack32,
	A2R10G10B10UnormPack32,
	A2R10G10B10SnormPack32,
	A2R10G10B10UscaledPack32,
	A2R10G10B10SscaledPack32,
	A2R10G10B10UintPack32,
	A2R10G10B10SintPack32,
	A2B10G10R10UnormPack32,
	A2B10G10R10SnormPack32,
	A2B10G10R10UscaledPack32,
	A2B10G10R10SscaledPack32,
	A2B10G10R10UintPack32,
	A2B10G10R10SintPack32,
	R16Unorm,
	R16Snorm,
	R16Uscaled,
	R16Sscaled,
	R16Uint,
	R16Sint,
	R16Sfloat,
	R16G16Unorm,
	R16G16Snorm,
	R16G16Uscaled,
	R16G16Sscaled,
	R16G16Uint,
	R16G16Sint,
	R16G16Sfloat,
	R16G16B16Unorm,
	R16G16B16Snorm,
	R16G16B16Uscaled,
	R16G16B16Sscaled,
	R16G16B16Uint,
	R16G16B16Sint,
	R16G16B16Sfloat,
	R16G16B16A16Unorm,
	R16G16B16A16Snorm,
	R16G16B16A16Uscaled,
	R16G16B16A16Sscaled,
	R16G16B16A16Uint,
	R16G16B16A16Sint,
	R16G16B16A16Sfloat,
	R32Uint,
	R32Sint,
	R32Sfloat,
	R32G32Uint,
	R32G32Sint,
	R32G32Sfloat,
	R32G32B32Uint,
	R32G32B32Sint,
	R32G32B32Sfloat,
	R32G32B32A32Uint,
	R32G32B32A32Sint,
	R32G32B32A32Sfloat,
	R64Uint,
	R64Sint,
	R64Sfloat,
	R64G64Uint,
	R64G64Sint,
	R64G64Sfloat,
	R64G64B64Uint,
	R64G64B64Sint,
	R64G64B64Sfloat,
	R64G64B64A64Uint,
	R64G64B64A64Sint,
	R64G64B64A64Sfloat,
	B10G11R11UfloatPack32,
	E5B9G9R9UfloatPack32,
	D16Unorm,
	X8D24UnormPack32,
	D32Sfloat,
	S8Uint,
	D16UnormS8Uint,
	D24UnormS8Uint,
	D32SfloatS8Uint,
	Bc1RgbUnormBlock,
	Bc1RgbSrgbBlock,
	Bc1RgbaUnormBlock,
	Bc1RgbaSrgbBlock,
	Bc2UnormBlock,
	Bc2SrgbBlock,
	Bc3UnormBlock,
	Bc3SrgbBlock,
	Bc4UnormBlock,
	Bc4SnormBlock,
	Bc5UnormBlock,
	Bc5SnormBlock,
	Bc6HUfloatBlock,
	Bc6HSfloatBlock,
	Bc7UnormBlock,
	Bc7SrgbBlock,
	Etc2R8G8B8UnormBlock,
	Etc2R8G8B8SrgbBlock,
	Etc2R8G8B8A1UnormBlock,
	Etc2R8G8B8A1SrgbBlock,
	Etc2R8G8B8A8UnormBlock,
	Etc2R8G8B8A8SrgbBlock,
	EacR11UnormBlock,
	EacR11SnormBlock,
	EacR11G11UnormBlock,
	EacR11G11SnormBlock,
	Astc4x4UnormBlock,
	Astc4x4SrgbBlock,
	Astc5x4UnormBlock,
	Astc5x4SrgbBlock,
	Astc5x5UnormBlock,
	Astc5x5SrgbBlock,
	Astc6x5UnormBlock,
	Astc6x5SrgbBlock,
	Astc6x6UnormBlock,
	Astc6x6SrgbBlock,
	Astc8x5UnormBlock,
	Astc8x5SrgbBlock,
	Astc8x6UnormBlock,
	Astc8x6SrgbBlock,
	Astc8x8UnormBlock,
	Astc8x8SrgbBlock,
	Astc10x5UnormBlock,
	Astc10x5SrgbBlock,
	Astc10x6UnormBlock,
	Astc10x6SrgbBlock,
	Astc10x8UnormBlock,
	Astc10x8SrgbBlock,
	Astc10x10UnormBlock,
	Astc10x10SrgbBlock,
	Astc12x10UnormBlock,
	Astc12x10SrgbBlock,
	Astc12x12UnormBlock,
	Astc12x12SrgbBlock,
	G8B8G8R8422Unorm,
	B8G8R8G8422Unorm,
	G8B8R83Plane420Unorm,
	G8B8R82Plane420Unorm,
	G8B8R83Plane422Unorm,
	G8B8R82Plane422Unorm,
	G8B8R83Plane444Unorm,
	R10X6UnormPack16,
	R10X6G10X6Unorm2Pack16,
	R10X6G10X6B10X6A10X6Unorm4Pack16,
	G10X6B10X6G10X6R10X6422Unorm4Pack16,
	B10X6G10X6R10X6G10X6422Unorm4Pack16,
	G10X6B10X6R10X63Plane420Unorm3Pack16,
	G10X6B10X6R10X62Plane420Unorm3Pack16,
	G10X6B10X6R10X63Plane422Unorm3Pack16,
	G10X6B10X6R10X62Plane422Unorm3Pack16,
	G10X6B10X6R10X63Plane444Unorm3Pack16,
	R12X4UnormPack16,
	R12X4G12X4Unorm2Pack16,
	R12X4G12X4B12X4A12X4Unorm4Pack16,
	G12X4B12X4G12X4R12X4422Unorm4Pack16,
	B12X4G12X4R12X4G12X4422Unorm4Pack16,
	G12X4B12X4R12X43Plane420Unorm3Pack16,
	G12X4B12X4R12X42Plane420Unorm3Pack16,
	G12X4B12X4R12X43Plane422Unorm3Pack16,
	G12X4B12X4R12X42Plane422Unorm3Pack16,
	G12X4B12X4R12X43Plane444Unorm3Pack16,
	G16B16G16R16422Unorm,
	B16G16R16G16422Unorm,
	G16B16R163Plane420Unorm,
	G16B16R162Plane420Unorm,
	G16B16R163Plane422Unorm,
	G16B16R162Plane422Unorm,
	G16B16R163Plane444Unorm,
	G8B8R82Plane444Unorm,
	G10X6B10X6R10X62Plane444Unorm3Pack16,
	G12X4B12X4R12X42Plane444Unorm3Pack16,
	G16B16R162Plane444Unorm,
	A4R4G4B4UnormPack16,
	A4B4G4R4UnormPack16,
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

enum class MemoryLocation
{
	Auto,
	GPU,
	CPU,
	CPU_GPU,
	GPU_CPU
};

enum class BufferMemoryUsage
{
	TransferSrc,
	TransferDst,
	UniformTexelBuffer,
	StorageTexelBuffer,
	UniformBuffer,
	StorageBuffer,
	IndexBuffer,
	VertexBuffer,
	IndirectBuffer,
	ShaderDeviceAddress,
	VideoDecodeSrc,
	VideoDecodeDst,
	TransformFeedbackBufferEXT,
	TransformFeedbackCounterBufferEXT,
	ConditionalRenderingEXT,
	AccelerationStructureBuildInputReadOnly,
	AccelerationStructureStorage,
	ShaderBindingTable,
	RayTracingNV,
	ShaderDeviceAddressEXT,
	VideoEncodeDst,
	VideoEncodeSrc,
	SamplerDescriptorBufferEXT,
	ResourceDescriptorBufferEXT,
	PushDescriptorsDescriptorBufferEXT,
	MicromapBuildInputReadOnlyEXT,
	MicromapStorageEXT,
};

enum class ImageMemoryUsage
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

enum class ImageAspect
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
	unsigned int NonPixelShaderResource : 1;
	unsigned int PixelShaderResource : 1;
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

// TODO should this be resource type instead
enum class ResourceBindingType
{
	Sampler,
	CombinedImageSampler,
	SampledImage,
	StorageImage,
	UniformTexelBuffer,
	StorageTexelBuffer,
	UniformBuffer,
	StorageBuffer,
	UniformBufferDynamic,
	StorageBufferDynamic,
	InputAttachment,
	InlineUniformBlock,
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
	Transfer,
	Presentation
};

struct PhysicalDeviceCapabilities
{
	bool DedicatedTransferQueue;
	bool RayTracing;
	bool ComputeShaders;
	bool Tearing;
	bool Tessellation;
	bool GeometryShaders;;
};

struct PhysicalDeviceProperties
{
	bool IsDedicated;
	unsigned int MemoryAvailableInMb;
};

struct PhysicalDeviceInfo
{
	long Id;
	std::string Name;
	PhysicalDeviceProperties Properties;
	PhysicalDeviceCapabilities Capabilities;
};