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

#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>

namespace DenOfIz
{
    struct SimplifiedPipelineDesc : PipelineDesc
    {
    private:
        std::string VertexShaderPath;
        std::string PixelShaderPath;
        bool        IsVsPsSetup = false;
        friend class SimplifiedPipeline;

    public:
        SimplifiedPipelineDesc( const std::string &vertexShaderPath, const std::string &pixelShaderPath )
        {
            VertexShaderPath = vertexShaderPath;
            PixelShaderPath  = pixelShaderPath;
            IsVsPsSetup      = true;
        }
    };

    class SimplifiedPipeline
    {
        std::unique_ptr<ShaderProgram> m_program;
        std::unique_ptr<IPipeline>     m_pipeline;

    public:
        SimplifiedPipeline( const GraphicsApi *graphicsApi, ILogicalDevice *logicalDevice, SimplifiedPipelineDesc &pipelineDesc )
        {
            std::vector<ShaderDesc> shaders{ };
            ShaderDesc              vertexShaderDesc{ };
            vertexShaderDesc.Path  = pipelineDesc.VertexShaderPath;
            vertexShaderDesc.Stage = ShaderStage::Vertex;
            shaders.push_back( vertexShaderDesc );
            ShaderDesc pixelShaderDesc{ };
            pixelShaderDesc.Path  = pipelineDesc.PixelShaderPath;
            pixelShaderDesc.Stage = ShaderStage::Pixel;
            shaders.push_back( pixelShaderDesc );

            m_program = graphicsApi->CreateShaderProgram( shaders );

            pipelineDesc.ShaderProgram = m_program.get( );
        }

        IPipeline *Pipeline( ) const
        {
            return m_pipeline.get( );
        }
    };
} // namespace DenOfIz
