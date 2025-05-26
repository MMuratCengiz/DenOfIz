#include <DenOfIzGraphics/UI/ClayWrapper.h>
#include <cstring>
#include <glog/logging.h>

#define CLAY_IMPLEMENTATION
#include "clay.h"

using namespace DenOfIz;

ClaySizingAxis ClaySizingAxis::Fit( const float min, const float max )
{
    ClaySizingAxis axis{ };
    axis.Type            = ClaySizingType::Fit;
    axis.Size.MinMax.Min = min;
    axis.Size.MinMax.Max = max;
    return axis;
}

ClaySizingAxis ClaySizingAxis::Grow( const float min, const float max )
{
    ClaySizingAxis axis{ };
    axis.Type            = ClaySizingType::Grow;
    axis.Size.MinMax.Min = min;
    axis.Size.MinMax.Max = max;
    return axis;
}

ClaySizingAxis ClaySizingAxis::Fixed( const float size )
{
    ClaySizingAxis axis;
    axis.Type            = ClaySizingType::Fixed;
    axis.Size.MinMax.Min = size;
    axis.Size.MinMax.Max = size;
    return axis;
}

ClaySizingAxis ClaySizingAxis::Percent( const float percent )
{
    ClaySizingAxis axis;
    axis.Type         = ClaySizingType::Percent;
    axis.Size.Percent = percent;
    return axis;
}

ClayLayoutDesc::ClayLayoutDesc( ) : Sizing{ }, ChildGap( 0 ), LayoutDirection( ClayLayoutDirection::LeftToRight )
{
    Sizing.Width  = ClaySizingAxis::Fit( );
    Sizing.Height = ClaySizingAxis::Fit( );
}

ClayTextDesc::ClayTextDesc( ) :
    TextColor( 255, 255, 255, 255 ), FontId( 0 ), FontSize( 16 ), LetterSpacing( 0 ), LineHeight( 0 ), WrapMode( ClayTextWrapMode::Words ),
    TextAlignment( ClayTextAlignment::Left ), HashStringContents( false )
{
}

ClayElementDeclaration::ClayElementDeclaration( ) :
    Id( 0 ), Image{ }, Floating{ }, Custom{ }, Scroll{ }, UserData( nullptr )
{
}

struct ClayWrapper::Impl
{
    Clay_Arena           arena;
    Clay_Context        *context;
    std::vector<uint8_t> memory;
    MeasureTextFunction  measureTextFunc;
    ClayWrapper         *wrapper;

    // ReSharper disable once CppPassValueParameterByConstReference used as callback below
    static Clay_Dimensions MeasureTextCallback( Clay_StringSlice text, Clay_TextElementConfig *config, void *userData )
    {
        const auto *impl = static_cast<Impl *>( userData );
        DZ_NOT_NULL( impl );

        if ( !impl->measureTextFunc )
        {
            return Clay_Dimensions{ 0, 0 };
        }
        const std::string   tempStr( text.chars, text.length );
        const InteropString str( tempStr.c_str( ) );

        ClayTextDesc textConfig;
        textConfig.FontId             = config->fontId;
        textConfig.FontSize           = config->fontSize;
        textConfig.LetterSpacing      = config->letterSpacing;
        textConfig.LineHeight         = config->lineHeight;
        textConfig.TextColor          = ClayColor( config->textColor.r, config->textColor.g, config->textColor.b, config->textColor.a );
        textConfig.WrapMode           = static_cast<ClayTextWrapMode>( config->wrapMode );
        textConfig.TextAlignment      = static_cast<ClayTextAlignment>( config->textAlignment );
        textConfig.HashStringContents = config->hashStringContents;

        const ClayDimensions dims = impl->measureTextFunc( str, textConfig );
        return Clay_Dimensions{ dims.Width, dims.Height };
    }

    // ReSharper disable once CppPassValueParameterByConstReference, used as callback below
    static void ErrorHandler( Clay_ErrorData error )
    {
        const InteropString errorText( error.errorText.chars );
        LOG( ERROR ) << "Clay error: " << errorText.Get( );
    }

    Clay_LayoutDirection ConvertLayoutDirection( const ClayLayoutDirection dir ) const
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

    Clay_LayoutAlignmentX ConvertAlignmentX( const ClayAlignmentX align ) const
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

    Clay_LayoutAlignmentY ConvertAlignmentY( const ClayAlignmentY align ) const
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

    Clay__SizingType ConvertSizingType( const ClaySizingType type ) const
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

    Clay_SizingAxis ConvertSizingAxis( const ClaySizingAxis &axis ) const
    {
        Clay_SizingAxis result;
        result.type = ConvertSizingType( axis.Type );

        if ( axis.Type == ClaySizingType::Percent )
        {
            result.size.percent = axis.Size.Percent;
        }
        else
        {
            result.size.minMax.min = axis.Size.MinMax.Min;
            result.size.minMax.max = axis.Size.MinMax.Max;
        }
        return result;
    }

    Clay_Sizing ConvertSizing( const ClaySizing &sizing ) const
    {
        Clay_Sizing result;
        result.width  = ConvertSizingAxis( sizing.Width );
        result.height = ConvertSizingAxis( sizing.Height );
        return result;
    }

    Clay_Padding ConvertPadding( const ClayPadding &padding ) const
    {
        Clay_Padding result;
        result.left   = padding.Left;
        result.right  = padding.Right;
        result.top    = padding.Top;
        result.bottom = padding.Bottom;
        return result;
    }

    Clay_ChildAlignment ConvertChildAlignment( const ClayChildAlignment &alignment ) const
    {
        Clay_ChildAlignment result;
        result.x = ConvertAlignmentX( alignment.X );
        result.y = ConvertAlignmentY( alignment.Y );
        return result;
    }

    Clay_LayoutConfig ConvertLayoutConfig( const ClayLayoutDesc &config ) const
    {
        Clay_LayoutConfig result;
        result.sizing          = ConvertSizing( config.Sizing );
        result.padding         = ConvertPadding( config.Padding );
        result.childGap        = config.ChildGap;
        result.childAlignment  = ConvertChildAlignment( config.ChildAlignment );
        result.layoutDirection = ConvertLayoutDirection( config.LayoutDirection );
        return result;
    }

    Clay_Color ConvertColor( const ClayColor &color ) const
    {
        Clay_Color result;
        result.r = color.R;
        result.g = color.G;
        result.b = color.B;
        result.a = color.A;
        return result;
    }

    Clay_CornerRadius ConvertCornerRadius( const ClayCornerRadius &radius ) const
    {
        Clay_CornerRadius result;
        result.topLeft     = radius.TopLeft;
        result.topRight    = radius.TopRight;
        result.bottomLeft  = radius.BottomLeft;
        result.bottomRight = radius.BottomRight;
        return result;
    }

    Clay_BorderWidth ConvertBorderWidth( const ClayBorderWidth &width ) const
    {
        Clay_BorderWidth result;
        result.left            = width.Left;
        result.right           = width.Right;
        result.top             = width.Top;
        result.bottom          = width.Bottom;
        result.betweenChildren = width.BetweenChildren;
        return result;
    }

    Clay_BorderElementConfig ConvertBorderConfig( const ClayBorderDesc &config ) const
    {
        Clay_BorderElementConfig result;
        result.color = ConvertColor( config.Color );
        result.width = ConvertBorderWidth( config.Width );
        return result;
    }

    Clay_ImageElementConfig ConvertImageConfig( const ClayImageDesc &config ) const
    {
        Clay_ImageElementConfig result;
        result.imageData               = config.ImageData;
        result.sourceDimensions.width  = config.SourceDimensions.Width;
        result.sourceDimensions.height = config.SourceDimensions.Height;
        return result;
    }

    Clay_FloatingAttachPointType ConvertFloatingAttachPoint( const ClayFloatingAttachPoint point ) const
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

    Clay_FloatingElementConfig ConvertFloatingConfig( const ClayFloatingDesc &config ) const
    {
        Clay_FloatingElementConfig result{ };
        result.offset.x             = config.Offset.X;
        result.offset.y             = config.Offset.Y;
        result.expand.width         = config.Expand.Width;
        result.expand.height        = config.Expand.Height;
        result.zIndex               = config.ZIndex;
        result.parentId             = config.ParentId;
        result.attachPoints.element = ConvertFloatingAttachPoint( config.ElementAttachPoint );
        result.attachPoints.parent  = ConvertFloatingAttachPoint( config.ParentAttachPoint );
        return result;
    }

    Clay_ScrollElementConfig ConvertScrollConfig( const ClayScrollDesc &config ) const
    {
        Clay_ScrollElementConfig result{ };
        result.horizontal = config.Horizontal;
        result.vertical   = config.Vertical;
        return result;
    }

    Clay_CustomElementConfig ConvertCustomConfig( const ClayCustomDesc &config ) const
    {
        Clay_CustomElementConfig result{ };
        result.customData = config.CustomData;
        return result;
    }

    Clay_TextElementConfigWrapMode ConvertTextWrapMode( const ClayTextWrapMode mode ) const
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

    Clay_TextAlignment ConvertTextAlignment( const ClayTextAlignment align ) const
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

    Clay_TextElementConfig ConvertTextConfig( const ClayTextDesc &config ) const
    {
        Clay_TextElementConfig result;
        result.textColor          = ConvertColor( config.TextColor );
        result.fontId             = config.FontId;
        result.fontSize           = config.FontSize;
        result.letterSpacing      = config.LetterSpacing;
        result.lineHeight         = config.LineHeight;
        result.wrapMode           = ConvertTextWrapMode( config.WrapMode );
        result.textAlignment      = ConvertTextAlignment( config.TextAlignment );
        result.hashStringContents = config.HashStringContents;
        return result;
    }

    ClayRenderCommandType ConvertRenderCommandType( const Clay_RenderCommandType type ) const
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

    ClayRenderCommand ConvertRenderCommand( const Clay_RenderCommand &cmd ) const
    {
        ClayRenderCommand result{ };
        result.BoundingBox.X      = cmd.boundingBox.x;
        result.BoundingBox.Y      = cmd.boundingBox.y;
        result.BoundingBox.Width  = cmd.boundingBox.width;
        result.BoundingBox.Height = cmd.boundingBox.height;
        result.CommandType        = ConvertRenderCommandType( cmd.commandType );
        result.UserData           = cmd.userData;
        result.Id                 = cmd.id;
        result.ZIndex             = cmd.zIndex;

        switch ( cmd.commandType )
        {
        case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
            {
                result.RenderData.Rectangle.BackgroundColor          = ClayColor( cmd.renderData.rectangle.backgroundColor.r, cmd.renderData.rectangle.backgroundColor.g,
                                                                                  cmd.renderData.rectangle.backgroundColor.b, cmd.renderData.rectangle.backgroundColor.a );
                result.RenderData.Rectangle.CornerRadius.TopLeft     = cmd.renderData.rectangle.cornerRadius.topLeft;
                result.RenderData.Rectangle.CornerRadius.TopRight    = cmd.renderData.rectangle.cornerRadius.topRight;
                result.RenderData.Rectangle.CornerRadius.BottomLeft  = cmd.renderData.rectangle.cornerRadius.bottomLeft;
                result.RenderData.Rectangle.CornerRadius.BottomRight = cmd.renderData.rectangle.cornerRadius.bottomRight;
                break;
            }
        case CLAY_RENDER_COMMAND_TYPE_TEXT:
            {
                auto contents                         = std::string( cmd.renderData.text.stringContents.chars, cmd.renderData.text.stringContents.length );
                result.RenderData.Text.StringContents = InteropString( contents.c_str( ) );
                result.RenderData.Text.TextColor =
                    ClayColor( cmd.renderData.text.textColor.r, cmd.renderData.text.textColor.g, cmd.renderData.text.textColor.b, cmd.renderData.text.textColor.a );
                result.RenderData.Text.FontId        = cmd.renderData.text.fontId;
                result.RenderData.Text.FontSize      = cmd.renderData.text.fontSize;
                result.RenderData.Text.LetterSpacing = cmd.renderData.text.letterSpacing;
                result.RenderData.Text.LineHeight    = cmd.renderData.text.lineHeight;
                break;
            }
        case CLAY_RENDER_COMMAND_TYPE_IMAGE:
            {
                result.RenderData.Image.BackgroundColor          = ClayColor( cmd.renderData.image.backgroundColor.r, cmd.renderData.image.backgroundColor.g,
                                                                              cmd.renderData.image.backgroundColor.b, cmd.renderData.image.backgroundColor.a );
                result.RenderData.Image.CornerRadius.TopLeft     = cmd.renderData.image.cornerRadius.topLeft;
                result.RenderData.Image.CornerRadius.TopRight    = cmd.renderData.image.cornerRadius.topRight;
                result.RenderData.Image.CornerRadius.BottomLeft  = cmd.renderData.image.cornerRadius.bottomLeft;
                result.RenderData.Image.CornerRadius.BottomRight = cmd.renderData.image.cornerRadius.bottomRight;
                result.RenderData.Image.SourceDimensions.Width   = cmd.renderData.image.sourceDimensions.width;
                result.RenderData.Image.SourceDimensions.Height  = cmd.renderData.image.sourceDimensions.height;
                result.RenderData.Image.ImageData                = cmd.renderData.image.imageData;
                break;
            }
        case CLAY_RENDER_COMMAND_TYPE_BORDER:
            {
                result.RenderData.Border.Color =
                    ClayColor( cmd.renderData.border.color.r, cmd.renderData.border.color.g, cmd.renderData.border.color.b, cmd.renderData.border.color.a );
                result.RenderData.Border.CornerRadius.TopLeft     = cmd.renderData.border.cornerRadius.topLeft;
                result.RenderData.Border.CornerRadius.TopRight    = cmd.renderData.border.cornerRadius.topRight;
                result.RenderData.Border.CornerRadius.BottomLeft  = cmd.renderData.border.cornerRadius.bottomLeft;
                result.RenderData.Border.CornerRadius.BottomRight = cmd.renderData.border.cornerRadius.bottomRight;
                result.RenderData.Border.Width.Left               = cmd.renderData.border.width.left;
                result.RenderData.Border.Width.Right              = cmd.renderData.border.width.right;
                result.RenderData.Border.Width.Top                = cmd.renderData.border.width.top;
                result.RenderData.Border.Width.Bottom             = cmd.renderData.border.width.bottom;
                result.RenderData.Border.Width.BetweenChildren    = cmd.renderData.border.width.betweenChildren;
                break;
            }
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
            {
                result.RenderData.Scroll.Horizontal = cmd.renderData.scroll.horizontal;
                result.RenderData.Scroll.Vertical   = cmd.renderData.scroll.vertical;
                break;
            }
        case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
            {
                result.RenderData.Custom.BackgroundColor          = ClayColor( cmd.renderData.custom.backgroundColor.r, cmd.renderData.custom.backgroundColor.g,
                                                                               cmd.renderData.custom.backgroundColor.b, cmd.renderData.custom.backgroundColor.a );
                result.RenderData.Custom.CornerRadius.TopLeft     = cmd.renderData.custom.cornerRadius.topLeft;
                result.RenderData.Custom.CornerRadius.TopRight    = cmd.renderData.custom.cornerRadius.topRight;
                result.RenderData.Custom.CornerRadius.BottomLeft  = cmd.renderData.custom.cornerRadius.bottomLeft;
                result.RenderData.Custom.CornerRadius.BottomRight = cmd.renderData.custom.cornerRadius.bottomRight;
                result.RenderData.Custom.CustomData               = cmd.renderData.custom.customData;
                break;
            }
        default:
            break;
        }

        return result;
    }
};

ClayWrapper::ClayWrapper( const ClayWrapperDesc &desc ) : m_impl( std::make_unique<Impl>( ) ), m_initialized( false )
{
    m_impl->wrapper = this;
    Clay_SetMaxElementCount( desc.MaxNumElements );
    Clay_SetMaxMeasureTextCacheWordCount( desc.MaxNumTextMeasureCacheElements );

    const uint32_t minMemorySize = Clay_MinMemorySize( );
    m_impl->memory.resize( minMemorySize );
    m_impl->arena = Clay_CreateArenaWithCapacityAndMemory( minMemorySize, m_impl->memory.data( ) );

    Clay_ErrorHandler errorHandler;
    errorHandler.errorHandlerFunction = Impl::ErrorHandler;
    errorHandler.userData             = m_impl.get( );

    m_impl->context = Clay_Initialize( m_impl->arena, Clay_Dimensions{ static_cast<float>( desc.Width ), static_cast<float>( desc.Height ) }, errorHandler );
    if ( !m_impl->context )
    {
        LOG( ERROR ) << "Failed to initialize Clay: ";
    }

    Clay_SetMeasureTextFunction( Impl::MeasureTextCallback, m_impl.get( ) );
}

ClayWrapper::~ClayWrapper( )
{
    m_impl->context = nullptr;
    m_impl->memory.clear( );
}

void ClayWrapper::SetLayoutDimensions( const float width, const float height ) const
{
    DZ_NOT_NULL( m_impl->context );
    Clay_SetLayoutDimensions( Clay_Dimensions{ width, height } );
}

void ClayWrapper::SetPointerState( const Float_2 position, const ClayPointerState state ) const
{
    DZ_NOT_NULL( m_impl->context );

    const bool pointerDown = ( state == ClayPointerState::Pressed || state == ClayPointerState::PressedThisFrame );
    Clay_SetPointerState( Clay_Vector2{ position.X, position.Y }, pointerDown );
}

void ClayWrapper::UpdateScrollContainers( const bool enableDragScrolling, const Float_2 scrollDelta, const float deltaTime ) const
{
    DZ_NOT_NULL( m_impl->context );
    Clay_UpdateScrollContainers( enableDragScrolling, Clay_Vector2{ scrollDelta.X, scrollDelta.Y }, deltaTime );
}

void ClayWrapper::BeginLayout( ) const
{
    DZ_NOT_NULL( m_impl->context );
    Clay_BeginLayout( );
}

InteropArray<ClayRenderCommand> ClayWrapper::EndLayout( ) const
{
    DZ_NOT_NULL( m_impl->context );

    Clay_RenderCommandArray clayCommands = Clay_EndLayout( );

    InteropArray<ClayRenderCommand> commands;
    commands.Resize( clayCommands.length );

    for ( int32_t i = 0; i < clayCommands.length; ++i )
    {
        if ( const Clay_RenderCommand *clayCmd = Clay_RenderCommandArray_Get( &clayCommands, i ) )
        {
            commands.SetElement( i, m_impl->ConvertRenderCommand( *clayCmd ) );
        }
    }

    return commands;
}

void ClayWrapper::OpenElement( const ClayElementDeclaration &declaration ) const
{
    DZ_NOT_NULL( m_impl->context );

    Clay__OpenElement( );

    Clay_ElementDeclaration clayDecl;
    clayDecl.id              = Clay_ElementId{ declaration.Id, 0, 0, Clay_String{} };
    clayDecl.layout          = m_impl->ConvertLayoutConfig( declaration.Layout );
    clayDecl.backgroundColor = m_impl->ConvertColor( declaration.BackgroundColor );
    clayDecl.cornerRadius    = m_impl->ConvertCornerRadius( declaration.CornerRadius );
    clayDecl.image           = m_impl->ConvertImageConfig( declaration.Image );
    clayDecl.floating        = m_impl->ConvertFloatingConfig( declaration.Floating );
    clayDecl.custom          = m_impl->ConvertCustomConfig( declaration.Custom );
    clayDecl.scroll          = m_impl->ConvertScrollConfig( declaration.Scroll );
    clayDecl.border          = m_impl->ConvertBorderConfig( declaration.Border );
    clayDecl.userData        = declaration.UserData;

    Clay__ConfigureOpenElement( clayDecl );
}

void ClayWrapper::CloseElement( ) const
{
    DZ_NOT_NULL( m_impl->context );
    Clay__CloseElement( );
}

void ClayWrapper::Text( const InteropString &text, const ClayTextDesc &desc ) const
{
    DZ_NOT_NULL( m_impl->context );

    Clay_String clayText;
    clayText.chars  = text.Get( );
    clayText.length = static_cast<int32_t>( strlen( text.Get( ) ) );

    Clay_TextElementConfig clayConfig = m_impl->ConvertTextConfig( desc );

    Clay__OpenTextElement( clayText, &clayConfig );
}

uint32_t ClayWrapper::HashString( const InteropString &str, const uint32_t index, const uint32_t baseId ) const
{
    Clay_String clayStr;
    clayStr.chars  = str.Get( );
    clayStr.length = static_cast<int32_t>( strlen( str.Get( ) ) );

    const Clay_ElementId id = Clay__HashString( clayStr, index, baseId );
    return id.id;
}

void ClayWrapper::SetMeasureTextFunction( const MeasureTextFunction &func ) const
{
    m_impl->measureTextFunc = func;
}

bool ClayWrapper::PointerOver( const uint32_t id ) const
{
    DZ_NOT_NULL( m_impl->context );
    const Clay_ElementId clayId{ id, 0, 0, Clay_String{} };
    return Clay_PointerOver( clayId );
}

ClayBoundingBox ClayWrapper::GetElementBoundingBox( const uint32_t id ) const
{
    DZ_NOT_NULL( m_impl->context );

    const Clay_ElementId   clayId{ id, 0, 0, Clay_String{} };
    const Clay_ElementData data = Clay_GetElementData( clayId );

    ClayBoundingBox result;
    result.X      = data.boundingBox.x;
    result.Y      = data.boundingBox.y;
    result.Width  = data.boundingBox.width;
    result.Height = data.boundingBox.height;

    return result;
}
