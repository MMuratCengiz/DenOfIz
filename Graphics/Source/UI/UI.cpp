/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphics/UI/UI.h"

#include <memory>
#include <utility>
#include <vector>
#include "DenOfIzGraphics/Input/Display.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/UI/UIContext.h"
#include "DenOfIzGraphicsInternal/UI/UIRenderer.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

namespace DenOfIz
{
    class UI
    {
        DenOfIz_Window                                          m_window            = DENOFIZ_NULL_HANDLE;
        uint32_t                                                m_windowId          = 0;
        bool                                                    m_followWindowScale = true;
        bool                                                    m_commandsPending   = false;
        std::vector<std::pair<uint16_t, Font *>>                m_pendingFonts;
        std::vector<std::pair<uint16_t, std::vector<uint16_t>>> m_pendingFallbacks;

    public:
        UIContext                   Context;
        std::unique_ptr<UIRenderer> Renderer;

        explicit UI( const DenOfIz_UIDesc &desc )
        {
            UIRendererDesc rendererDesc{ };
            rendererDesc.LogicalDevice      = desc.LogicalDevice;
            rendererDesc.ResourceTracking   = desc.ResourceTracking;
            rendererDesc.RenderTargetFormat = desc.RenderTargetFormat;
            rendererDesc.DepthFormat        = desc.DepthFormat;
            rendererDesc.NumFrames          = desc.NumFrames > 0 ? desc.NumFrames : 3;
            rendererDesc.Text               = &Context.Text( );
            Renderer                        = std::make_unique<UIRenderer>( rendererDesc );

            m_window   = desc.Window;
            m_windowId = DENOFIZ_HANDLE_IS_VALID( desc.Window ) ? DenOfIz_Window_GetWindowID( desc.Window ) : 0;
            uint32_t width  = desc.Width;
            uint32_t height = desc.Height;
            if ( ( width == 0 || height == 0 ) && DENOFIZ_HANDLE_IS_VALID( desc.Window ) )
            {
                const DenOfIz_DisplaySize size = DenOfIz_Window_GetSizeInPixels( desc.Window );
                width                          = static_cast<uint32_t>( size.Width );
                height                         = static_cast<uint32_t>( size.Height );
            }
            Context.SetViewportPixels( static_cast<float>( std::max( 1u, width ) ), static_cast<float>( std::max( 1u, height ) ) );
            FollowWindowScale( );
        }

        void FollowWindowScale( )
        {
            if ( !m_followWindowScale )
            {
                return;
            }
            const float scale = DENOFIZ_HANDLE_IS_VALID( m_window ) ? DenOfIz_Window_GetDisplayScale( m_window ) : DenOfIz_Display_GetPrimaryDisplay( ).DisplayScale;
            Context.SetDisplayScale( scale > 0.0f ? scale : 1.0f );
        }

        void SetDisplayScale( const float displayScale )
        {
            m_followWindowScale = displayScale <= 0.0f;
            if ( m_followWindowScale )
            {
                FollowWindowScale( );
                return;
            }
            Context.SetDisplayScale( displayScale );
        }

        void HandleEvent( const DenOfIz_Event &event )
        {
            switch ( event.Type )
            {
            case DENOFIZ_EVENT_TYPE_MOUSE_MOTION:
                if ( OwnsWindow( event.MouseMotion.Common.WindowID ) )
                {
                    Context.HandlePointerMotion( event.MouseMotion.X, event.MouseMotion.Y );
                }
                break;
            case DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_DOWN:
            case DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_UP:
                if ( OwnsWindow( event.MouseButton.Common.WindowID ) )
                {
                    Context.HandlePointerButton( event.Type == DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_DOWN, event.MouseButton.Button, event.MouseButton.X, event.MouseButton.Y,
                                                 event.MouseButton.Clicks );
                }
                break;
            case DENOFIZ_EVENT_TYPE_MOUSE_WHEEL:
                if ( OwnsWindow( event.MouseWheel.Common.WindowID ) )
                {
                    Context.HandleWheel( event.MouseWheel.X, event.MouseWheel.Y );
                }
                break;
            case DENOFIZ_EVENT_TYPE_WINDOW_EVENT:
                if ( !OwnsWindow( event.Window.Common.WindowID ) )
                {
                    break;
                }
                switch ( event.Window.Event )
                {
                case DENOFIZ_WINDOW_EVENT_TYPE_PIXEL_SIZE_CHANGED:
                    Context.SetViewportPixels( static_cast<float>( event.Window.Data1 ), static_cast<float>( event.Window.Data2 ) );
                    FollowWindowScale( );
                    break;
                case DENOFIZ_WINDOW_EVENT_TYPE_DISPLAY_SCALE_CHANGED:
                    FollowWindowScale( );
                    break;
                case DENOFIZ_WINDOW_EVENT_TYPE_LEAVE:
                    Context.HandlePointerLeave( );
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        void QueueFont( const uint16_t fontId, Font *font )
        {
            if ( m_commandsPending )
            {
                m_pendingFonts.emplace_back( fontId, font );
                return;
            }
            ApplyFont( fontId, font );
        }

        void QueueFallbacks( const uint16_t fontId, std::vector<uint16_t> fallbacks )
        {
            if ( m_commandsPending )
            {
                m_pendingFallbacks.emplace_back( fontId, std::move( fallbacks ) );
                return;
            }
            ApplyFallbacks( fontId, fallbacks );
        }

        void BeginFrame( const float deltaTime )
        {
            for ( const auto &[ fontId, font ] : m_pendingFonts )
            {
                ApplyFont( fontId, font );
            }
            for ( const auto &[ fontId, fallbacks ] : m_pendingFallbacks )
            {
                ApplyFallbacks( fontId, fallbacks );
            }
            m_pendingFonts.clear( );
            m_pendingFallbacks.clear( );
            Context.BeginFrame( deltaTime );
        }

        void EndFrame( )
        {
            Context.EndFrame( );
            m_commandsPending = true;
        }

        void Render( const uint32_t frameIndex, const DenOfIz_CommandList commandList )
        {
            const DenOfIz_Float2 pixels = Context.GetViewportPixels( );
            Renderer->Render( Context.Commands( ), frameIndex, commandList, pixels.X, pixels.Y, Context.GetPixelsPerPoint( ) );
            m_commandsPending = false;
        }

        void RenderToTexture( const uint32_t frameIndex, const DenOfIz_SemaphoreArray *waitSemaphores, DenOfIz_UIRenderResult *outResult )
        {
            const DenOfIz_Float2 pixels = Context.GetViewportPixels( );
            Renderer->RenderToTexture( Context.Commands( ), frameIndex, static_cast<uint32_t>( pixels.X ), static_cast<uint32_t>( pixels.Y ), Context.GetPixelsPerPoint( ),
                                       waitSemaphores, outResult );
            m_commandsPending = false;
        }

        bool Capture( const uint32_t frameIndex, const DenOfIz_SemaphoreArray *waitSemaphores, DenOfIz_ByteArray *outPixels )
        {
            const DenOfIz_Float2 pixels = Context.GetViewportPixels( );
            const bool captured = Renderer->Capture( Context.Commands( ), frameIndex, static_cast<uint32_t>( pixels.X ), static_cast<uint32_t>( pixels.Y ),
                                                     Context.GetPixelsPerPoint( ), waitSemaphores, outPixels );
            m_commandsPending = false;
            return captured;
        }

    private:
        bool OwnsWindow( const uint32_t windowId ) const
        {
            return m_windowId == 0 || windowId == 0 || windowId == m_windowId;
        }

        void ApplyFont( const uint16_t fontId, Font *font )
        {
            if ( font )
            {
                Context.Text( ).AddFont( fontId, font );
            }
            else
            {
                Context.Text( ).RemoveFont( fontId );
            }
            Context.FontsChanged( );
        }

        void ApplyFallbacks( const uint16_t fontId, const std::vector<uint16_t> &fallbacks )
        {
            Context.Text( ).SetFallbacks( fontId, fallbacks );
            Context.FontsChanged( );
        }
    };
} // namespace DenOfIz

#define UI_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::UI, handle )

extern "C"
{
    DenOfIz_UIColor DenOfIz_UIColor_Rgba( const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a )
    {
        return DenOfIz_UIColor{ r, g, b, a };
    }

    DenOfIz_UIColor DenOfIz_UIColor_Hex( const uint32_t rgba )
    {
        return DenOfIz_UIColor{ static_cast<uint8_t>( rgba >> 24 ), static_cast<uint8_t>( rgba >> 16 ), static_cast<uint8_t>( rgba >> 8 ), static_cast<uint8_t>( rgba ) };
    }

    DenOfIz_UIValue DenOfIz_UIValue_Auto( )
    {
        return DenOfIz_UIValue{ DENOFIZ_UI_VALUE_TYPE_AUTO, 0.0f };
    }

    DenOfIz_UIValue DenOfIz_UIValue_Points( const float points )
    {
        return DenOfIz_UIValue{ DENOFIZ_UI_VALUE_TYPE_POINTS, points };
    }

    DenOfIz_UIValue DenOfIz_UIValue_Percent( const float fraction )
    {
        return DenOfIz_UIValue{ DENOFIZ_UI_VALUE_TYPE_PERCENT, fraction };
    }

    DenOfIz_UILength DenOfIz_UILength_Points( const float points )
    {
        return DenOfIz_UILength{ DENOFIZ_UI_LENGTH_TYPE_POINTS, points };
    }

    DenOfIz_UILength DenOfIz_UILength_Percent( const float fraction )
    {
        return DenOfIz_UILength{ DENOFIZ_UI_LENGTH_TYPE_PERCENT, fraction };
    }

    DenOfIz_UILength DenOfIz_UILength_Auto( )
    {
        return DenOfIz_UILength{ DENOFIZ_UI_LENGTH_TYPE_AUTO, 0.0f };
    }

    DenOfIz_UILengthEdges DenOfIz_UILengthEdges_All( const float points )
    {
        const DenOfIz_UILength length{ DENOFIZ_UI_LENGTH_TYPE_POINTS, points };
        return DenOfIz_UILengthEdges{ length, length, length, length };
    }

    DenOfIz_UILengthEdges DenOfIz_UILengthEdges_Symmetric( const float horizontal, const float vertical )
    {
        const DenOfIz_UILength h{ DENOFIZ_UI_LENGTH_TYPE_POINTS, horizontal };
        const DenOfIz_UILength v{ DENOFIZ_UI_LENGTH_TYPE_POINTS, vertical };
        return DenOfIz_UILengthEdges{ h, v, h, v };
    }

    DenOfIz_UIEdges DenOfIz_UIEdges_All( const float value )
    {
        return DenOfIz_UIEdges{ value, value, value, value };
    }

    DenOfIz_UIEdges DenOfIz_UIEdges_Symmetric( const float horizontal, const float vertical )
    {
        return DenOfIz_UIEdges{ horizontal, vertical, horizontal, vertical };
    }

    DenOfIz_UICorners DenOfIz_UICorners_All( const float radius )
    {
        return DenOfIz_UICorners{ radius, radius, radius, radius };
    }

    uint32_t DenOfIz_UI_HashId( const DenOfIz_StringView name, const uint32_t seed )
    {
        return UIMath::HashId( name.Chars, name.Chars ? name.NumChars : 0, seed );
    }

    uint32_t DenOfIz_UI_CombineId( const uint32_t id, const uint32_t value )
    {
        return UIMath::CombineId( id, value );
    }

    DenOfIz_UI DenOfIz_UI_Create( const DenOfIz_UIDesc *desc )
    {
        if ( desc == nullptr || !DENOFIZ_HANDLE_IS_VALID( desc->LogicalDevice ) )
        {
            spdlog::error( "DenOfIz_UI_Create: a logical device is required" );
            return DENOFIZ_NULL_HANDLE;
        }
        return DENOFIZ_TO_HANDLE( new UI( *desc ) );
    }

    void DenOfIz_UI_Destroy( const DenOfIz_UI ui )
    {
        delete UI_IMPL( ui );
    }

    void DenOfIz_UI_HandleEvent( const DenOfIz_UI ui, const DenOfIz_Event *ev )
    {
        if ( ev )
        {
            UI_IMPL( ui )->HandleEvent( *ev );
        }
    }

    void DenOfIz_UI_SetViewportSize( const DenOfIz_UI ui, const uint32_t pixelWidth, const uint32_t pixelHeight )
    {
        UI_IMPL( ui )->Context.SetViewportPixels( static_cast<float>( pixelWidth ), static_cast<float>( pixelHeight ) );
    }

    DenOfIz_Float2 DenOfIz_UI_GetViewportSize( const DenOfIz_UI ui )
    {
        return UI_IMPL( ui )->Context.GetViewportSize( );
    }

    void DenOfIz_UI_SetDisplayScale( const DenOfIz_UI ui, const float displayScale )
    {
        UI_IMPL( ui )->SetDisplayScale( displayScale );
    }

    void DenOfIz_UI_SetScale( const DenOfIz_UI ui, const float scale )
    {
        UI_IMPL( ui )->Context.SetScale( scale );
    }

    float DenOfIz_UI_GetPixelsPerPoint( const DenOfIz_UI ui )
    {
        return UI_IMPL( ui )->Context.GetPixelsPerPoint( );
    }

    void DenOfIz_UI_SetDebugFlags( const DenOfIz_UI ui, const DenOfIz_UIDebugFlags flags )
    {
        UI_IMPL( ui )->Context.SetDebugFlags( flags );
    }

    void DenOfIz_UI_AddFont( const DenOfIz_UI ui, const uint16_t fontId, const DenOfIz_Font font )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( font ) )
        {
            spdlog::error( "DenOfIz_UI_AddFont: font is null" );
            return;
        }
        UI_IMPL( ui )->QueueFont( fontId, DENOFIZ_FROM_HANDLE( Font, font ) );
    }

    void DenOfIz_UI_RemoveFont( const DenOfIz_UI ui, const uint16_t fontId )
    {
        UI_IMPL( ui )->QueueFont( fontId, nullptr );
    }

    void DenOfIz_UI_SetFontFallbacks( const DenOfIz_UI ui, const uint16_t fontId, const DenOfIz_UInt16Array *fallbackIds )
    {
        std::vector<uint16_t> fallbacks;
        if ( fallbackIds != nullptr && fallbackIds->Elements != nullptr )
        {
            fallbacks.assign( fallbackIds->Elements, fallbackIds->Elements + fallbackIds->NumElements );
        }
        UI_IMPL( ui )->QueueFallbacks( fontId, std::move( fallbacks ) );
    }

    void DenOfIz_UI_BeginFrame( const DenOfIz_UI ui, const float deltaTime )
    {
        UI_IMPL( ui )->BeginFrame( deltaTime );
    }

    uint32_t DenOfIz_UI_OpenElement( const DenOfIz_UI ui, const DenOfIz_UIElementDesc *desc )
    {
        static const DenOfIz_UIElementDesc defaultDesc{ };
        return UI_IMPL( ui )->Context.OpenElement( desc ? *desc : defaultDesc );
    }

    void DenOfIz_UI_CloseElement( const DenOfIz_UI ui )
    {
        UI_IMPL( ui )->Context.CloseElement( );
    }

    uint32_t DenOfIz_UI_Text( const DenOfIz_UI ui, const DenOfIz_StringView text, const DenOfIz_UITextDesc *desc )
    {
        static const DenOfIz_UITextDesc defaultDesc{ };
        return UI_IMPL( ui )->Context.AddText( text, desc ? *desc : defaultDesc );
    }

    uint32_t DenOfIz_UI_RichText( const DenOfIz_UI ui, const DenOfIz_StringView text, const DenOfIz_UITextRunArray *runs, const DenOfIz_UITextDesc *desc )
    {
        static const DenOfIz_UITextDesc defaultDesc{ };
        return UI_IMPL( ui )->Context.AddText( text, desc ? *desc : defaultDesc, runs );
    }

    void DenOfIz_UI_EndFrame( const DenOfIz_UI ui )
    {
        UI_IMPL( ui )->EndFrame( );
    }

    void DenOfIz_UI_Render( const DenOfIz_UI ui, const uint32_t frameIndex, const DenOfIz_CommandList commandList )
    {
        UI_IMPL( ui )->Render( frameIndex, commandList );
    }

    void DenOfIz_UI_RenderToTexture( const DenOfIz_UI ui, const uint32_t frameIndex, const DenOfIz_SemaphoreArray *waitSemaphores, DenOfIz_UIRenderResult *outResult )
    {
        UI_IMPL( ui )->RenderToTexture( frameIndex, waitSemaphores, outResult );
    }

    void DenOfIz_UI_GetInteraction( const DenOfIz_UI ui, const uint32_t id, DenOfIz_UIInteraction *outInteraction )
    {
        UI_IMPL( ui )->Context.GetInteraction( id, outInteraction );
    }

    bool DenOfIz_UI_GetElementRect( const DenOfIz_UI ui, const uint32_t id, DenOfIz_UIRect *outRect )
    {
        return UI_IMPL( ui )->Context.GetElementRect( id, outRect );
    }

    bool DenOfIz_UI_IsPointerOverUI( const DenOfIz_UI ui )
    {
        return UI_IMPL( ui )->Context.IsPointerOverUI( );
    }

    uint32_t DenOfIz_UI_GetHoveredId( const DenOfIz_UI ui )
    {
        return UI_IMPL( ui )->Context.GetHoveredId( );
    }

    DenOfIz_Float2 DenOfIz_UI_GetPointerPosition( const DenOfIz_UI ui )
    {
        return UI_IMPL( ui )->Context.GetPointerPosition( );
    }

    DenOfIz_SystemCursor DenOfIz_UI_GetCursor( const DenOfIz_UI ui )
    {
        return UI_IMPL( ui )->Context.GetCursor( );
    }

    void DenOfIz_UI_SetFocus( const DenOfIz_UI ui, const uint32_t id )
    {
        UI_IMPL( ui )->Context.SetFocus( id );
    }

    uint32_t DenOfIz_UI_GetFocus( const DenOfIz_UI ui )
    {
        return UI_IMPL( ui )->Context.GetFocus( );
    }

    void DenOfIz_UI_FocusNext( const DenOfIz_UI ui, const bool reverse )
    {
        UI_IMPL( ui )->Context.FocusNext( reverse );
    }

    bool DenOfIz_UI_GetScrollState( const DenOfIz_UI ui, const uint32_t id, DenOfIz_UIScrollState *outState )
    {
        return UI_IMPL( ui )->Context.GetScrollState( id, outState );
    }

    void DenOfIz_UI_SetScrollOffset( const DenOfIz_UI ui, const uint32_t id, const DenOfIz_Float2 offset )
    {
        UI_IMPL( ui )->Context.SetScrollOffset( id, offset );
    }

    void DenOfIz_UI_ScrollIntoView( const DenOfIz_UI ui, const uint32_t id )
    {
        UI_IMPL( ui )->Context.ScrollIntoView( id );
    }

    void DenOfIz_UI_ScrollCaretIntoView( const DenOfIz_UI ui, const uint32_t textId, const uint32_t byteOffset )
    {
        UI_IMPL( ui )->Context.ScrollCaretIntoView( textId, byteOffset );
    }

    DenOfIz_Float2 DenOfIz_UI_MeasureText( const DenOfIz_UI ui, const DenOfIz_StringView text, const DenOfIz_UITextDesc *desc, const float maxWidth )
    {
        static const DenOfIz_UITextDesc defaultDesc{ };
        return UI_IMPL( ui )->Context.MeasureText( text, desc ? *desc : defaultDesc, maxWidth );
    }

    bool DenOfIz_UI_GetTextCaretRect( const DenOfIz_UI ui, const uint32_t textId, const uint32_t byteOffset, DenOfIz_UIRect *outRect )
    {
        return UI_IMPL( ui )->Context.GetTextCaretRect( textId, byteOffset, outRect );
    }

    uint32_t DenOfIz_UI_GetTextOffsetAtPoint( const DenOfIz_UI ui, const uint32_t textId, const DenOfIz_Float2 point )
    {
        return UI_IMPL( ui )->Context.GetTextOffsetAtPoint( textId, point );
    }

    uint32_t DenOfIz_UI_GetTextRunAtPoint( const DenOfIz_UI ui, const uint32_t textId, const DenOfIz_Float2 point )
    {
        return UI_IMPL( ui )->Context.GetTextRunAtPoint( textId, point );
    }

    uint32_t DenOfIz_UI_GetTextLineCount( const DenOfIz_UI ui, const uint32_t textId )
    {
        return UI_IMPL( ui )->Context.GetTextLineCount( textId );
    }

    bool DenOfIz_UI_GetTextLine( const DenOfIz_UI ui, const uint32_t textId, const uint32_t lineIndex, DenOfIz_UITextLine *outLine )
    {
        return UI_IMPL( ui )->Context.GetTextLine( textId, lineIndex, outLine );
    }

    uint32_t DenOfIz_UI_GetNodeCount( const DenOfIz_UI ui )
    {
        return UI_IMPL( ui )->Context.NodeCount( );
    }

    bool DenOfIz_UI_GetNode( const DenOfIz_UI ui, const uint32_t index, DenOfIz_UINodeInfo *outNode )
    {
        return UI_IMPL( ui )->Context.GetNode( index, outNode );
    }

    uint32_t DenOfIz_UI_FindNode( const DenOfIz_UI ui, const uint32_t id )
    {
        return UI_IMPL( ui )->Context.FindNode( id );
    }

    uint32_t DenOfIz_UI_HitTest( const DenOfIz_UI ui, const DenOfIz_Float2 point, DenOfIz_UInt32Array *chain )
    {
        return UI_IMPL( ui )->Context.HitTestPoint( point, chain );
    }

    bool DenOfIz_UI_IsAnimating( const DenOfIz_UI ui )
    {
        return UI_IMPL( ui )->Context.IsAnimating( );
    }

    void DenOfIz_UI_GetViewportPixelSize( const DenOfIz_UI ui, uint32_t *outWidth, uint32_t *outHeight )
    {
        const DenOfIz_Float2 pixels = UI_IMPL( ui )->Context.GetViewportPixels( );
        *outWidth                   = static_cast<uint32_t>( pixels.X );
        *outHeight                  = static_cast<uint32_t>( pixels.Y );
    }

    bool DenOfIz_UI_Capture( const DenOfIz_UI ui, const uint32_t frameIndex, const DenOfIz_SemaphoreArray *waitSemaphores, DenOfIz_ByteArray *pixels )
    {
        return UI_IMPL( ui )->Capture( frameIndex, waitSemaphores, pixels );
    }
}
