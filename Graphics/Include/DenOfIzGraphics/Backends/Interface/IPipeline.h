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

#include <DenOfIzGraphics/Backends/Common/ShaderProgram.h>
#include "IBufferResource.h"
#include "IInputLayout.h"
#include "IRootSignature.h"
#include "IShader.h"
#include "ITextureResource.h"

namespace DenOfIz
{
    enum class BindPoint
    {
        Graphics,
        Compute,
        RayTracing
    };

    enum class BlendMode
    {
        None,
        AlphaBlend
    };

    enum class CullMode
    {
        BackFace,
        FrontFace,
        None
    };

    namespace ViewMask
    {
        constexpr int R = 0x00000001;
        constexpr int G = 0x00000002;
        constexpr int B = 0x00000004;
        constexpr int A = 0x00000008;
    } // namespace ViewMask

    enum class Blend
    {
        Zero,
        One,
        SrcColor,
        InvSrcColor,
        SrcAlpha,
        InvSrcAlpha,
        DstAlpha,
        InvDstAlpha,
        DstColor,
        InvDstColor,
        SrcAlphaSaturate,
        BlendFactor,
        InvBlendFactor,
        Src1Color,
        InvSrc1Color,
        Src1Alpha,
        InvSrc1Alpha
    };

    enum class BlendOp
    {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };

    enum class LogicOp
    {
        Clear,
        Set,
        Copy,
        CopyInverted,
        Noop,
        Invert,
        And,
        Nand,
        Or,
        Nor,
        Xor,
        Equiv,
        AndReverse,
        AndInverted,
        OrReverse,
        OrInverted
    };

    struct DZ_API BlendDesc
    {
        bool         Enable                = false;
        Blend        SrcBlend              = Blend::One;
        Blend        DstBlend              = Blend::Zero;
        Blend        SrcBlendAlpha         = Blend::One;
        Blend        DstBlendAlpha         = Blend::Zero;
        BlendOp      BlendOp               = BlendOp::Add;
        enum BlendOp BlendOpAlpha          = BlendOp::Add;
        uint8_t      RenderTargetWriteMask = 0x0F;
    };

    struct DZ_API RenderTargetDesc
    {
        BlendDesc Blend  = { };
        Format    Format = Format::Undefined;
    };

#define DZ_MAX_RENDER_TARGETS 8
    struct DZ_API RenderTargetDescs
    {
        size_t           NumElements = 0;
        RenderTargetDesc Array[ DZ_MAX_RENDER_TARGETS ];

        void SetElement( size_t index, const RenderTargetDesc &value )
        {
            Array[ index ] = value;
        }
        const RenderTargetDesc &GetElement( size_t index )
        {
            return Array[ index ];
        }
    };

    struct DZ_API PipelineRendering
    {
        uint32_t          ViewMask               = 0;
        bool              AlphaToCoverageEnable  = false; // Todo check if required
        bool              IndependentBlendEnable = false; // Todo check if required
        bool              BlendLogicOpEnable     = false;
        LogicOp           BlendLogicOp           = LogicOp::Noop;
        RenderTargetDescs RenderTargets;
        Format            DepthStencilAttachmentFormat = Format::Undefined;
    };

    struct DZ_API DepthTest
    {
        bool      Enable    = false;
        CompareOp CompareOp = CompareOp::Always;
        bool      Write     = false;
    };

    struct DZ_API StencilFace
    {
        CompareOp CompareOp   = CompareOp::Always;
        StencilOp FailOp      = StencilOp::Keep;
        StencilOp PassOp      = StencilOp::Keep;
        StencilOp DepthFailOp = StencilOp::Keep;
    };

    struct DZ_API StencilTest
    {
        bool        Enable    = false;
        uint32_t    WriteMask = 0;
        uint32_t    ReadMask  = 0;
        StencilFace FrontFace;
        StencilFace BackFace;
    };

    struct DZ_API PipelineDesc
    {
        IInputLayout   *InputLayout   = nullptr;
        IRootSignature *RootSignature = nullptr;
        ShaderProgram  *ShaderProgram = nullptr;

        PrimitiveTopology PrimitiveTopology = PrimitiveTopology::Triangle;
        CullMode          CullMode          = CullMode::None;
        BindPoint         BindPoint         = BindPoint::Graphics;
        DepthTest         DepthTest;
        StencilTest       StencilTest;

        PipelineRendering Rendering;
        MSAASampleCount   MSAASampleCount = MSAASampleCount::_0; // 0 Disables MSAA
    };

    class DZ_API IPipeline
    {
    public:
        virtual ~IPipeline( ) = default;
    };

} // namespace DenOfIz
