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

#include "DenOfIzGraphics/UI/Clay.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"
#include "DenOfIzGraphicsInternal/UI/ClayContext.h"
#include "DenOfIzGraphicsInternal/UI/ClayRenderer.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include "DenOfIzGraphics/Input/Window.h"
#include "DenOfIzGraphics/Utilities/StepTimer.h"

#include <cfloat>
#include <cstdlib>
#include <cstring>
#include <memory>

using namespace DenOfIz;

#define CLAY_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::Clay, handle )
#define FONT_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::Font, handle )

namespace DenOfIz
{
    class Clay
    {
    public:
        DenOfIz_StepTimer             m_time = DENOFIZ_NULL_HANDLE;
        std::unique_ptr<ClayRenderer> m_renderer;
        std::unique_ptr<ClayContext>  m_clayContext;
        DenOfIz_ClayPointerState      m_pointerState = DENOFIZ_CLAY_POINTER_STATE_RELEASED;
        DenOfIz_Float2                m_pointerPosition{ 0, 0 };
        DenOfIz_Float2                m_scrollDelta{ 0, 0 };
        uint16_t                      m_fontId      = 1;
        bool                          m_isDebugMode = false;

        explicit Clay( const DenOfIz_ClayDesc &desc )
        {
            if ( desc.LogicalDevice == DENOFIZ_NULL_HANDLE )
            {
                spdlog::error( "Clay::Clay Logical device is null" );
                return;
            }

            if ( desc.Width == 0 || desc.Height == 0 )
            {
                spdlog::error( "Clay::Clay invalid dimensions provided: {}x{}", desc.Width, desc.Height );
                return;
            }

            ClayContextDesc clayContextDesc{ };
            clayContextDesc.LogicalDevice                  = desc.LogicalDevice;
            clayContextDesc.Width                          = desc.Width;
            clayContextDesc.Height                         = desc.Height;
            clayContextDesc.MaxNumElements                 = desc.MaxNumElements;
            clayContextDesc.MaxNumTextMeasureCacheElements = desc.MaxNumTextMeasureCacheElements;
            m_clayContext                                  = std::make_unique<ClayContext>( clayContextDesc );

            ClayRendererDesc clayRendererDesc{ };
            clayRendererDesc.LogicalDevice      = desc.LogicalDevice;
            clayRendererDesc.ResourceTracking   = desc.ResourceTracking;
            clayRendererDesc.Context            = m_clayContext.get( );
            clayRendererDesc.RenderTargetFormat = desc.RenderTargetFormat;
            clayRendererDesc.NumFrames          = desc.NumFrames;
            clayRendererDesc.MaxNumFonts        = desc.MaxNumFonts;
            clayRendererDesc.Width              = desc.Width;
            clayRendererDesc.Height             = desc.Height;

            m_renderer = std::make_unique<ClayRenderer>( clayRendererDesc );
            m_time     = DenOfIz_StepTimer_Create( );
            SetDpiScale( DenOfIz_Display_GetPrimaryDisplay( ).DpiScale );
        }

        ~Clay( )
        {
            if ( DENOFIZ_HANDLE_IS_VALID( m_time ) )
            {
                DenOfIz_StepTimer_Destroy( m_time );
            }
        }

        void SetViewportSize( const float width, const float height )
        {
            DZ_NOT_NULL( m_clayContext );
            m_clayContext->SetViewportSize( width, height );
            m_renderer->Resize( width, height );
        }

        void GetViewportSize( DenOfIz_ClayDimensions *outSize ) const
        {
            DZ_NOT_NULL( m_clayContext );
            *outSize = m_clayContext->GetViewportSize( );
        }

        void SetDpiScale( const float dpiScale )
        {
            m_clayContext->SetDpiScale( dpiScale );
        }

        void SetDebugModeEnabled( const bool enabled )
        {
            DZ_NOT_NULL( m_clayContext );
            m_isDebugMode = enabled;
            m_clayContext->SetDebugModeEnabled( enabled );
        }

        bool IsDebugModeEnabled( ) const
        {
            return m_isDebugMode;
        }

        void SetPointerState( const DenOfIz_Float2 position, const DenOfIz_ClayPointerState state )
        {
            DZ_NOT_NULL( m_clayContext );
            m_clayContext->SetPointerState( position, state );
        }

        void UpdateScrollContainers( const bool enableDragScrolling, const DenOfIz_Float2 scrollDelta, const float deltaTime )
        {
            DZ_NOT_NULL( m_clayContext );

            DenOfIz_Float2 totalScrollDelta = scrollDelta;
            totalScrollDelta.X += m_scrollDelta.X;
            totalScrollDelta.Y += m_scrollDelta.Y;

            m_clayContext->UpdateScrollContainers( enableDragScrolling, totalScrollDelta, deltaTime );

            m_scrollDelta.X = 0;
            m_scrollDelta.Y = 0;
        }

        void BeginLayout( )
        {
            DenOfIz_StepTimer_Tick( m_time );
            DZ_NOT_NULL( m_clayContext );
            SetPointerState( m_pointerPosition, m_pointerState );
            m_clayContext->SetDebugModeEnabled( m_isDebugMode );
            m_clayContext->BeginLayout( );
        }

        void EndLayout( const uint32_t frameIndex, const float deltaTime, DenOfIz_CommandList commandList )
        {
            DZ_NOT_NULL( m_clayContext );
            m_renderer->SetDeltaTime( deltaTime );
            m_renderer->RenderToCommandList( m_clayContext->EndLayoutAndGetCommands( deltaTime ), frameIndex, commandList );
        }

        void EndAndRenderLayout( const uint32_t frameIndex, const float deltaTime, DenOfIz_Texture *outTexture, DenOfIz_Semaphore *outSemaphore )
        {
            DZ_NOT_NULL( m_clayContext );
            m_renderer->SetDeltaTime( deltaTime );
            m_renderer->Render( m_clayContext->EndLayoutAndGetCommands( deltaTime ), frameIndex, outTexture, outSemaphore );
        }

        void OpenElement( const DenOfIz_ClayElementDeclaration *declaration )
        {
            DZ_NOT_NULL( m_clayContext );
            m_clayContext->OpenElement( declaration );
        }

        void CloseElement( )
        {
            DZ_NOT_NULL( m_clayContext );
            m_clayContext->CloseElement( );
        }

        uint32_t GetOpenElementId( ) const
        {
            DZ_NOT_NULL( m_clayContext );
            return m_clayContext->GetOpenElementId( );
        }

        DenOfIz_Float2 GetScrollOffset( ) const
        {
            DZ_NOT_NULL( m_clayContext );
            return m_clayContext->GetScrollOffset( );
        }

        void Text( const DenOfIz_StringView &text, const DenOfIz_ClayTextDesc *desc )
        {
            DZ_NOT_NULL( m_clayContext );
            m_clayContext->Text( text, desc );
        }

        void Texture( ITexture *texture, float width, float height )
        {
            DZ_NOT_NULL( m_clayContext );
            m_clayContext->Texture( texture, width, height );
        }

        uint32_t HashString( const DenOfIz_StringView &str, const uint32_t index, const uint32_t baseId ) const
        {
            return m_clayContext->HashString( str, index, baseId );
        }

        bool PointerOver( const uint32_t id ) const
        {
            return m_clayContext->PointerOver( id );
        }

        void GetElementBoundingBox( const uint32_t id, DenOfIz_ClayBoundingBox *outBox ) const
        {
            *outBox = m_clayContext->GetElementBoundingBox( id );
        }

        void HandleEvent( const DenOfIz_Event &ev )
        {
            if ( ev.Type == DENOFIZ_EVENT_TYPE_MOUSE_MOTION )
            {
                m_pointerPosition = DenOfIz_Float2{ static_cast<float>( ev.MouseMotion.X ), static_cast<float>( ev.MouseMotion.Y ) };
                m_clayContext->UpdatePointerPosition( m_pointerPosition, m_pointerState );
            }

            if ( ev.Type == DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_DOWN )
            {
                if ( ev.MouseButton.Button == DENOFIZ_MOUSE_BUTTON_LEFT )
                {
                    m_pointerPosition = DenOfIz_Float2{ static_cast<float>( ev.MouseButton.X ), static_cast<float>( ev.MouseButton.Y ) };
                    m_pointerState    = DENOFIZ_CLAY_POINTER_STATE_PRESSED;
                    m_clayContext->UpdatePointerPosition( m_pointerPosition, m_pointerState );
                }
            }

            if ( ev.Type == DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_UP )
            {
                if ( ev.MouseButton.Button == DENOFIZ_MOUSE_BUTTON_LEFT )
                {
                    m_pointerPosition = DenOfIz_Float2{ static_cast<float>( ev.MouseButton.X ), static_cast<float>( ev.MouseButton.Y ) };
                    m_pointerState    = DENOFIZ_CLAY_POINTER_STATE_RELEASED;
                    m_clayContext->UpdatePointerPosition( m_pointerPosition, m_pointerState );
                }
            }

            if ( ev.Type == DENOFIZ_EVENT_TYPE_WINDOW_EVENT )
            {
                const auto windowEvent = ev.Window.Event;
                if ( windowEvent == DENOFIZ_WINDOW_EVENT_TYPE_SIZE_CHANGED )
                {
                    SetViewportSize( static_cast<float>( ev.Window.Data1 ), static_cast<float>( ev.Window.Data2 ) );
                }
            }

            if ( ev.Type == DENOFIZ_EVENT_TYPE_KEY_DOWN )
            {
                if ( ev.Key.KeyCode == DENOFIZ_KEY_CODE_F11 )
                {
                    m_isDebugMode = !m_clayContext->IsDebugModeEnabled( );
                }
            }

            if ( ev.Type == DENOFIZ_EVENT_TYPE_MOUSE_WHEEL )
            {
                m_scrollDelta.X += static_cast<float>( ev.MouseWheel.X ) * 30.0f;
                m_scrollDelta.Y += static_cast<float>( ev.MouseWheel.Y ) * 30.0f;
            }
        }

        void MeasureText( const DenOfIz_StringView &text, const uint16_t fontId, const uint16_t fontSize, DenOfIz_ClayDimensions *outDimensions ) const
        {
            *outDimensions = m_clayContext->MeasureText( text, fontId, fontSize );
        }

        void AddFont( const uint16_t fontId, Font *font )
        {
            m_clayContext->AddFont( fontId, font );
        }

        void RemoveFont( const uint16_t fontId )
        {
            m_clayContext->RemoveFont( fontId );
        }

        void GetScrollContainerData( const uint32_t id, DenOfIz_ClayScrollContainerData *outData ) const
        {
            *outData = m_clayContext->GetScrollContainerData( id );
        }

        void SetScrollPosition( const uint32_t id, const DenOfIz_Float2 position ) const
        {
            m_clayContext->SetScrollPosition( id, position );
        }

        bool ElementClicked( const uint32_t id ) const
        {
            return m_clayContext->ElementClicked( id );
        }

        bool ElementPressed( const uint32_t id ) const
        {
            return m_clayContext->ElementPressed( id );
        }

        float GetCursorOffsetAtIndex( const DenOfIz_StringView &text, const uint32_t charIndex, const uint16_t fontId, const uint16_t fontSize ) const
        {
            return m_clayContext->GetCursorOffsetAtIndex( text, charIndex, fontId, fontSize );
        }

        uint32_t GetCharIndexAtOffset( const DenOfIz_StringView &text, const float pixelOffset, const uint16_t fontId, const uint16_t fontSize ) const
        {
            return m_clayContext->GetCharIndexAtOffset( text, pixelOffset, fontId, fontSize );
        }

        float GetDpiScale( ) const
        {
            return m_clayContext->GetDpiScale( );
        }

        float PointsToPixels( const float points ) const
        {
            return m_clayContext->PointsToPixels( points );
        }

        float PixelsToPoints( const float pixels ) const
        {
            return m_clayContext->PixelsToPoints( pixels );
        }
    };
} // namespace DenOfIz

extern "C"
{

    DenOfIz_ClaySizingAxis DenOfIz_ClaySizingAxis_Fit( float min, float max )
    {
        DenOfIz_ClaySizingAxis axis;
        axis.Type                  = DENOFIZ_CLAY_SIZING_TYPE_FIT;
        axis.Size.MinMaxFloats.Min = min;
        axis.Size.MinMaxFloats.Max = max;
        axis.Size.Percent          = 0.0f;
        return axis;
    }

    DenOfIz_ClaySizingAxis DenOfIz_ClaySizingAxis_Grow( float min, float max )
    {
        DenOfIz_ClaySizingAxis axis;
        axis.Type                  = DENOFIZ_CLAY_SIZING_TYPE_GROW;
        axis.Size.MinMaxFloats.Min = min;
        axis.Size.MinMaxFloats.Max = max;
        axis.Size.Percent          = 0.0f;
        return axis;
    }

    DenOfIz_ClaySizingAxis DenOfIz_ClaySizingAxis_Fixed( float size )
    {
        DenOfIz_ClaySizingAxis axis;
        axis.Type                  = DENOFIZ_CLAY_SIZING_TYPE_FIXED;
        axis.Size.MinMaxFloats.Min = size;
        axis.Size.MinMaxFloats.Max = size;
        axis.Size.Percent          = 0.0f;
        return axis;
    }

    DenOfIz_ClaySizingAxis DenOfIz_ClaySizingAxis_Percent( float percent )
    {
        DenOfIz_ClaySizingAxis axis;
        axis.Type                  = DENOFIZ_CLAY_SIZING_TYPE_PERCENT;
        axis.Size.MinMaxFloats.Min = 0.0f;
        axis.Size.MinMaxFloats.Max = 0.0f;
        axis.Size.Percent          = percent;
        return axis;
    }

    DenOfIz_ClayColor DenOfIz_ClayColor_Create( uint32_t r, uint32_t g, uint32_t b, uint32_t a )
    {
        DenOfIz_ClayColor color;
        color.R = r;
        color.G = g;
        color.B = b;
        color.A = a;
        return color;
    }

    DenOfIz_Float4 DenOfIz_ClayColor_ToFloat4( const DenOfIz_ClayColor *color )
    {
        DenOfIz_Float4 result;
        result.X = color->R / 255.0f;
        result.Y = color->G / 255.0f;
        result.Z = color->B / 255.0f;
        result.W = color->A / 255.0f;
        return result;
    }

    DenOfIz_ClayBorderRadius DenOfIz_ClayBorderRadius_Create( float topLeft, float topRight, float bottomLeft, float bottomRight )
    {
        DenOfIz_ClayBorderRadius radius;
        radius.TopLeft     = topLeft;
        radius.TopRight    = topRight;
        radius.BottomLeft  = bottomLeft;
        radius.BottomRight = bottomRight;
        return radius;
    }

    DenOfIz_ClayBorderRadius DenOfIz_ClayBorderRadius_CreateUniform( float radius )
    {
        return DenOfIz_ClayBorderRadius_Create( radius, radius, radius, radius );
    }

    DenOfIz_ClayBorderWidth DenOfIz_ClayBorderWidth_Create( float left, float right, float top, float bottom, float betweenChildren )
    {
        DenOfIz_ClayBorderWidth width;
        width.Left            = left;
        width.Right           = right;
        width.Top             = top;
        width.Bottom          = bottom;
        width.BetweenChildren = betweenChildren;
        return width;
    }

    DenOfIz_ClayBorderWidth DenOfIz_ClayBorderWidth_CreateUniform( float width )
    {
        return DenOfIz_ClayBorderWidth_Create( width, width, width, width, 0.0f );
    }

    DenOfIz_ClayPadding DenOfIz_ClayPadding_Create( float left, float right, float top, float bottom )
    {
        DenOfIz_ClayPadding padding;
        padding.Left   = left;
        padding.Right  = right;
        padding.Top    = top;
        padding.Bottom = bottom;
        return padding;
    }

    DenOfIz_ClayPadding DenOfIz_ClayPadding_CreateUniform( float padding )
    {
        return DenOfIz_ClayPadding_Create( padding, padding, padding, padding );
    }

    DenOfIz_ClayElementDeclaration DenOfIz_ClayElementDeclaration_Default( )
    {
        DenOfIz_ClayElementDeclaration decl;
        memset( &decl, 0, sizeof( decl ) );
        decl.Layout.Sizing.Width  = DenOfIz_ClaySizingAxis_Grow( 0.0f, FLT_MAX );
        decl.Layout.Sizing.Height = DenOfIz_ClaySizingAxis_Grow( 0.0f, FLT_MAX );
        return decl;
    }

    DenOfIz_ClayTextDesc DenOfIz_ClayTextDesc_Default( )
    {
        DenOfIz_ClayTextDesc desc;
        memset( &desc, 0, sizeof( desc ) );
        desc.TextColor          = DenOfIz_ClayColor_Create( 0, 0, 0, 255 );
        desc.FontId             = 0;
        desc.FontSize           = 16;
        desc.WrapMode           = DENOFIZ_CLAY_TEXT_WRAP_MODE_WORDS;
        desc.TextAlignment      = DENOFIZ_CLAY_TEXT_ALIGNMENT_LEFT;
        return desc;
    }

    DenOfIz_ClayTransitionDesc DenOfIz_ClayTransitionDesc_Default( )
    {
        DenOfIz_ClayTransitionDesc desc;
        memset( &desc, 0, sizeof( desc ) );
        desc.Enabled             = true;
        desc.Duration            = 0.25f;
        desc.Properties          = DENOFIZ_CLAY_TRANSITION_PROPERTY_BOUNDING_BOX_BIT | DENOFIZ_CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR_BIT |
                          DENOFIZ_CLAY_TRANSITION_PROPERTY_OVERLAY_COLOR_BIT | DENOFIZ_CLAY_TRANSITION_PROPERTY_BORDER_BIT;
        desc.Easing              = DENOFIZ_CLAY_TRANSITION_EASING_EASE_OUT;
        desc.InteractionHandling = DENOFIZ_CLAY_TRANSITION_INTERACTION_HANDLING_DISABLE_WHILE_MOVING;
        desc.Enter.Trigger       = DENOFIZ_CLAY_TRANSITION_ENTER_TRIGGER_SKIP_ON_FIRST_PARENT_FRAME;
        desc.Exit.Trigger        = DENOFIZ_CLAY_TRANSITION_EXIT_TRIGGER_SKIP_WHEN_PARENT_EXITS;
        desc.Exit.SiblingOrdering = DENOFIZ_CLAY_EXIT_TRANSITION_SIBLING_ORDERING_UNDERNEATH_SIBLINGS;
        return desc;
    }

    DenOfIz_Clay DenOfIz_Clay_Create( const DenOfIz_ClayDesc *desc )
    {
        if ( desc == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        auto *clay = new Clay( *desc );
        return DENOFIZ_TO_HANDLE( clay );
    }

    void DenOfIz_Clay_Destroy( DenOfIz_Clay clay )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return;
        }
        delete CLAY_IMPL( clay );
    }

    void DenOfIz_Clay_SetViewportSize( DenOfIz_Clay clay, float width, float height )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return;
        }
        CLAY_IMPL( clay )->SetViewportSize( width, height );
    }

    void DenOfIz_Clay_GetViewportSize( DenOfIz_Clay clay, DenOfIz_ClayDimensions *outSize )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) || outSize == NULL )
        {
            return;
        }
        CLAY_IMPL( clay )->GetViewportSize( outSize );
    }

    void DenOfIz_Clay_SetDpiScale( DenOfIz_Clay clay, float dpiScale )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return;
        }
        CLAY_IMPL( clay )->SetDpiScale( dpiScale );
    }

    void DenOfIz_Clay_SetPointerState( DenOfIz_Clay clay, DenOfIz_Float2 position, DenOfIz_ClayPointerState state )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return;
        }
        CLAY_IMPL( clay )->SetPointerState( position, state );
    }

    void DenOfIz_Clay_UpdateScrollContainers( DenOfIz_Clay clay, bool enableDragScrolling, DenOfIz_Float2 scrollDelta, float deltaTime )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return;
        }
        CLAY_IMPL( clay )->UpdateScrollContainers( enableDragScrolling, scrollDelta, deltaTime );
    }

    void DenOfIz_Clay_SetDebugModeEnabled( DenOfIz_Clay clay, bool enabled )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return;
        }
        CLAY_IMPL( clay )->SetDebugModeEnabled( enabled );
    }

    bool DenOfIz_Clay_IsDebugModeEnabled( DenOfIz_Clay clay )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return false;
        }
        return CLAY_IMPL( clay )->IsDebugModeEnabled( );
    }

    void DenOfIz_Clay_BeginLayout( DenOfIz_Clay clay )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return;
        }
        CLAY_IMPL( clay )->BeginLayout( );
    }

    void DenOfIz_Clay_EndLayout( DenOfIz_Clay clay, uint32_t frameIndex, float deltaTime, DenOfIz_CommandList commandList )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) || !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        CLAY_IMPL( clay )->EndLayout( frameIndex, deltaTime, commandList );
    }

    void DenOfIz_Clay_EndAndRenderLayout( DenOfIz_Clay clay, uint32_t frameIndex, float deltaTime, DenOfIz_ClayRenderResult *outResult )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) || outResult == NULL )
        {
            return;
        }
        CLAY_IMPL( clay )->EndAndRenderLayout( frameIndex, deltaTime, &outResult->Texture, &outResult->Semaphore );
    }

    void DenOfIz_Clay_HandleEvent( DenOfIz_Clay clay, const DenOfIz_Event *ev )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) || ev == NULL )
        {
            return;
        }
        CLAY_IMPL( clay )->HandleEvent( *ev );
    }

    void DenOfIz_Clay_OpenElement( DenOfIz_Clay clay, const DenOfIz_ClayElementDeclaration *declaration )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) || declaration == NULL )
        {
            return;
        }
        CLAY_IMPL( clay )->OpenElement( declaration );
    }

    void DenOfIz_Clay_CloseElement( DenOfIz_Clay clay )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return;
        }
        CLAY_IMPL( clay )->CloseElement( );
    }

    uint32_t DenOfIz_Clay_GetOpenElementId( DenOfIz_Clay clay )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return 0;
        }
        return CLAY_IMPL( clay )->GetOpenElementId( );
    }

    DenOfIz_Float2 DenOfIz_Clay_GetScrollOffset( DenOfIz_Clay clay )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return DenOfIz_Float2{ 0.0f, 0.0f };
        }
        return CLAY_IMPL( clay )->GetScrollOffset( );
    }

    void DenOfIz_Clay_Text( DenOfIz_Clay clay, DenOfIz_StringView text, const DenOfIz_ClayTextDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) || desc == NULL )
        {
            return;
        }
        CLAY_IMPL( clay )->Text( text, desc );
    }

    void DenOfIz_Clay_Texture( DenOfIz_Clay clay, DenOfIz_Texture texture, float width, float height )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) || !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return;
        }
        ITexture *textureImpl = DENOFIZ_FROM_HANDLE( ITexture, texture );
        CLAY_IMPL( clay )->Texture( textureImpl, width, height );
    }

    uint32_t DenOfIz_Clay_HashString( DenOfIz_Clay clay, DenOfIz_StringView str, uint32_t index, uint32_t baseId )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return 0;
        }
        return CLAY_IMPL( clay )->HashString( str, index, baseId );
    }

    bool DenOfIz_Clay_PointerOver( DenOfIz_Clay clay, uint32_t id )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return false;
        }
        return CLAY_IMPL( clay )->PointerOver( id );
    }

    void DenOfIz_Clay_GetElementBoundingBox( DenOfIz_Clay clay, uint32_t id, DenOfIz_ClayBoundingBox *outBox )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) || outBox == NULL )
        {
            return;
        }
        CLAY_IMPL( clay )->GetElementBoundingBox( id, outBox );
    }

    void DenOfIz_Clay_MeasureText( DenOfIz_Clay clay, DenOfIz_StringView text, uint16_t fontId, uint16_t fontSize, DenOfIz_ClayDimensions *outDimensions )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) || outDimensions == NULL )
        {
            return;
        }
        CLAY_IMPL( clay )->MeasureText( text, fontId, fontSize, outDimensions );
    }

    void DenOfIz_Clay_AddFont( DenOfIz_Clay clay, uint16_t fontId, DenOfIz_Font font )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) || !DENOFIZ_HANDLE_IS_VALID( font ) )
        {
            return;
        }
        CLAY_IMPL( clay )->AddFont( fontId, FONT_IMPL( font ) );
    }

    void DenOfIz_Clay_RemoveFont( DenOfIz_Clay clay, uint16_t fontId )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return;
        }
        CLAY_IMPL( clay )->RemoveFont( fontId );
    }

    void DenOfIz_Clay_GetScrollContainerData( DenOfIz_Clay clay, uint32_t id, DenOfIz_ClayScrollContainerData *outData )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) || outData == NULL )
        {
            return;
        }
        CLAY_IMPL( clay )->GetScrollContainerData( id, outData );
    }

    void DenOfIz_Clay_SetScrollPosition( DenOfIz_Clay clay, uint32_t id, DenOfIz_Float2 position )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return;
        }
        CLAY_IMPL( clay )->SetScrollPosition( id, position );
    }

    bool DenOfIz_Clay_ElementClicked( DenOfIz_Clay clay, uint32_t id )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return false;
        }
        return CLAY_IMPL( clay )->ElementClicked( id );
    }

    bool DenOfIz_Clay_ElementPressed( DenOfIz_Clay clay, uint32_t id )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return false;
        }
        return CLAY_IMPL( clay )->ElementPressed( id );
    }

    float DenOfIz_Clay_GetCursorOffsetAtIndex( DenOfIz_Clay clay, DenOfIz_StringView text, uint32_t charIndex, uint16_t fontId, uint16_t fontSize )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return 0.0f;
        }
        return CLAY_IMPL( clay )->GetCursorOffsetAtIndex( text, charIndex, fontId, fontSize );
    }

    uint32_t DenOfIz_Clay_GetCharIndexAtOffset( DenOfIz_Clay clay, DenOfIz_StringView text, float pixelOffset, uint16_t fontId, uint16_t fontSize )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return 0;
        }
        return CLAY_IMPL( clay )->GetCharIndexAtOffset( text, pixelOffset, fontId, fontSize );
    }

    float DenOfIz_Clay_GetDpiScale( DenOfIz_Clay clay )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return 1.0f;
        }
        return CLAY_IMPL( clay )->GetDpiScale( );
    }

    float DenOfIz_Clay_PointsToPixels( DenOfIz_Clay clay, float points )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return points;
        }
        return CLAY_IMPL( clay )->PointsToPixels( points );
    }

    float DenOfIz_Clay_PixelsToPoints( DenOfIz_Clay clay, float pixels )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( clay ) )
        {
            return pixels;
        }
        return CLAY_IMPL( clay )->PixelsToPoints( pixels );
    }
}
