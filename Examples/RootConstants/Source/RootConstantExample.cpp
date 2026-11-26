/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/
#include "DenOfIzExamples/RootConstantExample.h"

using namespace DenOfIz;

void RootConstantExample::Init( )
{
    m_quadPipeline = std::make_unique<QuadPipeline>( m_graphicsApi, m_logicalDevice, "Assets/Shaders/PushConstantColor.ps.hlsl" );
}

void RootConstantExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    DenOfIz_CommandList_Begin( commandList );

    DenOfIz_Texture renderTarget = m_frame.BackBuffer;

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_RenderingAttachmentDesc quadAttachmentDesc{ };
    quadAttachmentDesc.Resource = renderTarget;

    DenOfIz_RenderingDesc quadRenderingDesc{ };
    quadRenderingDesc.RTAttachments.Elements    = &quadAttachmentDesc;
    quadRenderingDesc.RTAttachments.NumElements = 1;
    DenOfIz_CommandList_BeginRendering( commandList, &quadRenderingDesc );

    const DenOfIz_Viewport *viewport = nullptr;
    DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );
    DenOfIz_CommandList_BindViewport( commandList, viewport->X, viewport->Y, viewport->Width, viewport->Height );
    DenOfIz_CommandList_BindScissorRect( commandList, viewport->X, viewport->Y, viewport->Width, viewport->Height );
    DenOfIz_CommandList_BindPipeline( commandList, m_quadPipeline->Pipeline( ) );

    DenOfIz_ByteArray colorData;
    colorData.Elements    = reinterpret_cast<Byte *>( m_color.data( ) );
    colorData.NumElements = m_color.size( ) * sizeof( float );
    DenOfIz_CommandList_SetRootConstants( commandList, 0, &colorData );

    DenOfIz_CommandList_Draw( commandList, 3, 1, 0, 0 );
    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( commandList );
}

void RootConstantExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
}

void RootConstantExample::Update( )
{
    const auto deltaTime = static_cast<float>( DenOfIz_StepTimer_GetDeltaTime( m_stepTimer ) );
    m_color[ m_rgbIterator ] += deltaTime;
    if ( m_color[ m_rgbIterator ] > 1.0f )
    {
        m_color[ m_rgbIterator ] = 0.0f;
        m_rgbIterator            = ( m_rgbIterator + 1 ) % 3;
    }
    m_worldData.DeltaTime = deltaTime;
    m_worldData.Camera->Update( m_worldData.DeltaTime );
    RenderAndPresentFrame( );
}

void RootConstantExample::HandleEvent( DenOfIz_Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

RootConstantExample::~RootConstantExample( )
{
}

void RootConstantExample::Quit( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );
    IExample::Quit( );
}
