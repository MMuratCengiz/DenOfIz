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
#include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>
#include <DenOfIzGraphics/Renderer/Graph/RenderGraph.h>

#include "DenOfIzGraphics/Utilities/Time.h"

namespace DenOfIz
{
    class BindlessExample final : public IExample
    {
        Time                             m_time;
        float                            m_elapsedTime = 0.0f;
        std::unique_ptr<ShaderProgram>   m_program;
        std::unique_ptr<IPipeline>       m_pipeline;
        std::unique_ptr<IInputLayout>    m_inputLayout;
        std::unique_ptr<IRootSignature>  m_rootSignature;
        std::unique_ptr<IBufferResource> m_vertexBuffer;
        std::unique_ptr<IBufferResource> m_indexBuffer;

        // Bindless resources
        static constexpr uint32_t           NUM_TEXTURES = 4;
        std::unique_ptr<ITextureResource>   m_textures[ NUM_TEXTURES ];
        std::unique_ptr<ISampler>           m_sampler;
        std::unique_ptr<IBufferResource>    m_constantBuffer;
        std::unique_ptr<IResourceBindGroup> m_bindGroup;
        std::unique_ptr<IResourceBindGroup> m_perFrameBindGroup;
        uint32_t                            m_currentTextureIndex = 0;

        struct PerFrameData
        {
            uint32_t textureIndex;
            float    time;
            uint32_t padding[ 2 ];
        };

    public:
        ~BindlessExample( ) override = default;
        void                     Init( ) override;
        void                     ModifyApiPreferences( APIPreference &defaultApiPreference ) override;
        void                     HandleEvent( Event &event ) override;
        void                     Update( ) override;
        void                     Render( uint32_t frameIndex, ICommandList *commandList ) override;
        void                     Quit( ) override;
        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc   = ExampleWindowDesc( );
            windowDesc.Title  = "BindlessExample";
            windowDesc.Width  = 1280;
            windowDesc.Height = 720;
            return windowDesc;
        }

    private:
        void                      CreateVertexBuffer( );
        void                      CreateTextures( );
        void                      CreateSampler( );
        void                      CreateConstantBuffer( );
        static InteropArray<Byte> VertexShader( );
        static InteropArray<Byte> PixelShader( );
    };
} // namespace DenOfIz
