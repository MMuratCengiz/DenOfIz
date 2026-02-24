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

#include <webgpu/webgpu.h>
#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Backends/Interface/ShaderData.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/IPipeline.h"

WGPUTextureFormat        DenOfIz_WebGPUEnumConverter_ConvertFormat( DenOfIz_Format format );
WGPUVertexFormat         DenOfIz_WebGPUEnumConverter_ConvertVertexFormat( DenOfIz_Format format );
WGPUTextureDimension     DenOfIz_WebGPUEnumConverter_ConvertTextureDimension( uint32_t width, uint32_t height, uint32_t depth );
WGPUTextureViewDimension DenOfIz_WebGPUEnumConverter_ConvertTextureViewDimension( uint32_t arraySize, uint32_t depth );
WGPUCompareFunction      DenOfIz_WebGPUEnumConverter_ConvertCompareOp( DenOfIz_CompareOp op );
WGPUStencilOperation     DenOfIz_WebGPUEnumConverter_ConvertStencilOp( DenOfIz_StencilOp op );
WGPUAddressMode          DenOfIz_WebGPUEnumConverter_ConvertSamplerAddressMode( DenOfIz_SamplerAddressMode mode );
WGPUFilterMode           DenOfIz_WebGPUEnumConverter_ConvertFilter( DenOfIz_Filter filter );
WGPUMipmapFilterMode     DenOfIz_WebGPUEnumConverter_ConvertMipmapMode( DenOfIz_MipmapMode mode );
WGPULoadOp               DenOfIz_WebGPUEnumConverter_ConvertLoadOp( DenOfIz_LoadOp op );
WGPUStoreOp              DenOfIz_WebGPUEnumConverter_ConvertStoreOp( DenOfIz_StoreOp op );
WGPUPrimitiveTopology    DenOfIz_WebGPUEnumConverter_ConvertPrimitiveTopology( DenOfIz_PrimitiveTopology topology );
WGPUIndexFormat          DenOfIz_WebGPUEnumConverter_ConvertIndexType( DenOfIz_IndexType type );
WGPUCullMode             DenOfIz_WebGPUEnumConverter_ConvertCullMode( DenOfIz_CullMode mode );
WGPUBlendFactor          DenOfIz_WebGPUEnumConverter_ConvertBlend( DenOfIz_Blend factor );
WGPUBlendOperation       DenOfIz_WebGPUEnumConverter_ConvertBlendOp( DenOfIz_BlendOp op );
WGPUBufferUsage          DenOfIz_WebGPUEnumConverter_ConvertBufferUsage( DenOfIz_ResourceDescriptorFlags descriptor, DenOfIz_HeapType heapType, DenOfIz_ResourceUsageFlags usages );
WGPUTextureUsage         DenOfIz_WebGPUEnumConverter_ConvertTextureUsage( DenOfIz_ResourceDescriptorFlags descriptor, DenOfIz_HeapType heapType );
WGPUTextureUsage         DenOfIz_WebGPUEnumConverter_ConvertSwapChainTextureUsage( DenOfIz_ResourceDescriptorFlags descriptor, DenOfIz_ResourceUsageFlags usages );
WGPUTextureAspect        DenOfIz_WebGPUEnumConverter_ConvertTextureAspect( DenOfIz_TextureAspect aspect );
WGPUTextureAspect        DenOfIz_WebGPUEnumConverter_GetTextureAspectFromFormat( DenOfIz_Format format );
WGPUVertexStepMode       DenOfIz_WebGPUEnumConverter_ConvertStepRate( DenOfIz_StepRate stepRate );
uint32_t                 DenOfIz_WebGPUEnumConverter_ConvertSampleCount( DenOfIz_MSAASampleCount sampleCount );
WGPUShaderStage          DenOfIz_WebGPUEnumConverter_ConvertShaderStage( DenOfIz_ShaderStageFlags stages );
