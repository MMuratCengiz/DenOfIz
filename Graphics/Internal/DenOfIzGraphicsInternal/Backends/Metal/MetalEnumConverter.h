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
