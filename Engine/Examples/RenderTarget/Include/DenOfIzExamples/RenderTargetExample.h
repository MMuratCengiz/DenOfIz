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
#include <DenOfIzExamples/DefaultRenderPipeline.h>
#include <DenOfIzExamples/IExample.h>
#include <DenOfIzExamples/QuadPipeline.h>
#include <DenOfIzExamples/SimplifiedPipeline.h>
#include <DenOfIzExamples/SphereAsset.h>
#include <DenOfIzGraphics/Renderer/Common/CommandListRing.h>
#include <DenOfIzGraphics/Renderer/Graph/RenderGraph.h>

namespace DenOfIz
{
    class RenderTargetExample final : public IExample
    {
        Time                                   m_time;
        std::unique_ptr<QuadPipeline>          m_quadPipeline;
        std::unique_ptr<DefaultRenderPipeline> m_renderPipeline;
        std::unique_ptr<SphereAsset>           m_sphere;

        std::vector<std::unique_ptr<ITextureResource>> m_deferredRenderTargets;
        std::unique_ptr<ISampler>                      m_defaultSampler;
        std::unique_ptr<IResourceBindGroup>            m_rootConstantBindGroup;

        std::unique_ptr<RenderGraph> m_renderGraph;

    public:
        ~RenderTargetExample( ) override = default;
        void              Init( ) override;
        void              ModifyApiPreferences( APIPreference &defaultApiPreference ) override;
        void              HandleEvent( SDL_Event &event ) override;
        void              Update( ) override;
        void              Quit( ) override;
        struct WindowDesc WindowDesc( ) override
        {
            auto windowDesc  = DenOfIz::WindowDesc( );
            windowDesc.Title = "RenderTargetExample";
            return windowDesc;
        }
    };
} // namespace DenOfIz
