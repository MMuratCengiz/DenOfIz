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
#include <DenOfIzExamples/RootConstantExample.h>
#include <DenOfIzGraphics/Utilities/Time.h>

using namespace DenOfIz;

void RootConstantExample::Init( )
{
    m_quadPipeline      = std::make_unique<QuadPipeline>( m_graphicsApi, m_logicalDevice, "Assets/Shaders/PushConstantColor.ps.hlsl" );
    m_resourceBindGroup = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( RootConstantBindGroupDesc( m_quadPipeline->RootSignature( ) ) ) );

    RenderGraphDesc renderGraphDesc{ };
    renderGraphDesc.GraphicsApi   = m_graphicsApi;
    renderGraphDesc.LogicalDevice = m_logicalDevice;
    renderGraphDesc.SwapChain     = m_swapChain.get( );

    m_renderGraph = std::make_unique<RenderGraph>( renderGraphDesc );

    PresentNodeDesc presentNode{ };
    presentNode.SwapChain = m_swapChain.get( );
    presentNode.Execute   = this;

    m_renderGraph->SetPresentNode( presentNode );
    m_renderGraph->BuildGraph( );

    m_time.OnEachSecond = []( const double fps ) { LOG( WARNING ) << "FPS: " << fps; };
}

void RootConstantExample::Execute( uint32_t frameIndex, ICommandList *commandList, ITextureResource *texture )
{
    RenderingAttachmentDesc quadAttachmentDesc{ };
    quadAttachmentDesc.Resource = texture;

    RenderingDesc quadRenderingDesc{ };
    quadRenderingDesc.RTAttachments.AddElement( quadAttachmentDesc );

    commandList->BeginRendering( quadRenderingDesc );
    const Viewport &viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindPipeline( m_quadPipeline->Pipeline( ) );
    commandList->BindResourceGroup( m_resourceBindGroup.get( ) );
    commandList->Draw( 3, 1, 0, 0 );
    commandList->EndRendering( );
}

void RootConstantExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    defaultApiPreference.Windows = APIPreferenceWindows::DirectX12;
}

void RootConstantExample::Update( )
{
    m_time.Tick( );
    m_color[ m_rgbIterator ] += m_time.GetDeltaTime( );
    if ( m_color[ m_rgbIterator ] > 1.0f )
    {
        m_color[ m_rgbIterator ] = 0.0f;
        m_rgbIterator            = ( m_rgbIterator + 1 ) % 3;
    }
    m_resourceBindGroup->SetRootConstants( 0, m_color.data( ) );
    m_worldData.DeltaTime = m_time.GetDeltaTime( );
    m_worldData.Camera->Update( m_worldData.DeltaTime );
    m_renderGraph->Update( );
}

void RootConstantExample::HandleEvent( SDL_Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void RootConstantExample::Quit( )
{
    m_renderGraph->WaitIdle( );
    IExample::Quit( );
}
