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

#include "IShader.h"
#include "IResource.h"
#include <DenOfIzCore/Common.h>
#include <DenOfIzGraphics/Backends/Common/SpvProgram.h>

namespace DenOfIz
{

    struct StencilTestState
    {
        bool enabled = false;
        CompareOp compareOp;
        uint32_t compareMask;
        uint32_t writeMask;
        uint32_t ref;

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
    };

    struct PipelineRendering
    {
        uint32_t ViewMask = ViewMask::R | ViewMask::G | ViewMask::B | ViewMask::A;

        std::vector<ImageFormat> ColorAttachmentFormats;
        ImageFormat DepthAttachmentFormat;
        ImageFormat StencilAttachmentFormat;
    };

    struct PipelineCreateInfo
    {
        CullMode CullMode = CullMode::None;
        BindPoint BindPoint = BindPoint::Graphics;

        CompareOp DepthCompareOp;
        bool EnableDepthTest = true;
        bool InterleavedMode = true;

        StencilTestState StencilTestStateFront{};
        StencilTestState StencilTestStateBack{};

        SpvProgram SpvProgram;

        std::vector<BlendMode> BlendModes;

        PipelineRendering Rendering;
        MSAASampleCount MSAASampleCount = MSAASampleCount::_0; // 0 Disables MSAA
    };

}
