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

#include "DenOfIzExamples/UIExample.h"
#include <cfloat>
#include <cstdio>
#include <cstring>
#include <vector>
#include "DenOfIzGraphics/Input/InputSystem.h"

using namespace DenOfIz;

void UIExample::Init( )
{
    m_bgColor          = DenOfIz_ClayColor_Create( 245, 245, 250, 255 );
    m_cardColor        = DenOfIz_ClayColor_Create( 255, 255, 255, 255 );
    m_textColor        = DenOfIz_ClayColor_Create( 20, 20, 20, 255 );
    m_buttonColor      = DenOfIz_ClayColor_Create( 100, 149, 237, 255 );
    m_buttonHoverColor = DenOfIz_ClayColor_Create( 80, 129, 217, 255 );
    m_cursorColor      = DenOfIz_ClayColor_Create( 20, 20, 20, 255 );

    DenOfIz_ClayDesc clayDesc{ };
    clayDesc.LogicalDevice                  = m_logicalDevice;
    clayDesc.ResourceTracking               = m_resourceTracking;
    clayDesc.RenderTargetFormat             = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    clayDesc.NumFrames                      = 3;
    clayDesc.Width                          = m_pixelWidth;
    clayDesc.Height                         = m_pixelHeight;
    clayDesc.MaxNumElements                 = 8192;
    clayDesc.MaxNumTextMeasureCacheElements = 16384;
    clayDesc.MaxNumFonts                    = 16;

    m_clay = DenOfIz_Clay_Create( &clayDesc );
    DenOfIz_Clay_SetViewportSize( m_clay, static_cast<float>( m_pixelWidth ), static_cast<float>( m_pixelHeight ) );

    m_containerId      = DenOfIz_Clay_HashString( m_clay, DENOFIZ_STRING( "Container" ), 0, 0 );
    m_showCubeButtonId = DenOfIz_Clay_HashString( m_clay, DENOFIZ_STRING( "ShowCubeButton" ), 0, 0 );
    m_hideCubeButtonId = DenOfIz_Clay_HashString( m_clay, DENOFIZ_STRING( "HideCubeButton" ), 0, 0 );
    m_textFieldId      = DenOfIz_Clay_HashString( m_clay, DENOFIZ_STRING( "TextField" ), 0, 0 );

    m_cursorIndex = static_cast<uint32_t>( strlen( m_textBuffer ) );

    m_spinningCubeWidget = std::make_unique<Spinning3DCubeWidget>( DenOfIz_Clay_HashString( m_clay, DENOFIZ_STRING( "SpinningCubeWidget" ), 0, 0 ) );
    m_spinningCubeWidget->SetRotationSpeed( 1.0f );
    m_spinningCubeWidget->SetCubeColor( DirectX::XMFLOAT4( 0.2f, 0.6f, 1.0f, 1.0f ) );
    m_spinningCubeWidget->InitializeRenderResources( m_logicalDevice, m_graphicsQueue, 256, 256 );
}

void UIExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
    // defaultApiPreference.Windows = DENOFIZ_API_PREFERENCE_WINDOWS_WEBGPU_NATIVE;
    // defaultApiPreference.OSX     = DENOFIZ_API_PREFERENCE_OSX_WEBGPU_NATIVE;
}

void UIExample::Update( )
{
    m_worldData.DeltaTime = DenOfIz_StepTimer_GetDeltaTime( m_stepTimer );
    m_worldData.Camera->Update( m_worldData.DeltaTime );
    DenOfIz_Clay_UpdateScrollContainers( m_clay, false, DenOfIz_Float2{ 0, 0 }, m_worldData.DeltaTime );

    m_spinningCubeWidget->Update( m_worldData.DeltaTime );

    m_cursorBlinkTimer += m_worldData.DeltaTime;
    if ( m_cursorBlinkTimer > 1.0f )
    {
        m_cursorBlinkTimer = 0.0f;
    }

    uint32_t frameIndex = 0;
    DenOfIz_FrameSync_NextFrame( m_frameSync, &frameIndex );
    if ( m_spinningCubeWidget )
    {
        m_spinningCubeWidget->RenderToTexture( frameIndex );
    }
    DenOfIz_CommandList commandList = DENOFIZ_NULL_HANDLE;
    DenOfIz_FrameSync_GetCommandList( m_frameSync, frameIndex, &commandList );
    Render( frameIndex, commandList );

    std::vector<DenOfIz_Semaphore> semaphores;
    if ( DENOFIZ_HANDLE_IS_VALID( m_uiSemaphore ) )
    {
        semaphores.push_back( m_uiSemaphore );
    }
    if ( m_spinningCubeWidget )
    {
        semaphores.push_back( m_spinningCubeWidget->GetRenderSemaphore( ) );
    }
    DenOfIz_SemaphoreArray additionalSemaphores{ };
    additionalSemaphores.Elements    = semaphores.data( );
    additionalSemaphores.NumElements = static_cast<uint32_t>( semaphores.size( ) );
    DenOfIz_FrameSync_ExecuteCommandList( m_frameSync, frameIndex, &additionalSemaphores );
    Present( frameIndex );
}

void UIExample::CreateUI( const uint32_t frameIndex )
{
    DenOfIz_ClayElementDeclaration containerDecl = DenOfIz_ClayElementDeclaration_Default( );
    containerDecl.Id                             = m_containerId;
    containerDecl.Layout.LayoutDirection         = DENOFIZ_CLAY_LAYOUT_DIRECTION_TOP_TO_BOTTOM;
    containerDecl.Layout.Padding                 = DenOfIz_ClayPadding_CreateUniform( 24 );
    containerDecl.Layout.ChildGap                = 24;
    containerDecl.Layout.ChildAlignment.X        = DENOFIZ_CLAY_ALIGNMENT_X_CENTER;
    containerDecl.Layout.ChildAlignment.Y        = DENOFIZ_CLAY_ALIGNMENT_Y_TOP;
    containerDecl.BackgroundColor                = m_bgColor;

    DenOfIz_Clay_OpenElement( m_clay, &containerDecl );
    {
        DenOfIz_ClayTextDesc titleText = DenOfIz_ClayTextDesc_Default( );
        titleText.TextColor            = m_textColor;
        titleText.FontSize             = 28;
        titleText.TextAlignment        = DENOFIZ_CLAY_TEXT_ALIGNMENT_CENTER;
        DenOfIz_Clay_Text( m_clay, DENOFIZ_STRING( "UI Example" ), &titleText );

        CreateTextField( );

        DenOfIz_ClayElementDeclaration buttonRowDecl = DenOfIz_ClayElementDeclaration_Default( );
        buttonRowDecl.Layout.LayoutDirection         = DENOFIZ_CLAY_LAYOUT_DIRECTION_LEFT_TO_RIGHT;
        buttonRowDecl.Layout.ChildGap                = 16;
        buttonRowDecl.Layout.ChildAlignment.X        = DENOFIZ_CLAY_ALIGNMENT_X_CENTER;
        buttonRowDecl.Layout.ChildAlignment.Y        = DENOFIZ_CLAY_ALIGNMENT_Y_CENTER;
        buttonRowDecl.Layout.Sizing.Width            = DenOfIz_ClaySizingAxis_Fit( 0, FLT_MAX );
        buttonRowDecl.Layout.Sizing.Height           = DenOfIz_ClaySizingAxis_Fixed( 50 );

        DenOfIz_Clay_OpenElement( m_clay, &buttonRowDecl );
        {
            CreateButton( "Show Cube", m_showCubeButtonId );
            CreateButton( "Hide Cube", m_hideCubeButtonId );

            if ( DenOfIz_Clay_ElementClicked( m_clay, m_showCubeButtonId ) )
            {
                m_showCubeWindow = true;
            }
            if ( DenOfIz_Clay_ElementClicked( m_clay, m_hideCubeButtonId ) )
            {
                m_showCubeWindow = false;
            }
        }
        DenOfIz_Clay_CloseElement( m_clay );

        if ( m_showCubeWindow )
        {
            DenOfIz_ClayElementDeclaration cardDecl = DenOfIz_ClayElementDeclaration_Default( );
            cardDecl.Layout.LayoutDirection         = DENOFIZ_CLAY_LAYOUT_DIRECTION_TOP_TO_BOTTOM;
            cardDecl.Layout.Padding                 = DenOfIz_ClayPadding_CreateUniform( 20 );
            cardDecl.Layout.ChildGap                = 12;
            cardDecl.Layout.ChildAlignment.X        = DENOFIZ_CLAY_ALIGNMENT_X_CENTER;
            cardDecl.Layout.Sizing.Width            = DenOfIz_ClaySizingAxis_Fixed( 300 );
            cardDecl.Layout.Sizing.Height           = DenOfIz_ClaySizingAxis_Fit( 0, FLT_MAX );
            cardDecl.BackgroundColor                = m_cardColor;
            cardDecl.BorderRadius                   = DenOfIz_ClayBorderRadius_CreateUniform( 12 );
            cardDecl.Border.Color                   = DenOfIz_ClayColor_Create( 200, 200, 200, 100 );
            cardDecl.Border.Width                   = DenOfIz_ClayBorderWidth_CreateUniform( 1 );

            DenOfIz_Clay_OpenElement( m_clay, &cardDecl );
            {
                DenOfIz_ClayTextDesc cardTitle = DenOfIz_ClayTextDesc_Default( );
                cardTitle.TextColor            = m_textColor;
                cardTitle.FontSize             = 18;
                cardTitle.TextAlignment        = DENOFIZ_CLAY_TEXT_ALIGNMENT_CENTER;
                DenOfIz_Clay_Text( m_clay, DENOFIZ_STRING( "3D Spinning Cube" ), &cardTitle );

                DenOfIz_Clay_Texture( m_clay, m_spinningCubeWidget->GetRenderTarget( frameIndex ), 256, 256 );
            }
            DenOfIz_Clay_CloseElement( m_clay );
        }

        DenOfIz_ClayTextDesc mousePosition = DenOfIz_ClayTextDesc_Default( );
        mousePosition.TextColor            = DenOfIz_ClayColor_Create( 128, 128, 128, 255 );
        mousePosition.FontSize             = 12;
        mousePosition.TextAlignment        = DENOFIZ_CLAY_TEXT_ALIGNMENT_CENTER;
        DenOfIz_MouseCoords mouseCoords    = DenOfIz_InputSystem_GetMouseState( );
        std::string         mousePos       = "Mouse Coordinates: (" + std::to_string( mouseCoords.X ) + ", " + std::to_string( mouseCoords.Y ) + ")";
        DenOfIz_Clay_Text( m_clay, DenOfIz_StringView{ mousePos.c_str( ), mousePos.length( ) }, &mousePosition );

        DenOfIz_ClayTextDesc footerText = DenOfIz_ClayTextDesc_Default( );
        footerText.TextColor            = DenOfIz_ClayColor_Create( 128, 128, 128, 255 );
        footerText.FontSize             = 12;
        footerText.TextAlignment        = DENOFIZ_CLAY_TEXT_ALIGNMENT_CENTER;
        DenOfIz_Clay_Text( m_clay, DENOFIZ_STRING( "Press F11 to toggle debug mode" ), &footerText );
    }
    DenOfIz_Clay_CloseElement( m_clay );
}

void UIExample::CreateButton( const char *text, const uint32_t buttonId )
{
    const bool isHovered = DenOfIz_Clay_PointerOver( m_clay, buttonId );
    const bool isPressed = DenOfIz_Clay_ElementPressed( m_clay, buttonId );

    DenOfIz_ClayColor bgColor = m_buttonColor;
    if ( isPressed )
    {
        bgColor = DenOfIz_ClayColor_Create( 60, 109, 197, 255 );
    }
    else if ( isHovered )
    {
        bgColor = m_buttonHoverColor;
    }

    DenOfIz_ClayElementDeclaration buttonDecl = DenOfIz_ClayElementDeclaration_Default( );
    buttonDecl.Id                             = buttonId;
    buttonDecl.Layout.Sizing.Width            = DenOfIz_ClaySizingAxis_Fixed( 120 );
    buttonDecl.Layout.Sizing.Height           = DenOfIz_ClaySizingAxis_Fixed( 40 );
    buttonDecl.Layout.ChildAlignment.X        = DENOFIZ_CLAY_ALIGNMENT_X_CENTER;
    buttonDecl.Layout.ChildAlignment.Y        = DENOFIZ_CLAY_ALIGNMENT_Y_CENTER;
    buttonDecl.BackgroundColor                = bgColor;
    buttonDecl.BorderRadius                   = DenOfIz_ClayBorderRadius_CreateUniform( 8 );

    DenOfIz_Clay_OpenElement( m_clay, &buttonDecl );
    {
        DenOfIz_ClayTextDesc buttonText = DenOfIz_ClayTextDesc_Default( );
        buttonText.TextColor            = DenOfIz_ClayColor_Create( 255, 255, 255, 255 );
        buttonText.FontSize             = 14;
        DenOfIz_Clay_Text( m_clay, DenOfIz_StringView( text, static_cast<uint32_t>( strlen( text ) ) ), &buttonText );
    }
    DenOfIz_Clay_CloseElement( m_clay );
}

void UIExample::CreateTextField( )
{
    const uint16_t fontSize     = 16;
    const uint16_t fontId       = 0;
    const float    fieldPadding = 12.0f;

    if ( DenOfIz_Clay_ElementClicked( m_clay, m_textFieldId ) )
    {
        if ( !m_textFieldFocused )
        {
            DenOfIz_InputSystem_StartTextInput( );
        }
        m_textFieldFocused = true;

        DenOfIz_ClayBoundingBox fieldBox{ };
        DenOfIz_Clay_GetElementBoundingBox( m_clay, m_textFieldId, &fieldBox );

        const float        scaledPadding = DenOfIz_Clay_PointsToPixels( m_clay, fieldPadding );
        const float        clickX        = m_mousePosition.X - fieldBox.X - scaledPadding;
        DenOfIz_StringView textView      = DenOfIz_StringView( m_textBuffer, static_cast<uint32_t>( strlen( m_textBuffer ) ) );
        m_cursorIndex                    = DenOfIz_Clay_GetCharIndexAtOffset( m_clay, textView, clickX, fontId, fontSize );
    }

    DenOfIz_ClayColor borderColor = m_textFieldFocused ? DenOfIz_ClayColor_Create( 100, 149, 237, 255 ) : DenOfIz_ClayColor_Create( 200, 200, 200, 255 );

    DenOfIz_ClayElementDeclaration fieldDecl = DenOfIz_ClayElementDeclaration_Default( );
    fieldDecl.Id                             = m_textFieldId;
    fieldDecl.Layout.Sizing.Width            = DenOfIz_ClaySizingAxis_Fixed( 300 );
    fieldDecl.Layout.Sizing.Height           = DenOfIz_ClaySizingAxis_Fixed( 40 );
    fieldDecl.Layout.Padding                 = DenOfIz_ClayPadding_Create( fieldPadding, fieldPadding, 0, 0 );
    fieldDecl.Layout.ChildAlignment.Y        = DENOFIZ_CLAY_ALIGNMENT_Y_CENTER;
    fieldDecl.BackgroundColor                = m_cardColor;
    fieldDecl.BorderRadius                   = DenOfIz_ClayBorderRadius_CreateUniform( 6 );
    fieldDecl.Border.Color                   = borderColor;
    fieldDecl.Border.Width                   = DenOfIz_ClayBorderWidth_CreateUniform( 2 );

    DenOfIz_Clay_OpenElement( m_clay, &fieldDecl );
    {
        DenOfIz_StringView textView = DenOfIz_StringView( m_textBuffer, static_cast<uint32_t>( strlen( m_textBuffer ) ) );

        DenOfIz_ClayTextDesc textDesc = DenOfIz_ClayTextDesc_Default( );
        textDesc.TextColor            = m_textColor;
        textDesc.FontSize             = fontSize;
        textDesc.FontId               = fontId;
        DenOfIz_Clay_Text( m_clay, textView, &textDesc );

        if ( m_textFieldFocused && m_cursorBlinkTimer < 0.5f )
        {
            const float cursorX = DenOfIz_Clay_GetCursorOffsetAtIndex( m_clay, textView, m_cursorIndex, fontId, fontSize );

            DenOfIz_ClayElementDeclaration cursorDecl = DenOfIz_ClayElementDeclaration_Default( );
            cursorDecl.Layout.Sizing.Width            = DenOfIz_ClaySizingAxis_Fixed( 2 );
            cursorDecl.Layout.Sizing.Height           = DenOfIz_ClaySizingAxis_Fixed( static_cast<float>( fontSize + 4 ) );
            cursorDecl.BackgroundColor                = m_cursorColor;
            cursorDecl.Floating.AttachTo              = DENOFIZ_CLAY_FLOATING_ATTACH_TO_PARENT;
            cursorDecl.Floating.ElementAttachPoint    = DENOFIZ_CLAY_FLOATING_ATTACH_POINT_LEFT_CENTER;
            cursorDecl.Floating.ParentAttachPoint     = DENOFIZ_CLAY_FLOATING_ATTACH_POINT_LEFT_CENTER;
            cursorDecl.Floating.Offset.X              = fieldPadding + DenOfIz_Clay_PixelsToPoints( m_clay, cursorX );
            cursorDecl.Floating.Offset.Y              = 0;

            DenOfIz_Clay_OpenElement( m_clay, &cursorDecl );
            DenOfIz_Clay_CloseElement( m_clay );
        }
    }
    DenOfIz_Clay_CloseElement( m_clay );
}

void UIExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    DenOfIz_Clay_BeginLayout( m_clay );
    CreateUI( frameIndex );

    DenOfIz_ClayRenderResult uiResult{ };
    DenOfIz_Clay_EndAndRenderLayout( m_clay, frameIndex, m_worldData.DeltaTime, &uiResult );
    m_uiSemaphore = uiResult.Semaphore;

    DenOfIz_CommandList_Begin( commandList );

    uint32_t imageIndex = 0;
    DenOfIz_FrameSync_AcquireNextImage( m_frameSync, &imageIndex );
    DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
    DenOfIz_SwapChain_GetRenderTarget( m_swapChain, imageIndex, &renderTarget );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, uiResult.Texture, DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CopyTextureRegionDesc copyDesc{ };
    copyDesc.SrcTexture = uiResult.Texture;
    copyDesc.DstTexture = renderTarget;
    copyDesc.SrcX       = 0;
    copyDesc.SrcY       = 0;
    copyDesc.SrcZ       = 0;
    copyDesc.DstX       = 0;
    copyDesc.DstY       = 0;
    copyDesc.DstZ       = 0;
    copyDesc.Width      = m_pixelWidth;
    copyDesc.Height     = m_pixelHeight;
    copyDesc.Depth      = 1;
    DenOfIz_CommandList_CopyTextureRegion( commandList, &copyDesc );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( commandList );
}

void UIExample::HandleEvent( DenOfIz_Event &event )
{
    if ( event.Type == DENOFIZ_EVENT_TYPE_MOUSE_MOTION )
    {
        m_mousePosition.X = static_cast<float>( event.MouseMotion.X );
        m_mousePosition.Y = static_cast<float>( event.MouseMotion.Y );
    }
    else if ( event.Type == DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_DOWN && event.MouseButton.Button == DENOFIZ_MOUSE_BUTTON_LEFT )
    {
        m_mousePosition.X = static_cast<float>( event.MouseButton.X );
        m_mousePosition.Y = static_cast<float>( event.MouseButton.Y );

        if ( !DenOfIz_Clay_PointerOver( m_clay, m_textFieldId ) )
        {
            if ( m_textFieldFocused )
            {
                DenOfIz_InputSystem_StopTextInput( );
            }
            m_textFieldFocused = false;
        }
    }

    if ( m_textFieldFocused && event.Type == DENOFIZ_EVENT_TYPE_KEY_DOWN )
    {
        const size_t textLen = strlen( m_textBuffer );

        if ( event.Key.KeyCode == DENOFIZ_KEY_CODE_BACKSPACE && m_cursorIndex > 0 )
        {
            memmove( &m_textBuffer[ m_cursorIndex - 1 ], &m_textBuffer[ m_cursorIndex ], textLen - m_cursorIndex + 1 );
            m_cursorIndex--;
            m_cursorBlinkTimer = 0.0f;
        }
        else if ( event.Key.KeyCode == DENOFIZ_KEY_CODE_DELETE && m_cursorIndex < textLen )
        {
            memmove( &m_textBuffer[ m_cursorIndex ], &m_textBuffer[ m_cursorIndex + 1 ], textLen - m_cursorIndex );
            m_cursorBlinkTimer = 0.0f;
        }
        else if ( event.Key.KeyCode == DENOFIZ_KEY_CODE_LEFT && m_cursorIndex > 0 )
        {
            m_cursorIndex--;
            m_cursorBlinkTimer = 0.0f;
        }
        else if ( event.Key.KeyCode == DENOFIZ_KEY_CODE_RIGHT && m_cursorIndex < textLen )
        {
            m_cursorIndex++;
            m_cursorBlinkTimer = 0.0f;
        }
        else if ( event.Key.KeyCode == DENOFIZ_KEY_CODE_HOME )
        {
            m_cursorIndex      = 0;
            m_cursorBlinkTimer = 0.0f;
        }
        else if ( event.Key.KeyCode == DENOFIZ_KEY_CODE_END )
        {
            m_cursorIndex      = static_cast<uint32_t>( textLen );
            m_cursorBlinkTimer = 0.0f;
        }
        else if ( event.Key.KeyCode == DENOFIZ_KEY_CODE_ESCAPE )
        {
            DenOfIz_InputSystem_StopTextInput( );
            m_textFieldFocused = false;
        }
    }

    if ( m_textFieldFocused && event.Type == DENOFIZ_EVENT_TYPE_TEXT_INPUT )
    {
        const size_t textLen  = strlen( m_textBuffer );
        const size_t inputLen = event.Text.Text.NumChars;

        if ( textLen + inputLen < sizeof( m_textBuffer ) - 1 )
        {
            memmove( &m_textBuffer[ m_cursorIndex + inputLen ], &m_textBuffer[ m_cursorIndex ], textLen - m_cursorIndex + 1 );
            memcpy( &m_textBuffer[ m_cursorIndex ], event.Text.Text.Chars, inputLen );
            m_cursorIndex += static_cast<uint32_t>( inputLen );
            m_cursorBlinkTimer = 0.0f;
        }
    }

    if ( event.Type == DENOFIZ_EVENT_TYPE_WINDOW_EVENT && event.Window.Event == DENOFIZ_WINDOW_EVENT_TYPE_SIZE_CHANGED )
    {
        DenOfIz_Clay_SetViewportSize( m_clay, static_cast<float>( event.Window.Data1 ), static_cast<float>( event.Window.Data2 ) );
    }

    DenOfIz_Clay_HandleEvent( m_clay, &event );
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void UIExample::Quit( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );
    m_spinningCubeWidget.reset( );
    DenOfIz_Clay_Destroy( m_clay );
    IExample::Quit( );
}
