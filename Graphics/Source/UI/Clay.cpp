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
#include <algorithm>
#include <cmath>
#include <cstring>
#include <unordered_map>
#include "DenOfIzGraphics/Assets/Font/FontLibrary.h"
#include "DenOfIzGraphics/UI/ClayClipboard.h"
#include "DenOfIzGraphics/UI/Widgets/CheckboxWidget.h"
#include "DenOfIzGraphics/UI/Widgets/ColorPickerWidget.h"
#include "DenOfIzGraphics/UI/Widgets/DockableContainerWidget.h"
#include "DenOfIzGraphics/UI/Widgets/DropdownWidget.h"
#include "DenOfIzGraphics/UI/Widgets/ResizableContainerWidget.h"
#include "DenOfIzGraphics/UI/Widgets/SliderWidget.h"
#include "DenOfIzGraphics/UI/Widgets/TextFieldWidget.h"
#include "DenOfIzGraphics/Utilities/Time.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Embedded/EmbeddedFonts.h"
#include "DenOfIzGraphicsInternal/UI/ClayContext.h"
#include "DenOfIzGraphicsInternal/UI/ClayRenderer.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

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

struct Clay::Impl
{
    Time                          time;
    std::unique_ptr<ClayRenderer> renderer;
    std::unique_ptr<ClayContext>  clayContext;
    ClayPointerState              pointerState = ClayPointerState::Released;
    Float_2                       pointerPosition{ 0, 0 };
    Float_2                       scrollDelta{ 0, 0 };
    uint16_t                      fontId      = 1;
    bool                          isDebugMode = false;

    std::unordered_map<uint32_t, std::unique_ptr<Widget>> ownedWidgets;    // Clay-created widgets
    std::unordered_map<uint32_t, Widget *>                externalWidgets; // User-managed widgets
    std::vector<Widget *>                                 widgetUpdateOrder;
};

Clay::Clay( const ClayDesc &desc ) : m_impl( std::make_unique<Impl>( ) )
{
    if ( desc.LogicalDevice == nullptr )
    {
        spdlog::error( "Clay::Clay Logical device is null" );
        return;
    }

    if ( desc.Width == 0 || desc.Height == 0 )
    {
        spdlog::error( "Clay::Clay invalid dimensions provided: {} x{}", desc.Width, desc.Height );
        return;
    }

    ClayContextDesc clayContextDesc{ };
    clayContextDesc.LogicalDevice                  = desc.LogicalDevice;
    clayContextDesc.Width                          = desc.Width;
    clayContextDesc.Height                         = desc.Height;
    clayContextDesc.MaxNumElements                 = desc.MaxNumElements;
    clayContextDesc.MaxNumTextMeasureCacheElements = desc.MaxNumTextMeasureCacheElements;
    m_impl->clayContext                            = std::make_unique<ClayContext>( clayContextDesc );

    ClayRendererDesc clayRendererDesc{ };
    clayRendererDesc.LogicalDevice      = desc.LogicalDevice;
    clayRendererDesc.ClayContext        = m_impl->clayContext.get( );
    clayRendererDesc.RenderTargetFormat = desc.RenderTargetFormat;
    clayRendererDesc.NumFrames          = desc.NumFrames;
    clayRendererDesc.MaxNumFonts        = desc.MaxNumFonts;
    clayRendererDesc.Width              = desc.Width;
    clayRendererDesc.Height             = desc.Height;
    clayRendererDesc.MaxPipelineWidgets = desc.MaxPipelineWidgets;

    m_impl->renderer = std::make_unique<ClayRenderer>( clayRendererDesc );
}

Clay::~Clay( ) = default;

void Clay::SetViewportSize( const float width, const float height ) const
{
    DZ_NOT_NULL( m_impl->clayContext );
    m_impl->clayContext->SetViewportSize( width, height );
    m_impl->renderer->Resize( width, height );
}

ClayDimensions Clay::GetViewportSize( ) const
{
    DZ_NOT_NULL( m_impl->clayContext );
    return m_impl->clayContext->GetViewportSize( );
}

void Clay::SetDpiScale( const float dpiScale ) const
{
    m_impl->renderer->SetDpiScale( dpiScale );
    m_impl->clayContext->SetDpiScale( dpiScale );
}

void Clay::SetDebugModeEnabled( const bool enabled ) const
{
    DZ_NOT_NULL( m_impl->clayContext );
    m_impl->isDebugMode = enabled;
    m_impl->clayContext->SetDebugModeEnabled( enabled );
}

bool Clay::IsDebugModeEnabled( ) const
{
    return m_impl->isDebugMode;
}

void Clay::SetPointerState( const Float_2 position, const ClayPointerState state ) const
{
    DZ_NOT_NULL( m_impl->clayContext );
    m_impl->clayContext->SetPointerState( position, state );
}

void Clay::UpdateScrollContainers( const bool enableDragScrolling, const Float_2 scrollDelta, const float deltaTime ) const
{
    DZ_NOT_NULL( m_impl->clayContext );

    Float_2 totalScrollDelta = scrollDelta;
    totalScrollDelta.X += m_impl->scrollDelta.X;
    totalScrollDelta.Y += m_impl->scrollDelta.Y;

    m_impl->clayContext->UpdateScrollContainers( enableDragScrolling, totalScrollDelta, deltaTime );

    m_impl->scrollDelta.X = 0;
    m_impl->scrollDelta.Y = 0;
}

void Clay::BeginLayout( ) const
{
    m_impl->time.Tick( );
    DZ_NOT_NULL( m_impl->clayContext );
    SetPointerState( m_impl->pointerPosition, m_impl->pointerState );
    m_impl->clayContext->SetDebugModeEnabled( m_impl->isDebugMode );
    m_impl->clayContext->BeginLayout( );
}

void Clay::EndLayout( ICommandList *commandList, const uint32_t frameIndex, const float deltaTime ) const
{
    DZ_NOT_NULL( m_impl->clayContext );

    this->UpdateWidgets( deltaTime );

    m_impl->renderer->SetDeltaTime( deltaTime );
    m_impl->renderer->Render( commandList, m_impl->clayContext->EndLayoutAndGetCommands( deltaTime ), frameIndex );
}

void Clay::OpenElement( const ClayElementDeclaration &declaration ) const
{
    DZ_NOT_NULL( m_impl->clayContext );
    m_impl->clayContext->OpenElement( declaration );
}

void Clay::CloseElement( ) const
{
    DZ_NOT_NULL( m_impl->clayContext );
    m_impl->clayContext->CloseElement( );
}

void Clay::Text( const InteropString &text, const ClayTextDesc &desc ) const
{
    DZ_NOT_NULL( m_impl->clayContext );
    m_impl->clayContext->Text( text, desc );
}

uint32_t Clay::HashString( const InteropString &str, const uint32_t index, const uint32_t baseId ) const
{
    return m_impl->clayContext->HashString( str, index, baseId );
}

bool Clay::PointerOver( const uint32_t id ) const
{
    return m_impl->clayContext->PointerOver( id );
}

ClayBoundingBox Clay::GetElementBoundingBox( const uint32_t id ) const
{
    return m_impl->clayContext->GetElementBoundingBox( id );
}

void Clay::HandleEvent( const Event &event ) const
{
    if ( event.Type == EventType::MouseMotion )
    {
        m_impl->pointerPosition = Float_2{ static_cast<float>( event.Motion.X ), static_cast<float>( event.Motion.Y ) };
    }

    if ( event.Type == EventType::MouseButtonDown )
    {
        if ( event.Button.Button == MouseButton::Left )
        {
            m_impl->pointerPosition = Float_2{ static_cast<float>( event.Button.X ), static_cast<float>( event.Button.Y ) };
            m_impl->pointerState    = ClayPointerState::Pressed;
        }
    }

    if ( event.Type == EventType::MouseButtonUp )
    {
        if ( event.Button.Button == MouseButton::Left )
        {
            m_impl->pointerPosition = Float_2{ static_cast<float>( event.Button.X ), static_cast<float>( event.Button.Y ) };
            m_impl->pointerState    = ClayPointerState::Released;
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
            m_impl->isDebugMode = !m_impl->clayContext->IsDebugModeEnabled( );
        }
    }

    if ( event.Type == EventType::MouseWheel )
    {
        m_impl->scrollDelta.X += static_cast<float>( event.Wheel.X ) * 30.0f;
        m_impl->scrollDelta.Y += static_cast<float>( event.Wheel.Y ) * 30.0f;
    }

    for ( auto *widget : m_impl->widgetUpdateOrder )
    {
        widget->HandleEvent( event );
    }
}

CheckboxWidget *Clay::CreateCheckbox( uint32_t id, bool initialChecked, const CheckboxStyle &style ) const
{
    auto            widget     = std::make_unique<CheckboxWidget>( m_impl->clayContext.get( ), id, initialChecked, style );
    CheckboxWidget *ptr        = widget.get( );
    m_impl->ownedWidgets[ id ] = std::move( widget );
    m_impl->widgetUpdateOrder.push_back( ptr );
    m_impl->renderer->RegisterWidget( id, ptr );
    return ptr;
}

SliderWidget *Clay::CreateSlider( uint32_t id, float initialValue, const SliderStyle &style ) const
{
    auto          widget       = std::make_unique<SliderWidget>( m_impl->clayContext.get( ), id, initialValue, style );
    SliderWidget *ptr          = widget.get( );
    m_impl->ownedWidgets[ id ] = std::move( widget );
    m_impl->widgetUpdateOrder.push_back( ptr );
    m_impl->renderer->RegisterWidget( id, ptr );
    return ptr;
}

DropdownWidget *Clay::CreateDropdown( uint32_t id, const InteropArray<InteropString> &options, const DropdownStyle &style ) const
{
    auto            widget     = std::make_unique<DropdownWidget>( m_impl->clayContext.get( ), id, options, style );
    DropdownWidget *ptr        = widget.get( );
    m_impl->ownedWidgets[ id ] = std::move( widget );
    m_impl->widgetUpdateOrder.push_back( ptr );
    m_impl->renderer->RegisterWidget( id, ptr );
    return ptr;
}

ColorPickerWidget *Clay::CreateColorPicker( uint32_t id, const Float_3 &initialRgb, const ColorPickerStyle &style ) const
{
    auto               widget  = std::make_unique<ColorPickerWidget>( m_impl->clayContext.get( ), id, initialRgb, style );
    ColorPickerWidget *ptr     = widget.get( );
    m_impl->ownedWidgets[ id ] = std::move( widget );
    m_impl->widgetUpdateOrder.push_back( ptr );
    m_impl->renderer->RegisterWidget( id, ptr );
    return ptr;
}

TextFieldWidget *Clay::CreateTextField( uint32_t id, const TextFieldStyle &style ) const
{
    auto             widget    = std::make_unique<TextFieldWidget>( m_impl->clayContext.get( ), id, style );
    TextFieldWidget *ptr       = widget.get( );
    m_impl->ownedWidgets[ id ] = std::move( widget );
    m_impl->widgetUpdateOrder.push_back( ptr );
    m_impl->renderer->RegisterWidget( id, ptr );
    return ptr;
}

ResizableContainerWidget *Clay::CreateResizableContainer( uint32_t id ) const
{
    auto                      widget = std::make_unique<ResizableContainerWidget>( m_impl->clayContext.get( ), id );
    ResizableContainerWidget *ptr    = widget.get( );
    m_impl->ownedWidgets[ id ]       = std::move( widget );
    m_impl->widgetUpdateOrder.push_back( ptr );
    m_impl->renderer->RegisterWidget( id, ptr );
    return ptr;
}

DockableContainerWidget *Clay::CreateDockableContainer( uint32_t id, DockingManager *dockingManager ) const
{
    auto                     widget = std::make_unique<DockableContainerWidget>( m_impl->clayContext.get( ), id, dockingManager );
    DockableContainerWidget *ptr    = widget.get( );
    m_impl->ownedWidgets[ id ]      = std::move( widget );
    m_impl->widgetUpdateOrder.push_back( ptr );
    m_impl->renderer->RegisterWidget( id, ptr );
    return ptr;
}

Widget *Clay::GetWidget( const uint32_t id ) const
{
    const auto ownedIt = m_impl->ownedWidgets.find( id );
    if ( ownedIt != m_impl->ownedWidgets.end( ) )
    {
        return ownedIt->second.get( );
    }
    const auto externalIt = m_impl->externalWidgets.find( id );
    if ( externalIt != m_impl->externalWidgets.end( ) )
    {
        return externalIt->second;
    }
    return nullptr;
}

void Clay::RemoveWidget( const uint32_t id ) const
{
    Widget    *widget  = nullptr;
    const auto ownedIt = m_impl->ownedWidgets.find( id );
    if ( ownedIt != m_impl->ownedWidgets.end( ) )
    {
        widget = ownedIt->second.get( );
        m_impl->ownedWidgets.erase( ownedIt );
    }
    else
    {
        const auto externalIt = m_impl->externalWidgets.find( id );
        if ( externalIt != m_impl->externalWidgets.end( ) )
        {
            widget = externalIt->second;
            m_impl->externalWidgets.erase( externalIt );
        }
    }

    if ( widget )
    {
        std::erase( m_impl->widgetUpdateOrder, widget );
        m_impl->renderer->UnregisterWidget( id );
    }
}

DockingManager *Clay::CreateDockingManager( ) const
{
    return new DockingManager( m_impl->clayContext.get( ) );
}

void Clay::UpdateWidgets( const float deltaTime ) const
{
    for ( auto *widget : m_impl->widgetUpdateOrder )
    {
        widget->Update( deltaTime );
    }
}

void Clay::RegisterPipelineWidget( Widget *widget ) const
{
    DZ_NOT_NULL( widget );

    const uint32_t id             = widget->GetId( );
    m_impl->externalWidgets[ id ] = widget;
    m_impl->widgetUpdateOrder.push_back( widget );
    m_impl->renderer->RegisterWidget( id, widget );
}

IClayContext *Clay::GetContext( ) const
{
    return m_impl->clayContext.get( );
}

ClayDimensions Clay::MeasureText( const InteropString &text, const uint16_t fontId, const uint16_t fontSize ) const
{
    return m_impl->clayContext->MeasureText( text, fontId, fontSize );
}

void Clay::AddFont( const uint16_t fontId, Font *font ) const
{
    m_impl->clayContext->AddFont( fontId, font );
    m_impl->renderer->Render( nullptr, Clay_RenderCommandArray{ }, 0 );
}

void Clay::RemoveFont( const uint16_t fontId ) const
{
    m_impl->clayContext->RemoveFont( fontId );
}
