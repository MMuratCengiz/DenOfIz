/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/UI/UIContext.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <unordered_set>
#include "DenOfIzGraphics/Backends/Interface/Texture.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

namespace
{
    constexpr uint32_t RootId = 0x52A0F00Du;

    float Ease( const DenOfIz_UIEasing easing, const float t )
    {
        switch ( easing )
        {
        case DENOFIZ_UI_EASING_LINEAR:
            return t;
        case DENOFIZ_UI_EASING_EASE_IN:
            return t * t * t;
        case DENOFIZ_UI_EASING_EASE_IN_OUT:
        {
            const float inv = -2.0f * t + 2.0f;
            return t < 0.5f ? 4.0f * t * t * t : 1.0f - inv * inv * inv * 0.5f;
        }
        case DENOFIZ_UI_EASING_EASE_OUT_BACK:
        {
            constexpr float c1 = 1.70158f;
            constexpr float c3 = c1 + 1.0f;
            const float     u  = t - 1.0f;
            return 1.0f + c3 * u * u * u + c1 * u * u;
        }
        case DENOFIZ_UI_EASING_EASE_OUT:
        default:
        {
            const float inv = 1.0f - t;
            return 1.0f - inv * inv * inv;
        }
        }
    }

    bool TweenEqual( const DenOfIz_Float2 &a, const DenOfIz_Float2 &b )
    {
        return std::fabs( a.X - b.X ) < 0.01f && std::fabs( a.Y - b.Y ) < 0.01f;
    }

    bool TweenEqual( const DenOfIz_UIColor &a, const DenOfIz_UIColor &b )
    {
        return UIMath::ColorEquals( a, b );
    }

    bool TweenEqual( const DenOfIz_UICorners &a, const DenOfIz_UICorners &b )
    {
        return std::fabs( a.TopLeft - b.TopLeft ) < 0.01f && std::fabs( a.TopRight - b.TopRight ) < 0.01f && std::fabs( a.BottomRight - b.BottomRight ) < 0.01f &&
               std::fabs( a.BottomLeft - b.BottomLeft ) < 0.01f;
    }

    DenOfIz_Float2 TweenLerp( const DenOfIz_Float2 &a, const DenOfIz_Float2 &b, const float t )
    {
        return DenOfIz_Float2{ UIMath::Lerp( a.X, b.X, t ), UIMath::Lerp( a.Y, b.Y, t ) };
    }

    DenOfIz_UIColor TweenLerp( const DenOfIz_UIColor &a, const DenOfIz_UIColor &b, const float t )
    {
        return UIMath::LerpColor( a, b, std::clamp( t, 0.0f, 1.0f ) );
    }

    DenOfIz_UICorners TweenLerp( const DenOfIz_UICorners &a, const DenOfIz_UICorners &b, const float t )
    {
        return DenOfIz_UICorners{ UIMath::Lerp( a.TopLeft, b.TopLeft, t ), UIMath::Lerp( a.TopRight, b.TopRight, t ), UIMath::Lerp( a.BottomRight, b.BottomRight, t ),
                                  UIMath::Lerp( a.BottomLeft, b.BottomLeft, t ) };
    }

    template <typename T>
    T UpdateTween( UITween<T> &tween, const T &target, const float dt, const float duration, const DenOfIz_UIEasing easing, const bool snap )
    {
        if ( snap || !tween.Initialized )
        {
            tween.From = tween.To = tween.Current = target;
            tween.Elapsed                         = duration;
            tween.Initialized                     = true;
            return target;
        }
        if ( !TweenEqual( target, tween.To ) )
        {
            tween.From    = tween.Current;
            tween.To      = target;
            tween.Elapsed = 0.0f;
        }
        if ( tween.Elapsed >= duration )
        {
            tween.Current = tween.To;
            return tween.Current;
        }
        tween.Elapsed += dt;
        tween.Current = TweenLerp( tween.From, tween.To, Ease( easing, std::min( 1.0f, tween.Elapsed / duration ) ) );
        return tween.Current;
    }

    template <typename T>
    bool TweenRunning( const UITween<T> &tween, const float duration )
    {
        return tween.Initialized && tween.Elapsed < duration;
    }

    bool TextTruncated( const UITextLayout &layout, const char *text, uint32_t length )
    {
        for ( const UITextLineLayout &line : layout.Lines )
        {
            if ( line.Ellipsis )
            {
                return true;
            }
        }
        while ( length > 0 && ( text[ length - 1 ] == ' ' || text[ length - 1 ] == '\n' || text[ length - 1 ] == '\r' || text[ length - 1 ] == '\t' ) )
        {
            --length;
        }
        return !layout.Lines.empty( ) && layout.Lines.back( ).ByteEnd < length;
    }

    YGFlexDirection ToYoga( const DenOfIz_UIDirection direction )
    {
        switch ( direction )
        {
        case DENOFIZ_UI_DIRECTION_COLUMN:
            return YGFlexDirectionColumn;
        case DENOFIZ_UI_DIRECTION_ROW_REVERSE:
            return YGFlexDirectionRowReverse;
        case DENOFIZ_UI_DIRECTION_COLUMN_REVERSE:
            return YGFlexDirectionColumnReverse;
        case DENOFIZ_UI_DIRECTION_ROW:
        default:
            return YGFlexDirectionRow;
        }
    }

    YGJustify ToYoga( const DenOfIz_UIJustify justify )
    {
        switch ( justify )
        {
        case DENOFIZ_UI_JUSTIFY_CENTER:
            return YGJustifyCenter;
        case DENOFIZ_UI_JUSTIFY_END:
            return YGJustifyFlexEnd;
        case DENOFIZ_UI_JUSTIFY_SPACE_BETWEEN:
            return YGJustifySpaceBetween;
        case DENOFIZ_UI_JUSTIFY_SPACE_AROUND:
            return YGJustifySpaceAround;
        case DENOFIZ_UI_JUSTIFY_SPACE_EVENLY:
            return YGJustifySpaceEvenly;
        case DENOFIZ_UI_JUSTIFY_START:
        default:
            return YGJustifyFlexStart;
        }
    }

    YGAlign ToYoga( const DenOfIz_UIAlign align )
    {
        switch ( align )
        {
        case DENOFIZ_UI_ALIGN_START:
            return YGAlignFlexStart;
        case DENOFIZ_UI_ALIGN_CENTER:
            return YGAlignCenter;
        case DENOFIZ_UI_ALIGN_END:
            return YGAlignFlexEnd;
        case DENOFIZ_UI_ALIGN_BASELINE:
            return YGAlignBaseline;
        case DENOFIZ_UI_ALIGN_STRETCH:
        default:
            return YGAlignStretch;
        }
    }

    YGAlign ToYoga( const DenOfIz_UIAlignSelf align )
    {
        switch ( align )
        {
        case DENOFIZ_UI_ALIGN_SELF_START:
            return YGAlignFlexStart;
        case DENOFIZ_UI_ALIGN_SELF_CENTER:
            return YGAlignCenter;
        case DENOFIZ_UI_ALIGN_SELF_END:
            return YGAlignFlexEnd;
        case DENOFIZ_UI_ALIGN_SELF_STRETCH:
            return YGAlignStretch;
        case DENOFIZ_UI_ALIGN_SELF_BASELINE:
            return YGAlignBaseline;
        case DENOFIZ_UI_ALIGN_SELF_AUTO:
        default:
            return YGAlignAuto;
        }
    }

    YGAlign ToYoga( const DenOfIz_UIAlignContent align )
    {
        switch ( align )
        {
        case DENOFIZ_UI_ALIGN_CONTENT_CENTER:
            return YGAlignCenter;
        case DENOFIZ_UI_ALIGN_CONTENT_END:
            return YGAlignFlexEnd;
        case DENOFIZ_UI_ALIGN_CONTENT_STRETCH:
            return YGAlignStretch;
        case DENOFIZ_UI_ALIGN_CONTENT_SPACE_BETWEEN:
            return YGAlignSpaceBetween;
        case DENOFIZ_UI_ALIGN_CONTENT_SPACE_AROUND:
            return YGAlignSpaceAround;
        case DENOFIZ_UI_ALIGN_CONTENT_SPACE_EVENLY:
            return YGAlignSpaceEvenly;
        case DENOFIZ_UI_ALIGN_CONTENT_START:
        default:
            return YGAlignFlexStart;
        }
    }

    YGWrap ToYoga( const DenOfIz_UIFlexWrap wrap )
    {
        switch ( wrap )
        {
        case DENOFIZ_UI_FLEX_WRAP_WRAP:
            return YGWrapWrap;
        case DENOFIZ_UI_FLEX_WRAP_WRAP_REVERSE:
            return YGWrapWrapReverse;
        case DENOFIZ_UI_FLEX_WRAP_NO_WRAP:
        default:
            return YGWrapNoWrap;
        }
    }

    YGPositionType ToYoga( const DenOfIz_UIPosition position )
    {
        switch ( position )
        {
        case DENOFIZ_UI_POSITION_ABSOLUTE:
            return YGPositionTypeAbsolute;
        case DENOFIZ_UI_POSITION_STATIC:
            return YGPositionTypeStatic;
        case DENOFIZ_UI_POSITION_RELATIVE:
        default:
            return YGPositionTypeRelative;
        }
    }

    YGBoxSizing ToYoga( const DenOfIz_UIBoxSizing boxSizing )
    {
        return boxSizing == DENOFIZ_UI_BOX_SIZING_CONTENT_BOX ? YGBoxSizingContentBox : YGBoxSizingBorderBox;
    }

    bool IsRow( const DenOfIz_UIDirection direction )
    {
        return direction == DENOFIZ_UI_DIRECTION_ROW || direction == DENOFIZ_UI_DIRECTION_ROW_REVERSE;
    }

    constexpr YGEdge EdgeOrder[ 4 ] = { YGEdgeLeft, YGEdgeTop, YGEdgeRight, YGEdgeBottom };

    void SetPadding( const YGNodeRef node, const DenOfIz_UILengthEdges &padding )
    {
        const DenOfIz_UILength *edges[ 4 ] = { &padding.Left, &padding.Top, &padding.Right, &padding.Bottom };
        for ( uint32_t i = 0; i < 4; ++i )
        {
            const DenOfIz_UILength &length = *edges[ i ];
            switch ( length.Type )
            {
            case DENOFIZ_UI_LENGTH_TYPE_PERCENT:
                YGNodeStyleSetPaddingPercent( node, EdgeOrder[ i ], length.Value * 100.0f );
                break;
            case DENOFIZ_UI_LENGTH_TYPE_AUTO:
                YGNodeStyleSetPadding( node, EdgeOrder[ i ], 0.0f );
                break;
            case DENOFIZ_UI_LENGTH_TYPE_POINTS:
            default:
                YGNodeStyleSetPadding( node, EdgeOrder[ i ], length.Value );
                break;
            }
        }
    }

    void SetMargin( const YGNodeRef node, const DenOfIz_UILengthEdges &margin )
    {
        const DenOfIz_UILength *edges[ 4 ] = { &margin.Left, &margin.Top, &margin.Right, &margin.Bottom };
        for ( uint32_t i = 0; i < 4; ++i )
        {
            const DenOfIz_UILength &length = *edges[ i ];
            switch ( length.Type )
            {
            case DENOFIZ_UI_LENGTH_TYPE_PERCENT:
                YGNodeStyleSetMarginPercent( node, EdgeOrder[ i ], length.Value * 100.0f );
                break;
            case DENOFIZ_UI_LENGTH_TYPE_AUTO:
                YGNodeStyleSetMarginAuto( node, EdgeOrder[ i ] );
                break;
            case DENOFIZ_UI_LENGTH_TYPE_POINTS:
            default:
                YGNodeStyleSetMargin( node, EdgeOrder[ i ], length.Value );
                break;
            }
        }
    }

    void ClearMargin( const YGNodeRef node )
    {
        for ( const YGEdge edge : EdgeOrder )
        {
            YGNodeStyleSetMargin( node, edge, 0.0f );
        }
    }

    void SetGap( const YGNodeRef node, const YGGutter gutter, const DenOfIz_UILength &gap )
    {
        switch ( gap.Type )
        {
        case DENOFIZ_UI_LENGTH_TYPE_PERCENT:
            YGNodeStyleSetGapPercent( node, gutter, gap.Value * 100.0f );
            break;
        case DENOFIZ_UI_LENGTH_TYPE_AUTO:
            YGNodeStyleSetGap( node, gutter, 0.0f );
            break;
        case DENOFIZ_UI_LENGTH_TYPE_POINTS:
        default:
            YGNodeStyleSetGap( node, gutter, gap.Value );
            break;
        }
    }

    void SetInsets( const YGNodeRef node, const DenOfIz_UIInsets &insets )
    {
        const DenOfIz_UIValue *edges[ 4 ] = { &insets.Left, &insets.Top, &insets.Right, &insets.Bottom };
        for ( uint32_t i = 0; i < 4; ++i )
        {
            const DenOfIz_UIValue &value = *edges[ i ];
            switch ( value.Type )
            {
            case DENOFIZ_UI_VALUE_TYPE_POINTS:
                YGNodeStyleSetPosition( node, EdgeOrder[ i ], value.Value );
                break;
            case DENOFIZ_UI_VALUE_TYPE_PERCENT:
                YGNodeStyleSetPositionPercent( node, EdgeOrder[ i ], value.Value * 100.0f );
                break;
            case DENOFIZ_UI_VALUE_TYPE_AUTO:
            default:
                YGNodeStyleSetPositionAuto( node, EdgeOrder[ i ] );
                break;
            }
        }
    }

    void ClearInsets( const YGNodeRef node )
    {
        for ( const YGEdge edge : EdgeOrder )
        {
            YGNodeStyleSetPositionAuto( node, edge );
        }
    }

    void SetSize( const YGNodeRef node, const DenOfIz_UIValue &value, void ( *points )( YGNodeRef, float ), void ( *percent )( YGNodeRef, float ),
                  void ( *automatic )( YGNodeRef ) )
    {
        switch ( value.Type )
        {
        case DENOFIZ_UI_VALUE_TYPE_POINTS:
            points( node, value.Value );
            break;
        case DENOFIZ_UI_VALUE_TYPE_PERCENT:
            percent( node, value.Value * 100.0f );
            break;
        case DENOFIZ_UI_VALUE_TYPE_AUTO:
        default:
            automatic( node );
            break;
        }
    }

    void SetLimit( const YGNodeRef node, const DenOfIz_UIValue &value, void ( *points )( YGNodeRef, float ), void ( *percent )( YGNodeRef, float ) )
    {
        switch ( value.Type )
        {
        case DENOFIZ_UI_VALUE_TYPE_POINTS:
            points( node, value.Value );
            break;
        case DENOFIZ_UI_VALUE_TYPE_PERCENT:
            percent( node, value.Value * 100.0f );
            break;
        case DENOFIZ_UI_VALUE_TYPE_AUTO:
        default:
            points( node, YGUndefined );
            break;
        }
    }

    bool IsSet( const DenOfIz_UIValue &value )
    {
        return value.Type == DENOFIZ_UI_VALUE_TYPE_POINTS || value.Type == DENOFIZ_UI_VALUE_TYPE_PERCENT;
    }

    DenOfIz_UIRect TransformRect( const DenOfIz_UIRect &r, const float scale, const float tx, const float ty )
    {
        return DenOfIz_UIRect{ r.X * scale + tx, r.Y * scale + ty, r.Width * scale, r.Height * scale };
    }

    DenOfIz_Float2 AnchorPoint( const DenOfIz_UIRect &r, const DenOfIz_UIAnchor anchor )
    {
        constexpr float factors[ 3 ] = { 0.0f, 0.5f, 1.0f };
        const uint32_t  index        = static_cast<uint32_t>( anchor ) < 9 ? static_cast<uint32_t>( anchor ) : 0;
        return DenOfIz_Float2{ r.X + r.Width * factors[ index % 3 ], r.Y + r.Height * factors[ index / 3 ] };
    }

    DenOfIz_UIAnchor MirrorY( const DenOfIz_UIAnchor anchor )
    {
        const uint32_t index = static_cast<uint32_t>( anchor );
        return static_cast<DenOfIz_UIAnchor>( ( 2 - index / 3 ) * 3 + index % 3 );
    }

    DenOfIz_UIAnchor MirrorX( const DenOfIz_UIAnchor anchor )
    {
        const uint32_t index = static_cast<uint32_t>( anchor );
        return static_cast<DenOfIz_UIAnchor>( index / 3 * 3 + ( 2 - index % 3 ) );
    }

    bool CapturesPointer( const DenOfIz_UIElementDesc &desc )
    {
        if ( desc.Pointer == DENOFIZ_UI_POINTER_MODE_BLOCK )
        {
            return true;
        }
        if ( desc.Pointer == DENOFIZ_UI_POINTER_MODE_PASSTHROUGH )
        {
            return false;
        }
        const auto &style = desc.Style;
        return style.BackgroundColor.A > 0 || ( style.BorderColor.A > 0 && UIMath::HasEdges( style.BorderWidth ) ) || DENOFIZ_HANDLE_IS_VALID( desc.Image.Texture ) ||
               desc.Scroll.OverflowX == DENOFIZ_UI_OVERFLOW_SCROLL || desc.Scroll.OverflowY == DENOFIZ_UI_OVERFLOW_SCROLL;
    }
} // namespace

UIStateMap::UIStateMap( )
{
    Rehash( 8192 );
}

uint32_t UIStateMap::Probe( const uint32_t id, bool *found ) const
{
    uint32_t index     = UIMath::Fmix32( id ) & m_mask;
    uint32_t insertion = Empty;
    for ( ;; )
    {
        const uint32_t slot = m_slots[ index ];
        if ( slot == Empty )
        {
            *found = false;
            return insertion != Empty ? insertion : index;
        }
        if ( slot == Tomb )
        {
            if ( insertion == Empty )
            {
                insertion = index;
            }
        }
        else if ( m_keys[ index ] == id )
        {
            *found = true;
            return index;
        }
        index = ( index + 1 ) & m_mask;
    }
}

void UIStateMap::Rehash( const uint32_t capacity )
{
    const std::vector<uint32_t> keys  = std::move( m_keys );
    const std::vector<uint32_t> slots = std::move( m_slots );
    m_keys.assign( capacity, 0 );
    m_slots.assign( capacity, Empty );
    m_mask = capacity - 1;
    m_used = 0;
    for ( size_t i = 0; i < slots.size( ); ++i )
    {
        if ( slots[ i ] != Empty && slots[ i ] != Tomb )
        {
            bool           found = false;
            const uint32_t index = Probe( keys[ i ], &found );
            m_keys[ index ]      = keys[ i ];
            m_slots[ index ]     = slots[ i ];
            ++m_used;
        }
    }
}

UIElementState *UIStateMap::Find( const uint32_t id ) const
{
    bool           found = false;
    const uint32_t index = Probe( id, &found );
    return found ? &m_pool[ m_slots[ index ] ] : nullptr;
}

std::pair<UIElementState *, bool> UIStateMap::TryEmplace( const uint32_t id )
{
    bool     found = false;
    uint32_t index = Probe( id, &found );
    if ( found )
    {
        return { &m_pool[ m_slots[ index ] ], false };
    }
    if ( ( m_used + 1 ) * 4 > ( m_mask + 1 ) * 3 )
    {
        Rehash( ( m_live + 1 ) * 2 > m_mask + 1 ? ( m_mask + 1 ) * 2 : m_mask + 1 );
        index = Probe( id, &found );
    }
    uint32_t slot;
    if ( !m_free.empty( ) )
    {
        slot = m_free.back( );
        m_free.pop_back( );
    }
    else
    {
        slot = static_cast<uint32_t>( m_pool.size( ) );
        m_pool.emplace_back( );
    }
    if ( m_slots[ index ] == Empty )
    {
        ++m_used;
    }
    m_keys[ index ]       = id;
    m_slots[ index ]      = slot;
    UIElementState &state = m_pool[ slot ];
    state                 = UIElementState{ };
    state.Id              = id;
    state.Live            = true;
    ++m_live;
    return { &state, true };
}

void UIStateMap::Erase( const uint32_t id )
{
    bool           found = false;
    const uint32_t index = Probe( id, &found );
    if ( !found )
    {
        return;
    }
    const uint32_t slot = m_slots[ index ];
    m_slots[ index ]    = Tomb;
    m_pool[ slot ]      = UIElementState{ };
    m_free.push_back( slot );
    --m_live;
}

void UIFrame::Clear( )
{
    Nodes.clear( );
    Elements.clear( );
    Texts.clear( );
    TextArena.clear( );
    TextRuns.clear( );
    Layers.clear( );
    Commands.clear( );
    RoundClips.clear( );
}

UIContext::UIContext( )
{
    m_config = YGConfigNew( );
    YGConfigSetPointScaleFactor( m_config, 0.0f );
    YGConfigSetUseWebDefaults( m_config, false );
    m_template = YGNodeNewWithConfig( m_config );

    UIElementState &root = *m_states.TryEmplace( RootId ).first;
    root.Id              = RootId;
    root.Owner           = this;
    root.Node            = YGNodeNewWithConfig( m_config );
    m_root               = root.Node;
    YGNodeSetContext( m_root, &root );
    YGNodeStyleSetFlexDirection( m_root, YGFlexDirectionColumn );
    YGNodeStyleSetAlignItems( m_root, YGAlignStretch );
}

UIContext::~UIContext( )
{
    m_states.ForEach(
        []( UIElementState &state )
        {
            if ( state.Node )
            {
                YGNodeRemoveAllChildren( state.Node );
            }
        } );
    m_states.ForEach(
        []( UIElementState &state )
        {
            if ( state.Node )
            {
                YGNodeFree( state.Node );
                state.Node = nullptr;
            }
        } );
    YGNodeFree( m_template );
    YGConfigFree( m_config );
}

UIText &UIContext::Text( )
{
    return m_text;
}

void UIContext::SetViewportPixels( const float width, const float height )
{
    if ( width < 1.0f || height < 1.0f )
    {
        return;
    }
    if ( width != m_viewportPixelWidth || height != m_viewportPixelHeight )
    {
        m_viewportPixelWidth  = width;
        m_viewportPixelHeight = height;
        m_snapTransitions     = true;
    }
}

DenOfIz_Float2 UIContext::GetViewportSize( ) const
{
    return DenOfIz_Float2{ m_viewportPixelWidth / m_pixelsPerPoint, m_viewportPixelHeight / m_pixelsPerPoint };
}

DenOfIz_Float2 UIContext::GetViewportPixels( ) const
{
    return DenOfIz_Float2{ m_viewportPixelWidth, m_viewportPixelHeight };
}

void UIContext::SetDisplayScale( const float displayScale )
{
    if ( displayScale > 0.0f && displayScale != m_displayScale )
    {
        m_displayScale = displayScale;
        UpdatePixelsPerPoint( );
    }
}

void UIContext::SetScale( const float scale )
{
    if ( scale > 0.0f && scale != m_scale )
    {
        m_scale = scale;
        UpdatePixelsPerPoint( );
    }
}

float UIContext::GetPixelsPerPoint( ) const
{
    return m_pixelsPerPoint;
}

void UIContext::SetDebugFlags( const DenOfIz_UIDebugFlags flags )
{
    m_debugFlags = flags;
}

void UIContext::UpdatePixelsPerPoint( )
{
    const float pixelsPerPoint = m_displayScale * m_scale;
    if ( pixelsPerPoint != m_pixelsPerPoint )
    {
        m_pixelsPerPoint  = pixelsPerPoint;
        m_snapTransitions = true;
    }
}

void UIContext::FontsChanged( )
{
    m_ghosts.clear( );
    m_frames[ 0 ].Commands.clear( );
    m_frames[ 1 ].Commands.clear( );
    m_states.ForEach(
        []( UIElementState &state )
        {
            state.Run          = nullptr;
            state.Layout       = nullptr;
            state.CommandBegin = state.CommandEnd = 0;
            if ( state.NodeIsText && state.Node )
            {
                YGNodeMarkDirty( state.Node );
            }
        } );
}

const std::vector<UICommand> &UIContext::Commands( ) const
{
    return m_last->Commands;
}

UIElementState &UIContext::AcquireState( const uint32_t requested, const uint32_t autoId, const bool isText )
{
    uint32_t id = requested != 0 ? requested : autoId;
    for ( ;; )
    {
        auto [ found, inserted ] = m_states.TryEmplace( id );
        UIElementState &state    = *found;
        if ( !inserted && state.LastFrame == m_frameNumber )
        {
            if ( requested != 0 )
            {
                static std::unordered_set<uint32_t> reported;
                if ( reported.insert( requested ).second )
                {
                    spdlog::error( "UI: element id {:#010x} is declared more than once in a frame, the duplicate gets a derived id", requested );
                }
                id = UIMath::CombineId( id, 0x51ED270Bu );
            }
            else
            {
                id = UIMath::Fmix32( id + 0x9E3779B9u );
            }
            continue;
        }
        if ( inserted || state.Node == nullptr )
        {
            state.Id           = id;
            state.Owner        = this;
            state.Node         = YGNodeNewWithConfig( m_config );
            state.CreatedFrame = m_frameNumber;
            YGNodeSetContext( state.Node, &state );
        }
        if ( state.LastFrame != m_frameNumber )
        {
            state.PrevNodeIndex = state.NodeIndex;
            state.PrevFrame     = state.LastFrame;
        }
        state.Appeared    = inserted || state.LastFrame + 1 < m_frameNumber;
        state.IsText      = isText;
        state.LastFrame   = m_frameNumber;
        state.IdleSeconds = 0.0f;
        return state;
    }
}

uint32_t UIContext::AppendNode( const UINodeKind kind, const uint32_t requestedId, const bool isText )
{
    uint32_t autoId = 0;
    if ( requestedId == 0 && !m_openStack.empty( ) )
    {
        const UINode &parent = m_frame->Nodes[ m_openStack.back( ) ];
        autoId               = UIMath::Fmix32( parent.Id * 0x2C1B3C6Du + ( parent.ChildCount + 1 ) * 0x297A2D39u );
    }
    const auto      index = static_cast<uint32_t>( m_frame->Nodes.size( ) );
    UIElementState &s     = AcquireState( requestedId, autoId, isText );
    UINode         &node  = m_frame->Nodes.emplace_back( );
    node.Id               = s.Id;
    node.Kind             = kind;
    node.SubtreeEnd       = index + 1;
    node.State            = &s;
    s.NodeIndex           = index;
    s.CommandBegin        = 0;
    s.CommandEnd          = 0;

    if ( !m_openStack.empty( ) )
    {
        const uint32_t parentIndex = m_openStack.back( );
        UINode        &parent      = m_frame->Nodes[ parentIndex ];
        node.Parent                = parentIndex;
        node.Layer                 = parent.Floating ? parentIndex : parent.Layer;
        node.Hidden                = parent.Hidden;
        if ( parent.LastChild == UINode::Invalid )
        {
            parent.FirstChild = index;
        }
        else
        {
            m_frame->Nodes[ parent.LastChild ].NextSibling = index;
            node.PrevSibling                               = parent.LastChild;
        }
        parent.LastChild = index;
        ++parent.ChildCount;
        s.ParentChanged = s.ParentId != parent.Id && !s.Appeared;
        s.ParentId      = parent.Id;
    }
    return index;
}

void UIContext::BeginFrame( const float deltaTime )
{
    if ( m_inFrame )
    {
        spdlog::warn( "UI: BeginFrame called twice without EndFrame" );
    }
    m_deltaTime = std::clamp( deltaTime, 0.0f, 0.25f );
    ProcessPointer( );

    ++m_frameNumber;
    m_text.BeginFrame( m_frameNumber );

    m_frame->Clear( );
    m_floating.clear( );
    m_openStack.clear( );

    const uint32_t root = AppendNode( UINodeKind::Root, RootId, false );
    m_openStack.push_back( root );
    m_inFrame = true;
}

uint32_t UIContext::OpenElement( const DenOfIz_UIElementDesc &desc )
{
    if ( !m_inFrame )
    {
        spdlog::error( "UI: OpenElement called outside BeginFrame/EndFrame" );
        return 0;
    }
    const uint32_t index = AppendNode( UINodeKind::Element, desc.Id, false );
    UINode        &node  = m_frame->Nodes[ index ];
    const uint32_t id    = node.Id;
    node.Data            = static_cast<uint32_t>( m_frame->Elements.size( ) );
    m_frame->Elements.push_back( desc );

    node.Floating  = desc.Floating.Target != DENOFIZ_UI_FLOATING_TARGET_NONE;
    node.Hidden    = node.Hidden || desc.Layout.Display == DENOFIZ_UI_DISPLAY_NONE;
    node.Contents  = !node.Floating && desc.Layout.Display == DENOFIZ_UI_DISPLAY_CONTENTS;
    node.Pointer   = desc.Pointer;
    node.Cursor    = node.Contents ? DENOFIZ_SYSTEM_CURSOR_DEFAULT : desc.Cursor;
    node.Disabled  = ( desc.Flags & DENOFIZ_UI_ELEMENT_DISABLED_BIT ) != 0;
    node.Focusable = ( desc.Flags & DENOFIZ_UI_ELEMENT_FOCUSABLE_BIT ) != 0;
    node.Clips     = !node.Contents && ( desc.Scroll.OverflowX != DENOFIZ_UI_OVERFLOW_VISIBLE || desc.Scroll.OverflowY != DENOFIZ_UI_OVERFLOW_VISIBLE );
    node.Captures  = !node.Contents && CapturesPointer( desc );
    UIElementState &state  = *node.State;
    state.FloatingTargetId = 0;
    if ( node.Floating )
    {
        node.Layer = index;
        if ( !node.Hidden )
        {
            m_floating.push_back( index );
        }
        if ( desc.Floating.Target == DENOFIZ_UI_FLOATING_TARGET_ELEMENT )
        {
            state.FloatingTargetId = desc.Floating.TargetId;
        }
        else if ( desc.Floating.Target == DENOFIZ_UI_FLOATING_TARGET_PARENT )
        {
            state.FloatingTargetId = m_frame->Nodes[ node.Parent ].Id;
        }
    }
    m_openStack.push_back( index );
    return id;
}

void UIContext::CloseElement( )
{
    if ( m_openStack.size( ) <= 1 )
    {
        spdlog::error( "UI: CloseElement called without a matching OpenElement" );
        return;
    }
    m_frame->Nodes[ m_openStack.back( ) ].SubtreeEnd = static_cast<uint32_t>( m_frame->Nodes.size( ) );
    m_openStack.pop_back( );
}

uint32_t UIContext::AddText( const DenOfIz_StringView &text, const DenOfIz_UITextDesc &desc, const DenOfIz_UITextRunArray *runs )
{
    if ( !m_inFrame )
    {
        spdlog::error( "UI: Text called outside BeginFrame/EndFrame" );
        return 0;
    }
    const uint32_t index  = AppendNode( UINodeKind::Text, desc.Id, true );
    const uint32_t id     = m_frame->Nodes[ index ].Id;
    const auto     length = static_cast<uint32_t>( text.Chars ? text.NumChars : 0 );
    const auto     offset = static_cast<uint32_t>( m_frame->TextArena.size( ) );
    m_frame->TextArena.insert( m_frame->TextArena.end( ), text.Chars, text.Chars + length );

    UINode &node  = m_frame->Nodes[ index ];
    node.Data     = static_cast<uint32_t>( m_frame->Texts.size( ) );
    node.Captures = true;
    UITextNode &textNode = m_frame->Texts.emplace_back( UITextNode{ desc, offset, length } );
    if ( runs != nullptr && runs->Elements != nullptr && runs->NumElements > 0 )
    {
        textNode.RunOffset = static_cast<uint32_t>( m_frame->TextRuns.size( ) );
        textNode.RunCount  = static_cast<uint32_t>( runs->NumElements );
        m_frame->TextRuns.insert( m_frame->TextRuns.end( ), runs->Elements, runs->Elements + runs->NumElements );
    }
    return id;
}

void UIContext::EndFrame( )
{
    if ( !m_inFrame )
    {
        spdlog::error( "UI: EndFrame called without BeginFrame" );
        return;
    }
    if ( m_openStack.size( ) > 1 )
    {
        spdlog::error( "UI: {} elements were not closed before EndFrame", m_openStack.size( ) - 1 );
        while ( m_openStack.size( ) > 1 )
        {
            CloseElement( );
        }
    }
    m_frame->Nodes[ 0 ].SubtreeEnd = static_cast<uint32_t>( m_frame->Nodes.size( ) );
    m_openStack.clear( );

    ResolveTexts( );
    ApplyStyles( );
    ReleaseStates( );
    ReconcileChildren( );
    LayoutRoot( );

    auto &nodes = m_frame->Nodes;
    for ( uint32_t i = 0; i < nodes.size( ); ++i )
    {
        if ( nodes[ i ].Layer == 0 && !nodes[ i ].Hidden )
        {
            AnimateNode( i );
        }
    }
    ComposeLayer( 0 );
    LayoutFloating( );

    m_frame->Layers.clear( );
    m_frame->Layers.push_back( 0 );
    m_frame->Layers.insert( m_frame->Layers.end( ), m_floating.begin( ), m_floating.end( ) );
    std::stable_sort( m_frame->Layers.begin( ) + 1, m_frame->Layers.end( ), [ &nodes ]( const uint32_t a, const uint32_t b ) { return nodes[ a ].ZIndex < nodes[ b ].ZIndex; } );

    if ( ProcessScrollRequests( ) )
    {
        m_frame->RoundClips.clear( );
        ComposeLayer( 0 );
        for ( const uint32_t floating : m_floating )
        {
            PositionFloating( floating );
            ComposeLayer( floating );
        }
    }

    UpdateGhosts( );
    UpdateHover( );
    BuildCommands( );
    StoreResults( );

    if ( m_frameNumber % 60 == 0 )
    {
        m_text.Evict( CacheAge );
    }

    m_snapTransitions = false;
    m_inFrame         = false;
    std::swap( m_frame, m_last );
}

void UIContext::ResolveTexts( )
{
    for ( UINode &node : m_frame->Nodes )
    {
        if ( node.Kind != UINodeKind::Text || node.Hidden )
        {
            continue;
        }
        const UITextNode  &text   = m_frame->Texts[ node.Data ];
        UIElementState    &state  = *node.State;
        const char        *chars  = m_frame->TextArena.data( ) + text.Offset;
        const UITextParams params = UITextParams::From( text.Desc );
        const UIShapedRun *run    = state.Run;
        const bool         rich   = text.RunCount > 0;
        if ( rich )
        {
            ResolveStyles( text, params );
        }
        const bool cached = run != nullptr && state.TextEpoch == m_text.Epoch( ) && run->Text.size( ) == text.Length && run->FontId == m_text.ResolveFontId( params.FontId ) &&
                            ( rich ? std::ranges::equal( run->Styles, m_styles ) : run->Styles.empty( ) ) &&
                            ( text.Length == 0 || std::memcmp( run->Text.data( ), chars, text.Length ) == 0 );
        uint64_t hash = cached ? run->Hash : UIText::Hash( chars, text.Length );
        if ( cached )
        {
            run->LastUsedFrame = m_text.Frame( );
        }
        else if ( rich )
        {
            hash = UIText::HashStyles( hash, m_styles );
            run  = m_text.ShapeRich( text.Desc.FontId, chars, text.Length, m_styles, hash );
        }
        else
        {
            run = m_text.Shape( text.Desc.FontId, chars, text.Length, hash );
        }
        const bool changed = run != state.Run || !( params == state.TextParams ) || state.TextHash != hash || state.TextLength != text.Length;
        if ( changed || state.TextEpoch != m_text.Epoch( ) )
        {
            state.Run        = run;
            state.TextParams = params;
            state.TextHash   = hash;
            state.TextLength = text.Length;
            state.Layout     = nullptr;
            state.TextEpoch  = m_text.Epoch( );
        }
        if ( changed )
        {
            state.TextChanged = true;
            float minWidth    = m_text.MinContentWidth( run, params );
            if ( minWidth < 0.0f )
            {
                minWidth = m_text.Layout( run, params, -1.0f )->Width;
            }
            state.TextMinWidth = minWidth;
        }
    }
}

void UIContext::ResolveStyles( const UITextNode &text, const UITextParams &params )
{
    m_styles.clear( );
    uint32_t start = 0;
    for ( uint32_t i = 0; i < text.RunCount && start < text.Length; ++i )
    {
        const DenOfIz_UITextRun &source = m_frame->TextRuns[ text.RunOffset + i ];
        const uint32_t           end    = std::min( source.ByteEnd, text.Length );
        if ( end <= start )
        {
            continue;
        }
        UIRunStyle &style     = m_styles.emplace_back( );
        style.ByteStart       = start;
        style.ByteEnd         = end;
        style.Id              = source.Id;
        style.FontId          = m_text.ResolveFontId( source.FontId );
        style.Size            = source.FontSize > 0.0f ? source.FontSize : params.FontSize;
        style.Shift           = source.BaselineShift;
        style.Color           = source.Color;
        style.Background      = source.BackgroundColor;
        style.DecorationColor = source.DecorationColor;
        style.Decoration      = source.Decoration;
        style.Object          = source.ObjectSize.X > 0.0f || source.ObjectSize.Y > 0.0f;
        style.ObjectSize      = style.Object ? source.ObjectSize : DenOfIz_Float2{ };
        style.Image           = style.Object ? source.Image : DENOFIZ_NULL_HANDLE;
        start                 = end;
    }
    if ( start < text.Length || m_styles.empty( ) )
    {
        UIRunStyle &style = m_styles.emplace_back( );
        style.ByteStart   = start;
        style.ByteEnd     = text.Length;
        style.FontId      = m_text.ResolveFontId( params.FontId );
        style.Size        = params.FontSize;
    }
}

void UIContext::ApplyStyles( )
{
    const DenOfIz_Float2 viewport = GetViewportSize( );
    YGNodeStyleSetWidth( m_root, viewport.X );
    YGNodeStyleSetHeight( m_root, viewport.Y );

    for ( uint32_t i = 1; i < m_frame->Nodes.size( ); ++i )
    {
        if ( m_frame->Nodes[ i ].Kind == UINodeKind::Text )
        {
            ApplyTextStyle( i );
        }
        else
        {
            ApplyElementStyle( i );
        }
    }
}

void UIContext::ApplyElementStyle( const uint32_t nodeIndex )
{
    const UINode                &node  = m_frame->Nodes[ nodeIndex ];
    const DenOfIz_UIElementDesc &desc  = m_frame->Elements[ node.Data ];
    UIElementState              &state = *node.State;
    const YGNodeRef              yn    = state.Node;

    if ( state.NodeIsText )
    {
        YGNodeSetMeasureFunc( yn, nullptr );
        YGNodeSetBaselineFunc( yn, nullptr );
        YGNodeSetNodeType( yn, YGNodeTypeDefault );
        state.NodeIsText   = false;
        state.StyleApplied = false;
    }

    const bool hasImage  = !node.Contents && DENOFIZ_HANDLE_IS_VALID( desc.Image.Texture );
    const bool leafImage = hasImage && node.ChildCount == 0;
    if ( leafImage != state.NodeIsImage )
    {
        if ( leafImage )
        {
            YGNodeRemoveAllChildren( yn );
            YGNodeSetMeasureFunc( yn, MeasureImageNode );
            YGNodeMarkDirty( yn );
        }
        else
        {
            YGNodeSetMeasureFunc( yn, nullptr );
        }
        state.NodeIsImage = leafImage;
    }
    if ( hasImage && desc.Image.Texture != state.ImageTexture )
    {
        DenOfIz_TextureDesc textureDesc{ };
        DenOfIz_Texture_GetDesc( desc.Image.Texture, &textureDesc );
        const auto width  = static_cast<float>( textureDesc.Width );
        const auto height = static_cast<float>( textureDesc.Height );
        if ( width != state.ImageWidth || height != state.ImageHeight )
        {
            state.ImageWidth  = width;
            state.ImageHeight = height;
            if ( leafImage )
            {
                YGNodeMarkDirty( yn );
            }
        }
    }
    state.ImageTexture = hasImage ? desc.Image.Texture : DENOFIZ_NULL_HANDLE;

    const auto     overflowX = node.Contents ? DENOFIZ_UI_OVERFLOW_VISIBLE : desc.Scroll.OverflowX;
    const auto     overflowY = node.Contents ? DENOFIZ_UI_OVERFLOW_VISIBLE : desc.Scroll.OverflowY;
    const uint32_t context   = ( node.Floating ? 1u : 0u ) | ( node.Contents ? 2u : 0u ) | ( static_cast<uint32_t>( overflowX ) & 3u ) << 2 | ( static_cast<uint32_t>( overflowY ) & 3u ) << 4;
    if ( state.StyleApplied && state.AppliedContext == context && std::memcmp( &state.AppliedLayout, &desc.Layout, sizeof( DenOfIz_UILayoutDesc ) ) == 0 &&
         std::memcmp( &state.AppliedBorder, &desc.Style.BorderWidth, sizeof( DenOfIz_UIEdges ) ) == 0 )
    {
        return;
    }
    state.StyleApplied   = true;
    state.AppliedContext = context;
    state.AppliedLayout  = desc.Layout;
    state.AppliedBorder  = desc.Style.BorderWidth;

    const DenOfIz_UILayoutDesc &layout = desc.Layout;
    const bool                  row    = IsRow( layout.Direction );
    YGNodeStyleSetDisplay( yn, node.Contents ? YGDisplayContents : layout.Display == DENOFIZ_UI_DISPLAY_NONE ? YGDisplayNone : YGDisplayFlex );
    YGNodeStyleSetFlexDirection( yn, ToYoga( layout.Direction ) );
    YGNodeStyleSetJustifyContent( yn, ToYoga( layout.Justify ) );
    YGNodeStyleSetAlignItems( yn, ToYoga( layout.AlignItems ) );
    YGNodeStyleSetAlignContent( yn, ToYoga( layout.AlignContent ) );
    YGNodeStyleSetFlexWrap( yn, ToYoga( layout.Wrap ) );
    YGNodeStyleSetBoxSizing( yn, ToYoga( layout.BoxSizing ) );
    SetPadding( yn, layout.Padding );
    YGNodeStyleSetBorder( yn, YGEdgeLeft, desc.Style.BorderWidth.Left );
    YGNodeStyleSetBorder( yn, YGEdgeTop, desc.Style.BorderWidth.Top );
    YGNodeStyleSetBorder( yn, YGEdgeRight, desc.Style.BorderWidth.Right );
    YGNodeStyleSetBorder( yn, YGEdgeBottom, desc.Style.BorderWidth.Bottom );
    SetGap( yn, row ? YGGutterColumn : YGGutterRow, layout.Gap );
    SetGap( yn, row ? YGGutterRow : YGGutterColumn, layout.LineGap );
    YGNodeStyleSetAspectRatio( yn, layout.AspectRatio > 0.0f ? layout.AspectRatio : YGUndefined );
    SetSize( yn, layout.Width, YGNodeStyleSetWidth, YGNodeStyleSetWidthPercent, YGNodeStyleSetWidthAuto );
    SetSize( yn, layout.Height, YGNodeStyleSetHeight, YGNodeStyleSetHeightPercent, YGNodeStyleSetHeightAuto );
    SetLimit( yn, layout.MinWidth, YGNodeStyleSetMinWidth, YGNodeStyleSetMinWidthPercent );
    SetLimit( yn, layout.MinHeight, YGNodeStyleSetMinHeight, YGNodeStyleSetMinHeightPercent );
    SetLimit( yn, layout.MaxWidth, YGNodeStyleSetMaxWidth, YGNodeStyleSetMaxWidthPercent );
    SetLimit( yn, layout.MaxHeight, YGNodeStyleSetMaxHeight, YGNodeStyleSetMaxHeightPercent );

    if ( overflowX == DENOFIZ_UI_OVERFLOW_SCROLL || overflowY == DENOFIZ_UI_OVERFLOW_SCROLL )
    {
        YGNodeStyleSetOverflow( yn, YGOverflowScroll );
    }
    else if ( overflowX == DENOFIZ_UI_OVERFLOW_CLIP || overflowY == DENOFIZ_UI_OVERFLOW_CLIP )
    {
        YGNodeStyleSetOverflow( yn, YGOverflowHidden );
    }
    else
    {
        YGNodeStyleSetOverflow( yn, YGOverflowVisible );
    }

    if ( node.Floating )
    {
        ClearMargin( yn );
        ClearInsets( yn );
        YGNodeStyleSetPositionType( yn, YGPositionTypeRelative );
        YGNodeStyleSetFlexGrow( yn, 0.0f );
        YGNodeStyleSetFlexShrink( yn, 0.0f );
        YGNodeStyleSetFlexBasisAuto( yn );
        YGNodeStyleSetAlignSelf( yn, YGAlignAuto );
        return;
    }
    SetMargin( yn, layout.Margin );
    SetInsets( yn, layout.Insets );
    YGNodeStyleSetPositionType( yn, ToYoga( layout.Position ) );
    YGNodeStyleSetFlexGrow( yn, std::max( 0.0f, layout.Grow ) );
    YGNodeStyleSetFlexShrink( yn, std::max( 0.0f, layout.Shrink ) );
    SetSize( yn, layout.Basis, YGNodeStyleSetFlexBasis, YGNodeStyleSetFlexBasisPercent, YGNodeStyleSetFlexBasisAuto );
    YGNodeStyleSetAlignSelf( yn, ToYoga( layout.AlignSelf ) );
}

void UIContext::ApplyTextStyle( const uint32_t nodeIndex )
{
    const UINode   &node  = m_frame->Nodes[ nodeIndex ];
    UIElementState &state = *node.State;
    const YGNodeRef yn    = state.Node;

    if ( !state.NodeIsText )
    {
        YGNodeRemoveAllChildren( yn );
        YGNodeCopyStyle( yn, m_template );
        YGNodeSetMeasureFunc( yn, MeasureTextNode );
        YGNodeSetBaselineFunc( yn, TextBaseline );
        YGNodeSetNodeType( yn, YGNodeTypeText );
        state.NodeIsText   = true;
        state.NodeIsImage  = false;
        state.StyleApplied = false;
        state.TextChanged  = true;
    }
    if ( state.TextChanged )
    {
        YGNodeMarkDirty( yn );
        state.TextChanged = false;
    }

    const UINode &parent    = m_frame->Nodes[ LayoutParent( nodeIndex ) ];
    const bool    parentRow = parent.Kind == UINodeKind::Element && IsRow( m_frame->Elements[ parent.Data ].Layout.Direction );
    YGNodeStyleSetFlexShrink( yn, parentRow ? 1.0f : 0.0f );
    YGNodeStyleSetMinWidth( yn, state.TextMinWidth );
    if ( !state.StyleApplied )
    {
        YGNodeStyleSetFlexGrow( yn, 0.0f );
        YGNodeStyleSetFlexBasisAuto( yn );
        YGNodeStyleSetWidthAuto( yn );
        YGNodeStyleSetHeightAuto( yn );
        YGNodeStyleSetAlignSelf( yn, YGAlignAuto );
        YGNodeStyleSetPositionType( yn, YGPositionTypeRelative );
        state.StyleApplied = true;
    }
}

uint32_t UIContext::LayoutParent( const uint32_t nodeIndex ) const
{
    const auto &nodes  = m_frame->Nodes;
    uint32_t    parent = nodes[ nodeIndex ].Parent;
    while ( parent != UINode::Invalid && nodes[ parent ].Contents )
    {
        parent = nodes[ parent ].Parent;
    }
    return parent;
}

void UIContext::ReleaseStates( )
{
    m_expired.clear( );
    m_states.ForEach(
        [ this ]( UIElementState &state )
        {
            if ( state.LastFrame == m_frameNumber )
            {
                return;
            }
            if ( state.LastFrame + 1 == m_frameNumber )
            {
                if ( state.HasExit )
                {
                    CreateGhost( state );
                }
                if ( state.Node )
                {
                    if ( const YGNodeRef owner = YGNodeGetOwner( state.Node ) )
                    {
                        YGNodeRemoveChild( owner, state.Node );
                    }
                    YGNodeRemoveAllChildren( state.Node );
                }
                if ( state.Id == m_focus )
                {
                    m_focusVanished = state.Id;
                }
            }
            state.IdleSeconds += m_deltaTime;
            if ( state.IdleSeconds > StateLifetime )
            {
                if ( state.Node )
                {
                    YGNodeFree( state.Node );
                    state.Node = nullptr;
                }
                m_expired.push_back( state.Id );
            }
        } );
    for ( const uint32_t id : m_expired )
    {
        m_states.Erase( id );
    }
}

void UIContext::ReconcileChildren( )
{
    auto &nodes = m_frame->Nodes;
    m_reconcile.clear( );
    m_reconcileRanges.clear( );
    m_childScratch.clear( );

    for ( uint32_t i = 0; i < nodes.size( ); ++i )
    {
        const UINode &node = nodes[ i ];
        if ( node.Kind == UINodeKind::Text )
        {
            continue;
        }
        const auto begin = static_cast<uint32_t>( m_childScratch.size( ) );
        for ( uint32_t c = node.FirstChild; c != UINode::Invalid; c = nodes[ c ].NextSibling )
        {
            if ( !nodes[ c ].Floating )
            {
                m_childScratch.push_back( nodes[ c ].State->Node );
            }
        }
        const auto      count = static_cast<uint32_t>( m_childScratch.size( ) ) - begin;
        const YGNodeRef yn    = node.State->Node;
        const size_t    have  = YGNodeGetChildCount( yn );
        bool            same  = have == count;
        for ( uint32_t k = 0; same && k < count; ++k )
        {
            same = YGNodeGetChild( yn, k ) == m_childScratch[ begin + k ];
        }
        if ( same )
        {
            m_childScratch.resize( begin );
            continue;
        }
        m_reconcile.push_back( i );
        m_reconcileRanges.emplace_back( begin, count );
    }

    for ( size_t r = 0; r < m_reconcile.size( ); ++r )
    {
        const UINode   &node = nodes[ m_reconcile[ r ] ];
        const YGNodeRef yn   = node.State->Node;
        for ( size_t k = YGNodeGetChildCount( yn ); k-- > 0; )
        {
            const YGNodeRef child      = YGNodeGetChild( yn, k );
            const auto     *childState = static_cast<const UIElementState *>( YGNodeGetContext( child ) );
            const bool      stays      = childState && childState->LastFrame == m_frameNumber && childState->ParentId == node.Id &&
                                         !nodes[ childState->NodeIndex ].Floating;
            if ( !stays )
            {
                YGNodeRemoveChild( yn, child );
            }
        }
    }

    for ( size_t r = 0; r < m_reconcile.size( ); ++r )
    {
        const UINode   &node              = nodes[ m_reconcile[ r ] ];
        const YGNodeRef yn                = node.State->Node;
        const auto [ begin, count ]       = m_reconcileRanges[ r ];
        for ( uint32_t k = 0; k < count; ++k )
        {
            const YGNodeRef child = m_childScratch[ begin + k ];
            const YGNodeRef owner = YGNodeGetOwner( child );
            if ( owner != nullptr && owner != yn )
            {
                YGNodeRemoveChild( owner, child );
            }
        }
        const size_t    have              = YGNodeGetChildCount( yn );
        bool            prefix            = have <= count;
        for ( size_t k = 0; prefix && k < have; ++k )
        {
            prefix = YGNodeGetChild( yn, k ) == m_childScratch[ begin + k ];
        }
        if ( prefix )
        {
            for ( size_t k = have; k < count; ++k )
            {
                YGNodeInsertChild( yn, m_childScratch[ begin + k ], k );
            }
            continue;
        }
        YGNodeSetChildren( yn, m_childScratch.data( ) + begin, count );
    }
}

void UIContext::LayoutRoot( )
{
    const DenOfIz_Float2 viewport = GetViewportSize( );
    YGNodeCalculateLayout( m_root, viewport.X, viewport.Y, YGDirectionLTR );
}

YGSize UIContext::MeasureTextNode( const YGNodeConstRef node, const float width, const YGMeasureMode widthMode, const float height, const YGMeasureMode heightMode )
{
    const auto *state = static_cast<const UIElementState *>( YGNodeGetContext( node ) );
    if ( state == nullptr || state->Run == nullptr )
    {
        return YGSize{ 0.0f, 0.0f };
    }
    const float         maxWidth = widthMode == YGMeasureModeUndefined ? -1.0f : width;
    const UITextLayout *layout   = state->Owner->m_text.Layout( state->Run, state->TextParams, maxWidth );

    YGSize size{ layout->Width, layout->Height };
    if ( widthMode == YGMeasureModeExactly )
    {
        size.width = width;
    }
    else if ( widthMode == YGMeasureModeAtMost )
    {
        size.width = std::min( size.width, width );
    }
    if ( heightMode == YGMeasureModeExactly )
    {
        size.height = height;
    }
    else if ( heightMode == YGMeasureModeAtMost )
    {
        size.height = std::min( size.height, height );
    }
    return size;
}

YGSize UIContext::MeasureImageNode( const YGNodeConstRef node, const float width, const YGMeasureMode widthMode, const float height, const YGMeasureMode heightMode )
{
    const auto *state         = static_cast<const UIElementState *>( YGNodeGetContext( node ) );
    const float textureWidth  = state != nullptr ? state->ImageWidth : 0.0f;
    const float textureHeight = state != nullptr ? state->ImageHeight : 0.0f;
    if ( widthMode == YGMeasureModeExactly && heightMode == YGMeasureModeExactly )
    {
        return YGSize{ width, height };
    }
    if ( textureWidth <= 0.0f || textureHeight <= 0.0f )
    {
        return YGSize{ widthMode == YGMeasureModeExactly ? width : 0.0f, heightMode == YGMeasureModeExactly ? height : 0.0f };
    }
    if ( widthMode == YGMeasureModeExactly )
    {
        const float derived = width * textureHeight / textureWidth;
        return YGSize{ width, heightMode == YGMeasureModeAtMost ? std::min( derived, height ) : derived };
    }
    if ( heightMode == YGMeasureModeExactly )
    {
        const float derived = height * textureWidth / textureHeight;
        return YGSize{ widthMode == YGMeasureModeAtMost ? std::min( derived, width ) : derived, height };
    }
    float scale = 1.0f;
    if ( widthMode == YGMeasureModeAtMost )
    {
        scale = std::min( scale, width / textureWidth );
    }
    if ( heightMode == YGMeasureModeAtMost )
    {
        scale = std::min( scale, height / textureHeight );
    }
    scale = std::max( scale, 0.0f );
    return YGSize{ textureWidth * scale, textureHeight * scale };
}

float UIContext::TextBaseline( const YGNodeConstRef node, const float width, const float )
{
    const auto *state = static_cast<const UIElementState *>( YGNodeGetContext( node ) );
    if ( state == nullptr || state->Run == nullptr )
    {
        return 0.0f;
    }
    return state->Owner->m_text.Layout( state->Run, state->TextParams, width )->Baseline;
}

void UIContext::AnimateNode( const uint32_t nodeIndex )
{
    UINode         &node  = m_frame->Nodes[ nodeIndex ];
    UIElementState &state = *node.State;
    const YGNodeRef yn    = state.Node;

    DenOfIz_Float2 local{ YGNodeLayoutGetLeft( yn ), YGNodeLayoutGetTop( yn ) };
    DenOfIz_Float2 size{ YGNodeLayoutGetWidth( yn ), YGNodeLayoutGetHeight( yn ) };
    if ( node.Kind == UINodeKind::Root || node.Floating )
    {
        local = DenOfIz_Float2{ 0.0f, 0.0f };
    }
    node.Effect    = UIEffect{ };
    node.Animating = false;

    if ( node.Kind == UINodeKind::Text )
    {
        const UITextLayout *layout = state.Layout;
        const bool widthMatches = layout != nullptr && ( layout->MaxWidth == size.X || ( layout->MaxWidth < 0.0f && layout->Width <= size.X + 0.01f ) );
        if ( widthMatches && state.TextEpoch == m_text.Epoch( ) && layout->Run == state.Run && layout->Params == state.TextParams )
        {
            layout->LastUsedFrame = layout->Run->LastUsedFrame = m_text.Frame( );
        }
        else
        {
            state.Layout    = m_text.Layout( state.Run, state.TextParams, size.X );
            state.TextEpoch = m_text.Epoch( );
        }
        node.Local = local;
        node.Size  = size;
        return;
    }
    if ( node.Kind == UINodeKind::Root )
    {
        node.Local = local;
        node.Size  = size;
        return;
    }
    if ( node.Contents )
    {
        node.Local                 = local;
        node.Size                  = DenOfIz_Float2{ 0.0f, 0.0f };
        node.Background            = DenOfIz_UIColor{ };
        node.BorderColor           = DenOfIz_UIColor{ };
        node.Overlay               = DenOfIz_UIColor{ };
        node.Radius                = DenOfIz_UICorners{ };
        state.Position.Initialized = state.Size.Initialized = false;
        state.Background.Initialized = state.BorderColor.Initialized = state.Overlay.Initialized = state.Radius.Initialized = false;
        state.EnterElapsed = -1.0f;
        state.HasExit      = false;
        state.Scrollable   = false;
        state.ScrollOffset = state.ScrollTarget = state.MaxScroll = DenOfIz_Float2{ 0.0f, 0.0f };
        return;
    }

    const DenOfIz_UIElementDesc    &desc       = m_frame->Elements[ node.Data ];
    const DenOfIz_UITransitionDesc &transition = desc.Transition;
    DenOfIz_UIColor                 background = desc.Style.BackgroundColor;
    DenOfIz_UIColor                 border     = desc.Style.BorderColor;
    DenOfIz_UIColor                 overlay    = desc.Style.OverlayColor;
    DenOfIz_UICorners               radius     = desc.Style.CornerRadius;

    if ( transition.Duration > 0.0f )
    {
        const uint32_t properties = transition.Properties;
        const bool     snap       = m_snapTransitions || state.Appeared;
        const float    dt         = m_deltaTime;
        const float    duration   = transition.Duration;
        const auto     easing     = transition.Easing;
        if ( !node.Floating )
        {
            local = UpdateTween( state.Position, local, dt, duration, easing, snap || state.ParentChanged || !( properties & DENOFIZ_UI_TRANSITION_PROPERTY_POSITION_BIT ) );
        }
        size       = UpdateTween( state.Size, size, dt, duration, easing, snap || !( properties & DENOFIZ_UI_TRANSITION_PROPERTY_SIZE_BIT ) );
        background = UpdateTween( state.Background, background, dt, duration, easing, snap || !( properties & DENOFIZ_UI_TRANSITION_PROPERTY_BACKGROUND_COLOR_BIT ) );
        border     = UpdateTween( state.BorderColor, border, dt, duration, easing, snap || !( properties & DENOFIZ_UI_TRANSITION_PROPERTY_BORDER_COLOR_BIT ) );
        overlay    = UpdateTween( state.Overlay, overlay, dt, duration, easing, snap || !( properties & DENOFIZ_UI_TRANSITION_PROPERTY_OVERLAY_COLOR_BIT ) );
        radius     = UpdateTween( state.Radius, radius, dt, duration, easing, snap || !( properties & DENOFIZ_UI_TRANSITION_PROPERTY_CORNER_RADIUS_BIT ) );

        const auto &enter         = transition.Enter;
        const bool  parentEntered = m_frame->Nodes[ node.Parent ].State->Appeared && m_frame->Nodes[ node.Parent ].Kind != UINodeKind::Root;
        if ( state.Appeared && enter.Flags != 0 && m_frameNumber > 1 && ( !parentEntered || ( transition.Flags & DENOFIZ_UI_TRANSITION_ENTER_WITH_PARENT_BIT ) ) )
        {
            state.EnterElapsed = 0.0f;
        }
        if ( state.EnterElapsed >= 0.0f )
        {
            state.EnterElapsed += dt;
            const float t = Ease( easing, std::min( 1.0f, state.EnterElapsed / duration ) );
            if ( enter.Flags & DENOFIZ_UI_TRANSITION_STATE_OFFSET_BIT )
            {
                node.Effect.Offset = DenOfIz_Float2{ enter.Offset.X * ( 1.0f - t ), enter.Offset.Y * ( 1.0f - t ) };
            }
            if ( enter.Flags & DENOFIZ_UI_TRANSITION_STATE_SCALE_BIT )
            {
                node.Effect.Scale = UIMath::Lerp( enter.Scale, 1.0f, t );
            }
            if ( enter.Flags & DENOFIZ_UI_TRANSITION_STATE_OPACITY_BIT )
            {
                node.Effect.Opacity = UIMath::Lerp( enter.Opacity, 1.0f, t );
            }
            if ( enter.Flags & DENOFIZ_UI_TRANSITION_STATE_BACKGROUND_COLOR_BIT )
            {
                background = UIMath::LerpColor( enter.BackgroundColor, background, std::clamp( t, 0.0f, 1.0f ) );
            }
            if ( enter.Flags & DENOFIZ_UI_TRANSITION_STATE_OVERLAY_COLOR_BIT )
            {
                overlay = UIMath::LerpColor( enter.OverlayColor, overlay, std::clamp( t, 0.0f, 1.0f ) );
            }
            if ( state.EnterElapsed >= duration )
            {
                state.EnterElapsed = -1.0f;
            }
        }
        state.HasExit    = transition.Exit.Flags != 0;
        state.Transition = transition;
        node.Animating   = state.EnterElapsed >= 0.0f || ( !node.Floating && TweenRunning( state.Position, duration ) ) || TweenRunning( state.Size, duration ) ||
                         TweenRunning( state.Background, duration ) || TweenRunning( state.BorderColor, duration ) || TweenRunning( state.Overlay, duration ) ||
                         TweenRunning( state.Radius, duration );
    }
    else
    {
        state.Position.Initialized = state.Size.Initialized = false;
        state.Background.Initialized = state.BorderColor.Initialized = state.Overlay.Initialized = state.Radius.Initialized = false;
        state.EnterElapsed = -1.0f;
        state.HasExit      = false;
    }

    node.Local       = local;
    node.Size        = size;
    node.Background  = background;
    node.BorderColor = border;
    node.Overlay     = overlay;
    node.Radius      = radius;
    UpdateScroll( node, desc );
}

void UIContext::UpdateScroll( UINode &node, const DenOfIz_UIElementDesc &desc )
{
    UIElementState &state   = *node.State;
    const bool      scrollX = desc.Scroll.OverflowX == DENOFIZ_UI_OVERFLOW_SCROLL;
    const bool      scrollY = desc.Scroll.OverflowY == DENOFIZ_UI_OVERFLOW_SCROLL;
    state.Scrollable        = scrollX || scrollY;
    if ( !state.Scrollable )
    {
        state.ScrollOffset = state.ScrollTarget = state.MaxScroll = DenOfIz_Float2{ 0.0f, 0.0f };
        return;
    }

    const YGNodeRef yn     = state.Node;
    float           right  = 0.0f;
    float           bottom = 0.0f;
    ExtendContentBounds( node, right, bottom );
    const auto &border  = desc.Style.BorderWidth;
    const float width   = YGNodeLayoutGetWidth( yn );
    const float height  = YGNodeLayoutGetHeight( yn );
    right += YGNodeLayoutGetPadding( yn, YGEdgeRight ) + border.Right;
    bottom += YGNodeLayoutGetPadding( yn, YGEdgeBottom ) + border.Bottom;

    state.MaxScroll    = DenOfIz_Float2{ scrollX ? std::max( 0.0f, right - width ) : 0.0f, scrollY ? std::max( 0.0f, bottom - height ) : 0.0f };
    state.ViewportSize = DenOfIz_Float2{ width - border.Left - border.Right, height - border.Top - border.Bottom };
    state.ContentSize  = DenOfIz_Float2{ std::max( right, width ) - border.Left - border.Right, std::max( bottom, height ) - border.Top - border.Bottom };

    state.ScrollTarget.X = std::clamp( state.ScrollTarget.X, 0.0f, state.MaxScroll.X );
    state.ScrollTarget.Y = std::clamp( state.ScrollTarget.Y, 0.0f, state.MaxScroll.Y );
    const float follow   = 1.0f - std::exp( -m_deltaTime * 22.0f );
    const float snap     = 0.25f / m_pixelsPerPoint;
    for ( int axis = 0; axis < 2; ++axis )
    {
        float      &offset = axis == 0 ? state.ScrollOffset.X : state.ScrollOffset.Y;
        const float target = axis == 0 ? state.ScrollTarget.X : state.ScrollTarget.Y;
        offset += ( target - offset ) * follow;
        if ( std::fabs( target - offset ) < snap || m_snapTransitions )
        {
            offset = target;
        }
    }
    state.ScrollOffset.X = std::clamp( state.ScrollOffset.X, 0.0f, state.MaxScroll.X );
    state.ScrollOffset.Y = std::clamp( state.ScrollOffset.Y, 0.0f, state.MaxScroll.Y );
    node.Animating       = node.Animating || state.ScrollOffset.X != state.ScrollTarget.X || state.ScrollOffset.Y != state.ScrollTarget.Y;
}

void UIContext::ExtendContentBounds( const UINode &node, float &right, float &bottom ) const
{
    const auto &nodes = m_frame->Nodes;
    for ( uint32_t c = node.FirstChild; c != UINode::Invalid; c = nodes[ c ].NextSibling )
    {
        const UINode &child = nodes[ c ];
        if ( child.Floating || child.Hidden )
        {
            continue;
        }
        if ( child.Contents )
        {
            ExtendContentBounds( child, right, bottom );
            continue;
        }
        const YGNodeRef yn = child.State->Node;
        right              = std::max( right, YGNodeLayoutGetLeft( yn ) + YGNodeLayoutGetWidth( yn ) + YGNodeLayoutGetMargin( yn, YGEdgeRight ) );
        bottom             = std::max( bottom, YGNodeLayoutGetTop( yn ) + YGNodeLayoutGetHeight( yn ) + YGNodeLayoutGetMargin( yn, YGEdgeBottom ) );
    }
}

void UIContext::ComposeLayer( const uint32_t layerRoot )
{
    const DenOfIz_Float2 viewport = GetViewportSize( );
    DenOfIz_UIRect       clip{ 0.0f, 0.0f, viewport.X, viewport.Y };
    uint32_t             roundClip = UINode::Invalid;
    if ( layerRoot != 0 )
    {
        const UINode                &node = m_frame->Nodes[ layerRoot ];
        const DenOfIz_UIElementDesc &desc = m_frame->Elements[ node.Data ];
        if ( desc.Floating.Flags & DENOFIZ_UI_FLOATING_CLIP_TO_TARGET_BIT )
        {
            clip      = node.TargetClip;
            roundClip = node.TargetRoundClip;
        }
    }
    ComposeNode( layerRoot, 1.0f, 0.0f, 0.0f, clip, roundClip );
}

void UIContext::ComposeNode( const uint32_t nodeIndex, const float scale, const float tx, const float ty, const DenOfIz_UIRect &clip, const uint32_t roundClip )
{
    auto  &nodes = m_frame->Nodes;
    UINode &node = nodes[ nodeIndex ];

    float originX = 0.0f;
    float originY = 0.0f;
    if ( node.Kind != UINodeKind::Root && !node.Floating )
    {
        const UINode         &parent = nodes[ node.Parent ];
        const DenOfIz_Float2 &scroll = parent.State->ScrollOffset;
        originX                      = parent.LayoutRect.X - std::round( scroll.X * m_pixelsPerPoint ) / m_pixelsPerPoint;
        originY                      = parent.LayoutRect.Y - std::round( scroll.Y * m_pixelsPerPoint ) / m_pixelsPerPoint;
    }
    const float ppp   = m_pixelsPerPoint;
    const float left  = std::round( ( originX + node.Local.X ) * ppp ) / ppp;
    const float top   = std::round( ( originY + node.Local.Y ) * ppp ) / ppp;
    const float right = node.Kind == UINodeKind::Text ? std::ceil( ( originX + node.Local.X + node.Size.X ) * ppp - 0.01f ) / ppp
                                                      : std::round( ( originX + node.Local.X + node.Size.X ) * ppp ) / ppp;
    const float bottom = std::round( ( originY + node.Local.Y + node.Size.Y ) * ppp ) / ppp;
    node.LayoutRect    = DenOfIz_UIRect{ left, top, std::max( 0.0f, right - left ), std::max( 0.0f, bottom - top ) };

    float          s  = scale;
    float          x  = tx;
    float          y  = ty;
    const UIEffect &e = node.Effect;
    if ( e.Scale != 1.0f || e.Offset.X != 0.0f || e.Offset.Y != 0.0f )
    {
        const float cx = node.LayoutRect.X + node.LayoutRect.Width * 0.5f;
        const float cy = node.LayoutRect.Y + node.LayoutRect.Height * 0.5f;
        x              = scale * ( cx * ( 1.0f - e.Scale ) + e.Offset.X ) + tx;
        y              = scale * ( cy * ( 1.0f - e.Scale ) + e.Offset.Y ) + ty;
        s              = scale * e.Scale;
    }
    node.State->ComposedFrame = m_frameNumber;
    node.Scale        = s;
    node.TranslateX   = x;
    node.TranslateY   = y;
    node.Rect         = TransformRect( node.LayoutRect, s, x, y );
    node.Clip         = clip;
    node.ChildClip    = clip;
    node.RoundClip    = roundClip;
    node.OwnRoundClip = UINode::Invalid;
    node.HasUnion     = false;

    if ( node.Kind == UINodeKind::Text )
    {
        return;
    }
    uint32_t childRoundClip = roundClip;
    if ( node.Clips && node.Kind == UINodeKind::Element )
    {
        const DenOfIz_UIElementDesc &desc   = m_frame->Elements[ node.Data ];
        const auto                  &border = desc.Style.BorderWidth;
        const DenOfIz_UIRect         inner =
            TransformRect( DenOfIz_UIRect{ node.LayoutRect.X + border.Left, node.LayoutRect.Y + border.Top, std::max( 0.0f, node.LayoutRect.Width - border.Left - border.Right ),
                                           std::max( 0.0f, node.LayoutRect.Height - border.Top - border.Bottom ) },
                           s, x, y );
        const bool     clipX   = desc.Scroll.OverflowX != DENOFIZ_UI_OVERFLOW_VISIBLE;
        const bool     clipY   = desc.Scroll.OverflowY != DENOFIZ_UI_OVERFLOW_VISIBLE;
        DenOfIz_UIRect bounded = inner;
        if ( !clipX )
        {
            bounded.X     = clip.X;
            bounded.Width = clip.Width;
        }
        if ( !clipY )
        {
            bounded.Y      = clip.Y;
            bounded.Height = clip.Height;
        }
        node.ChildClip = UIMath::Intersect( clip, bounded );
        if ( clipX && clipY && UIMath::HasCorners( node.Radius ) )
        {
            const DenOfIz_UICorners radius =
                UIMath::FitCorners( inner.Width, inner.Height, UIMath::InnerCorners( UIMath::ScaleCorners( node.Radius, s ), UIMath::ScaleEdges( border, s ) ) );
            if ( UIMath::HasCorners( radius ) )
            {
                childRoundClip    = static_cast<uint32_t>( m_frame->RoundClips.size( ) );
                node.OwnRoundClip = childRoundClip;
                m_frame->RoundClips.push_back( UIRoundClip{ inner, radius, roundClip } );
            }
        }
    }

    for ( uint32_t c = node.FirstChild; c != UINode::Invalid; c = nodes[ c ].NextSibling )
    {
        if ( !nodes[ c ].Floating && !nodes[ c ].Hidden )
        {
            ComposeNode( c, s, x, y, node.ChildClip, childRoundClip );
        }
    }
    if ( node.Contents )
    {
        for ( uint32_t c = node.FirstChild; c != UINode::Invalid; c = nodes[ c ].NextSibling )
        {
            const UINode &child = nodes[ c ];
            if ( child.Floating || child.Hidden || ( child.Contents && !child.HasUnion ) )
            {
                continue;
            }
            const DenOfIz_UIRect &rect = child.Contents ? child.Union : child.Rect;
            node.Union                 = node.HasUnion ? UIMath::Union( node.Union, rect ) : rect;
            node.HasUnion              = true;
        }
    }
}

void UIContext::LayoutFloating( )
{
    auto                &nodes    = m_frame->Nodes;
    const DenOfIz_Float2 viewport = GetViewportSize( );
    for ( const uint32_t index : m_floating )
    {
        UINode                      &node   = nodes[ index ];
        const DenOfIz_UIElementDesc &desc   = m_frame->Elements[ node.Data ];
        const YGNodeRef              yn     = node.State->Node;
        const DenOfIz_UIRect         target = FloatingTarget( index );

        const DenOfIz_UILayoutDesc &layout = desc.Layout;
        if ( !IsSet( layout.Width ) && !IsSet( layout.MaxWidth ) )
        {
            YGNodeStyleSetMaxWidth( yn, viewport.X );
        }
        if ( !IsSet( layout.Height ) && !IsSet( layout.MaxHeight ) )
        {
            YGNodeStyleSetMaxHeight( yn, viewport.Y );
        }
        YGNodeCalculateLayout( yn, target.Width, target.Height, YGDirectionLTR );

        const int32_t enclosing = node.Parent != UINode::Invalid && nodes[ node.Parent ].Layer != 0 ? nodes[ nodes[ node.Parent ].Layer ].ZIndex : 0;
        node.ZIndex             = ( desc.Floating.Flags & DENOFIZ_UI_FLOATING_ABSOLUTE_Z_BIT ) ? desc.Floating.ZIndex : enclosing + desc.Floating.ZIndex;

        for ( uint32_t i = index; i < node.SubtreeEnd; ++i )
        {
            if ( nodes[ i ].Layer == index && !nodes[ i ].Hidden )
            {
                AnimateNode( i );
            }
        }
        PositionFloating( index );
        ComposeLayer( index );
    }
}

DenOfIz_UIRect UIContext::FloatingTarget( const uint32_t nodeIndex )
{
    auto                        &nodes = m_frame->Nodes;
    UINode                      &node  = nodes[ nodeIndex ];
    const DenOfIz_UIElementDesc &desc  = m_frame->Elements[ node.Data ];
    const DenOfIz_Float2         vp    = GetViewportSize( );
    const DenOfIz_UIRect         viewport{ 0.0f, 0.0f, vp.X, vp.Y };
    node.TargetClip      = viewport;
    node.TargetRoundClip = UINode::Invalid;

    if ( desc.Floating.Target == DENOFIZ_UI_FLOATING_TARGET_VIEWPORT )
    {
        return viewport;
    }
    const UINode *target = &nodes[ node.Parent ];
    if ( desc.Floating.Target == DENOFIZ_UI_FLOATING_TARGET_ELEMENT )
    {
        if ( const UIElementState *state = m_states.Find( desc.Floating.TargetId ) )
        {
            const UINode *candidate = state->LastFrame == m_frameNumber ? &nodes[ state->NodeIndex ] : nullptr;
            const bool    hidden    = candidate != nullptr && candidate->Hidden;
            if ( !hidden && candidate != nullptr && state->ComposedFrame == m_frameNumber )
            {
                target = candidate;
            }
            else if ( !hidden && state->LastFrame + 1 >= m_frameNumber && state->HasRect )
            {
                return state->Rect;
            }
        }
    }
    while ( target->Contents && !target->HasUnion )
    {
        target = &nodes[ target->Parent ];
    }
    if ( target->Kind == UINodeKind::Root )
    {
        return viewport;
    }
    node.TargetClip      = target->Clip;
    node.TargetRoundClip = target->RoundClip;
    return target->Contents ? target->Union : target->Rect;
}

void UIContext::PositionFloating( const uint32_t nodeIndex )
{
    UINode                      &node     = m_frame->Nodes[ nodeIndex ];
    const DenOfIz_UIElementDesc &desc     = m_frame->Elements[ node.Data ];
    const auto                  &floating = desc.Floating;
    const DenOfIz_Float2         viewport = GetViewportSize( );
    const DenOfIz_UIRect         target   = FloatingTarget( nodeIndex );
    const DenOfIz_UIRect         local{ 0.0f, 0.0f, node.Size.X, node.Size.Y };

    auto place = [ & ]( const DenOfIz_UIAnchor targetAnchor, const DenOfIz_UIAnchor anchor, const float offsetX, const float offsetY )
    {
        const DenOfIz_Float2 t = AnchorPoint( target, targetAnchor );
        const DenOfIz_Float2 a = AnchorPoint( local, anchor );
        return DenOfIz_Float2{ t.X - a.X + offsetX, t.Y - a.Y + offsetY };
    };

    DenOfIz_Float2 position = place( floating.TargetAnchor, floating.Anchor, floating.Offset.X, floating.Offset.Y );
    if ( floating.Flags & DENOFIZ_UI_FLOATING_KEEP_IN_VIEWPORT_BIT )
    {
        const float w = node.Size.X;
        const float h = node.Size.Y;
        if ( position.Y < 0.0f || position.Y + h > viewport.Y )
        {
            const DenOfIz_Float2 flipped = place( MirrorY( floating.TargetAnchor ), MirrorY( floating.Anchor ), floating.Offset.X, -floating.Offset.Y );
            if ( flipped.Y >= 0.0f && flipped.Y + h <= viewport.Y )
            {
                position.Y = flipped.Y;
            }
        }
        if ( position.X < 0.0f || position.X + w > viewport.X )
        {
            const DenOfIz_Float2 flipped = place( MirrorX( floating.TargetAnchor ), MirrorX( floating.Anchor ), -floating.Offset.X, floating.Offset.Y );
            if ( flipped.X >= 0.0f && flipped.X + w <= viewport.X )
            {
                position.X = flipped.X;
            }
        }
        position.X = w >= viewport.X ? 0.0f : std::clamp( position.X, 0.0f, viewport.X - w );
        position.Y = h >= viewport.Y ? 0.0f : std::clamp( position.Y, 0.0f, viewport.Y - h );
    }

    UIElementState &state      = *node.State;
    const auto     &transition = desc.Transition;
    if ( transition.Duration > 0.0f && ( transition.Properties & DENOFIZ_UI_TRANSITION_PROPERTY_POSITION_BIT ) )
    {
        const DenOfIz_Float2 anchor = AnchorPoint( target, floating.TargetAnchor );
        DenOfIz_Float2       offset{ position.X - anchor.X, position.Y - anchor.Y };
        if ( state.PositionTweenFrame != m_frameNumber )
        {
            state.PositionTweenFrame = m_frameNumber;
            offset                   = UpdateTween( state.Position, offset, m_deltaTime, transition.Duration, transition.Easing, m_snapTransitions || state.Appeared );
        }
        else
        {
            offset = state.Position.Current;
        }
        position = DenOfIz_Float2{ anchor.X + offset.X, anchor.Y + offset.Y };
    }

    position.X = std::round( position.X * m_pixelsPerPoint ) / m_pixelsPerPoint;
    position.Y = std::round( position.Y * m_pixelsPerPoint ) / m_pixelsPerPoint;
    node.Local = position;
}

bool UIContext::ProcessScrollRequests( )
{
    bool  changed = false;
    auto &nodes   = m_frame->Nodes;
    for ( const UIScrollRequest &request : m_scrollRequests )
    {
        UIElementState *it = m_states.Find( request.Id );
        if ( it == nullptr || it->LastFrame != m_frameNumber )
        {
            continue;
        }
        const UINode  &node = nodes[ it->NodeIndex ];
        DenOfIz_UIRect rect = node.Contents ? node.Union : node.LayoutRect;
        if ( node.Floating || node.Hidden || ( node.Contents && !node.HasUnion ) )
        {
            continue;
        }
        if ( request.Caret )
        {
            const UIElementState &state = (*it);
            if ( node.Kind != UINodeKind::Text || state.Layout == nullptr )
            {
                continue;
            }
            const UITextLayout &layout = *state.Layout;
            const uint32_t      line   = layout.LineOfOffset( request.ByteOffset );
            const auto          align  = m_frame->Texts[ node.Data ].Desc.Align;
            const float         caretX = layout.AlignOffset( line, node.Size.X, align ) + layout.CaretX( line, request.ByteOffset );
            rect = DenOfIz_UIRect{ rect.X + caretX - 2.0f, rect.Y + layout.Lines[ line ].Top, 4.0f, layout.Lines[ line ].Height };
        }

        for ( uint32_t p = node.Parent; p != UINode::Invalid; p = nodes[ p ].Parent )
        {
            const UINode   &parent = nodes[ p ];
            UIElementState &ps     = *parent.State;
            if ( parent.Kind == UINodeKind::Element && ps.Scrollable )
            {
                const auto          &border = m_frame->Elements[ parent.Data ].Style.BorderWidth;
                const DenOfIz_UIRect view{ parent.LayoutRect.X + border.Left, parent.LayoutRect.Y + border.Top, ps.ViewportSize.X, ps.ViewportSize.Y };
                auto                 reveal = [ & ]( const float start, const float length, const float viewStart, const float viewLength, float &offset, const float max )
                {
                    float delta = 0.0f;
                    if ( start < viewStart )
                    {
                        delta = start - viewStart;
                    }
                    else if ( start + length > viewStart + viewLength )
                    {
                        delta = std::min( start + length - ( viewStart + viewLength ), start - viewStart );
                    }
                    const float next    = std::clamp( offset + delta, 0.0f, max );
                    const float applied = next - offset;
                    offset              = next;
                    return applied;
                };
                const float dx = reveal( rect.X, rect.Width, view.X, view.Width, ps.ScrollOffset.X, ps.MaxScroll.X );
                const float dy = reveal( rect.Y, rect.Height, view.Y, view.Height, ps.ScrollOffset.Y, ps.MaxScroll.Y );
                ps.ScrollTarget = ps.ScrollOffset;
                rect.X -= dx;
                rect.Y -= dy;
                changed |= dx != 0.0f || dy != 0.0f;
            }
            if ( parent.Floating )
            {
                break;
            }
        }
    }
    m_scrollRequests.clear( );
    return changed;
}

void UIContext::CreateGhost( UIElementState &state )
{
    if ( state.CommandEnd <= state.CommandBegin || state.CommandEnd > m_last->Commands.size( ) || state.Transition.Duration <= 0.0f )
    {
        return;
    }
    UIElementState *parent = m_states.Find( state.ParentId );
    if ( !( state.Transition.Flags & DENOFIZ_UI_TRANSITION_EXIT_WITH_PARENT_BIT ) && ( parent == nullptr || parent->LastFrame != m_frameNumber ) )
    {
        return;
    }
    UIGhost &ghost    = m_ghosts.emplace_back( );
    ghost.Id          = state.Id;
    ghost.ParentId    = state.ParentId;
    ghost.Floating    = state.WasFloating;
    ghost.Rect        = state.Rect;
    ghost.LocalOffset = state.LocalOffset;
    ghost.Transition  = state.Transition;
    ghost.Commands.assign( m_last->Commands.begin( ) + state.CommandBegin, m_last->Commands.begin( ) + state.CommandEnd );

    const auto &nodes = m_last->Nodes;
    if ( state.NodeIndex >= nodes.size( ) || nodes[ state.NodeIndex ].Id != state.Id )
    {
        return;
    }
    const UINode &node = nodes[ state.NodeIndex ];
    ghost.Clip         = node.Clip;
    std::array<uint32_t, UIMaxRoundedClips> chain{ };
    uint32_t                                count = 0;
    for ( uint32_t clip = node.RoundClip; clip != UINode::Invalid && count < UIMaxRoundedClips; clip = m_last->RoundClips[ clip ].Parent )
    {
        chain[ count++ ] = clip;
    }
    ghost.ChainCount = count;
    for ( uint32_t i = 0; i < count; ++i )
    {
        ghost.Chain[ i ] = m_last->RoundClips[ chain[ count - 1 - i ] ];
    }
    if ( node.Floating )
    {
        return;
    }
    for ( uint32_t n = node.Parent; n != UINode::Invalid; n = nodes[ n ].Parent )
    {
        const UINode &ancestor = nodes[ n ];
        if ( ancestor.Kind == UINodeKind::Element && !ancestor.Contents && ( ancestor.Effect.Opacity < 1.0f || ancestor.Overlay.A > 0 ) )
        {
            ghost.Layer = UIMath::ComposeLayer( UIMath::OverlayLayer( ancestor.Overlay, ancestor.Effect.Opacity ), ghost.Layer );
        }
        if ( ancestor.Floating )
        {
            break;
        }
    }
}

void UIContext::UpdateGhosts( )
{
    for ( UIGhost &ghost : m_ghosts )
    {
        ghost.Elapsed += m_deltaTime;
        for ( const UICommand &command : ghost.Commands )
        {
            if ( command.Type == UICommandType::Text )
            {
                command.Text.Layout->LastUsedFrame = command.Text.Layout->Run->LastUsedFrame = m_text.Frame( );
            }
        }
    }
    std::erase_if( m_ghosts,
                   [ this ]( const UIGhost &ghost )
                   {
                       if ( ghost.Elapsed >= ghost.Transition.Duration )
                       {
                           return true;
                       }
                       UIElementState *it = m_states.Find( ghost.Id );
                       return it != nullptr && it->LastFrame == m_frameNumber;
                   } );
}

void UIContext::StoreResults( )
{
    auto &nodes = m_frame->Nodes;
    m_animating = !m_ghosts.empty( );
    for ( UINode &node : nodes )
    {
        m_animating = m_animating || ( node.Animating && !node.Hidden );
        UIElementState &state = *node.State;
        state.Rect            = node.Contents ? node.Union : node.Rect;
        state.HasRect         = !node.Hidden && ( !node.Contents || node.HasUnion );
        state.LayoutRect      = node.LayoutRect;
        state.WasFloating     = node.Floating;
        if ( node.Parent != UINode::Invalid )
        {
            const UINode &parent = nodes[ node.Parent ];
            state.LocalOffset    = DenOfIz_Float2{ node.Rect.X - parent.Rect.X, node.Rect.Y - parent.Rect.Y };
        }
        if ( node.Hidden && node.Id == m_focus )
        {
            m_focusVanished = m_focus;
        }
    }
}

void UIContext::ScrollIntoView( const uint32_t id )
{
    m_scrollRequests.push_back( UIScrollRequest{ id, false, 0 } );
}

void UIContext::ScrollCaretIntoView( const uint32_t textId, const uint32_t byteOffset )
{
    m_scrollRequests.push_back( UIScrollRequest{ textId, true, byteOffset } );
}

bool UIContext::DeclaredInLastFrame( const UIElementState &state ) const
{
    const uint64_t completed = m_inFrame ? m_frameNumber - 1 : m_frameNumber;
    return state.LastFrame == completed || ( m_inFrame && state.LastFrame == m_frameNumber && state.PrevFrame == completed );
}

bool UIContext::GetScrollState( const uint32_t id, DenOfIz_UIScrollState *out ) const
{
    UIElementState *it = m_states.Find( id );
    if ( it == nullptr || !it->Scrollable || !DeclaredInLastFrame( *it ) )
    {
        *out = DenOfIz_UIScrollState{ };
        return false;
    }
    const UIElementState &state = (*it);
    out->Offset                 = state.ScrollOffset;
    out->MaxOffset              = state.MaxScroll;
    out->ViewportSize           = state.ViewportSize;
    out->ContentSize            = state.ContentSize;
    return true;
}

void UIContext::SetScrollOffset( const uint32_t id, const DenOfIz_Float2 offset )
{
    UIElementState *it = m_states.Find( id );
    if ( it == nullptr )
    {
        return;
    }
    UIElementState &state = (*it);
    state.ScrollOffset    = DenOfIz_Float2{ std::max( 0.0f, offset.X ), std::max( 0.0f, offset.Y ) };
    state.ScrollTarget    = state.ScrollOffset;
}

bool UIContext::GetElementRect( const uint32_t id, DenOfIz_UIRect *out ) const
{
    UIElementState *it = m_states.Find( id );
    if ( it == nullptr || !DeclaredInLastFrame( *it ) || !it->HasRect )
    {
        *out = DenOfIz_UIRect{ };
        return false;
    }
    *out = it->Rect;
    return true;
}

DenOfIz_Float2 UIContext::MeasureText( const DenOfIz_StringView &text, const DenOfIz_UITextDesc &desc, const float maxWidth )
{
    return m_text.Measure( text.Chars, static_cast<uint32_t>( text.Chars ? text.NumChars : 0 ), desc, maxWidth );
}

bool UIContext::GetTextCaretRect( const uint32_t textId, const uint32_t byteOffset, DenOfIz_UIRect *out ) const
{
    *out              = DenOfIz_UIRect{ };
    const UINode *node = FindLastNode( textId );
    if ( node == nullptr || node->Kind != UINodeKind::Text || node->Hidden || node->State->Layout == nullptr )
    {
        return false;
    }
    const UITextLayout &layout = *node->State->Layout;
    const auto          align  = m_last->Texts[ node->Data ].Desc.Align;
    const uint32_t      line   = layout.LineOfOffset( byteOffset );
    const float         x      = layout.AlignOffset( line, node->LayoutRect.Width, align ) + layout.CaretX( line, byteOffset );
    *out = DenOfIz_UIRect{ node->Rect.X + x * node->Scale, node->Rect.Y + layout.Lines[ line ].Top * node->Scale, 0.0f, layout.Lines[ line ].Height * node->Scale };
    return true;
}

uint32_t UIContext::GetTextOffsetAtPoint( const uint32_t textId, const DenOfIz_Float2 point ) const
{
    const UINode *node = FindLastNode( textId );
    if ( node == nullptr || node->Kind != UINodeKind::Text || node->Hidden || node->State->Layout == nullptr || node->Scale <= 0.0f )
    {
        return 0;
    }
    const float x = ( point.X - node->Rect.X ) / node->Scale;
    const float y = ( point.Y - node->Rect.Y ) / node->Scale;
    return node->State->Layout->OffsetAt( x, y, node->LayoutRect.Width, m_last->Texts[ node->Data ].Desc.Align );
}

uint32_t UIContext::GetTextRunAtPoint( const uint32_t textId, const DenOfIz_Float2 point ) const
{
    const UINode *node = FindLastNode( textId );
    if ( node == nullptr || node->Kind != UINodeKind::Text || node->Hidden || node->State->Layout == nullptr || node->Scale <= 0.0f )
    {
        return 0;
    }
    const UITextLayout &layout = *node->State->Layout;
    const float         x      = ( point.X - node->Rect.X ) / node->Scale;
    const float         y      = ( point.Y - node->Rect.Y ) / node->Scale;
    const uint32_t      style  = layout.StyleAtPoint( x, y, node->LayoutRect.Width, m_last->Texts[ node->Data ].Desc.Align );
    return style < layout.Run->Styles.size( ) ? layout.Run->Styles[ style ].Id : 0;
}

uint32_t UIContext::GetTextLineCount( const uint32_t textId ) const
{
    const UINode *node = FindLastNode( textId );
    if ( node == nullptr || node->Kind != UINodeKind::Text || node->Hidden || node->State->Layout == nullptr )
    {
        return 0;
    }
    return static_cast<uint32_t>( node->State->Layout->Lines.size( ) );
}

bool UIContext::GetTextLine( const uint32_t textId, const uint32_t lineIndex, DenOfIz_UITextLine *out ) const
{
    *out               = DenOfIz_UITextLine{ };
    const UINode *node = FindLastNode( textId );
    if ( node == nullptr || node->Kind != UINodeKind::Text || node->Hidden || node->State->Layout == nullptr || lineIndex >= node->State->Layout->Lines.size( ) )
    {
        return false;
    }
    const UITextLayout     &layout = *node->State->Layout;
    const UITextLineLayout &line   = layout.Lines[ lineIndex ];
    const float             x      = layout.AlignOffset( lineIndex, node->LayoutRect.Width, m_last->Texts[ node->Data ].Desc.Align );
    out->StartOffset               = line.ByteStart;
    out->EndOffset                 = line.ByteEnd;
    out->Rect = DenOfIz_UIRect{ node->Rect.X + x * node->Scale, node->Rect.Y + line.Top * node->Scale, line.Width * node->Scale, line.Height * node->Scale };
    return true;
}

uint32_t UIContext::NodeCount( ) const
{
    return static_cast<uint32_t>( m_last->Nodes.size( ) );
}

bool UIContext::GetNode( const uint32_t index, DenOfIz_UINodeInfo *out ) const
{
    *out                 = DenOfIz_UINodeInfo{ };
    const UIFrame &frame = *m_last;
    if ( index >= frame.Nodes.size( ) )
    {
        return false;
    }
    const UINode         &node  = frame.Nodes[ index ];
    const UIElementState &state = *node.State;
    out->Id                     = node.Id;
    out->Parent                 = node.Parent;
    out->Layer                  = node.Layer;
    out->ZIndex                 = node.ZIndex;
    out->Kind = node.Kind == UINodeKind::Root ? DENOFIZ_UI_NODE_KIND_ROOT : node.Kind == UINodeKind::Text ? DENOFIZ_UI_NODE_KIND_TEXT : DENOFIZ_UI_NODE_KIND_ELEMENT;
    out->Opacity = 1.0f;
    for ( uint32_t n = index; n != UINode::Invalid; n = frame.Nodes[ n ].Parent )
    {
        out->Depth += n != index ? 1 : 0;
    }
    for ( uint32_t n = index; n != UINode::Invalid; n = frame.Nodes[ n ].Parent )
    {
        out->Opacity *= frame.Nodes[ n ].Effect.Opacity;
        if ( frame.Nodes[ n ].Floating )
        {
            break;
        }
    }

    DenOfIz_UINodeFlags flags = 0;
    flags |= node.Hidden ? DENOFIZ_UI_NODE_HIDDEN_BIT : 0;
    flags |= node.Floating ? DENOFIZ_UI_NODE_FLOATING_BIT : 0;
    flags |= node.Contents ? DENOFIZ_UI_NODE_CONTENTS_BIT : 0;
    flags |= node.Clips ? DENOFIZ_UI_NODE_CLIPS_BIT : 0;
    flags |= state.Scrollable && !node.Hidden ? DENOFIZ_UI_NODE_SCROLLABLE_BIT : 0;
    flags |= node.Focusable ? DENOFIZ_UI_NODE_FOCUSABLE_BIT : 0;
    flags |= node.Disabled ? DENOFIZ_UI_NODE_DISABLED_BIT : 0;
    flags |= node.Captures && node.Kind == UINodeKind::Element ? DENOFIZ_UI_NODE_CAPTURES_BIT : 0;
    flags |= node.Animating ? DENOFIZ_UI_NODE_ANIMATING_BIT : 0;

    if ( !node.Hidden )
    {
        out->Rect        = node.Contents ? ( node.HasUnion ? node.Union : DenOfIz_UIRect{ } ) : node.Rect;
        out->LayoutRect  = node.LayoutRect;
        out->VisibleRect = UIMath::Intersect( out->Rect, node.Clip );
        if ( out->VisibleRect.Width <= 0.0f || out->VisibleRect.Height <= 0.0f )
        {
            out->VisibleRect = DenOfIz_UIRect{ out->Rect.X, out->Rect.Y, 0.0f, 0.0f };
        }
    }

    if ( node.Kind == UINodeKind::Text )
    {
        const UITextNode &text = frame.Texts[ node.Data ];
        const char       *utf8 = frame.TextArena.data( ) + text.Offset;
        out->Text              = DenOfIz_StringView{ utf8, text.Length };
        out->TextColor         = text.Desc.Color;
        out->FontId            = state.TextParams.FontId;
        out->FontSize          = state.TextParams.FontSize;
        if ( state.Layout != nullptr && !node.Hidden )
        {
            const UITextLayout &layout = *state.Layout;
            const float         slack  = 1.0f / m_pixelsPerPoint + 0.01f;
            out->ContentSize           = DenOfIz_Float2{ layout.Width, layout.Height };
            out->LineCount             = static_cast<uint32_t>( layout.Lines.size( ) );
            flags |= TextTruncated( layout, utf8, text.Length ) ? DENOFIZ_UI_NODE_TRUNCATED_BIT : 0;
            flags |= layout.Width > node.LayoutRect.Width + slack || layout.Height > node.LayoutRect.Height + slack ? DENOFIZ_UI_NODE_OVERFLOWS_BIT : 0;
        }
    }
    else if ( node.Kind == UINodeKind::Element )
    {
        const DenOfIz_UIElementDesc &desc = frame.Elements[ node.Data ];
        out->BackgroundColor              = node.Background;
        out->BorderColor                  = node.BorderColor;
        out->OverlayColor                 = node.Overlay;
        out->BorderWidth                  = node.Contents ? DenOfIz_UIEdges{ } : desc.Style.BorderWidth;
        out->CornerRadius                 = node.Radius;
        out->Image                        = state.ImageTexture;
        flags |= desc.Layout.Position == DENOFIZ_UI_POSITION_ABSOLUTE && !node.Floating ? DENOFIZ_UI_NODE_ABSOLUTE_BIT : 0;
        if ( state.Scrollable && !node.Hidden )
        {
            out->ContentSize  = state.ContentSize;
            out->ScrollOffset = state.ScrollOffset;
        }
    }
    out->Flags = flags;
    return true;
}

uint32_t UIContext::FindNode( const uint32_t id ) const
{
    const UINode *node = FindLastNode( id );
    return node != nullptr ? static_cast<uint32_t>( node - m_last->Nodes.data( ) ) : UINode::Invalid;
}

bool UIContext::IsAnimating( ) const
{
    return m_animating;
}
