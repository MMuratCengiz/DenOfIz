#include <DenOfIzGraphics/Assets/Font/Embedded/EmbeddedFonts.h>
#include <DenOfIzGraphics/Assets/Font/FontLibrary.h>
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

    ClayContextDesc clayContextDesc{ };
    clayContextDesc.LogicalDevice                  = desc.LogicalDevice;
    clayContextDesc.Width                          = desc.Width;
    clayContextDesc.Height                         = desc.Height;
    clayContextDesc.MaxNumElements                 = desc.MaxNumElements;
    clayContextDesc.MaxNumTextMeasureCacheElements = desc.MaxNumTextMeasureCacheElements;
    m_clayContext                                  = std::make_unique<ClayContext>( clayContextDesc );

    ClayRendererDesc clayRendererDesc{ };
    clayRendererDesc.LogicalDevice      = desc.LogicalDevice;
    clayRendererDesc.ClayContext        = m_clayContext.get( );
    clayRendererDesc.RenderTargetFormat = desc.RenderTargetFormat;
    clayRendererDesc.NumFrames          = desc.NumFrames;
    clayRendererDesc.Width              = desc.Width;
    clayRendererDesc.Height             = desc.Height;
    clayRendererDesc.MaxPipelineWidgets = desc.MaxPipelineWidgets;

    m_renderer = std::make_unique<ClayRenderer>( clayRendererDesc );
}

Clay::~Clay( ) = default;

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

    for ( auto *widget : m_widgetUpdateOrder )
    {
        widget->HandleEvent( event );
    }
}

CheckboxWidget *Clay::CreateCheckbox( uint32_t id, bool initialChecked, const CheckboxStyle &style )
{
    auto            widget = std::make_unique<CheckboxWidget>( m_clayContext.get( ), id, initialChecked, style );
    CheckboxWidget *ptr    = widget.get( );
    m_ownedWidgets[ id ]   = std::move( widget );
    m_widgetUpdateOrder.push_back( ptr );
    m_renderer->RegisterWidget( id, ptr );
    return ptr;
}

SliderWidget *Clay::CreateSlider( uint32_t id, float initialValue, const SliderStyle &style )
{
    auto          widget = std::make_unique<SliderWidget>( m_clayContext.get( ), id, initialValue, style );
    SliderWidget *ptr    = widget.get( );
    m_ownedWidgets[ id ] = std::move( widget );
    m_widgetUpdateOrder.push_back( ptr );
    m_renderer->RegisterWidget( id, ptr );
    return ptr;
}

DropdownWidget *Clay::CreateDropdown( uint32_t id, const InteropArray<InteropString> &options, const DropdownStyle &style )
{
    auto            widget = std::make_unique<DropdownWidget>( m_clayContext.get( ), id, options, style );
    DropdownWidget *ptr    = widget.get( );
    m_ownedWidgets[ id ]   = std::move( widget );
    m_widgetUpdateOrder.push_back( ptr );
    m_renderer->RegisterWidget( id, ptr );
    return ptr;
}

ColorPickerWidget *Clay::CreateColorPicker( uint32_t id, const Float_3 &initialRgb, const ColorPickerStyle &style )
{
    auto               widget = std::make_unique<ColorPickerWidget>( m_clayContext.get( ), id, initialRgb, style );
    ColorPickerWidget *ptr    = widget.get( );
    m_ownedWidgets[ id ]      = std::move( widget );
    m_widgetUpdateOrder.push_back( ptr );
    m_renderer->RegisterWidget( id, ptr );
    return ptr;
}

TextFieldWidget *Clay::CreateTextField( uint32_t id, const TextFieldStyle &style )
{
    auto             widget = std::make_unique<TextFieldWidget>( m_clayContext.get( ), id, style );
    TextFieldWidget *ptr    = widget.get( );
    m_ownedWidgets[ id ]    = std::move( widget );
    m_widgetUpdateOrder.push_back( ptr );
    m_renderer->RegisterWidget( id, ptr );
    return ptr;
}

ResizableContainerWidget *Clay::CreateResizableContainer( uint32_t id, const ResizableContainerStyle &style )
{
    auto                      widget = std::make_unique<ResizableContainerWidget>( m_clayContext.get( ), id, style );
    ResizableContainerWidget *ptr    = widget.get( );
    m_ownedWidgets[ id ]             = std::move( widget );
    m_widgetUpdateOrder.push_back( ptr );
    m_renderer->RegisterWidget( id, ptr );
    return ptr;
}

DockableContainerWidget *Clay::CreateDockableContainer( uint32_t id, DockingManager *dockingManager, const DockableContainerStyle &style )
{
    auto                     widget = std::make_unique<DockableContainerWidget>( m_clayContext.get( ), id, dockingManager, style );
    DockableContainerWidget *ptr    = widget.get( );
    m_ownedWidgets[ id ]            = std::move( widget );
    m_widgetUpdateOrder.push_back( ptr );
    m_renderer->RegisterWidget( id, ptr );
    return ptr;
}

Widget *Clay::GetWidget( const uint32_t id ) const
{
    const auto ownedIt = m_ownedWidgets.find( id );
    if ( ownedIt != m_ownedWidgets.end( ) )
    {
        return ownedIt->second.get( );
    }
    const auto externalIt = m_externalWidgets.find( id );
    if ( externalIt != m_externalWidgets.end( ) )
    {
        return externalIt->second;
    }
    return nullptr;
}

void Clay::RemoveWidget( const uint32_t id )
{
    Widget    *widget  = nullptr;
    const auto ownedIt = m_ownedWidgets.find( id );
    if ( ownedIt != m_ownedWidgets.end( ) )
    {
        widget = ownedIt->second.get( );
        m_ownedWidgets.erase( ownedIt );
    }
    else
    {
        const auto externalIt = m_externalWidgets.find( id );
        if ( externalIt != m_externalWidgets.end( ) )
        {
            widget = externalIt->second;
            m_externalWidgets.erase( externalIt );
        }
    }

    if ( widget )
    {
        std::erase( m_widgetUpdateOrder, widget );
        m_renderer->UnregisterWidget( id );
    }
}

DockingManager *Clay::CreateDockingManager( ) const
{
    return new DockingManager( m_clayContext.get( ) );
}

void Clay::UpdateWidgets( const float deltaTime ) const
{
    for ( auto *widget : m_widgetUpdateOrder )
    {
        widget->Update( deltaTime );
    }
}

void Clay::RegisterPipelineWidget( Widget *widget )
{
    DZ_NOT_NULL( widget );

    const uint32_t id       = widget->GetId( );
    m_externalWidgets[ id ] = widget;
    m_widgetUpdateOrder.push_back( widget );
    m_renderer->RegisterWidget( id, widget );
}

IClayContext *Clay::GetContext( ) const
{
    return m_clayContext.get( );
}

ClayDimensions Clay::MeasureText( const InteropString &text, const uint16_t fontId, const uint16_t fontSize ) const
{
    return m_clayContext->MeasureText( text, fontId, fontSize );
}

void Clay::AddFont( const uint16_t fontId, Font *font ) const
{
    m_clayContext->AddFont( fontId, font );
    m_renderer->Render( nullptr, Clay_RenderCommandArray{ }, 0 );
}

void Clay::RemoveFont( const uint16_t fontId ) const
{
    m_clayContext->RemoveFont( fontId );
}
