#include <DenOfIzGraphics/UI/Clay.h>
#include <DenOfIzGraphics/UI/ClayClipboard.h>
#include <DenOfIzGraphics/UI/Widgets/CheckboxWidget.h>
#include <DenOfIzGraphics/UI/Widgets/ColorPickerWidget.h>
#include <DenOfIzGraphics/UI/Widgets/DockableContainerWidget.h>
#include <DenOfIzGraphics/UI/Widgets/DropdownWidget.h>
#include <DenOfIzGraphics/UI/Widgets/ResizableContainerWidget.h>
#include <DenOfIzGraphics/UI/Widgets/SliderWidget.h>
#include <DenOfIzGraphics/UI/Widgets/TextFieldWidget.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <glog/logging.h>
#include <unordered_map>

using namespace DenOfIz;

InteropString ClayTextFieldState::GetSelectedText( ) const
{
    if ( !HasSelection || SelectionStart == SelectionEnd || IsTextEmpty( ) )
    {
        return InteropString( "" );
    }

    size_t start = std::min( SelectionStart, SelectionEnd );
    size_t end   = std::max( SelectionStart, SelectionEnd );
    start        = std::min( start, GetTextLength( ) );
    end          = std::min( end, GetTextLength( ) );

    if ( start >= end )
    {
        return InteropString( "" );
    }

    return GetTextSubstring( start, end - start );
}

void ClayTextFieldState::ClearSelection( )
{
    HasSelection   = false;
    SelectionStart = 0;
    SelectionEnd   = 0;
}

void ClayTextFieldState::DeleteSelection( )
{
    if ( !HasSelection || SelectionStart == SelectionEnd )
    {
        return;
    }

    size_t start = std::min( SelectionStart, SelectionEnd );
    size_t end   = std::max( SelectionStart, SelectionEnd );
    start        = std::min( start, GetTextLength( ) );
    end          = std::min( end, GetTextLength( ) );

    if ( start < end )
    {
        EraseText( start, end - start );
        CursorPosition = start;
    }

    ClearSelection( );
}

bool ClayTextFieldState::IsTextEmpty( ) const
{
    return Text.IsEmpty( );
}

size_t ClayTextFieldState::GetTextLength( ) const
{
    return Text.NumChars( );
}

void ClayTextFieldState::InsertText( size_t position, const InteropString &text )
{
    std::string       current( Text.Get( ) );
    const std::string toInsert( text.Get( ) );

    if ( position > current.length( ) )
    {
        position = current.length( );
    }

    current.insert( position, toInsert );
    Text = InteropString( current.c_str( ) );
}

void ClayTextFieldState::EraseText( const size_t position, const size_t count )
{
    std::string current( Text.Get( ) );

    if ( position < current.length( ) )
    {
        current.erase( position, count );
        Text = InteropString( current.c_str( ) );
    }
}

InteropString ClayTextFieldState::GetTextSubstring( const size_t start, const size_t length ) const
{
    const std::string current( Text.Get( ) );

    if ( start >= current.length( ) )
    {
        return InteropString( "" );
    }

    return InteropString( current.substr( start, length ).c_str( ) );
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

ClayTextDesc::ClayTextDesc( ) : TextColor( 255, 255, 255, 255 )
{
}

ClayElementDeclaration::ClayElementDeclaration( ) : Id( 0 ), Image{ }, Floating{ }, Custom{ }, Scroll{ }, UserData( nullptr )
{
}

// ReSharper disable once CppPassValueParameterByConstReference used as callback below
// ReSharper disable once CppParameterMayBeConstPtrOrRef
Clay_Dimensions Clay::MeasureTextCallback( Clay_StringSlice text, Clay_TextElementConfig *config, void *userData )
{
    const auto *clay = static_cast<Clay *>( userData );
    DZ_NOT_NULL( clay );

    if ( text.length == 0 )
    {
        return Clay_Dimensions{ 0, 0 };
    }

    const InteropString  str( text.chars, static_cast<size_t>( text.length ) );
    const ClayDimensions dims = clay->m_renderer->MeasureText( str, *config );
    return Clay_Dimensions{ dims.Width, dims.Height };
}

// ReSharper disable once CppPassValueParameterByConstReference, used as callback below
static void ErrorHandler( Clay_ErrorData error )
{
    const InteropString errorText( error.errorText.chars );
    LOG( ERROR ) << "Clay error: " << errorText.Get( );
}

Clay_LayoutDirection Clay::ConvertLayoutDirection( const ClayLayoutDirection dir ) const
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

Clay_LayoutAlignmentX Clay::ConvertAlignmentX( const ClayAlignmentX align ) const
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

Clay_LayoutAlignmentY Clay::ConvertAlignmentY( const ClayAlignmentY align ) const
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

Clay__SizingType Clay::ConvertSizingType( const ClaySizingType type ) const
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

Clay_SizingAxis Clay::ConvertSizingAxis( const ClaySizingAxis &axis ) const
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

Clay_Sizing Clay::ConvertSizing( const ClaySizing &sizing ) const
{
    Clay_Sizing result;
    result.width  = ConvertSizingAxis( sizing.Width );
    result.height = ConvertSizingAxis( sizing.Height );
    return result;
}

Clay_Padding Clay::ConvertPadding( const ClayPadding &padding ) const
{
    Clay_Padding result;
    result.left   = padding.Left;
    result.right  = padding.Right;
    result.top    = padding.Top;
    result.bottom = padding.Bottom;
    return result;
}

Clay_ChildAlignment Clay::ConvertChildAlignment( const ClayChildAlignment &alignment ) const
{
    Clay_ChildAlignment result;
    result.x = ConvertAlignmentX( alignment.X );
    result.y = ConvertAlignmentY( alignment.Y );
    return result;
}

Clay_LayoutConfig Clay::ConvertLayoutConfig( const ClayLayoutDesc &config ) const
{
    Clay_LayoutConfig result;
    result.sizing          = ConvertSizing( config.Sizing );
    result.padding         = ConvertPadding( config.Padding );
    result.childGap        = config.ChildGap;
    result.childAlignment  = ConvertChildAlignment( config.ChildAlignment );
    result.layoutDirection = ConvertLayoutDirection( config.LayoutDirection );
    return result;
}

Clay_Color Clay::ConvertColor( const ClayColor &color ) const
{
    Clay_Color result;
    result.r = color.R;
    result.g = color.G;
    result.b = color.B;
    result.a = color.A;
    return result;
}

Clay_CornerRadius Clay::ConvertCornerRadius( const ClayCornerRadius &radius ) const
{
    Clay_CornerRadius result;
    result.topLeft     = radius.TopLeft;
    result.topRight    = radius.TopRight;
    result.bottomLeft  = radius.BottomLeft;
    result.bottomRight = radius.BottomRight;
    return result;
}

Clay_BorderWidth Clay::ConvertBorderWidth( const ClayBorderWidth &width ) const
{
    Clay_BorderWidth result;
    result.left            = width.Left;
    result.right           = width.Right;
    result.top             = width.Top;
    result.bottom          = width.Bottom;
    result.betweenChildren = width.BetweenChildren;
    return result;
}

Clay_BorderElementConfig Clay::ConvertBorderConfig( const ClayBorderDesc &config ) const
{
    Clay_BorderElementConfig result;
    result.color = ConvertColor( config.Color );
    result.width = ConvertBorderWidth( config.Width );
    return result;
}

Clay_ImageElementConfig Clay::ConvertImageConfig( const ClayImageDesc &config ) const
{
    Clay_ImageElementConfig result;
    result.imageData               = config.ImageData;
    result.sourceDimensions.width  = config.SourceDimensions.Width;
    result.sourceDimensions.height = config.SourceDimensions.Height;
    return result;
}

Clay_FloatingAttachPointType Clay::ConvertFloatingAttachPoint( const ClayFloatingAttachPoint point ) const
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

Clay_FloatingAttachToElement Clay::ConvertFloatingAttachTo( const ClayFloatingAttachTo attachTo ) const
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

Clay_FloatingElementConfig Clay::ConvertFloatingConfig( const ClayFloatingDesc &config ) const
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
    result.attachTo             = ConvertFloatingAttachTo( config.AttachTo );
    return result;
}

Clay_ScrollElementConfig Clay::ConvertScrollConfig( const ClayScrollDesc &config ) const
{
    Clay_ScrollElementConfig result{ };
    result.horizontal = config.Horizontal;
    result.vertical   = config.Vertical;
    return result;
}

Clay_CustomElementConfig Clay::ConvertCustomConfig( const ClayCustomDesc &config ) const
{
    Clay_CustomElementConfig result{ };
    result.customData = config.CustomData;
    return result;
}

Clay_TextElementConfigWrapMode Clay::ConvertTextWrapMode( const ClayTextWrapMode mode ) const
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

Clay_TextAlignment Clay::ConvertTextAlignment( const ClayTextAlignment align ) const
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

Clay_TextElementConfig Clay::ConvertTextConfig( const ClayTextDesc &config ) const
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

ClayRenderCommandType Clay::ConvertRenderCommandType( const Clay_RenderCommandType type ) const
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

Clay::Clay( const ClayDesc &desc )
{
    if ( desc.LogicalDevice == nullptr )
    {
        LOG( ERROR ) << "Clay::Clay Logical device is null";
        return;
    }

    if ( desc.Width == 0 || desc.Height == 0 )
    {
        LOG( ERROR ) << "Clay::Clay invalid dimensions provided: " << desc.Width << "x" << desc.Height;
        return;
    }

    TextRendererDesc textRendererDesc{ };
    textRendererDesc.LogicalDevice = desc.LogicalDevice;
    textRendererDesc.Width         = desc.Width;
    textRendererDesc.Height        = desc.Height;
    m_textRenderer                 = std::make_unique<TextRenderer>( textRendererDesc );

    ClayRendererDesc clayRendererDesc{ };
    clayRendererDesc.LogicalDevice      = desc.LogicalDevice;
    clayRendererDesc.RenderTargetFormat = desc.RenderTargetFormat;
    clayRendererDesc.NumFrames          = desc.NumFrames;
    clayRendererDesc.Width              = desc.Width;
    clayRendererDesc.Height             = desc.Height;

    m_renderer = std::make_unique<ClayRenderer>( clayRendererDesc );

    ClayContextDesc clayContextDesc{ };
    clayContextDesc.LogicalDevice                  = desc.LogicalDevice;
    clayContextDesc.Width                          = desc.Width;
    clayContextDesc.Height                         = desc.Height;
    clayContextDesc.MaxNumElements                 = desc.MaxNumElements;
    clayContextDesc.MaxNumTextMeasureCacheElements = desc.MaxNumTextMeasureCacheElements;
    m_clayContext                                  = std::make_unique<ClayContext>( clayContextDesc );
}

Clay::~Clay( ) = default;

uint16_t Clay::AddFont( Font *font ) const
{
    const uint16_t fontId = m_textRenderer->AddFont( font );
    if ( m_textRenderer->GetFont( 0 ) == nullptr && fontId != 0 )
    {
        m_textRenderer->AddFont( font, 0 );
    }
    return fontId;
}

void Clay::SetViewportSize( const float width, const float height ) const
{
    DZ_NOT_NULL( m_clayContext );
    m_clayContext->SetViewportSize( width, height );
    m_renderer->Resize( width, height );
}

ClayDimensions Clay::GetViewportSize( ) const
{
    DZ_NOT_NULL( m_clayContext );
    return m_clayContext->GetViewportSize( );
}

void Clay::SetDpiScale( const float dpiScale ) const
{
    m_renderer->SetDpiScale( dpiScale );
    m_clayContext->SetDpiScale( dpiScale );
}

void Clay::SetDebugModeEnabled( const bool enabled )
{
    DZ_NOT_NULL( m_clayContext );
    m_isDebugMode = enabled;
    m_clayContext->SetDebugModeEnabled( enabled );
}

bool Clay::IsDebugModeEnabled( ) const
{
    return m_isDebugMode;
}

void Clay::SetPointerState( const Float_2 position, const ClayPointerState state ) const
{
    DZ_NOT_NULL( m_clayContext );
    m_clayContext->SetPointerState( position, state );
}

void Clay::UpdateScrollContainers( const bool enableDragScrolling, const Float_2 scrollDelta, const float deltaTime )
{
    DZ_NOT_NULL( m_clayContext );

    Float_2 totalScrollDelta = scrollDelta;
    totalScrollDelta.X += m_scrollDelta.X;
    totalScrollDelta.Y += m_scrollDelta.Y;

    m_clayContext->UpdateScrollContainers( enableDragScrolling, totalScrollDelta, deltaTime );

    m_scrollDelta.X = 0;
    m_scrollDelta.Y = 0;
}

void Clay::BeginLayout( )
{
    m_time.Tick( );
    DZ_NOT_NULL( m_clayContext );
    SetPointerState( m_pointerPosition, m_pointerState );
    m_clayContext->SetDebugModeEnabled( m_isDebugMode );
    m_clayContext->BeginLayout( );
}

void Clay::EndLayout( ICommandList *commandList, const uint32_t frameIndex, const float deltaTime ) const
{
    DZ_NOT_NULL( m_clayContext );

    this->UpdateWidgets( deltaTime );

    m_renderer->SetDeltaTime( deltaTime );
    m_renderer->Render( commandList, m_clayContext->EndLayoutAndGetCommands( deltaTime ), frameIndex );
}

void Clay::OpenElement( const ClayElementDeclaration &declaration ) const
{
    DZ_NOT_NULL( m_clayContext );
    m_clayContext->OpenElement( declaration );
}

void Clay::CloseElement( ) const
{
    DZ_NOT_NULL( m_clayContext );
    m_clayContext->CloseElement( );
}

void Clay::Text( const InteropString &text, const ClayTextDesc &desc ) const
{
    DZ_NOT_NULL( m_clayContext );
    m_clayContext->Text( text, desc );
}

uint32_t Clay::HashString( const InteropString &str, const uint32_t index, const uint32_t baseId ) const
{
    return m_clayContext->HashString( str, index, baseId );
}

bool Clay::PointerOver( const uint32_t id ) const
{
    return m_clayContext->PointerOver( id );
}

ClayBoundingBox Clay::GetElementBoundingBox( const uint32_t id ) const
{
    return m_clayContext->GetElementBoundingBox( id );
}

void Clay::HandleEvent( const Event &event )
{
    if ( event.Type == EventType::MouseMotion )
    {
        m_pointerPosition = Float_2{ static_cast<float>( event.Motion.X ), static_cast<float>( event.Motion.Y ) };
    }

    if ( event.Type == EventType::MouseButtonDown )
    {
        if ( event.Button.Button == MouseButton::Left )
        {
            m_pointerPosition = Float_2{ static_cast<float>( event.Button.X ), static_cast<float>( event.Button.Y ) };
            m_pointerState    = ClayPointerState::Pressed;
        }
    }

    if ( event.Type == EventType::MouseButtonUp )
    {
        if ( event.Button.Button == MouseButton::Left )
        {
            m_pointerPosition = Float_2{ static_cast<float>( event.Button.X ), static_cast<float>( event.Button.Y ) };
            m_pointerState    = ClayPointerState::Released;
        }
    }

    if ( event.Type == EventType::WindowEvent )
    {
        const auto windowEvent = event.Window.Event;
        if ( windowEvent == WindowEventType::SizeChanged )
        {
            SetViewportSize( event.Window.Data1, event.Window.Data2 );
        }
    }

    if ( event.Type == EventType::KeyDown )
    {
        if ( event.Key.Keycode == KeyCode::F11 )
        {
            m_isDebugMode = !m_clayContext->IsDebugModeEnabled( );
        }
    }

    if ( event.Type == EventType::MouseWheel )
    {
        m_scrollDelta.X += static_cast<float>( event.Wheel.X ) * 30.0f;
        m_scrollDelta.Y += static_cast<float>( event.Wheel.Y ) * 30.0f;
    }

    // Forward event to all widgets
    for ( auto *widget : m_widgetUpdateOrder )
    {
        widget->HandleEvent( event );
    }
}

CheckboxWidget *Clay::CreateCheckbox( uint32_t id, bool initialChecked, const CheckboxStyle &style )
{
    auto            widget = std::make_unique<CheckboxWidget>( m_clayContext.get( ), id, initialChecked, style );
    CheckboxWidget *ptr    = widget.get( );
    m_widgets[ id ]        = std::move( widget );
    m_widgetUpdateOrder.push_back( ptr );
    m_renderer->RegisterWidget( id, ptr );
    return ptr;
}

SliderWidget *Clay::CreateSlider( uint32_t id, float initialValue, const SliderStyle &style )
{
    auto          widget = std::make_unique<SliderWidget>( m_clayContext.get( ), id, initialValue, style );
    SliderWidget *ptr    = widget.get( );
    m_widgets[ id ]      = std::move( widget );
    m_widgetUpdateOrder.push_back( ptr );
    m_renderer->RegisterWidget( id, ptr );
    return ptr;
}

DropdownWidget *Clay::CreateDropdown( uint32_t id, const InteropArray<InteropString> &options, const DropdownStyle &style )
{
    auto            widget = std::make_unique<DropdownWidget>( m_clayContext.get( ), id, options, style );
    DropdownWidget *ptr    = widget.get( );
    m_widgets[ id ]        = std::move( widget );
    m_widgetUpdateOrder.push_back( ptr );
    m_renderer->RegisterWidget( id, ptr );
    return ptr;
}

ColorPickerWidget *Clay::CreateColorPicker( uint32_t id, const Float_3 &initialRgb, const ColorPickerStyle &style )
{
    auto               widget = std::make_unique<ColorPickerWidget>( m_clayContext.get( ), id, initialRgb, style );
    ColorPickerWidget *ptr    = widget.get( );
    m_widgets[ id ]           = std::move( widget );
    m_widgetUpdateOrder.push_back( ptr );
    m_renderer->RegisterWidget( id, ptr );
    return ptr;
}

TextFieldWidget *Clay::CreateTextField( uint32_t id, const TextFieldStyle &style )
{
    auto             widget = std::make_unique<TextFieldWidget>( m_clayContext.get( ), id, style );
    TextFieldWidget *ptr    = widget.get( );
    m_widgets[ id ]         = std::move( widget );
    m_widgetUpdateOrder.push_back( ptr );
    m_renderer->RegisterWidget( id, ptr );
    return ptr;
}

ResizableContainerWidget *Clay::CreateResizableContainer( uint32_t id, const ResizableContainerStyle &style )
{
    auto                      widget = std::make_unique<ResizableContainerWidget>( m_clayContext.get( ), id, style );
    ResizableContainerWidget *ptr    = widget.get( );
    m_widgets[ id ]                  = std::move( widget );
    m_widgetUpdateOrder.push_back( ptr );
    m_renderer->RegisterWidget( id, ptr );
    return ptr;
}

DockableContainerWidget *Clay::CreateDockableContainer( uint32_t id, DockingManager *dockingManager, const DockableContainerStyle &style )
{
    auto                     widget = std::make_unique<DockableContainerWidget>( m_clayContext.get( ), id, dockingManager, style );
    DockableContainerWidget *ptr    = widget.get( );
    m_widgets[ id ]                 = std::move( widget );
    m_widgetUpdateOrder.push_back( ptr );
    m_renderer->RegisterWidget( id, ptr );
    return ptr;
}

Widget *Clay::GetWidget( const uint32_t id ) const
{
    const auto it = m_widgets.find( id );
    if ( it != m_widgets.end( ) )
    {
        return it->second.get( );
    }
    return nullptr;
}

void Clay::RemoveWidget( const uint32_t id )
{
    const auto it = m_widgets.find( id );
    if ( it != m_widgets.end( ) )
    {
        Widget *widget = it->second.get( );
        std::erase( m_widgetUpdateOrder, widget );
        m_widgets.erase( it );
    }
}

std::unique_ptr<DockingManager> Clay::CreateDockingManager( )
{
    return std::make_unique<DockingManager>( m_clayContext.get( ) );
}

void Clay::UpdateWidgets( const float deltaTime ) const
{
    for ( auto *widget : m_widgetUpdateOrder )
    {
        widget->Update( deltaTime );
    }
}

ClayDimensions Clay::MeasureText( const InteropString &text, const uint16_t fontId, const uint16_t fontSize ) const
{
    return m_clayContext->MeasureText( text, fontId, fontSize );
}
