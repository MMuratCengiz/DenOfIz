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

using namespace DenOfIz;

// ReSharper disable once CppPassValueParameterByConstReference used as callback below
// ReSharper disable once CppParameterMayBeConstPtrOrRef
Clay_Dimensions ClayContext::MeasureTextCallback( Clay_StringSlice text, Clay_TextElementConfig *config, void *userData )
{
    const auto *context = static_cast<ClayContext *>( userData );
    DZ_NOT_NULL( context );

    if ( text.length == 0 )
    {
        return Clay_Dimensions{ 0, 0 };
    }

    const InteropString  str( text.chars, static_cast<size_t>( text.length ) );
    const ClayDimensions dims = context->MeasureText( str, config->fontId, config->fontSize );
    return Clay_Dimensions{ dims.Width, dims.Height };
}

// ReSharper disable once CppPassValueParameterByConstReference, used as callback below
static void ErrorHandler( Clay_ErrorData error )
{
    const InteropString errorText( error.errorText.chars );
    spdlog::error( "Clay error: {}", errorText.Get( ) );
}

ClayContext::ClayContext( const ClayContextDesc &desc )
{
    if ( desc.LogicalDevice == nullptr )
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
    Clay_BeginLayout( );
}

void ClayContext::SetViewportSize( const float width, const float height ) const
{
    DZ_NOT_NULL( m_context );
    Clay_SetLayoutDimensions( Clay_Dimensions{ width, height } );
}

ClayDimensions ClayContext::GetViewportSize( ) const
{
    DZ_NOT_NULL( m_context );
    const Clay_Dimensions dimensions = Clay_GetCurrentContext( )->layoutDimensions;
    return ClayDimensions{ dimensions.width, dimensions.height };
}

void ClayContext::SetDpiScale( const float dpiScale )
{
    m_clayText->SetDpiScale( dpiScale );
    m_dpiScale = dpiScale;
    Clay_ResetMeasureTextCache( );
}

void ClayContext::SetPointerState( const Float_2 position, const ClayPointerState state )
{
    DZ_NOT_NULL( m_context );
    m_pointerPosition = position;
    m_pointerState    = state;
    Clay_SetPointerState( Clay_Vector2{ position.X, position.Y }, state == ClayPointerState::Pressed );
}

void ClayContext::UpdateScrollContainers( const bool enableDragScrolling, const Float_2 scrollDelta, const float deltaTime )
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

void ClayContext::OpenElement( const ClayElementDeclaration &declaration ) const
{
    DZ_NOT_NULL( m_context );

    Clay__OpenElement( );

    Clay_ElementDeclaration clayDecl;
    clayDecl.id              = Clay_ElementId{ declaration.Id, 0, 0, Clay_String{} };
    clayDecl.layout          = ConvertLayoutConfig( declaration.Layout );
    clayDecl.backgroundColor = ConvertColor( declaration.BackgroundColor );
    clayDecl.cornerRadius    = ConvertCornerRadius( declaration.CornerRadius );
    clayDecl.image           = ConvertImageConfig( declaration.Image );
    clayDecl.floating        = ConvertFloatingConfig( declaration.Floating );
    clayDecl.custom          = ConvertCustomConfig( declaration.Custom );
    clayDecl.scroll          = ConvertScrollConfig( declaration.Scroll );
    clayDecl.border          = ConvertBorderConfig( declaration.Border );
    clayDecl.userData        = nullptr;

    Clay__ConfigureOpenElement( clayDecl );
}

void ClayContext::CloseElement( ) const
{
    DZ_NOT_NULL( m_context );
    Clay__CloseElement( );
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

void ClayContext::Text( const InteropString &text, const ClayTextDesc &desc ) const
{
    DZ_NOT_NULL( m_context );

    Clay_String tempString;
    tempString.chars  = text.Get( );
    tempString.length = static_cast<int>( text.NumChars( ) );

    const Clay_String clayText = Clay__WriteStringToCharBuffer( &Clay_GetCurrentContext( )->dynamicStringData, tempString );

    const Clay_TextElementConfig tempConfig   = ConvertTextConfig( desc );
    Clay_TextElementConfig      *storedConfig = Clay__StoreTextElementConfig( tempConfig );

    Clay__OpenTextElement( clayText, storedConfig );
}

uint32_t ClayContext::HashString( const InteropString &str, const uint32_t index, const uint32_t baseId ) const
{
    Clay_String clayStr;
    clayStr.chars  = str.Get( );
    clayStr.length = static_cast<int32_t>( strlen( str.Get( ) ) );

    const Clay_ElementId id = Clay__HashString( clayStr, index, baseId );
    return id.id;
}

bool ClayContext::PointerOver( const uint32_t id ) const
{
    DZ_NOT_NULL( m_context );
    Clay_ElementId elementId;
    elementId.id = id;
    return Clay_PointerOver( elementId );
}

ClayBoundingBox ClayContext::GetElementBoundingBox( const uint32_t id ) const
{
    DZ_NOT_NULL( m_context );
    Clay_ElementId elementId;
    elementId.id                   = id;
    const Clay_ElementData data    = Clay_GetElementData( elementId );
    const Clay_BoundingBox clayBox = data.boundingBox;
    return ClayBoundingBox{ clayBox.x, clayBox.y, clayBox.width, clayBox.height };
}

ClayTextCache *ClayContext::GetClayText( ) const
{
    return m_clayText.get( );
}

ClayDimensions ClayContext::MeasureText( const InteropString &text, const uint16_t fontId, const uint16_t fontSize ) const
{
    if ( text.IsEmpty( ) )
    {
        return ClayDimensions{ 0, 0 };
    }

    Clay_TextElementConfig config{ };
    config.fontId   = fontId;
    config.fontSize = fontSize;
    return m_clayText->MeasureText( text, config );
}

Clay_RenderCommandArray ClayContext::EndLayoutAndGetCommands( const float deltaTime ) const
{
    DZ_NOT_NULL( m_context );
    return Clay_EndLayout( );
}

// Type conversion methods
Clay_LayoutDirection ClayContext::ConvertLayoutDirection( const ClayLayoutDirection dir ) const
{
    switch ( dir )
    {
    case ClayLayoutDirection::LeftToRight:
        return CLAY_LEFT_TO_RIGHT;
    case ClayLayoutDirection::TopToBottom:
        return CLAY_TOP_TO_BOTTOM;
    default:
        return CLAY_LEFT_TO_RIGHT;
    }
}

Clay_LayoutAlignmentX ClayContext::ConvertAlignmentX( const ClayAlignmentX align ) const
{
    switch ( align )
    {
    case ClayAlignmentX::Left:
        return CLAY_ALIGN_X_LEFT;
    case ClayAlignmentX::Right:
        return CLAY_ALIGN_X_RIGHT;
    case ClayAlignmentX::Center:
        return CLAY_ALIGN_X_CENTER;
    default:
        return CLAY_ALIGN_X_LEFT;
    }
}

Clay_LayoutAlignmentY ClayContext::ConvertAlignmentY( const ClayAlignmentY align ) const
{
    switch ( align )
    {
    case ClayAlignmentY::Top:
        return CLAY_ALIGN_Y_TOP;
    case ClayAlignmentY::Bottom:
        return CLAY_ALIGN_Y_BOTTOM;
    case ClayAlignmentY::Center:
        return CLAY_ALIGN_Y_CENTER;
    default:
        return CLAY_ALIGN_Y_TOP;
    }
}

Clay__SizingType ClayContext::ConvertSizingType( const ClaySizingType type ) const
{
    switch ( type )
    {
    case ClaySizingType::Fit:
        return CLAY__SIZING_TYPE_FIT;
    case ClaySizingType::Grow:
        return CLAY__SIZING_TYPE_GROW;
    case ClaySizingType::Percent:
        return CLAY__SIZING_TYPE_PERCENT;
    case ClaySizingType::Fixed:
        return CLAY__SIZING_TYPE_FIXED;
    default:
        return CLAY__SIZING_TYPE_FIT;
    }
}

Clay_SizingAxis ClayContext::ConvertSizingAxis( const ClaySizingAxis &axis ) const
{
    Clay_SizingAxis clayAxis{ };
    clayAxis.type = ConvertSizingType( axis.Type );

    if ( axis.Type == ClaySizingType::Percent )
    {
        clayAxis.size.percent = axis.Size.Percent;
    }
    else
    {
        clayAxis.size.minMax.min = axis.Size.MinMax.Min;
        clayAxis.size.minMax.max = axis.Size.MinMax.Max;
    }

    return clayAxis;
}

Clay_Sizing ClayContext::ConvertSizing( const ClaySizing &sizing ) const
{
    return Clay_Sizing{ ConvertSizingAxis( sizing.Width ), ConvertSizingAxis( sizing.Height ) };
}

Clay_Padding ClayContext::ConvertPadding( const ClayPadding &padding ) const
{
    return Clay_Padding{ padding.Left, padding.Right, padding.Top, padding.Bottom };
}

Clay_ChildAlignment ClayContext::ConvertChildAlignment( const ClayChildAlignment &alignment ) const
{
    return Clay_ChildAlignment{ ConvertAlignmentX( alignment.X ), ConvertAlignmentY( alignment.Y ) };
}

Clay_LayoutConfig ClayContext::ConvertLayoutConfig( const ClayLayoutDesc &config ) const
{
    return Clay_LayoutConfig{ ConvertSizing( config.Sizing ), ConvertPadding( config.Padding ), config.ChildGap, ConvertChildAlignment( config.ChildAlignment ),
                              ConvertLayoutDirection( config.LayoutDirection ) };
}

Clay_Color ClayContext::ConvertColor( const ClayColor &color ) const
{
    return Clay_Color{ color.R, color.G, color.B, color.A };
}

Clay_CornerRadius ClayContext::ConvertCornerRadius( const ClayCornerRadius &radius ) const
{
    return Clay_CornerRadius{ radius.TopLeft, radius.TopRight, radius.BottomLeft, radius.BottomRight };
}

Clay_BorderWidth ClayContext::ConvertBorderWidth( const ClayBorderWidth &width ) const
{
    return Clay_BorderWidth{ width.Left, width.Right, width.Top, width.Bottom, width.BetweenChildren };
}

Clay_BorderElementConfig ClayContext::ConvertBorderConfig( const ClayBorderDesc &config ) const
{
    Clay_BorderElementConfig result;
    result.width = ConvertBorderWidth( config.Width );
    result.color = ConvertColor( config.Color );
    return result;
}

Clay_ImageElementConfig ClayContext::ConvertImageConfig( const ClayImageDesc &config ) const
{
    return Clay_ImageElementConfig{ config.ImageData, Clay_Dimensions{ config.SourceDimensions.Width, config.SourceDimensions.Height } };
}

Clay_FloatingAttachPointType ClayContext::ConvertFloatingAttachPoint( const ClayFloatingAttachPoint point ) const
{
    switch ( point )
    {
    case ClayFloatingAttachPoint::LeftTop:
        return CLAY_ATTACH_POINT_LEFT_TOP;
    case ClayFloatingAttachPoint::LeftCenter:
        return CLAY_ATTACH_POINT_LEFT_CENTER;
    case ClayFloatingAttachPoint::LeftBottom:
        return CLAY_ATTACH_POINT_LEFT_BOTTOM;
    case ClayFloatingAttachPoint::CenterTop:
        return CLAY_ATTACH_POINT_CENTER_TOP;
    case ClayFloatingAttachPoint::CenterCenter:
        return CLAY_ATTACH_POINT_CENTER_CENTER;
    case ClayFloatingAttachPoint::CenterBottom:
        return CLAY_ATTACH_POINT_CENTER_BOTTOM;
    case ClayFloatingAttachPoint::RightTop:
        return CLAY_ATTACH_POINT_RIGHT_TOP;
    case ClayFloatingAttachPoint::RightCenter:
        return CLAY_ATTACH_POINT_RIGHT_CENTER;
    case ClayFloatingAttachPoint::RightBottom:
        return CLAY_ATTACH_POINT_RIGHT_BOTTOM;
    default:
        return CLAY_ATTACH_POINT_LEFT_TOP;
    }
}

Clay_FloatingAttachToElement ClayContext::ConvertFloatingAttachTo( const ClayFloatingAttachTo attachTo ) const
{
    switch ( attachTo )
    {
    case ClayFloatingAttachTo::None:
        return CLAY_ATTACH_TO_NONE;
    case ClayFloatingAttachTo::Parent:
        return CLAY_ATTACH_TO_PARENT;
    case ClayFloatingAttachTo::ElementWithId:
        return CLAY_ATTACH_TO_ELEMENT_WITH_ID;
    case ClayFloatingAttachTo::Root:
        return CLAY_ATTACH_TO_ROOT;
    default:
        return CLAY_ATTACH_TO_NONE;
    }
}

Clay_FloatingElementConfig ClayContext::ConvertFloatingConfig( const ClayFloatingDesc &config ) const
{
    Clay_FloatingElementConfig result{ };
    result.offset.x             = config.Offset.X;
    result.offset.y             = config.Offset.Y;
    result.expand.width         = config.Expand.Width;
    result.expand.height        = config.Expand.Height;
    result.zIndex               = config.ZIndex;
    result.parentId             = static_cast<int16_t>( config.ParentId );
    result.attachPoints.element = ConvertFloatingAttachPoint( config.ElementAttachPoint );
    result.attachPoints.parent  = ConvertFloatingAttachPoint( config.ParentAttachPoint );
    result.attachTo             = ConvertFloatingAttachTo( config.AttachTo );
    result.pointerCaptureMode   = CLAY_POINTER_CAPTURE_MODE_CAPTURE;
    return result;
}

Clay_ScrollElementConfig ClayContext::ConvertScrollConfig( const ClayScrollDesc &config ) const
{
    return Clay_ScrollElementConfig{ config.Horizontal, config.Vertical };
}

Clay_CustomElementConfig ClayContext::ConvertCustomConfig( const ClayCustomDesc &config ) const
{
    return Clay_CustomElementConfig{ config.CustomData };
}

Clay_TextElementConfigWrapMode ClayContext::ConvertTextWrapMode( const ClayTextWrapMode mode ) const
{
    switch ( mode )
    {
    case ClayTextWrapMode::Words:
        return CLAY_TEXT_WRAP_WORDS;
    case ClayTextWrapMode::Newlines:
        return CLAY_TEXT_WRAP_NEWLINES;
    case ClayTextWrapMode::None:
        return CLAY_TEXT_WRAP_NONE;
    default:
        return CLAY_TEXT_WRAP_WORDS;
    }
}

Clay_TextAlignment ClayContext::ConvertTextAlignment( const ClayTextAlignment align ) const
{
    switch ( align )
    {
    case ClayTextAlignment::Left:
        return CLAY_TEXT_ALIGN_LEFT;
    case ClayTextAlignment::Center:
        return CLAY_TEXT_ALIGN_CENTER;
    case ClayTextAlignment::Right:
        return CLAY_TEXT_ALIGN_RIGHT;
    default:
        return CLAY_TEXT_ALIGN_LEFT;
    }
}

Clay_TextElementConfig ClayContext::ConvertTextConfig( const ClayTextDesc &config ) const
{
    return Clay_TextElementConfig{ ConvertColor( config.TextColor ),
                                   config.FontId,
                                   config.FontSize,
                                   config.LetterSpacing,
                                   config.LineHeight,
                                   ConvertTextWrapMode( config.WrapMode ),
                                   ConvertTextAlignment( config.TextAlignment ) };
}

ClayRenderCommandType ClayContext::ConvertRenderCommandType( const Clay_RenderCommandType type ) const
{
    switch ( type )
    {
    case CLAY_RENDER_COMMAND_TYPE_NONE:
        return ClayRenderCommandType::None;
    case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
        return ClayRenderCommandType::Rectangle;
    case CLAY_RENDER_COMMAND_TYPE_BORDER:
        return ClayRenderCommandType::Border;
    case CLAY_RENDER_COMMAND_TYPE_TEXT:
        return ClayRenderCommandType::Text;
    case CLAY_RENDER_COMMAND_TYPE_IMAGE:
        return ClayRenderCommandType::Image;
    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
        return ClayRenderCommandType::ScissorStart;
    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
        return ClayRenderCommandType::ScissorEnd;
    case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
        return ClayRenderCommandType::Custom;
    default:
        return ClayRenderCommandType::None;
    }
}
