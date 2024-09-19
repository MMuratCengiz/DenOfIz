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
#include <DenOfIzExamples/RenderTargetExample.h>

using namespace DenOfIz;

void RenderTargetExample::Init( )
{

    SimplifiedPipelineDesc quadPipelineDesc("Assets/Shaders/FullScreenQuad.vs.hlsl", "Assets/Shaders/FullScreenQuad.ps.hlsl");
    m_quadPipeline = std::make_unique<SimplifiedPipeline>( m_graphicsApi, m_logicalDevice, quadPipelineDesc );
    PipelineDesc pipelineDesc{ };
    TextureDesc  textureDesc{ };
    textureDesc.Width        = m_windowDesc.Width;
    textureDesc.Height       = m_windowDesc.Height;
    textureDesc.Format       = Format::R32G32B32A32Float;
    textureDesc.Descriptor   = ResourceDescriptor::RWTexture;
    textureDesc.InitialState = ResourceState::RenderTarget;
    m_deferredRenderTarget   = m_logicalDevice->CreateTextureResource( textureDesc );
}

void RenderTargetExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
}
void RenderTargetExample::Tick( )
{
}
void RenderTargetExample::Quit( )
{
}
