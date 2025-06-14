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

#include "DenOfIzGraphics/Backends/Interface/ReflectionData.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"
#include "MetalContext.h"
#include "MetalPipeline.h"

namespace DenOfIz
{
    class MetalEnumConverter final : public NonCopyable
    {
        MetalEnumConverter( ) = delete;

    public:
        static MTLBindingAccess          ConvertDescriptorToBindingAccess( const uint32_t &descriptor );
        static MTLPixelFormat            ConvertFormat( Format format );
        static MTLVertexFormat           ConvertFormatToVertexFormat( Format format );
        static MTLAttributeFormat        ConvertFormatToAttributeFormat( Format format );
        static MTLDataType               ConvertFormatToDataType( Format format );
        static MTLLoadAction             ConvertLoadAction( LoadOp op );
        static MTLStoreAction            ConvertStoreAction( StoreOp op );
        static MTLBlendFactor            ConvertBlendFactor( Blend blend );
        static MTLBlendOperation         ConvertBlendOp( BlendOp op );
        static MTLStencilOperation       ConvertStencilOp( StencilOp op );
        static MTLCompareFunction        ConvertCompareOp( CompareOp op );
        static MTLPrimitiveTopologyClass ConvertTopologyClass( PrimitiveTopology topology );
        static MTLSamplerMinMagFilter    ConvertFilter( Filter filter );
        static MTLSamplerMipFilter       ConvertMipMapFilter( MipmapMode mode );
        static MTLSamplerAddressMode     ConvertSamplerAddressMode( SamplerAddressMode mode );
        static MTLCompareFunction        ConvertCompareFunction( CompareOp op );
        static MTLRenderStages           ConvertRenderStage( const ShaderStage &stage );
        static MTLRenderStages           ConvertRenderStages( const ShaderStageArray &stages );
    };
} // namespace DenOfIz
