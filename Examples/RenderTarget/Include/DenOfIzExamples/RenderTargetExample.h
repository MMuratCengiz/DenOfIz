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

#include "DenOfIzExamples/Assets/SphereAsset.h"
#include "DenOfIzExamples/DefaultRenderPipeline.h"
#include "DenOfIzExamples/IExample.h"
#include "DenOfIzExamples/QuadPipeline.h"

namespace DenOfIz
{
    class RenderTargetExample final : public IExample
    {
        std::unique_ptr<QuadPipeline>          m_quadPipeline;
        std::unique_ptr<DefaultRenderPipeline> m_renderPipeline;
        std::unique_ptr<SphereAsset>           m_sphere;

        std::vector<std::unique_ptr<ITextureResource>> m_deferredRenderTargets;
        std::unique_ptr<ISampler>                      m_defaultSampler;
        std::unique_ptr<IResourceBindGroup>            m_rootConstantBindGroup;

        std::unique_ptr<ICommandListPool>          m_deferredCommandListPool;
        std::array<ICommandList *, 3>              m_deferredCommandLists = { };
        std::array<std::unique_ptr<ISemaphore>, 3> m_deferredSemaphores   = { };

    public:
        ~RenderTargetExample( ) override = default;
        void              Init( ) override;
        void              RenderDeferredImage( uint32_t frameIndex );
        void              ModifyApiPreferences( APIPreference &defaultApiPreference ) override;
        void              HandleEvent( Event &event ) override;
        void              Update( ) override;
        void              Render( uint32_t frameIndex, ICommandList *commandList ) override;
        void              Quit( ) override;
        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc  = DenOfIz::ExampleWindowDesc( );
            windowDesc.Title = "RenderTargetExample";
            return windowDesc;
        }
    };
} // namespace DenOfIz
