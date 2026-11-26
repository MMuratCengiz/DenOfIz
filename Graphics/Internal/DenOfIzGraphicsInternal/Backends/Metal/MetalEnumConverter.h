/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphics/Backends/Interface/CommonData.h>
#include <DenOfIzGraphics/Backends/Interface/ShaderData.h>
#include <DenOfIzGraphicsInternal/Backends/Interface/IPipeline.h>
#include "MetalContext.h"

MTLBindingAccess          DenOfIz_MetalEnumConverter_ConvertDescriptorToBindingAccess( DenOfIz_ResourceDescriptorFlags descriptor );
MTLPixelFormat            DenOfIz_MetalEnumConverter_ConvertFormat( DenOfIz_Format format );
MTLVertexFormat           DenOfIz_MetalEnumConverter_ConvertFormatToVertexFormat( DenOfIz_Format format );
MTLAttributeFormat        DenOfIz_MetalEnumConverter_ConvertFormatToAttributeFormat( DenOfIz_Format format );
MTLDataType               DenOfIz_MetalEnumConverter_ConvertFormatToDataType( DenOfIz_Format format );
MTLLoadAction             DenOfIz_MetalEnumConverter_ConvertLoadAction( DenOfIz_LoadOp op );
MTLStoreAction            DenOfIz_MetalEnumConverter_ConvertStoreAction( DenOfIz_StoreOp op );
MTLBlendFactor            DenOfIz_MetalEnumConverter_ConvertBlendFactor( DenOfIz_Blend blend );
MTLBlendOperation         DenOfIz_MetalEnumConverter_ConvertBlendOp( DenOfIz_BlendOp op );
MTLStencilOperation       DenOfIz_MetalEnumConverter_ConvertStencilOp( DenOfIz_StencilOp op );
MTLCompareFunction        DenOfIz_MetalEnumConverter_ConvertCompareOp( DenOfIz_CompareOp op );
MTLPrimitiveTopologyClass DenOfIz_MetalEnumConverter_ConvertTopologyClass( DenOfIz_PrimitiveTopology topology );
MTLPrimitiveType          DenOfIz_MetalEnumConverter_ConvertTopologyToPrimitiveType( DenOfIz_PrimitiveTopology topology );
MTLSamplerMinMagFilter    DenOfIz_MetalEnumConverter_ConvertFilter( DenOfIz_Filter filter );
MTLSamplerMipFilter       DenOfIz_MetalEnumConverter_ConvertMipMapFilter( DenOfIz_MipmapMode mode );
MTLSamplerAddressMode     DenOfIz_MetalEnumConverter_ConvertSamplerAddressMode( DenOfIz_SamplerAddressMode mode );
MTLCompareFunction        DenOfIz_MetalEnumConverter_ConvertCompareFunction( DenOfIz_CompareOp op );
MTLRenderStages           DenOfIz_MetalEnumConverter_ConvertRenderStages( DenOfIz_ShaderStageFlags stages );
