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

#include <DenOfIzGraphics/Backends/Common/ShaderProgram.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include <memory>

namespace DenOfIz
{
    struct FullscreenQuadPipelineDesc
    {
        ILogicalDevice *LogicalDevice = nullptr;
        Format          OutputFormat  = Format::B8G8R8A8Unorm;
    };

    class FullscreenQuadPipeline
    {
        ILogicalDevice                     *m_logicalDevice = nullptr;
        std::unique_ptr<ShaderProgram>      m_shaderProgram;
        std::unique_ptr<IPipeline>          m_pipeline;
        std::unique_ptr<IRootSignature>     m_rootSignature;
        std::unique_ptr<IResourceBindGroup> m_resourceBindGroup;
        std::unique_ptr<ISampler>           m_linearSampler;

    public:
        explicit FullscreenQuadPipeline( const FullscreenQuadPipelineDesc &desc );
        ~FullscreenQuadPipeline( ) = default;

        void DrawTextureToScreen( ICommandList *commandList, ITextureResource *sourceTexture ) const;

    private:
        void CreateShaderProgram( );
        void CreatePipeline( const FullscreenQuadPipelineDesc &desc );
        void CreateSampler( );
    };

} // namespace DenOfIz
