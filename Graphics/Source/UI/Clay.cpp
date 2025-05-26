#include <DenOfIzGraphics/UI/Clay.h>
#include <cstring>
#include <glog/logging.h>
#include <unordered_map>

#define CLAY_IMPLEMENTATION
#include "clay.h"

using namespace DenOfIz;

namespace DenOfIz
{
    // Equality operator for ClayTextDesc
    bool operator==( const ClayTextDesc &lhs, const ClayTextDesc &rhs )
    {
        return lhs.FontId == rhs.FontId &&
               lhs.FontSize == rhs.FontSize &&
               lhs.LetterSpacing == rhs.LetterSpacing &&
               lhs.LineHeight == rhs.LineHeight &&
               lhs.TextColor.R == rhs.TextColor.R &&
               lhs.TextColor.G == rhs.TextColor.G &&
               lhs.TextColor.B == rhs.TextColor.B &&
               lhs.TextColor.A == rhs.TextColor.A &&
               lhs.WrapMode == rhs.WrapMode &&
               lhs.TextAlignment == rhs.TextAlignment &&
               lhs.HashStringContents == rhs.HashStringContents;
    }
}

namespace std
{
    template<>
    struct hash<DenOfIz::ClayTextDesc>
    {
        size_t operator()( const DenOfIz::ClayTextDesc &desc ) const noexcept
        {
            size_t h1 = hash<uint32_t>{}( desc.FontId );
            size_t h2 = hash<uint32_t>{}( desc.FontSize );
            size_t h3 = hash<float>{}( desc.LetterSpacing );
            size_t h4 = hash<float>{}( desc.LineHeight );
            size_t h5 = hash<uint8_t>{}( desc.TextColor.R );
            size_t h6 = hash<uint8_t>{}( desc.TextColor.G );
            size_t h7 = hash<uint8_t>{}( desc.TextColor.B );
            size_t h8 = hash<uint8_t>{}( desc.TextColor.A );
            size_t h9 = hash<int>{}( static_cast<int>( desc.WrapMode ) );
            size_t h10 = hash<int>{}( static_cast<int>( desc.TextAlignment ) );
            size_t h11 = hash<bool>{}( desc.HashStringContents );

            // Combine hashes using XOR with shifts
            return h1 ^ ( h2 << 1 ) ^ ( h3 << 2 ) ^ ( h4 << 3 ) ^ ( h5 << 4 ) ^ 
                   ( h6 << 5 ) ^ ( h7 << 6 ) ^ ( h8 << 7 ) ^ ( h9 << 8 ) ^ 
                   ( h10 << 9 ) ^ ( h11 << 10 );
        }
    };
}

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

ClayElementDeclaration::ClayElementDeclaration( ) : Id( 0 ), Image{ }, Floating{ }, Custom{ }, Scroll{ }, UserData( nullptr )
{
}

struct Clay::Impl
{
    ClayRenderer            *renderer;
    Clay_Arena               arena;
    Clay_Context            *context;
    std::vector<uint8_t>     memory;
    Clay                    *wrapper;
    std::vector<std::string> frameStrings;
    std::unordered_map<ClayTextDesc, Clay_TextElementConfig> textConfigCache;

    // ReSharper disable once CppPassValueParameterByConstReference used as callback below
    static Clay_Dimensions MeasureTextCallback( Clay_StringSlice text, Clay_TextElementConfig *config, void *userData )
    {
        const auto *impl = static_cast<Impl *>( userData );
        DZ_NOT_NULL( impl );

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

        const ClayDimensions dims = impl->renderer->MeasureText( str, *config );
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

    const Clay_TextElementConfig* GetOrCreateCachedTextConfig( const ClayTextDesc &desc )
    {
        auto it = textConfigCache.find( desc );
        if ( it != textConfigCache.end( ) )
        {
            return &it->second;
        }
        
        // Create new config and cache it
        Clay_TextElementConfig newConfig = ConvertTextConfig( desc );
        auto result = textConfigCache.emplace( desc, newConfig );
        return &result.first->second;
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
};

Clay::Clay( const ClayWrapperDesc &desc ) : m_impl( std::make_unique<Impl>( ) ), m_initialized( false )
{
    ClayRendererDesc clayRendererDesc{ };
    clayRendererDesc.LogicalDevice      = desc.LogicalDevice;
    clayRendererDesc.TextRenderer       = desc.TextRenderer;
    clayRendererDesc.RenderTargetFormat = desc.RenderTargetFormat;
    clayRendererDesc.NumFrames          = desc.NumFrames;
    clayRendererDesc.MaxNumQuads        = desc.MaxNumQuads;
    clayRendererDesc.MaxNumMaterials    = desc.MaxNumMaterials;
    clayRendererDesc.Width              = desc.Width;
    clayRendererDesc.Height             = desc.Height;

    m_renderer      = std::make_unique<ClayRenderer>( clayRendererDesc );
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
    m_impl->renderer = m_renderer.get( );
}

Clay::~Clay( )
{
    m_impl->context = nullptr;
    m_impl->memory.clear( );
}

void Clay::SetLayoutDimensions( const float width, const float height ) const
{
    DZ_NOT_NULL( m_impl->context );
    Clay_SetLayoutDimensions( Clay_Dimensions{ width, height } );
    m_renderer->Resize( width, height );
}

void Clay::SetPointerState( const Float_2 position, const ClayPointerState state ) const
{
    DZ_NOT_NULL( m_impl->context );
    Clay_SetPointerState( Clay_Vector2{ position.X, position.Y }, state == ClayPointerState::Pressed );
}

void Clay::UpdateScrollContainers( const bool enableDragScrolling, const Float_2 scrollDelta, const float deltaTime ) const
{
    DZ_NOT_NULL( m_impl->context );
    Clay_UpdateScrollContainers( enableDragScrolling, Clay_Vector2{ scrollDelta.X, scrollDelta.Y }, deltaTime );
}

void Clay::BeginLayout( ) const
{
    DZ_NOT_NULL( m_impl->context );
    Clay_BeginLayout( );
}

void Clay::EndLayout( ICommandList *commandList, const uint32_t frameIndex ) const
{
    DZ_NOT_NULL( m_impl->context );
    // m_impl->frameStrings.clear( );
    m_renderer->Render( commandList, Clay_EndLayout( ), frameIndex );
}

void Clay::OpenElement( const ClayElementDeclaration &declaration ) const
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

void Clay::CloseElement( ) const
{
    DZ_NOT_NULL( m_impl->context );
    Clay__CloseElement( );
}

void Clay::Text( const InteropString &text, const ClayTextDesc &desc ) const
{
    DZ_NOT_NULL( m_impl->context );

    Clay_String clayText;
    clayText.chars  = text.Get( );
    clayText.length = text.NumChars( );

    const Clay_TextElementConfig* clayConfig = m_impl->GetOrCreateCachedTextConfig( desc );
    Clay__OpenTextElement( clayText, const_cast<Clay_TextElementConfig*>( clayConfig ) );
}

uint32_t Clay::HashString( const InteropString &str, const uint32_t index, const uint32_t baseId ) const
{
    Clay_String clayStr;
    clayStr.chars  = str.Get( );
    clayStr.length = static_cast<int32_t>( strlen( str.Get( ) ) );

    const Clay_ElementId id = Clay__HashString( clayStr, index, baseId );
    return id.id;
}

bool Clay::PointerOver( const uint32_t id ) const
{
    DZ_NOT_NULL( m_impl->context );
    const Clay_ElementId clayId{ id, 0, 0, Clay_String{} };
    return Clay_PointerOver( clayId );
}

ClayBoundingBox Clay::GetElementBoundingBox( const uint32_t id ) const
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
