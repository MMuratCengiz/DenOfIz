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
#include <DenOfIzExamples/QuadPipeline.h>
#include <DenOfIzGraphics/Renderer/Graph/RenderGraph.h>
#include <DenOfIzGraphics/Utilities/Time.h>

namespace DenOfIz
{
    struct NormalizedViewport
    {
        float Left;
        float Top;
        float Right;
        float Bottom;
    };

    struct RayGenConstantBuffer
    {
        NormalizedViewport Viewport;
        NormalizedViewport Stencil;
    };

    class SimpleTriangleExample final : public IExample
    {
        Time                             m_time;
        std::unique_ptr<IBufferResource> m_rayGenCBResource;
        std::unique_ptr<ShaderProgram>   m_program;
        std::unique_ptr<IPipeline>       m_pipeline;
        std::unique_ptr<IInputLayout>    m_inputLayout;
        std::unique_ptr<IRootSignature>  m_rootSignature;
        std::unique_ptr<IBufferResource> m_vertexBuffer;
        std::unique_ptr<IBufferResource> m_indexBuffer;
        //

    public:
        ~SimpleTriangleExample( ) override = default;
        void              Init( ) override;
        void              ModifyApiPreferences( APIPreference &defaultApiPreference ) override;
        void              HandleEvent( SDL_Event &event ) override;
        void              Update( ) override;
        void              Render( uint32_t frameIndex, ICommandList *commandList ) override;
        void              Quit( ) override;
        struct WindowDesc WindowDesc( ) override
        {
            auto windowDesc   = DenOfIz::WindowDesc( );
            windowDesc.Title  = "SimpleTriangleExample";
            windowDesc.Width  = 1280;
            windowDesc.Height = 720;
            return windowDesc;
        }

    private:
        void                      CreateVertexBuffer( );
        static InteropArray<Byte> VertexShader( );
        static InteropArray<Byte> PixelShader( );
    };
} // namespace DenOfIz
