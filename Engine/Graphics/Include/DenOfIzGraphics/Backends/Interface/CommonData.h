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
	Unknown,
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
	BC7UnormSrgb,
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
	unsigned int Undefined: 1;
	unsigned int VertexAndConstantBuffer: 1;
	unsigned int IndexBuffer: 1;
	unsigned int RenderTarget: 1;
	unsigned int UnorderedAccess: 1;
	unsigned int DepthWrite: 1;
	unsigned int DepthRead: 1;
	unsigned int NonPixelShaderResource: 1;
	unsigned int PixelShaderResource: 1;
	unsigned int ShaderResource: 1;
	unsigned int StreamOut: 1;
	unsigned int IndirectArgument: 1;
	unsigned int CopyDst: 1;
	unsigned int CopySource: 1;
	unsigned int GenericRead: 1;
	unsigned int Present: 1;
	unsigned int Common: 1;
	unsigned int AccelerationStructureRead: 1;
	unsigned int AccelerationStructureWrite: 1;
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