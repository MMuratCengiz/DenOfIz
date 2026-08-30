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

#define CLAY_IMPLEMENTATION
#include "clay.h"

#include "DenOfIzGraphicsInternal/UI/ClayContext.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <utility>

using namespace DenOfIz;

namespace
{
    constexpr uint32_t MaxTransitionStateSlots = 256;

    struct TransitionStateSlot
    {
        DenOfIz_ClayTransitionStateDesc Desc{ };
        uint64_t                        LastUsedFrame = 0;
        bool                            InUse         = false;
    };

    struct TransitionStateTable
    {
        TransitionStateSlot Slots[ MaxTransitionStateSlots ];
    };

    TransitionStateTable g_enterStates;
    TransitionStateTable g_exitStates;
    uint64_t             g_transitionFrame        = 1;
    bool                 g_slotExhaustionReported = false;

    Clay_Color ToClayColor( const DenOfIz_ClayColor &color )
    {
        return Clay_Color{ static_cast<float>( color.R ), static_cast<float>( color.G ), static_cast<float>( color.B ), static_cast<float>( color.A ) };
    }

    Clay_TransitionData ApplyTransitionState( Clay_TransitionData state, const DenOfIz_ClayTransitionStateDesc &desc )
    {
        state.boundingBox.x += desc.PositionOffset.X;
        state.boundingBox.y += desc.PositionOffset.Y;
        if ( desc.Scale > 0.0f && desc.Scale != 1.0f )
        {
            const float newWidth  = state.boundingBox.width * desc.Scale;
            const float newHeight = state.boundingBox.height * desc.Scale;
            state.boundingBox.x += ( state.boundingBox.width - newWidth ) * 0.5f;
            state.boundingBox.y += ( state.boundingBox.height - newHeight ) * 0.5f;
            state.boundingBox.width  = newWidth;
            state.boundingBox.height = newHeight;
        }
        if ( desc.OverlayColor.A > 0 )
        {
            state.overlayColor = ToClayColor( desc.OverlayColor );
        }
        if ( desc.HasBackgroundColor )
        {
            state.backgroundColor = ToClayColor( desc.BackgroundColor );
        }
        if ( desc.HasBorderColor )
        {
            state.borderColor = ToClayColor( desc.BorderColor );
        }
        if ( desc.HasBorderWidth )
        {
            state.borderWidth.left            = static_cast<uint16_t>( desc.BorderWidth.Left );
            state.borderWidth.right           = static_cast<uint16_t>( desc.BorderWidth.Right );
            state.borderWidth.top             = static_cast<uint16_t>( desc.BorderWidth.Top );
            state.borderWidth.bottom          = static_cast<uint16_t>( desc.BorderWidth.Bottom );
            state.borderWidth.betweenChildren = static_cast<uint16_t>( desc.BorderWidth.BetweenChildren );
        }
        return state;
    }

    template <uint32_t Slot>
    Clay_TransitionData EnterStateThunk( const Clay_TransitionData targetState, Clay_TransitionProperty )
    {
        return ApplyTransitionState( targetState, g_enterStates.Slots[ Slot ].Desc );
    }

    template <uint32_t Slot>
    Clay_TransitionData ExitStateThunk( const Clay_TransitionData initialState, Clay_TransitionProperty )
    {
        return ApplyTransitionState( initialState, g_exitStates.Slots[ Slot ].Desc );
    }

    using TransitionStateFn = Clay_TransitionData ( * )( Clay_TransitionData, Clay_TransitionProperty );

    template <uint32_t... Slots>
    constexpr std::array<TransitionStateFn, sizeof...( Slots )> MakeEnterThunks( std::integer_sequence<uint32_t, Slots...> )
    {
        return { &EnterStateThunk<Slots>... };
    }

    template <uint32_t... Slots>
    constexpr std::array<TransitionStateFn, sizeof...( Slots )> MakeExitThunks( std::integer_sequence<uint32_t, Slots...> )
    {
        return { &ExitStateThunk<Slots>... };
    }

    const auto g_enterThunks = MakeEnterThunks( std::make_integer_sequence<uint32_t, MaxTransitionStateSlots>{ } );
    const auto g_exitThunks  = MakeExitThunks( std::make_integer_sequence<uint32_t, MaxTransitionStateSlots>{ } );

    DenOfIz_ClayTransitionStateDesc NormalizeTransitionState( const DenOfIz_ClayTransitionStateDesc &desc, const float dpiScale )
    {
        DenOfIz_ClayTransitionStateDesc result;
        memset( &result, 0, sizeof( result ) );
        result.PositionOffset.X = desc.PositionOffset.X * dpiScale;
        result.PositionOffset.Y = desc.PositionOffset.Y * dpiScale;
        result.Scale            = desc.Scale;
        if ( desc.OverlayColor.A > 0 )
        {
            result.OverlayColor = desc.OverlayColor;
        }
        result.HasBackgroundColor = desc.HasBackgroundColor;
        if ( desc.HasBackgroundColor )
        {
            result.BackgroundColor = desc.BackgroundColor;
        }
        result.HasBorderColor = desc.HasBorderColor;
        if ( desc.HasBorderColor )
        {
            result.BorderColor = desc.BorderColor;
        }
        result.HasBorderWidth = desc.HasBorderWidth;
        if ( desc.HasBorderWidth )
        {
            result.BorderWidth.Left            = desc.BorderWidth.Left * dpiScale;
            result.BorderWidth.Right           = desc.BorderWidth.Right * dpiScale;
            result.BorderWidth.Top             = desc.BorderWidth.Top * dpiScale;
            result.BorderWidth.Bottom          = desc.BorderWidth.Bottom * dpiScale;
            result.BorderWidth.BetweenChildren = desc.BorderWidth.BetweenChildren * dpiScale;
        }
        return result;
    }

    int32_t AcquireTransitionStateSlot( TransitionStateTable &table, const DenOfIz_ClayTransitionStateDesc &normalizedDesc )
    {
        int32_t freeSlot = -1;
        for ( uint32_t i = 0; i < MaxTransitionStateSlots; ++i )
        {
            TransitionStateSlot &slot = table.Slots[ i ];
            if ( slot.InUse && memcmp( &slot.Desc, &normalizedDesc, sizeof( normalizedDesc ) ) == 0 )
            {
                slot.LastUsedFrame = g_transitionFrame;
                return static_cast<int32_t>( i );
            }
            if ( freeSlot < 0 && ( !slot.InUse || slot.LastUsedFrame + 2 < g_transitionFrame ) )
            {
                freeSlot = static_cast<int32_t>( i );
            }
        }

        if ( freeSlot < 0 )
        {
            if ( !g_slotExhaustionReported )
            {
                spdlog::error( "ClayContext: More than {} distinct transition enter/exit states are in use, additional states are ignored", MaxTransitionStateSlots );
                g_slotExhaustionReported = true;
            }
            return -1;
        }

        TransitionStateSlot &slot = table.Slots[ freeSlot ];
        slot.Desc                 = normalizedDesc;
        slot.LastUsedFrame        = g_transitionFrame;
        slot.InUse                = true;
        return freeSlot;
    }

    float TransitionRatio( const Clay_TransitionCallbackArguments &arguments )
    {
        if ( arguments.duration <= 0.0f )
        {
            return 1.0f;
        }
        const float ratio = arguments.elapsedTime / arguments.duration;
        return ratio > 1.0f ? 1.0f : ratio;
    }

    float Lerp( const float from, const float to, const float amount )
    {
        return from + ( to - from ) * amount;
    }

    Clay_Color LerpColor( const Clay_Color &from, const Clay_Color &to, const float amount )
    {
        return Clay_Color{ Lerp( from.r, to.r, amount ), Lerp( from.g, to.g, amount ), Lerp( from.b, to.b, amount ), Lerp( from.a, to.a, amount ) };
    }

    bool ApplyEasedTransition( const Clay_TransitionCallbackArguments &arguments, const float ratio, const float lerpAmount )
    {
        Clay_TransitionData       &current = *arguments.current;
        const Clay_TransitionData &initial = arguments.initial;
        const Clay_TransitionData &target  = arguments.target;

        if ( arguments.properties & CLAY_TRANSITION_PROPERTY_X )
        {
            current.boundingBox.x = Lerp( initial.boundingBox.x, target.boundingBox.x, lerpAmount );
        }
        if ( arguments.properties & CLAY_TRANSITION_PROPERTY_Y )
        {
            current.boundingBox.y = Lerp( initial.boundingBox.y, target.boundingBox.y, lerpAmount );
        }
        if ( arguments.properties & CLAY_TRANSITION_PROPERTY_WIDTH )
        {
            current.boundingBox.width = Lerp( initial.boundingBox.width, target.boundingBox.width, lerpAmount );
        }
        if ( arguments.properties & CLAY_TRANSITION_PROPERTY_HEIGHT )
        {
            current.boundingBox.height = Lerp( initial.boundingBox.height, target.boundingBox.height, lerpAmount );
        }
        if ( arguments.properties & CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR )
        {
            current.backgroundColor = LerpColor( initial.backgroundColor, target.backgroundColor, lerpAmount );
        }
        if ( arguments.properties & CLAY_TRANSITION_PROPERTY_OVERLAY_COLOR )
        {
            current.overlayColor = LerpColor( initial.overlayColor, target.overlayColor, lerpAmount );
        }
        if ( arguments.properties & CLAY_TRANSITION_PROPERTY_BORDER_COLOR )
        {
            current.borderColor = LerpColor( initial.borderColor, target.borderColor, lerpAmount );
        }
        if ( arguments.properties & CLAY_TRANSITION_PROPERTY_BORDER_WIDTH )
        {
            current.borderWidth.left            = static_cast<uint16_t>( Lerp( initial.borderWidth.left, target.borderWidth.left, lerpAmount ) );
            current.borderWidth.right           = static_cast<uint16_t>( Lerp( initial.borderWidth.right, target.borderWidth.right, lerpAmount ) );
            current.borderWidth.top             = static_cast<uint16_t>( Lerp( initial.borderWidth.top, target.borderWidth.top, lerpAmount ) );
            current.borderWidth.bottom          = static_cast<uint16_t>( Lerp( initial.borderWidth.bottom, target.borderWidth.bottom, lerpAmount ) );
            current.borderWidth.betweenChildren = static_cast<uint16_t>( Lerp( initial.borderWidth.betweenChildren, target.borderWidth.betweenChildren, lerpAmount ) );
        }
        return ratio >= 1.0f;
    }

    bool LinearTransitionHandler( const Clay_TransitionCallbackArguments arguments )
    {
        const float ratio = TransitionRatio( arguments );
        return ApplyEasedTransition( arguments, ratio, ratio );
    }

    bool EaseInTransitionHandler( const Clay_TransitionCallbackArguments arguments )
    {
        const float ratio = TransitionRatio( arguments );
        return ApplyEasedTransition( arguments, ratio, ratio * ratio * ratio );
    }

    bool EaseInOutTransitionHandler( const Clay_TransitionCallbackArguments arguments )
    {
        const float ratio   = TransitionRatio( arguments );
        const float inverse = -2.0f * ratio + 2.0f;
        const float amount  = ratio < 0.5f ? 4.0f * ratio * ratio * ratio : 1.0f - inverse * inverse * inverse * 0.5f;
        return ApplyEasedTransition( arguments, ratio, amount );
    }
} // namespace

Clay_Dimensions ClayContext::MeasureTextCallback( Clay_StringSlice text, Clay_TextElementConfig *config, void *userData )
{
    const auto *context = static_cast<ClayContext *>( userData );
    DZ_NOT_NULL( context );

    if ( text.length == 0 )
    {
        return Clay_Dimensions{ 0, 0 };
    }

    const DenOfIz_ClayDimensions dims = context->MeasureTextRaw( { text.chars, static_cast<uint32_t>( text.length ) }, config->fontId, config->fontSize );
    return Clay_Dimensions{ dims.Width, dims.Height };
}

static void ErrorHandler( Clay_ErrorData error )
{
    const std::string errorText( error.errorText.chars, error.errorText.length );
    spdlog::error( "Clay error: {}", errorText );
}

ClayContext::ClayContext( const ClayContextDesc &desc )
{
    if ( desc.LogicalDevice == DENOFIZ_NULL_HANDLE )
    {
        spdlog::error( "ClayContext: Logical device is null" );
        return;
    }

    if ( desc.Width == 0 || desc.Height == 0 )
    {
        spdlog::error( "ClayContext: invalid dimensions provided: {} x{}", desc.Width, desc.Height );
        return;
    }

    ClayTextCacheDesc clayTextDesc{ };
    clayTextDesc.LogicalDevice = desc.LogicalDevice;
    clayTextDesc.MaxTextures   = 128;
    m_clayText                 = std::make_unique<ClayTextCache>( clayTextDesc );

    Clay_SetMaxElementCount( desc.MaxNumElements );
    Clay_SetMaxMeasureTextCacheWordCount( desc.MaxNumTextMeasureCacheElements );

    m_textArenaCapacity = desc.MaxNumElements;

    const uint32_t minMemorySize = Clay_MinMemorySize( );
    m_memory.resize( minMemorySize );
    m_arena = Clay_CreateArenaWithCapacityAndMemory( minMemorySize, m_memory.data( ) );

    Clay_ErrorHandler errorHandler;
    errorHandler.errorHandlerFunction = ErrorHandler;
    errorHandler.userData             = this;

    m_context = Clay_Initialize( m_arena, Clay_Dimensions{ static_cast<float>( desc.Width ), static_cast<float>( desc.Height ) }, errorHandler );
    if ( !m_context )
    {
        spdlog::error( "Failed to initialize Clay" );
    }

    Clay_SetDebugModeEnabled( false );
    Clay_SetMeasureTextFunction( MeasureTextCallback, this );
    SetViewportSize( desc.Width, desc.Height );
}

ClayContext::~ClayContext( )
{
    m_context = nullptr;
    m_memory.clear( );
}

void ClayContext::BeginLayout( ) const
{
    DZ_NOT_NULL( m_context );
    ++g_transitionFrame;
    Clay_BeginLayout( );
}

uint32_t ClayContext::GetOpenElementId( ) const
{
    DZ_NOT_NULL( m_context );
    return Clay_GetOpenElementId( );
}

DenOfIz_Float2 ClayContext::GetScrollOffset( ) const
{
    DZ_NOT_NULL( m_context );
    const Clay_Vector2 offset = Clay_GetScrollOffset( );
    return DenOfIz_Float2{ offset.x, offset.y };
}

void ClayContext::SetViewportSize( const float width, const float height ) const
{
    DZ_NOT_NULL( m_context );
    Clay_SetLayoutDimensions( Clay_Dimensions{ width, height } );
}

DenOfIz_ClayDimensions ClayContext::GetViewportSize( ) const
{
    DZ_NOT_NULL( m_context );
    const Clay_Dimensions dimensions = Clay_GetCurrentContext( )->layoutDimensions;
    return DenOfIz_ClayDimensions{ dimensions.width, dimensions.height };
}

void ClayContext::SetDpiScale( const float dpiScale )
{
    m_dpiScale = dpiScale;
    Clay_ResetMeasureTextCache( );
}

float ClayContext::GetDpiScale( ) const
{
    return m_dpiScale;
}

float ClayContext::PointsToPixels( float points ) const
{
    return points * m_dpiScale;
}

float ClayContext::PixelsToPoints( float pixels ) const
{
    return pixels / m_dpiScale;
}

void ClayContext::UpdatePointerPosition( const DenOfIz_Float2 position, const DenOfIz_ClayPointerState state )
{
    DZ_NOT_NULL( m_context );
    m_pointerPosition = position;
    Clay_SetPointerState( Clay_Vector2{ position.X, position.Y }, state == DENOFIZ_CLAY_POINTER_STATE_PRESSED );
}

void ClayContext::SetPointerState( const DenOfIz_Float2 position, const DenOfIz_ClayPointerState state )
{
    DZ_NOT_NULL( m_context );
    m_pointerPosition  = position;
    m_prevPointerState = m_pointerState;
    m_pointerState     = state;
    Clay_SetPointerState( Clay_Vector2{ position.X, position.Y }, state == DENOFIZ_CLAY_POINTER_STATE_PRESSED );
}

void ClayContext::UpdateScrollContainers( const bool enableDragScrolling, const DenOfIz_Float2 scrollDelta, const float deltaTime )
{
    DZ_NOT_NULL( m_context );
    m_scrollDelta = scrollDelta;
    Clay_UpdateScrollContainers( enableDragScrolling, Clay_Vector2{ scrollDelta.X, scrollDelta.Y }, deltaTime );
}

void ClayContext::SetDebugModeEnabled( const bool enabled )
{
    DZ_NOT_NULL( m_context );
    m_isDebugMode = enabled;
    Clay_SetDebugModeEnabled( enabled );
}

bool ClayContext::IsDebugModeEnabled( ) const
{
    return m_isDebugMode;
}

void ClayContext::OpenElement( const DenOfIz_ClayElementDeclaration *declaration ) const
{
    DZ_NOT_NULL( m_context );

    if ( declaration->Id != 0 )
    {
        Clay_ElementId elementId{ };
        elementId.id     = declaration->Id;
        elementId.baseId = declaration->Id;
        Clay__OpenElementWithId( elementId );
    }
    else
    {
        Clay__OpenElement( );
    }

    const bool isTextureElement = declaration->Custom.CustomData != nullptr;
    const bool enableDpiScaling = !isTextureElement;

    Clay_ElementDeclaration clayDecl{ };
    clayDecl.layout          = ConvertLayoutConfig( &declaration->Layout, enableDpiScaling );
    clayDecl.backgroundColor = ConvertColor( &declaration->BackgroundColor );
    clayDecl.overlayColor    = ConvertColor( &declaration->OverlayColor );
    clayDecl.cornerRadius    = ConvertBorderRadius( &declaration->BorderRadius, enableDpiScaling );
    clayDecl.aspectRatio     = ConvertAspectRatioConfig( declaration );
    clayDecl.image           = ConvertImageConfig( &declaration->Image );
    clayDecl.floating        = ConvertFloatingConfig( &declaration->Floating, enableDpiScaling );
    clayDecl.custom          = ConvertCustomConfig( &declaration->Custom );
    clayDecl.clip            = ConvertClipConfig( &declaration->Scroll, &declaration->Clip, enableDpiScaling );
    clayDecl.border          = ConvertBorderConfig( &declaration->Border, enableDpiScaling );
    clayDecl.transition      = ConvertTransitionConfig( &declaration->Transition, enableDpiScaling );
    clayDecl.userData        = declaration->UserData;

    Clay__ConfigureOpenElementPtr( &clayDecl );
}

void ClayContext::CloseElement( ) const
{
    DZ_NOT_NULL( m_context );
    Clay__CloseElement( );
}

ClayContext::RetainedTextArena &ClayContext::AcquireTextArena( const uint32_t minCapacity ) const
{
    const auto now = std::chrono::steady_clock::now( );
    for ( size_t i = 0; i < m_textArenas.size( ); ++i )
    {
        RetainedTextArena &arena       = m_textArenas[ i ];
        const double       idleSeconds = std::chrono::duration<double>( now - arena.LastUsed ).count( );
        if ( arena.Capacity >= minCapacity && idleSeconds >= TEXT_RETENTION_SECONDS )
        {
            arena.Used         = 0;
            m_currentTextArena = i;
            return arena;
        }
    }

    RetainedTextArena arena;
    arena.Capacity = std::max( minCapacity, m_textArenaCapacity );
    arena.Memory   = std::make_unique<char[]>( arena.Capacity );
    arena.LastUsed = now;
    m_textArenas.push_back( std::move( arena ) );
    m_currentTextArena = m_textArenas.size( ) - 1;
    return m_textArenas.back( );
}

const char *ClayContext::AppendRetainedText( const char *chars, const uint32_t length ) const
{
    RetainedTextArena *arena = m_currentTextArena < m_textArenas.size( ) ? &m_textArenas[ m_currentTextArena ] : nullptr;
    if ( arena == nullptr || arena->Used + length > arena->Capacity )
    {
        arena = &AcquireTextArena( length );
    }

    char *destination = arena->Memory.get( ) + arena->Used;
    memcpy( destination, chars, length );
    arena->Used += length;
    arena->LastUsed = std::chrono::steady_clock::now( );
    return destination;
}

void ClayContext::Text( const char *text, const DenOfIz_ClayTextDesc *desc ) const
{
    DZ_NOT_NULL( m_context );
    Text( DENOFIZ_STRING( text ), desc );
}

void ClayContext::Text( const DenOfIz_StringView &text, const DenOfIz_ClayTextDesc *desc ) const
{
    DZ_NOT_NULL( m_context );

    Clay_String clayText{ };
    clayText.isStaticallyAllocated = false;
    clayText.chars                 = text.Chars;
    clayText.length                = static_cast<int32_t>( text.NumChars );

    if ( clayText.length > 0 )
    {
        // Clay may reference this string beyond the current frame for example during exit-transition replay,
        clayText.chars = AppendRetainedText( text.Chars, static_cast<uint32_t>( text.NumChars ) );
    }

    Clay_TextElementConfig config = ConvertTextConfig( desc );
    config.fontSize               = static_cast<uint16_t>( config.fontSize * m_dpiScale );

    Clay__OpenTextElement( clayText, config );
}

void ClayContext::AddFont( const uint16_t fontId, Font *font ) const
{
    m_clayText->AddFont( fontId, font );
}

void ClayContext::RemoveFont( const uint16_t fontId ) const
{
    m_clayText->RemoveFont( fontId );
}

Font *ClayContext::GetFont( const uint16_t fontId ) const
{
    return m_clayText->GetFont( fontId );
}

void ClayContext::Texture( ITexture *texture, float width, float height ) const
{
    DZ_NOT_NULL( m_context );

    if ( texture == nullptr )
    {
        return;
    }

    DenOfIz_ClayElementDeclaration elem = DenOfIz_ClayElementDeclaration_Default( );
    elem.Custom.CustomData              = texture;

    if ( width > 0 && height > 0 )
    {
        elem.Layout.Sizing.Width  = DenOfIz_ClaySizingAxis_Fixed( width );
        elem.Layout.Sizing.Height = DenOfIz_ClaySizingAxis_Fixed( height );
    }

    OpenElement( &elem );
    CloseElement( );
}

uint32_t ClayContext::HashString( const char *str, const uint32_t index, const uint32_t baseId ) const
{
    return HashString( DENOFIZ_STRING( str ), index, baseId );
}

uint32_t ClayContext::HashString( const DenOfIz_StringView &str, const uint32_t index, const uint32_t baseId ) const
{
    Clay_String clayStr{ };
    clayStr.isStaticallyAllocated = false;
    clayStr.chars                 = str.Chars;
    clayStr.length                = static_cast<int32_t>( str.NumChars );

    const Clay_ElementId id = index == 0 ? Clay__HashString( clayStr, baseId ) : Clay__HashStringWithOffset( clayStr, index, baseId );
    return id.id;
}

bool ClayContext::PointerOver( const uint32_t id ) const
{
    DZ_NOT_NULL( m_context );
    Clay_ElementId elementId{ };
    elementId.id = id;
    return Clay_PointerOver( elementId );
}

DenOfIz_ClayBoundingBox ClayContext::GetElementBoundingBox( const uint32_t id ) const
{
    DZ_NOT_NULL( m_context );
    Clay_ElementId elementId{ };
    elementId.id                   = id;
    const Clay_ElementData data    = Clay_GetElementData( elementId );
    const Clay_BoundingBox clayBox = data.boundingBox;
    return DenOfIz_ClayBoundingBox{ clayBox.x, clayBox.y, clayBox.width, clayBox.height };
}

ClayTextCache *ClayContext::GetClayText( ) const
{
    return m_clayText.get( );
}

DenOfIz_ClayDimensions ClayContext::MeasureTextRaw( const DenOfIz_StringView &text, const uint16_t fontId, const uint16_t fontSize ) const
{
    if ( text.NumChars == 0 )
    {
        return DenOfIz_ClayDimensions{ 0, 0 };
    }

    Clay_TextElementConfig config{ };
    config.fontId   = fontId;
    config.fontSize = fontSize;
    return m_clayText->MeasureText( text, config );
}

DenOfIz_ClayDimensions ClayContext::MeasureText( const char *text, uint16_t fontId, uint16_t fontSize ) const
{
    return MeasureText( DENOFIZ_STRING( text ), fontId, fontSize );
}

DenOfIz_ClayDimensions ClayContext::MeasureText( const DenOfIz_StringView &text, const uint16_t fontId, const uint16_t fontSize ) const
{
    const uint16_t scaledFontSize = static_cast<uint16_t>( fontSize * m_dpiScale );
    return MeasureTextRaw( text, fontId, scaledFontSize );
}

Clay_RenderCommandArray ClayContext::EndLayoutAndGetCommands( const float deltaTime ) const
{
    DZ_NOT_NULL( m_context );
    return Clay_EndLayout( deltaTime );
}

Clay_LayoutDirection ClayContext::ConvertLayoutDirection( const DenOfIz_ClayLayoutDirection dir ) const
{
    switch ( dir )
    {
    case DENOFIZ_CLAY_LAYOUT_DIRECTION_LEFT_TO_RIGHT:
        return CLAY_LEFT_TO_RIGHT;
    case DENOFIZ_CLAY_LAYOUT_DIRECTION_TOP_TO_BOTTOM:
        return CLAY_TOP_TO_BOTTOM;
    default:
        return CLAY_LEFT_TO_RIGHT;
    }
}

Clay_LayoutAlignmentX ClayContext::ConvertAlignmentX( const DenOfIz_ClayAlignmentX align ) const
{
    switch ( align )
    {
    case DENOFIZ_CLAY_ALIGNMENT_X_LEFT:
        return CLAY_ALIGN_X_LEFT;
    case DENOFIZ_CLAY_ALIGNMENT_X_RIGHT:
        return CLAY_ALIGN_X_RIGHT;
    case DENOFIZ_CLAY_ALIGNMENT_X_CENTER:
        return CLAY_ALIGN_X_CENTER;
    default:
        return CLAY_ALIGN_X_LEFT;
    }
}

Clay_LayoutAlignmentY ClayContext::ConvertAlignmentY( const DenOfIz_ClayAlignmentY align ) const
{
    switch ( align )
    {
    case DENOFIZ_CLAY_ALIGNMENT_Y_TOP:
        return CLAY_ALIGN_Y_TOP;
    case DENOFIZ_CLAY_ALIGNMENT_Y_BOTTOM:
        return CLAY_ALIGN_Y_BOTTOM;
    case DENOFIZ_CLAY_ALIGNMENT_Y_CENTER:
        return CLAY_ALIGN_Y_CENTER;
    default:
        return CLAY_ALIGN_Y_TOP;
    }
}

Clay__SizingType ClayContext::ConvertSizingType( const DenOfIz_ClaySizingType type ) const
{
    switch ( type )
    {
    case DENOFIZ_CLAY_SIZING_TYPE_FIT:
        return CLAY__SIZING_TYPE_FIT;
    case DENOFIZ_CLAY_SIZING_TYPE_GROW:
        return CLAY__SIZING_TYPE_GROW;
    case DENOFIZ_CLAY_SIZING_TYPE_PERCENT:
        return CLAY__SIZING_TYPE_PERCENT;
    case DENOFIZ_CLAY_SIZING_TYPE_FIXED:
        return CLAY__SIZING_TYPE_FIXED;
    default:
        return CLAY__SIZING_TYPE_FIT;
    }
}

Clay_SizingAxis ClayContext::ConvertSizingAxis( const DenOfIz_ClaySizingAxis *axis, const bool enableDpiScaling ) const
{
    Clay_SizingAxis clayAxis{ };
    clayAxis.type = ConvertSizingType( axis->Type );

    if ( axis->Type == DENOFIZ_CLAY_SIZING_TYPE_PERCENT )
    {
        clayAxis.size.percent = axis->Size.Percent;
    }
    else
    {
        if ( enableDpiScaling )
        {
            clayAxis.size.minMax.min = PointsToPixels( axis->Size.MinMaxFloats.Min );
            clayAxis.size.minMax.max = PointsToPixels( axis->Size.MinMaxFloats.Max );
        }
        else
        {
            clayAxis.size.minMax.min = axis->Size.MinMaxFloats.Min;
            clayAxis.size.minMax.max = axis->Size.MinMaxFloats.Max;
        }
    }

    return clayAxis;
}

Clay_Sizing ClayContext::ConvertSizing( const DenOfIz_ClaySizing *sizing, const bool enableDpiScaling ) const
{
    return Clay_Sizing{ ConvertSizingAxis( &sizing->Width, enableDpiScaling ), ConvertSizingAxis( &sizing->Height, enableDpiScaling ) };
}

Clay_Padding ClayContext::ConvertPadding( const DenOfIz_ClayPadding *padding, const bool enableDpiScaling ) const
{
    if ( enableDpiScaling )
    {
        return Clay_Padding{ static_cast<uint16_t>( PointsToPixels( padding->Left ) ), static_cast<uint16_t>( PointsToPixels( padding->Right ) ),
                             static_cast<uint16_t>( PointsToPixels( padding->Top ) ), static_cast<uint16_t>( PointsToPixels( padding->Bottom ) ) };
    }
    else
    {
        return Clay_Padding{ static_cast<uint16_t>( padding->Left ), static_cast<uint16_t>( padding->Right ), static_cast<uint16_t>( padding->Top ),
                             static_cast<uint16_t>( padding->Bottom ) };
    }
}

Clay_ChildAlignment ClayContext::ConvertChildAlignment( const DenOfIz_ClayChildAlignment *alignment ) const
{
    return Clay_ChildAlignment{ ConvertAlignmentX( alignment->X ), ConvertAlignmentY( alignment->Y ) };
}

Clay_LayoutConfig ClayContext::ConvertLayoutConfig( const DenOfIz_ClayLayoutDesc *config, const bool enableDpiScaling ) const
{
    return Clay_LayoutConfig{ ConvertSizing( &config->Sizing, enableDpiScaling ), ConvertPadding( &config->Padding, enableDpiScaling ), config->ChildGap,
                              ConvertChildAlignment( &config->ChildAlignment ), ConvertLayoutDirection( config->LayoutDirection ) };
}

Clay_Color ClayContext::ConvertColor( const DenOfIz_ClayColor *color ) const
{
    return Clay_Color{ static_cast<float>( color->R ), static_cast<float>( color->G ), static_cast<float>( color->B ), static_cast<float>( color->A ) };
}

Clay_CornerRadius ClayContext::ConvertBorderRadius( const DenOfIz_ClayBorderRadius *radius, const bool enableDpiScaling ) const
{
    if ( enableDpiScaling )
    {
        return Clay_CornerRadius{ PointsToPixels( radius->TopLeft ), PointsToPixels( radius->TopRight ), PointsToPixels( radius->BottomLeft ),
                                  PointsToPixels( radius->BottomRight ) };
    }
    else
    {
        return Clay_CornerRadius{ radius->TopLeft, radius->TopRight, radius->BottomLeft, radius->BottomRight };
    }
}

Clay_BorderWidth ClayContext::ConvertBorderWidth( const DenOfIz_ClayBorderWidth *width, const bool enableDpiScaling ) const
{
    if ( enableDpiScaling )
    {
        return Clay_BorderWidth{ static_cast<uint16_t>( PointsToPixels( width->Left ) ), static_cast<uint16_t>( PointsToPixels( width->Right ) ),
                                 static_cast<uint16_t>( PointsToPixels( width->Top ) ), static_cast<uint16_t>( PointsToPixels( width->Bottom ) ),
                                 static_cast<uint16_t>( PointsToPixels( width->BetweenChildren ) ) };
    }
    return Clay_BorderWidth{ static_cast<uint16_t>( width->Left ), static_cast<uint16_t>( width->Right ), static_cast<uint16_t>( width->Top ),
                             static_cast<uint16_t>( width->Bottom ), static_cast<uint16_t>( width->BetweenChildren ) };
}

Clay_BorderElementConfig ClayContext::ConvertBorderConfig( const DenOfIz_ClayBorderDesc *config, const bool enableDpiScaling ) const
{
    Clay_BorderElementConfig result;
    result.width = ConvertBorderWidth( &config->Width, enableDpiScaling );
    result.color = ConvertColor( &config->Color );
    return result;
}

Clay_ImageElementConfig ClayContext::ConvertImageConfig( const DenOfIz_ClayImageDesc *config ) const
{
    Clay_ImageElementConfig result{ };
    result.imageData = config->ImageData;
    return result;
}

Clay_AspectRatioElementConfig ClayContext::ConvertAspectRatioConfig( const DenOfIz_ClayElementDeclaration *declaration ) const
{
    Clay_AspectRatioElementConfig result{ };
    if ( declaration->AspectRatio > 0.0f )
    {
        result.aspectRatio = declaration->AspectRatio;
    }
    else if ( declaration->Image.ImageData != nullptr && declaration->Image.SourceDimensions.Width > 0.0f && declaration->Image.SourceDimensions.Height > 0.0f )
    {
        result.aspectRatio = declaration->Image.SourceDimensions.Width / declaration->Image.SourceDimensions.Height;
    }
    return result;
}

Clay_FloatingAttachPointType ClayContext::ConvertFloatingAttachPoint( const DenOfIz_ClayFloatingAttachPoint point ) const
{
    switch ( point )
    {
    case DENOFIZ_CLAY_FLOATING_ATTACH_POINT_LEFT_TOP:
        return CLAY_ATTACH_POINT_LEFT_TOP;
    case DENOFIZ_CLAY_FLOATING_ATTACH_POINT_LEFT_CENTER:
        return CLAY_ATTACH_POINT_LEFT_CENTER;
    case DENOFIZ_CLAY_FLOATING_ATTACH_POINT_LEFT_BOTTOM:
        return CLAY_ATTACH_POINT_LEFT_BOTTOM;
    case DENOFIZ_CLAY_FLOATING_ATTACH_POINT_CENTER_TOP:
        return CLAY_ATTACH_POINT_CENTER_TOP;
    case DENOFIZ_CLAY_FLOATING_ATTACH_POINT_CENTER_CENTER:
        return CLAY_ATTACH_POINT_CENTER_CENTER;
    case DENOFIZ_CLAY_FLOATING_ATTACH_POINT_CENTER_BOTTOM:
        return CLAY_ATTACH_POINT_CENTER_BOTTOM;
    case DENOFIZ_CLAY_FLOATING_ATTACH_POINT_RIGHT_TOP:
        return CLAY_ATTACH_POINT_RIGHT_TOP;
    case DENOFIZ_CLAY_FLOATING_ATTACH_POINT_RIGHT_CENTER:
        return CLAY_ATTACH_POINT_RIGHT_CENTER;
    case DENOFIZ_CLAY_FLOATING_ATTACH_POINT_RIGHT_BOTTOM:
        return CLAY_ATTACH_POINT_RIGHT_BOTTOM;
    default:
        return CLAY_ATTACH_POINT_LEFT_TOP;
    }
}

Clay_FloatingAttachToElement ClayContext::ConvertFloatingAttachTo( const DenOfIz_ClayFloatingAttachTo attachTo ) const
{
    switch ( attachTo )
    {
    case DENOFIZ_CLAY_FLOATING_ATTACH_TO_NONE:
        return CLAY_ATTACH_TO_NONE;
    case DENOFIZ_CLAY_FLOATING_ATTACH_TO_PARENT:
        return CLAY_ATTACH_TO_PARENT;
    case DENOFIZ_CLAY_FLOATING_ATTACH_TO_ELEMENT_WITH_ID:
        return CLAY_ATTACH_TO_ELEMENT_WITH_ID;
    case DENOFIZ_CLAY_FLOATING_ATTACH_TO_ROOT:
        return CLAY_ATTACH_TO_ROOT;
    default:
        return CLAY_ATTACH_TO_NONE;
    }
}

Clay_FloatingElementConfig ClayContext::ConvertFloatingConfig( const DenOfIz_ClayFloatingDesc *config, const bool enableDpiScaling ) const
{
    Clay_FloatingElementConfig result{ };
    if ( enableDpiScaling )
    {
        result.offset.x      = PointsToPixels( config->Offset.X );
        result.offset.y      = PointsToPixels( config->Offset.Y );
        result.expand.width  = PointsToPixels( config->Expand.Width );
        result.expand.height = PointsToPixels( config->Expand.Height );
    }
    else
    {
        result.offset.x      = config->Offset.X;
        result.offset.y      = config->Offset.Y;
        result.expand.width  = config->Expand.Width;
        result.expand.height = config->Expand.Height;
    }
    result.zIndex               = config->ZIndex;
    result.parentId             = config->ParentId;
    result.attachPoints.element = ConvertFloatingAttachPoint( config->ElementAttachPoint );
    result.attachPoints.parent  = ConvertFloatingAttachPoint( config->ParentAttachPoint );
    result.attachTo             = ConvertFloatingAttachTo( config->AttachTo );
    result.clipTo               = config->ClipTo == DENOFIZ_CLAY_FLOATING_CLIP_TO_ATTACHED_PARENT ? CLAY_CLIP_TO_ATTACHED_PARENT : CLAY_CLIP_TO_NONE;
    result.pointerCaptureMode   = config->PointerCaptureMode == DENOFIZ_CLAY_POINTER_CAPTURE_MODE_CAPTURE ? CLAY_POINTER_CAPTURE_MODE_CAPTURE : CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH;
    return result;
}

Clay_ClipElementConfig ClayContext::ConvertClipConfig( const DenOfIz_ClayScrollDesc *scroll, const DenOfIz_ClayClipDesc *clip, const bool enableDpiScaling ) const
{
    Clay_ClipElementConfig result{ };
    result.horizontal = scroll->Horizontal || clip->Horizontal;
    result.vertical   = scroll->Vertical || clip->Vertical;

    if ( scroll->Horizontal || scroll->Vertical )
    {
        const Clay_Vector2 scrollOffset = Clay_GetScrollOffset( );
        if ( scroll->Horizontal )
        {
            result.childOffset.x = scrollOffset.x;
        }
        if ( scroll->Vertical )
        {
            result.childOffset.y = scrollOffset.y;
        }
    }

    result.childOffset.x += enableDpiScaling ? PointsToPixels( clip->ChildOffset.X ) : clip->ChildOffset.X;
    result.childOffset.y += enableDpiScaling ? PointsToPixels( clip->ChildOffset.Y ) : clip->ChildOffset.Y;
    return result;
}

Clay_TransitionElementConfig ClayContext::ConvertTransitionConfig( const DenOfIz_ClayTransitionDesc *config, const bool enableDpiScaling ) const
{
    Clay_TransitionElementConfig result{ };
    if ( !config->Enabled )
    {
        return result;
    }

    switch ( config->Easing )
    {
    case DENOFIZ_CLAY_TRANSITION_EASING_LINEAR:
        result.handler = LinearTransitionHandler;
        break;
    case DENOFIZ_CLAY_TRANSITION_EASING_EASE_IN:
        result.handler = EaseInTransitionHandler;
        break;
    case DENOFIZ_CLAY_TRANSITION_EASING_EASE_IN_OUT:
        result.handler = EaseInOutTransitionHandler;
        break;
    case DENOFIZ_CLAY_TRANSITION_EASING_EASE_OUT:
    default:
        result.handler = Clay_EaseOut;
        break;
    }

    result.duration            = config->Duration;
    result.properties          = static_cast<Clay_TransitionProperty>( config->Properties & DENOFIZ_CLAY_TRANSITION_PROPERTY_ALL_BIT );
    result.interactionHandling = config->InteractionHandling == DENOFIZ_CLAY_TRANSITION_INTERACTION_HANDLING_ALLOW_WHILE_MOVING
                                     ? CLAY_TRANSITION_ALLOW_INTERACTIONS_WHILE_TRANSITIONING_POSITION
                                     : CLAY_TRANSITION_DISABLE_INTERACTIONS_WHILE_TRANSITIONING_POSITION;

    result.enter.trigger = config->Enter.Trigger == DENOFIZ_CLAY_TRANSITION_ENTER_TRIGGER_ON_FIRST_PARENT_FRAME ? CLAY_TRANSITION_ENTER_TRIGGER_ON_FIRST_PARENT_FRAME
                                                                                                                : CLAY_TRANSITION_ENTER_SKIP_ON_FIRST_PARENT_FRAME;
    result.exit.trigger  = config->Exit.Trigger == DENOFIZ_CLAY_TRANSITION_EXIT_TRIGGER_WHEN_PARENT_EXITS ? CLAY_TRANSITION_EXIT_TRIGGER_WHEN_PARENT_EXITS
                                                                                                          : CLAY_TRANSITION_EXIT_SKIP_WHEN_PARENT_EXITS;
    switch ( config->Exit.SiblingOrdering )
    {
    case DENOFIZ_CLAY_EXIT_TRANSITION_SIBLING_ORDERING_NATURAL_ORDER:
        result.exit.siblingOrdering = CLAY_EXIT_TRANSITION_ORDERING_NATURAL_ORDER;
        break;
    case DENOFIZ_CLAY_EXIT_TRANSITION_SIBLING_ORDERING_ABOVE_SIBLINGS:
        result.exit.siblingOrdering = CLAY_EXIT_TRANSITION_ORDERING_ABOVE_SIBLINGS;
        break;
    case DENOFIZ_CLAY_EXIT_TRANSITION_SIBLING_ORDERING_UNDERNEATH_SIBLINGS:
    default:
        result.exit.siblingOrdering = CLAY_EXIT_TRANSITION_ORDERING_UNDERNEATH_SIBLINGS;
        break;
    }

    const float dpiScale = enableDpiScaling ? m_dpiScale : 1.0f;
    if ( config->Enter.Enabled )
    {
        const int32_t slot = AcquireTransitionStateSlot( g_enterStates, NormalizeTransitionState( config->Enter.State, dpiScale ) );
        if ( slot >= 0 )
        {
            result.enter.setInitialState = g_enterThunks[ slot ];
        }
    }
    if ( config->Exit.Enabled )
    {
        const int32_t slot = AcquireTransitionStateSlot( g_exitStates, NormalizeTransitionState( config->Exit.State, dpiScale ) );
        if ( slot >= 0 )
        {
            result.exit.setFinalState = g_exitThunks[ slot ];
        }
    }
    return result;
}

Clay_CustomElementConfig ClayContext::ConvertCustomConfig( const DenOfIz_ClayCustomDesc *config ) const
{
    return Clay_CustomElementConfig{ config->CustomData };
}

Clay_TextElementConfigWrapMode ClayContext::ConvertTextWrapMode( const DenOfIz_ClayTextWrapMode mode ) const
{
    switch ( mode )
    {
    case DENOFIZ_CLAY_TEXT_WRAP_MODE_WORDS:
        return CLAY_TEXT_WRAP_WORDS;
    case DENOFIZ_CLAY_TEXT_WRAP_MODE_NEWLINES:
        return CLAY_TEXT_WRAP_NEWLINES;
    case DENOFIZ_CLAY_TEXT_WRAP_MODE_NONE:
        return CLAY_TEXT_WRAP_NONE;
    default:
        return CLAY_TEXT_WRAP_WORDS;
    }
}

Clay_TextAlignment ClayContext::ConvertTextAlignment( const DenOfIz_ClayTextAlignment align ) const
{
    switch ( align )
    {
    case DENOFIZ_CLAY_TEXT_ALIGNMENT_LEFT:
        return CLAY_TEXT_ALIGN_LEFT;
    case DENOFIZ_CLAY_TEXT_ALIGNMENT_CENTER:
        return CLAY_TEXT_ALIGN_CENTER;
    case DENOFIZ_CLAY_TEXT_ALIGNMENT_RIGHT:
        return CLAY_TEXT_ALIGN_RIGHT;
    default:
        return CLAY_TEXT_ALIGN_LEFT;
    }
}

Clay_TextElementConfig ClayContext::ConvertTextConfig( const DenOfIz_ClayTextDesc *config ) const
{
    Clay_TextElementConfig result{ };
    result.userData      = nullptr;
    result.textColor     = ConvertColor( &config->TextColor );
    result.fontId        = config->FontId;
    result.fontSize      = config->FontSize;
    result.letterSpacing = config->LetterSpacing;
    result.lineHeight    = config->LineHeight;
    result.wrapMode      = ConvertTextWrapMode( config->WrapMode );
    result.textAlignment = ConvertTextAlignment( config->TextAlignment );
    return result;
}

DenOfIz_ClayRenderCommandType ClayContext::ConvertRenderCommandType( const Clay_RenderCommandType type ) const
{
    switch ( type )
    {
    case CLAY_RENDER_COMMAND_TYPE_NONE:
        return DENOFIZ_CLAY_RENDER_COMMAND_TYPE_NONE;
    case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
        return DENOFIZ_CLAY_RENDER_COMMAND_TYPE_RECTANGLE;
    case CLAY_RENDER_COMMAND_TYPE_BORDER:
        return DENOFIZ_CLAY_RENDER_COMMAND_TYPE_BORDER;
    case CLAY_RENDER_COMMAND_TYPE_TEXT:
        return DENOFIZ_CLAY_RENDER_COMMAND_TYPE_TEXT;
    case CLAY_RENDER_COMMAND_TYPE_IMAGE:
        return DENOFIZ_CLAY_RENDER_COMMAND_TYPE_IMAGE;
    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
        return DENOFIZ_CLAY_RENDER_COMMAND_TYPE_SCISSOR_START;
    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
        return DENOFIZ_CLAY_RENDER_COMMAND_TYPE_SCISSOR_END;
    case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_START:
        return DENOFIZ_CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_START;
    case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_END:
        return DENOFIZ_CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_END;
    case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
        return DENOFIZ_CLAY_RENDER_COMMAND_TYPE_CUSTOM;
    default:
        return DENOFIZ_CLAY_RENDER_COMMAND_TYPE_NONE;
    }
}

DenOfIz_ClayScrollContainerData ClayContext::GetScrollContainerData( const uint32_t id ) const
{
    DZ_NOT_NULL( m_context );

    DenOfIz_ClayScrollContainerData result{ };
    result.Found = false;

    Clay_ElementId elementId{ };
    elementId.id = id;

    const Clay_ScrollContainerData data = Clay_GetScrollContainerData( elementId );
    if ( data.found && data.scrollPosition != nullptr )
    {
        result.ScrollPosition.X     = data.scrollPosition->x;
        result.ScrollPosition.Y     = data.scrollPosition->y;
        result.ContainerSize.Width  = data.scrollContainerDimensions.width;
        result.ContainerSize.Height = data.scrollContainerDimensions.height;
        result.ContentSize.Width    = data.contentDimensions.width;
        result.ContentSize.Height   = data.contentDimensions.height;
        result.Found                = true;
    }

    return result;
}

void ClayContext::SetScrollPosition( const uint32_t id, const DenOfIz_Float2 position ) const
{
    DZ_NOT_NULL( m_context );

    Clay_ElementId elementId{ };
    elementId.id = id;

    const Clay_ScrollContainerData data = Clay_GetScrollContainerData( elementId );
    if ( data.found && data.scrollPosition != nullptr )
    {
        data.scrollPosition->x = position.X;
        data.scrollPosition->y = position.Y;
    }
}

bool ClayContext::ElementClicked( const uint32_t id ) const
{
    DZ_NOT_NULL( m_context );

    const bool isOver        = PointerOver( id );
    const bool wasPressed    = m_prevPointerState == DENOFIZ_CLAY_POINTER_STATE_PRESSED;
    const bool isNowReleased = m_pointerState == DENOFIZ_CLAY_POINTER_STATE_RELEASED;
    const bool justReleased  = wasPressed && isNowReleased;

    return isOver && justReleased;
}

bool ClayContext::ElementPressed( const uint32_t id ) const
{
    DZ_NOT_NULL( m_context );
    return PointerOver( id ) && m_pointerState == DENOFIZ_CLAY_POINTER_STATE_PRESSED;
}

float ClayContext::GetCursorOffsetAtIndex( const DenOfIz_StringView &text, const uint32_t charIndex, const uint16_t fontId, const uint16_t fontSize ) const
{
    if ( text.NumChars == 0 || charIndex == 0 )
    {
        return 0.0f;
    }

    const uint32_t               clampedIndex = charIndex > text.NumChars ? static_cast<uint32_t>( text.NumChars ) : charIndex;
    const DenOfIz_StringView     substring{ text.Chars, clampedIndex };
    const DenOfIz_ClayDimensions dims = MeasureText( substring, fontId, fontSize );
    return dims.Width;
}

uint32_t ClayContext::GetCharIndexAtOffset( const DenOfIz_StringView &text, const float pixelOffset, const uint16_t fontId, const uint16_t fontSize ) const
{
    if ( text.NumChars == 0 || pixelOffset <= 0.0f )
    {
        return 0;
    }

    float prevWidth = 0.0f;
    for ( uint32_t i = 1; i <= text.NumChars; ++i )
    {
        const DenOfIz_StringView     substring{ text.Chars, i };
        const DenOfIz_ClayDimensions dims = MeasureText( substring, fontId, fontSize );

        if ( dims.Width >= pixelOffset )
        {
            const float midPoint = prevWidth + ( dims.Width - prevWidth ) * 0.5f;
            return pixelOffset < midPoint ? i - 1 : i;
        }
        prevWidth = dims.Width;
    }

    return static_cast<uint32_t>( text.NumChars );
}
