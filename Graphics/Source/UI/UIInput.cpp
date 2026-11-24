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

#include <cmath>
#include "DenOfIzGraphicsInternal/UI/UIContext.h"

using namespace DenOfIz;

namespace
{
    bool ContainsId( const std::vector<uint32_t> &ids, const uint32_t id )
    {
        return std::find( ids.begin( ), ids.end( ), id ) != ids.end( );
    }

    void AppendUnique( std::vector<uint32_t> &ids, const std::vector<uint32_t> &source )
    {
        for ( const uint32_t id : source )
        {
            if ( !ContainsId( ids, id ) )
            {
                ids.push_back( id );
            }
        }
    }

    DenOfIz_UIRect Inflate( const DenOfIz_UIRect &r, const float amount )
    {
        return DenOfIz_UIRect{ r.X - amount, r.Y - amount, r.Width + amount * 2.0f, r.Height + amount * 2.0f };
    }
} // namespace

void UIContext::HandlePointerMotion( const float pixelX, const float pixelY )
{
    m_pointer = DenOfIz_Float2{ pixelX / m_pixelsPerPoint, pixelY / m_pixelsPerPoint };
}

void UIContext::HandlePointerButton( const bool down, const DenOfIz_MouseButton button, const float pixelX, const float pixelY, const uint32_t clicks )
{
    m_pointer = DenOfIz_Float2{ pixelX / m_pixelsPerPoint, pixelY / m_pixelsPerPoint };
    m_pointerEvents.push_back( UIPointerEvent{ down, button, m_pointer.X, m_pointer.Y, clicks } );
}

void UIContext::HandleWheel( const float x, const float y )
{
    m_wheel.X += x;
    m_wheel.Y += y;
}

void UIContext::HandlePointerLeave( )
{
    m_pointer = DenOfIz_Float2{ -1.0e6f, -1.0e6f };
}

const UINode *UIContext::FindLastNode( const uint32_t id ) const
{
    UIElementState *it = m_states.Find( id );
    if ( it == nullptr )
    {
        return nullptr;
    }
    const UIElementState &state     = (*it);
    const uint64_t        completed = m_inFrame ? m_frameNumber - 1 : m_frameNumber;
    uint32_t              index     = 0;
    if ( state.LastFrame == completed )
    {
        index = state.NodeIndex;
    }
    else if ( m_inFrame && state.LastFrame == m_frameNumber && state.PrevFrame == completed )
    {
        index = state.PrevNodeIndex;
    }
    else
    {
        return nullptr;
    }
    if ( index >= m_last->Nodes.size( ) || m_last->Nodes[ index ].Id != id )
    {
        return nullptr;
    }
    return &m_last->Nodes[ index ];
}

bool UIContext::InsideRoundClips( const UIFrame &frame, uint32_t clip, const float x, const float y ) const
{
    for ( uint32_t i = 0; i < UIMaxRoundedClips && clip != UINode::Invalid; ++i, clip = frame.RoundClips[ clip ].Parent )
    {
        const UIRoundClip &entry = frame.RoundClips[ clip ];
        if ( UIMath::RoundedRectDistance( entry.Rect, entry.Radius, x, y ) > 0.0f )
        {
            return false;
        }
    }
    return true;
}

int64_t UIContext::FindDeepest( const UIFrame &frame, const uint32_t nodeIndex, const float x, const float y, const bool passthrough ) const
{
    const UINode &node = frame.Nodes[ nodeIndex ];
    if ( node.Hidden || ( node.Kind != UINodeKind::Root && !UIMath::Contains( node.Clip, x, y ) ) )
    {
        return -1;
    }
    bool ignored = passthrough;
    if ( node.Kind == UINodeKind::Element )
    {
        ignored = passthrough ? node.Pointer != DENOFIZ_UI_POINTER_MODE_BLOCK : node.Pointer == DENOFIZ_UI_POINTER_MODE_PASSTHROUGH;
    }
    for ( uint32_t c = node.LastChild; c != UINode::Invalid; c = frame.Nodes[ c ].PrevSibling )
    {
        if ( frame.Nodes[ c ].Floating )
        {
            continue;
        }
        const int64_t hit = FindDeepest( frame, c, x, y, ignored );
        if ( hit >= 0 )
        {
            return hit;
        }
    }
    if ( !ignored && node.Kind != UINodeKind::Root && !node.Contents && UIMath::Contains( node.Rect, x, y ) && InsideRoundClips( frame, node.RoundClip, x, y ) )
    {
        return nodeIndex;
    }
    return -1;
}

void UIContext::HitTest( const UIFrame &frame, const float x, const float y, std::vector<uint32_t> &outChain, bool *outCaptured, uint32_t *outDeepest ) const
{
    outChain.clear( );
    bool     captured = false;
    uint32_t deepest  = 0;
    if ( !frame.Nodes.empty( ) && !frame.Layers.empty( ) )
    {
        for ( size_t layer = frame.Layers.size( ); layer-- > 0; )
        {
            const uint32_t layerRoot = frame.Layers[ layer ];
            const int64_t  hit       = FindDeepest( frame, layerRoot, x, y, false );
            if ( hit < 0 )
            {
                continue;
            }
            if ( deepest == 0 )
            {
                deepest = frame.Nodes[ hit ].Id;
            }
            bool layerCaptured = false;
            for ( uint32_t n = static_cast<uint32_t>( hit ); n != UINode::Invalid; n = frame.Nodes[ n ].Parent )
            {
                const UINode &node = frame.Nodes[ n ];
                if ( node.Kind == UINodeKind::Root )
                {
                    break;
                }
                if ( !ContainsId( outChain, node.Id ) )
                {
                    outChain.push_back( node.Id );
                }
                const bool inLayer = n == layerRoot || node.Layer == layerRoot;
                if ( inLayer && node.Captures && node.Pointer != DENOFIZ_UI_POINTER_MODE_PASSTHROUGH )
                {
                    layerCaptured = true;
                }
            }
            TrimChainAtScrollbar( frame, outChain, x, y );
            if ( layerCaptured )
            {
                captured = true;
                break;
            }
        }
    }
    if ( outCaptured )
    {
        *outCaptured = captured;
    }
    if ( outDeepest )
    {
        *outDeepest = deepest;
    }
}

void UIContext::TrimChainAtScrollbar( const UIFrame &frame, std::vector<uint32_t> &chain, const float x, const float y ) const
{
    for ( size_t i = 0; i < chain.size( ); ++i )
    {
        UIElementState *it = m_states.Find( chain[ i ] );
        if ( it == nullptr || !it->Scrollable || it->NodeIndex >= frame.Nodes.size( ) )
        {
            continue;
        }
        const UINode &node = frame.Nodes[ it->NodeIndex ];
        if ( node.Id != chain[ i ] )
        {
            continue;
        }
        for ( int axis = 0; axis < 2; ++axis )
        {
            DenOfIz_UIRect track{ };
            DenOfIz_UIRect thumb{ };
            if ( ScrollbarRects( frame, node, axis, &track, &thumb ) && UIMath::Contains( Inflate( track, ScrollbarInset ), x, y ) && UIMath::Contains( node.Clip, x, y ) &&
                 InsideRoundClips( frame, node.RoundClip, x, y ) )
            {
                chain.erase( chain.begin( ), chain.begin( ) + static_cast<std::ptrdiff_t>( i ) );
                return;
            }
        }
    }
}

bool UIContext::ScrollbarRects( const UIFrame &frame, const UINode &node, const int axis, DenOfIz_UIRect *track, DenOfIz_UIRect *thumb ) const
{
    if ( node.Kind != UINodeKind::Element )
    {
        return false;
    }
    const DenOfIz_UIElementDesc &desc   = frame.Elements[ node.Data ];
    const UIElementState        &state  = *node.State;
    const DenOfIz_UIScrollDesc  &scroll = desc.Scroll;
    if ( scroll.Scrollbar == DENOFIZ_UI_SCROLLBAR_MODE_HIDDEN || !state.Scrollable )
    {
        return false;
    }
    const bool vertical = axis == 1;
    const bool hasX     = scroll.OverflowX == DENOFIZ_UI_OVERFLOW_SCROLL && state.MaxScroll.X > 0.0f;
    const bool hasY     = scroll.OverflowY == DENOFIZ_UI_OVERFLOW_SCROLL && state.MaxScroll.Y > 0.0f;
    if ( vertical ? !hasY : !hasX )
    {
        return false;
    }

    const float s      = node.Scale;
    const float width  = ( scroll.ScrollbarWidth > 0.0f ? scroll.ScrollbarWidth : DefaultScrollbar ) * s;
    const float inset  = ScrollbarInset * s;
    const auto &border = desc.Style.BorderWidth;
    const float left   = node.Rect.X + border.Left * s;
    const float top    = node.Rect.Y + border.Top * s;
    const float right  = node.Rect.X + node.Rect.Width - border.Right * s;
    const float bottom = node.Rect.Y + node.Rect.Height - border.Bottom * s;
    const float corner = hasX && hasY ? width + inset : 0.0f;

    if ( vertical )
    {
        *track = DenOfIz_UIRect{ right - inset - width, top + inset, width, std::max( 0.0f, bottom - top - inset * 2.0f - corner ) };
    }
    else
    {
        *track = DenOfIz_UIRect{ left + inset, bottom - inset - width, std::max( 0.0f, right - left - inset * 2.0f - corner ), width };
    }

    const float trackLength = vertical ? track->Height : track->Width;
    const float viewport    = vertical ? state.ViewportSize.Y : state.ViewportSize.X;
    const float content     = vertical ? state.ContentSize.Y : state.ContentSize.X;
    const float maxScroll   = vertical ? state.MaxScroll.Y : state.MaxScroll.X;
    const float offset      = vertical ? state.ScrollOffset.Y : state.ScrollOffset.X;
    const float length      = std::min( trackLength, std::max( MinThumb * s, content > 0.0f ? trackLength * viewport / content : trackLength ) );
    const float position    = maxScroll > 0.0f ? ( trackLength - length ) * offset / maxScroll : 0.0f;

    *thumb = vertical ? DenOfIz_UIRect{ track->X, track->Y + position, track->Width, length } : DenOfIz_UIRect{ track->X + position, track->Y, length, track->Height };
    return true;
}

bool UIContext::BeginScrollbarDrag( const float x, const float y )
{
    for ( const uint32_t id : m_scratchChain )
    {
        const UINode *node = FindLastNode( id );
        if ( node == nullptr || !node->State->Scrollable )
        {
            continue;
        }
        for ( int axis = 1; axis >= 0; --axis )
        {
            DenOfIz_UIRect track{ };
            DenOfIz_UIRect thumb{ };
            if ( !ScrollbarRects( *m_last, *node, axis, &track, &thumb ) || !UIMath::Contains( Inflate( track, ScrollbarInset ), x, y ) ||
                 !UIMath::Contains( node->Clip, x, y ) || !InsideRoundClips( *m_last, node->RoundClip, x, y ) )
            {
                continue;
            }
            m_scrollbarDragId = id;
            m_scrollbarAxis   = axis;
            if ( UIMath::Contains( Inflate( thumb, ScrollbarInset ), x, y ) )
            {
                m_scrollbarGrab = axis == 1 ? y - thumb.Y : x - thumb.X;
            }
            else
            {
                m_scrollbarGrab = ( axis == 1 ? thumb.Height : thumb.Width ) * 0.5f;
                UpdateScrollbarDrag( );
            }
            return true;
        }
    }
    return false;
}

void UIContext::UpdateScrollbarDrag( )
{
    if ( m_scrollbarDragId == 0 || !m_primaryDown )
    {
        return;
    }
    const UINode *node = FindLastNode( m_scrollbarDragId );
    if ( node == nullptr )
    {
        m_scrollbarDragId = 0;
        return;
    }
    DenOfIz_UIRect track{ };
    DenOfIz_UIRect thumb{ };
    if ( !ScrollbarRects( *m_last, *node, m_scrollbarAxis, &track, &thumb ) )
    {
        return;
    }
    UIElementState &state    = *node->State;
    const bool      vertical = m_scrollbarAxis == 1;
    const float     travel   = vertical ? track.Height - thumb.Height : track.Width - thumb.Width;
    const float     position = ( vertical ? m_pointer.Y - track.Y : m_pointer.X - track.X ) - m_scrollbarGrab;
    const float     ratio    = travel > 0.0f ? std::clamp( position / travel, 0.0f, 1.0f ) : 0.0f;
    if ( vertical )
    {
        state.ScrollOffset.Y = state.ScrollTarget.Y = ratio * state.MaxScroll.Y;
    }
    else
    {
        state.ScrollOffset.X = state.ScrollTarget.X = ratio * state.MaxScroll.X;
    }
}

void UIContext::ProcessPointer( )
{
    m_prevHoverChain = m_hoverChain;
    m_pressStarted.clear( );
    m_pressEnded.clear( );
    m_clicked.clear( );
    m_rightClicked.clear( );
    m_middleClicked.clear( );
    m_dragStarted.clear( );
    m_dragEnded.clear( );
    m_pressedThisFrame.clear( );
    m_anyPressThisFrame = false;
    m_doubleClick       = false;
    m_focusGained       = 0;
    m_focusLost         = 0;
    if ( m_focusVanished != 0 && m_focus == m_focusVanished )
    {
        SetFocusInternal( 0 );
    }
    m_focusVanished = 0;
    if ( m_hasPendingFocus )
    {
        SetFocusInternal( m_pendingFocus );
        m_hasPendingFocus = false;
    }

    auto enabledOnly = [ this ]( const std::vector<uint32_t> &chain, std::vector<uint32_t> &out )
    {
        out.clear( );
        for ( const uint32_t id : chain )
        {
            const UINode *node = FindLastNode( id );
            if ( node && !node->Disabled )
            {
                out.push_back( id );
            }
        }
    };

    for ( const UIPointerEvent &event : m_pointerEvents )
    {
        bool captured = false;
        HitTest( *m_last, event.X, event.Y, m_scratchChain, &captured, nullptr );
        if ( event.Down )
        {
            m_anyPressThisFrame = true;
            AppendUnique( m_pressedThisFrame, m_scratchChain );
            switch ( event.Button )
            {
            case DENOFIZ_MOUSE_BUTTON_LEFT:
            {
                m_primaryDown     = true;
                m_dragging        = false;
                m_pressPosition   = DenOfIz_Float2{ event.X, event.Y };
                m_clickCount      = event.Clicks;
                m_scrollbarDragId = 0;
                if ( BeginScrollbarDrag( event.X, event.Y ) )
                {
                    m_pressChain.clear( );
                    break;
                }
                enabledOnly( m_scratchChain, m_pressChain );
                AppendUnique( m_pressStarted, m_pressChain );

                uint32_t focus = 0;
                for ( const uint32_t id : m_pressChain )
                {
                    const UINode *node = FindLastNode( id );
                    if ( node && node->Focusable )
                    {
                        focus = id;
                        break;
                    }
                }
                SetFocusInternal( focus );
                break;
            }
            case DENOFIZ_MOUSE_BUTTON_RIGHT:
                enabledOnly( m_scratchChain, m_secondaryChain );
                break;
            case DENOFIZ_MOUSE_BUTTON_MIDDLE:
                enabledOnly( m_scratchChain, m_middleChain );
                break;
            default:
                break;
            }
            continue;
        }

        switch ( event.Button )
        {
        case DENOFIZ_MOUSE_BUTTON_LEFT:
            if ( m_primaryDown )
            {
                if ( m_scrollbarDragId == 0 )
                {
                    for ( const uint32_t id : m_pressChain )
                    {
                        if ( ContainsId( m_scratchChain, id ) )
                        {
                            m_clicked.push_back( id );
                        }
                    }
                    m_doubleClick = m_clickCount >= 2 && !m_clicked.empty( );
                }
                AppendUnique( m_pressEnded, m_pressChain );
                if ( m_dragging )
                {
                    AppendUnique( m_dragEnded, m_pressChain );
                }
            }
            m_primaryDown     = false;
            m_dragging        = false;
            m_scrollbarDragId = 0;
            break;
        case DENOFIZ_MOUSE_BUTTON_RIGHT:
            for ( const uint32_t id : m_secondaryChain )
            {
                if ( ContainsId( m_scratchChain, id ) )
                {
                    m_rightClicked.push_back( id );
                }
            }
            m_secondaryChain.clear( );
            break;
        case DENOFIZ_MOUSE_BUTTON_MIDDLE:
            for ( const uint32_t id : m_middleChain )
            {
                if ( ContainsId( m_scratchChain, id ) )
                {
                    m_middleClicked.push_back( id );
                }
            }
            m_middleChain.clear( );
            break;
        default:
            break;
        }
    }
    m_pointerEvents.clear( );

    if ( m_primaryDown && !m_dragging && m_scrollbarDragId == 0 )
    {
        const float dx = m_pointer.X - m_pressPosition.X;
        const float dy = m_pointer.Y - m_pressPosition.Y;
        if ( dx * dx + dy * dy > DragThreshold * DragThreshold )
        {
            m_dragging = true;
            AppendUnique( m_dragStarted, m_pressChain );
        }
    }
    UpdateScrollbarDrag( );

    bool captured = false;
    HitTest( *m_last, m_pointer.X, m_pointer.Y, m_hoverChain, &captured, &m_hoveredId );
    m_pointerOverUI = captured;
    ApplyWheel( );
}

void UIContext::ApplyWheel( )
{
    if ( m_wheel.X == 0.0f && m_wheel.Y == 0.0f )
    {
        return;
    }
    const float dx = m_wheel.X * WheelPoints;
    const float dy = -m_wheel.Y * WheelPoints;
    m_wheel        = DenOfIz_Float2{ 0.0f, 0.0f };

    for ( const uint32_t id : m_hoverChain )
    {
        UIElementState *it   = m_states.Find( id );
        const UINode   *node = FindLastNode( id );
        if ( it == nullptr || !it->Scrollable )
        {
            if ( node && node->Floating && node->Captures )
            {
                return;
            }
            continue;
        }
        UIElementState &state = (*it);
        float           x     = dx;
        float           y     = dy;
        if ( state.MaxScroll.Y <= 0.0f && state.MaxScroll.X > 0.0f && x == 0.0f )
        {
            x = y;
            y = 0.0f;
        }
        const bool moveY = ( y > 0.0f && state.ScrollTarget.Y < state.MaxScroll.Y ) || ( y < 0.0f && state.ScrollTarget.Y > 0.0f );
        const bool moveX = ( x > 0.0f && state.ScrollTarget.X < state.MaxScroll.X ) || ( x < 0.0f && state.ScrollTarget.X > 0.0f );
        if ( !moveX && !moveY )
        {
            if ( node && node->Floating && node->Captures )
            {
                return;
            }
            continue;
        }
        state.ScrollTarget.X = std::clamp( state.ScrollTarget.X + x, 0.0f, state.MaxScroll.X );
        state.ScrollTarget.Y = std::clamp( state.ScrollTarget.Y + y, 0.0f, state.MaxScroll.Y );
        return;
    }
}

void UIContext::UpdateHover( )
{
    bool     captured = false;
    uint32_t deepest  = 0;
    HitTest( *m_frame, m_pointer.X, m_pointer.Y, m_scratchChain, &captured, &deepest );
    m_pointerOverUI = captured;
    m_hoveredId     = deepest;
    m_cursor        = DENOFIZ_SYSTEM_CURSOR_DEFAULT;
    for ( const uint32_t id : m_scratchChain )
    {
        UIElementState *it = m_states.Find( id );
        if ( it == nullptr || it->LastFrame != m_frameNumber )
        {
            continue;
        }
        const UINode &node = m_frame->Nodes[ it->NodeIndex ];
        if ( node.Cursor != DENOFIZ_SYSTEM_CURSOR_DEFAULT )
        {
            m_cursor = node.Cursor;
            break;
        }
    }
}

void UIContext::SetFocusInternal( const uint32_t id )
{
    if ( id == m_focus )
    {
        return;
    }
    if ( m_focus != 0 )
    {
        m_focusLost = m_focus;
    }
    m_focus = id;
    if ( id != 0 )
    {
        m_focusGained = id;
    }
}

void UIContext::SetFocus( const uint32_t id )
{
    m_pendingFocus    = id;
    m_hasPendingFocus = true;
}

uint32_t UIContext::GetFocus( ) const
{
    return m_hasPendingFocus ? m_pendingFocus : m_focus;
}

void UIContext::FocusNext( const bool reverse )
{
    std::vector<uint32_t> focusable;
    for ( const UINode &node : m_last->Nodes )
    {
        if ( node.Focusable && !node.Disabled && !node.Hidden )
        {
            focusable.push_back( node.Id );
        }
    }
    if ( focusable.empty( ) )
    {
        return;
    }
    const uint32_t current = GetFocus( );
    const auto     it      = std::find( focusable.begin( ), focusable.end( ), current );
    size_t     index;
    if ( it == focusable.end( ) )
    {
        index = reverse ? focusable.size( ) - 1 : 0;
    }
    else
    {
        const size_t position = static_cast<size_t>( it - focusable.begin( ) );
        index                 = reverse ? ( position + focusable.size( ) - 1 ) % focusable.size( ) : ( position + 1 ) % focusable.size( );
    }
    SetFocus( focusable[ index ] );
}

void UIContext::GetInteraction( const uint32_t id, DenOfIz_UIInteraction *out ) const
{
    *out = DenOfIz_UIInteraction{ };
    if ( id == 0 )
    {
        return;
    }
    DenOfIz_UIInteractionFlags flags   = 0;
    const bool                 hovered = ContainsId( m_hoverChain, id );
    const bool                 was     = ContainsId( m_prevHoverChain, id );
    const bool                 pressed = ContainsId( m_pressChain, id );
    if ( hovered )
    {
        flags |= DENOFIZ_UI_INTERACTION_HOVERED_BIT;
    }
    if ( hovered && !was )
    {
        flags |= DENOFIZ_UI_INTERACTION_HOVER_STARTED_BIT;
    }
    if ( !hovered && was )
    {
        flags |= DENOFIZ_UI_INTERACTION_HOVER_ENDED_BIT;
    }
    if ( m_primaryDown && pressed )
    {
        flags |= DENOFIZ_UI_INTERACTION_PRESSED_BIT;
    }
    if ( ContainsId( m_pressStarted, id ) )
    {
        flags |= DENOFIZ_UI_INTERACTION_PRESS_STARTED_BIT;
    }
    if ( ContainsId( m_pressEnded, id ) )
    {
        flags |= DENOFIZ_UI_INTERACTION_PRESS_ENDED_BIT;
    }
    const bool clicked = ContainsId( m_clicked, id );
    if ( clicked )
    {
        flags |= DENOFIZ_UI_INTERACTION_CLICKED_BIT;
        if ( m_doubleClick )
        {
            flags |= DENOFIZ_UI_INTERACTION_DOUBLE_CLICKED_BIT;
        }
    }
    if ( ContainsId( m_rightClicked, id ) )
    {
        flags |= DENOFIZ_UI_INTERACTION_RIGHT_CLICKED_BIT;
    }
    if ( ContainsId( m_middleClicked, id ) )
    {
        flags |= DENOFIZ_UI_INTERACTION_MIDDLE_CLICKED_BIT;
    }
    if ( m_primaryDown && m_dragging && pressed )
    {
        flags |= DENOFIZ_UI_INTERACTION_DRAGGING_BIT;
    }
    if ( ContainsId( m_dragStarted, id ) )
    {
        flags |= DENOFIZ_UI_INTERACTION_DRAG_STARTED_BIT;
    }
    if ( ContainsId( m_dragEnded, id ) )
    {
        flags |= DENOFIZ_UI_INTERACTION_DRAG_ENDED_BIT;
    }
    if ( m_focus == id )
    {
        flags |= DENOFIZ_UI_INTERACTION_FOCUSED_BIT;
    }
    if ( m_focusGained == id )
    {
        flags |= DENOFIZ_UI_INTERACTION_FOCUS_GAINED_BIT;
    }
    if ( m_focusLost == id )
    {
        flags |= DENOFIZ_UI_INTERACTION_FOCUS_LOST_BIT;
    }
    UIElementState *state = m_states.Find( id );
    if ( m_anyPressThisFrame && !ContainsId( m_pressedThisFrame, id ) )
    {
        const uint32_t target = state != nullptr ? state->FloatingTargetId : 0;
        if ( target == 0 || !ContainsId( m_pressedThisFrame, target ) )
        {
            flags |= DENOFIZ_UI_INTERACTION_PRESSED_OUTSIDE_BIT;
        }
    }

    out->Flags = flags;
    if ( clicked || ContainsId( m_pressStarted, id ) )
    {
        out->ClickCount = m_clickCount;
    }
    if ( state != nullptr )
    {
        out->Rect            = state->Rect;
        out->PointerPosition = DenOfIz_Float2{ m_pointer.X - out->Rect.X, m_pointer.Y - out->Rect.Y };
    }
    if ( pressed && ( m_primaryDown || ContainsId( m_pressEnded, id ) ) )
    {
        out->DragDelta = DenOfIz_Float2{ m_pointer.X - m_pressPosition.X, m_pointer.Y - m_pressPosition.Y };
    }
}

bool UIContext::IsPointerOverUI( ) const
{
    return m_pointerOverUI;
}

uint32_t UIContext::GetHoveredId( ) const
{
    return m_hoveredId;
}

DenOfIz_Float2 UIContext::GetPointerPosition( ) const
{
    return m_pointer;
}

DenOfIz_SystemCursor UIContext::GetCursor( ) const
{
    return m_cursor;
}
