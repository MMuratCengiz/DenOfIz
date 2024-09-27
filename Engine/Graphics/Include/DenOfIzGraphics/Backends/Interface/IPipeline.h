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

    struct StencilTestState
    {
        bool      enabled = false;
        CompareOp compareOp;
        uint32_t  compareMask;
        uint32_t  writeMask;
        uint32_t  ref;

        StencilOp failOp;
        StencilOp passOp;
        StencilOp depthFailOp;
    };

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
        // Remove?
        FrontAndBackFace,
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
    }; // namespace ViewMask

    struct PipelineRendering
    {
        uint32_t ViewMask = 0;

        std::vector<Format> ColorAttachmentFormats;
        Format              DepthStencilAttachmentFormat = Format::Undefined;
    };

    struct DepthTest
    {
        bool      Enable    = true;
        CompareOp CompareOp = CompareOp::Always;
        bool      Write;
    };

    struct StencilFace
    {
        CompareOp CompareOp   = CompareOp::Always;
        StencilOp FailOp      = StencilOp::Keep;
        StencilOp PassOp      = StencilOp::Keep;
        StencilOp DepthFailOp = StencilOp::Keep;
    };

    struct StencilTest
    {
        bool        Enable    = false;
        uint32_t    WriteMask = 0;
        uint32_t    ReadMask  = 0;
        StencilFace FrontFace;
        StencilFace BackFace;
    };

    struct PipelineDesc
    {
        IInputLayout   *InputLayout   = nullptr;
        IRootSignature *RootSignature = nullptr;
        ShaderProgram  *ShaderProgram = nullptr;

        PrimitiveTopology PrimitiveTopology = PrimitiveTopology::Triangle;
        CullMode          CullMode          = CullMode::None;
        BindPoint         BindPoint         = BindPoint::Graphics;
        DepthTest         DepthTest;
        StencilTest       StencilTest;

        std::vector<BlendMode> BlendModes;

        PipelineRendering Rendering;
        MSAASampleCount   MSAASampleCount = MSAASampleCount::_0; // 0 Disables MSAA
    };

    class IPipeline
    {
    public:
        virtual ~IPipeline( ) = default;
    };

} // namespace DenOfIz
