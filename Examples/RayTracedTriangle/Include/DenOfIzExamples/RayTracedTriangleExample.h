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

#include <DenOfIzExamples/DefaultRenderPipeline.h>
#include <DenOfIzExamples/IExample.h>
#include <DenOfIzExamples/QuadPipeline.h>
#include <DenOfIzExamples/SphereAsset.h>
#include <DenOfIzGraphics/Renderer/Common/CommandListRing.h>
#include <DenOfIzGraphics/Renderer/Graph/RenderGraph.h>
#include <DenOfIzGraphics/Utilities/Time.h>

namespace DenOfIz
{
    class RayTracedTriangleExample final : public IExample, public NodeExecutionCallback, public PresentExecutionCallback
    {
        Time                                   m_time;
        std::unique_ptr<ShaderProgram>         m_rayTracingProgram;
        std::unique_ptr<IPipeline>             m_rayTracingPipeline;
        std::unique_ptr<IRootSignature>        m_rayTracingRootSignature;
        std::unique_ptr<QuadPipeline>          m_quadPipeline;
        std::unique_ptr<SphereAsset>           m_sphere;

        std::vector<std::unique_ptr<ITextureResource>> m_deferredRenderTargets;
        std::unique_ptr<ISampler>                      m_defaultSampler;
        std::unique_ptr<IResourceBindGroup>            m_rootConstantBindGroup;

        std::unique_ptr<RenderGraph> m_renderGraph;

    public:
        ~RayTracedTriangleExample( ) override = default;
        void              Init( ) override;
        void              ModifyApiPreferences( APIPreference &defaultApiPreference ) override;
        void              HandleEvent( SDL_Event &event ) override;
        void              Update( ) override;
        void              Quit( ) override;
        void              Execute( uint32_t frameIndex, ICommandList *commandList ) override;
        void              Execute( uint32_t frameIndex, ICommandList *commandList, ITextureResource *renderTarget ) override;
        struct WindowDesc WindowDesc( ) override
        {
            auto windowDesc  = DenOfIz::WindowDesc( );
            windowDesc.Title = "RayTracedTriangleExample";
            return windowDesc;
        }
    };
} // namespace DenOfIz
