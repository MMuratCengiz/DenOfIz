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
#include "ITextureResource.h"
#include "RayTracing/IBottomLevelAS.h"
#include "ShaderData.h"

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

    enum class FillMode
    {
        Solid,
        Wireframe
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
    template class DZ_API InteropArray<RenderTargetDesc>;

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

    struct DZ_API GraphicsPipelineDesc
    {
        uint32_t                       ViewMask               = 0;
        bool                           AlphaToCoverageEnable  = false; // Todo check if required
        bool                           IndependentBlendEnable = false; // Todo check if required
        bool                           BlendLogicOpEnable     = false;
        LogicOp                        BlendLogicOp           = LogicOp::Noop;
        InteropArray<RenderTargetDesc> RenderTargets;
        Format                         DepthStencilAttachmentFormat = Format::Undefined;

        PrimitiveTopology PrimitiveTopology = PrimitiveTopology::Triangle;
        CullMode          CullMode          = CullMode::None;
        FillMode          FillMode          = FillMode::Solid;
        DepthTest         DepthTest;
        StencilTest       StencilTest;
        MSAASampleCount   MSAASampleCount = MSAASampleCount::_0; // 0 Disables MSAA
    };

    struct DZ_API HitGroupDesc
    {
        InteropString Name;

        int32_t              IntersectionShaderIndex = -1; // -1 = use built-in triangle intersection
        int32_t              AnyHitShaderIndex       = -1; // -1 = no any hit shader
        int32_t              ClosestHitShaderIndex   = -1; // -1 = no closest hit shader
        ILocalRootSignature *LocalRootSignature      = nullptr;

        HitGroupType Type = HitGroupType::Triangles;
    };

    struct DZ_API RayTracingPipelineDesc
    {
        InteropArray<HitGroupDesc> HitGroups;
        // Index must match with the index of the shader provided to ShaderProgram
        InteropArray<ILocalRootSignature *> LocalRootSignatures;
    };

    struct DZ_API ComputePipelineDesc{ };

    struct DZ_API PipelineDesc
    {
        BindPoint       BindPoint     = BindPoint::Graphics;
        IInputLayout   *InputLayout   = nullptr;
        IRootSignature *RootSignature = nullptr;
        ShaderProgram  *ShaderProgram = nullptr;

        GraphicsPipelineDesc   Graphics;
        RayTracingPipelineDesc RayTracing;
        ComputePipelineDesc    Compute;
    };

    class DZ_API IPipeline
    {
    public:
        virtual ~IPipeline( ) = default;
    };

} // namespace DenOfIz
