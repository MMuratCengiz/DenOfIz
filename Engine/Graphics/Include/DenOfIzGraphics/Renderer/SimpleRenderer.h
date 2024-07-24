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

#include <DenOfIzCore/Time.h>
#include <DenOfIzGraphics/Backends/Common/ShaderProgram.h>
#include <DenOfIzGraphics/Backends/GraphicsAPI.h>
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include <DenOfIzGraphics/Data/Geometry.h>
#include <DenOfIzGraphics/Renderer/Common/CommandListRing.h>
#include <DenOfIzGraphics/Helpers/BufferHelper.h>

namespace DenOfIz
{

    class SimpleRenderer
    {
    private:
        const uint32_t                           mc_framesInFlight = 3;
        std::unique_ptr<ILogicalDevice>          m_logicalDevice;
        GraphicsWindowHandle                    *m_window;
        bool                                     m_isFirstFrame = true;
        GeometryData                             m_sphere       = Geometry::BuildSphere({ .Diameter = 1.0f, .Tessellation = 32 });
        GeometryData                             m_plane        = Geometry::BuildBox({ .Size = { 1.0f, 0.2f, 1.0f } });
        XMFLOAT4X4                               m_mvpMatrix;
        std::unique_ptr<ShaderProgram>           m_program;
        std::unique_ptr<IPipeline>               m_pipeline;
        std::unique_ptr<BatchResourceCopy>       m_batchResourceCopy;
        std::unique_ptr<IBufferResource>         m_mvpMatrixBuffer;
        std::unique_ptr<IBufferResource>         m_vertexBuffer;
        std::unique_ptr<IBufferResource>         m_indexBuffer;
        std::unique_ptr<ITextureResource>        m_texture;
        std::unique_ptr<ISampler>                m_sampler;
        std::unique_ptr<IBufferResource>         m_timePassedBuffer;
        void                                    *m_mappedTimePassedBuffer;
        std::unique_ptr<Time>                    m_time = std::make_unique<Time>();
        std::unique_ptr<IInputLayout>            m_inputLayout;
        std::unique_ptr<IRootSignature>          m_rootSignature;
        std::unique_ptr<IResourceBindGroup>      m_resourceBindGroup;
        std::unique_ptr<ISwapChain>              m_swapChain;
        std::vector<std::unique_ptr<IFence>>     m_fences;
        std::vector<std::unique_ptr<ISemaphore>> m_imageReadySemaphores;
        std::vector<std::unique_ptr<ISemaphore>> m_imageRenderedSemaphores;
        std::unique_ptr<CommandListRing>         m_commandListRing;

    public:
        void Init(GraphicsWindowHandle *window);
        void Render();
        void Quit();
    private:
        void UpdateMVPMatrix();
    };

} // namespace DenOfIz
