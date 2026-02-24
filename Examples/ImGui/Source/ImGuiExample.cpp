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

#include "DenOfIzExamples/ImGuiExample.h"
#include <imgui.h>
#include "../../../ImGui/ImGuiBackend.h"

using namespace DenOfIz;

void ImGuiExample::Init( )
{
    const DenOfIz_Viewport *viewport = nullptr;
    DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );

    ImGuiBackendDesc backendDesc{ };
    backendDesc.LogicalDevice      = m_logicalDevice;
    backendDesc.RenderTargetFormat = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    backendDesc.NumFrames          = 3;
    backendDesc.Viewport           = *viewport;
    m_imguiContext                 = ImGuiRenderer_Create( backendDesc );
}

void ImGuiExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
    defaultApiPreference.Windows = DENOFIZ_API_PREFERENCE_WINDOWS_WEBGPU_NATIVE;
    // defaultApiPreference.OSX = DENOFIZ_API_PREFERENCE_OSX_WEBGPU_NATIVE;
}

void ImGuiExample::Update( )
{
    m_worldData.DeltaTime = static_cast<float>( DenOfIz_StepTimer_GetDeltaTime( m_stepTimer ) );

    const DenOfIz_Viewport *viewport = nullptr;
    DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );

    ImGuiRenderer_NewFrame( m_imguiContext, static_cast<uint32_t>( viewport->Width ), static_cast<uint32_t>( viewport->Height ), m_worldData.DeltaTime );

    if ( m_showDemoWindow )
    {
        ImGui::ShowDemoWindow( &m_showDemoWindow );
    }

    {
        static float f       = 0.0f;
        static int   counter = 0;

        ImGui::Begin( "Hello, world!" );

        ImGui::Text( "This is some useful text." );
        ImGui::Checkbox( "Demo Window", &m_showDemoWindow );
        ImGui::Checkbox( "Another Window", &m_showAnotherWindow );

        ImGui::SliderFloat( "float", &f, 0.0f, 1.0f );
        ImGui::ColorEdit3( "clear color", reinterpret_cast<float *>( &m_clearColor ) );

        if ( ImGui::Button( "Button" ) )
        {
            counter++;
        }
        ImGui::SameLine( );
        ImGui::Text( "counter = %d", counter );

        ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO( ).Framerate, ImGui::GetIO( ).Framerate );
        ImGui::End( );
    }

    if ( m_showAnotherWindow )
    {
        ImGui::Begin( "Another Window", &m_showAnotherWindow );
        ImGui::Text( "Hello from another window!" );
        if ( ImGui::Button( "Close Me" ) )
        {
            m_showAnotherWindow = false;
        }
        ImGui::End( );
    }
    ImGui::Render( );
    RenderAndPresentFrame( );
}

void ImGuiExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    DenOfIz_CommandList_Begin( commandList );

    uint32_t imageIndex = 0;
    DenOfIz_FrameSync_AcquireNextImage( m_frameSync, &imageIndex );

    DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
    DenOfIz_SwapChain_GetRenderTarget( m_swapChain, imageIndex, &renderTarget );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    ImGuiRenderer_Render( m_imguiContext, renderTarget, commandList, frameIndex );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    DenOfIz_CommandList_End( commandList );
}

void ImGuiExample::HandleEvent( DenOfIz_Event &event )
{
    ImGuiRenderer_ProcessEvent( m_imguiContext, event );
    IExample::HandleEvent( event );
}

void ImGuiExample::Quit( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );
    ImGuiRenderer_Destroy( m_imguiContext );
    m_imguiContext = nullptr;
    IExample::Quit( );
}
