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

#include <DenOfIzExamples/IExample.h>
#include <DenOfIzExamples/SimplifiedPipeline.h>

namespace DenOfIz
{
    class RenderTargetExample final : public IExample
    {
        std::unique_ptr<ShaderProgram>      m_quadProgram;
        std::unique_ptr<SimplifiedPipeline> m_quadPipeline;
        std::unique_ptr<SimplifiedPipeline> m_renderPipeline;
        std::unique_ptr<IBufferResource>    m_sphereVb;
        std::unique_ptr<IBufferResource>    m_sphereIb;

        std::unique_ptr<ITextureResource> m_deferredRenderTarget;

    public:
        ~                 RenderTargetExample( ) override = default;
        void              Init( ) override;
        void              ModifyApiPreferences( APIPreference &defaultApiPreference ) override;
        void              Tick( ) override;
        void              Quit( ) override;
        struct WindowDesc WindowDesc( ) override
        {
            auto windowDesc  = DenOfIz::WindowDesc( );
            windowDesc.Title = "RenderTargetExample";
            return windowDesc;
        }
    };
} // namespace DenOfIz
