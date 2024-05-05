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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanDevice.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanPipeline.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanRenderPass.h>
#include <DenOfIzGraphics/Backends/Vulkan/Resource/VulkanBufferResource.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanDevice.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanRenderPass.h>
#include <DenOfIzGraphics/Backends/Common/ShaderCompiler.h>
#include <DenOfIzGraphics/Backends/Common/SpvProgram.h>
#include <DenOfIzCore/Time.h>
#include <iostream>

namespace DenOfIz
{

    class TestVulkanRenderer
    {
        std::vector<float> triangle{ 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                                     1.0 };

        SDL_Window *window;

        VulkanDevice device{};
        std::unique_ptr<SpvProgram> program;
        std::unique_ptr<VulkanPipeline> pipeline;
        std::unique_ptr<VulkanBufferResource> vertexBuffer;
        std::unique_ptr<VulkanBufferResource> timePassedBuffer;
        std::unique_ptr<Time> time = std::make_unique<Time>();
        std::vector<std::unique_ptr<VulkanLock>> fences;
        std::vector<std::unique_ptr<VulkanRenderPass>> renderPasses;
        int frameIndex = 0;

    public:
        void Setup( SDL_Window *w );
        void Render();
        void Exit();
    };

}
