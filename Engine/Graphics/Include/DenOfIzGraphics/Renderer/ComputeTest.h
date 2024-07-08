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

#include <DenOfIzGraphics/Backends/GraphicsAPI.h>

namespace DenOfIz
{
    class ComputeTest
    {
    private:
        std::unique_ptr<ILogicalDevice>     m_logicalDevice;
        ShaderProgram                       m_program;
        std::unique_ptr<ICommandListPool>   m_commandListPool;
        std::unique_ptr<IPipeline>          m_pipeline;
        std::unique_ptr<IBufferResource>    buffer;
        std::unique_ptr<IInputLayout>       m_inputLayout;
        std::unique_ptr<IRootSignature>     m_rootSignature;
        std::unique_ptr<IResourceBindGroup> m_resourceBindGroup;
        std::unique_ptr<IFence>             m_fence;

    public:
        ComputeTest();
        ~ComputeTest();

        int Run();
    };
} // namespace DenOfIz
