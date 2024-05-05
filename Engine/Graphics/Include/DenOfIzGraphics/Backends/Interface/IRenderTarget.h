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

#include <DenOfIzCore/Common.h>
#include "IResource.h"

namespace DenOfIz
{

    enum class RenderTargetType
    {
        Color,
        Depth,
        Stencil,
        DepthAndStencil
    };

    enum class SubmitResult
    {
        Success,
        OtherError,
        SwapChainInvalidated
    };

    struct RenderPassCreateInfo
    {
        uint32_t SwapChainImageIndex;

        bool RenderToSwapChain;

        MSAASampleCount MSAASampleCount = MSAASampleCount::_0;

        RenderTargetType RenderTargetType = RenderTargetType::Color;
        ImageFormat Format = ImageFormat::B8G8R8A8Unorm;

        uint32_t Width = 0; // 0 == Match Swap Chain
        uint32_t Height = 0; // 0 == Match Swap Chain
    };

    class IRenderPassCommands;
    class ICommandQueue;

    class ICommandQueue
    {
    public:
        ICommandQueue *Begin();
        IRenderPassCommands *RenderPass();

        void End();
    };

    class IRenderPassCommands
    {

    };

}
