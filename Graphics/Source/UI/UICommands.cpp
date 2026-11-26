/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include <array>
#include <cmath>
#include <cstdio>
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/UI/UIContext.h"

using namespace DenOfIz;

namespace
{
    using UIMath::InnerCorners;
    using UIMath::ScaleCorners;
    using UIMath::ScaleEdges;

    DenOfIz_UIRect Inset( const DenOfIz_UIRect &r, const DenOfIz_UIEdges &e )
    {
        return DenOfIz_UIRect{ r.X + e.Left, r.Y + e.Top, std::max( 0.0f, r.Width - e.Left - e.Right ), std::max( 0.0f, r.Height - e.Top - e.Bottom ) };
    }

    DenOfIz_UIRect TransformRect( const DenOfIz_UIRect &r, const float scale, const float tx, const float ty )
    {
        return DenOfIz_UIRect{ r.X * scale + tx, r.Y * scale + ty, r.Width * scale, r.Height * scale };
    }

    float EaseExit( const DenOfIz_UIEasing easing, const float t )
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
        default:
        {
            const float inv = 1.0f - t;
            return 1.0f - inv * inv * inv;
        }
        }
    }
} // namespace

void UIContext::PushFill( const DenOfIz_UIRect &rect, const DenOfIz_UIColor &color, const DenOfIz_UICorners &radius )
{
    if ( color.A == 0 || rect.Width <= 0.0f || rect.Height <= 0.0f )
    {
        return;
    }
    UICommand &command  = m_frame->Commands.emplace_back( );
    command.Type        = UICommandType::Fill;
    command.Rect        = rect;
    command.Fill.Color  = color;
    command.Fill.Radius = radius;
}

void UIContext::PushBorder( const DenOfIz_UIRect &rect, const DenOfIz_UIColor &color, const DenOfIz_UIEdges &widths, const DenOfIz_UICorners &radius )
{
    if ( color.A == 0 || !UIMath::HasEdges( widths ) || rect.Width <= 0.0f || rect.Height <= 0.0f )
    {
        return;
    }
    UICommand &command    = m_frame->Commands.emplace_back( );
    command.Type          = UICommandType::Border;
    command.Rect          = rect;
    command.Border.Color  = color;
    command.Border.Widths = widths;
    command.Border.Radius = radius;
}

void UIContext::PushClip( const DenOfIz_UIRect &rect, const DenOfIz_UICorners &radius, const bool rounded, const bool fixed )
{
    UICommand &command   = m_frame->Commands.emplace_back( );
    command.Type         = UICommandType::PushClip;
    command.Rect         = rect;
    command.Clip.Radius  = radius;
    command.Clip.Rounded = rounded;
    command.Clip.Fixed   = fixed;
}

void UIContext::PopClips( const uint32_t count )
{
    for ( uint32_t i = 0; i < count; ++i )
    {
        m_frame->Commands.emplace_back( ).Type = UICommandType::PopClip;
    }
}

uint32_t UIContext::PushClipChain( const UIFrame &frame, const uint32_t roundClip, const DenOfIz_UIRect &clip )
{
    std::array<uint32_t, UIMaxRoundedClips> chain{ };
    uint32_t                                count = 0;
    for ( uint32_t index = roundClip; index != UINode::Invalid && count < UIMaxRoundedClips; index = frame.RoundClips[ index ].Parent )
    {
        chain[ count++ ] = index;
    }
    for ( uint32_t i = count; i-- > 0; )
    {
        const UIRoundClip &entry = frame.RoundClips[ chain[ i ] ];
        PushClip( entry.Rect, entry.Radius, true, true );
    }
    PushClip( clip, DenOfIz_UICorners{ }, false, true );
    return count + 1;
}

void UIContext::BuildCommands( )
{
    auto &commands = m_frame->Commands;
    commands.clear( );
    commands.reserve( m_frame->Nodes.size( ) * 2 );
    m_ghostParents.clear( );
    for ( const UIGhost &ghost : m_ghosts )
    {
        if ( !ghost.Floating )
        {
            m_ghostParents.push_back( ghost.ParentId );
        }
    }
    std::sort( m_ghostParents.begin( ), m_ghostParents.end( ) );

    for ( size_t layer = 0; layer < m_frame->Layers.size( ); ++layer )
    {
        EmitNode( m_frame->Layers[ layer ] );
        if ( layer == 0 )
        {
            for ( const UIGhost &ghost : m_ghosts )
            {
                UIElementState *parent = m_states.Find( ghost.ParentId );
                if ( !ghost.Floating && ( parent == nullptr || parent->LastFrame != m_frameNumber ) )
                {
                    EmitGhost( ghost, true );
                }
            }
        }
    }
    for ( const UIGhost &ghost : m_ghosts )
    {
        if ( ghost.Floating )
        {
            EmitGhost( ghost, false );
        }
    }
    if ( m_debugFlags != 0 )
    {
        EmitDebug( );
    }
}

void UIContext::EmitNode( const uint32_t nodeIndex )
{
    const auto     &nodes    = m_frame->Nodes;
    const UINode   &node     = nodes[ nodeIndex ];
    UIElementState &state    = *node.State;
    auto           &commands = m_frame->Commands;

    if ( node.Kind == UINodeKind::Root )
    {
        EmitGhosts( node.Id, false );
        for ( uint32_t c = node.FirstChild; c != UINode::Invalid; c = nodes[ c ].NextSibling )
        {
            if ( !nodes[ c ].Floating && !nodes[ c ].Hidden )
            {
                EmitNode( c );
            }
        }
        EmitGhosts( node.Id, true );
        return;
    }

    const auto begin   = static_cast<uint32_t>( commands.size( ) );
    state.CommandBegin = state.CommandEnd = begin;
    if ( node.Hidden || node.Clip.Width <= 0.0f || node.Clip.Height <= 0.0f )
    {
        return;
    }
    if ( node.Kind == UINodeKind::Text )
    {
        EmitText( node );
        state.CommandEnd = static_cast<uint32_t>( commands.size( ) );
        return;
    }

    const DenOfIz_UIElementDesc &desc        = m_frame->Elements[ node.Data ];
    uint32_t                     targetClips = 0;
    if ( node.Floating && ( desc.Floating.Flags & DENOFIZ_UI_FLOATING_CLIP_TO_TARGET_BIT ) )
    {
        targetClips = PushClipChain( *m_frame, node.TargetRoundClip, node.TargetClip );
    }

    if ( node.Contents )
    {
        EmitGhosts( node.Id, false );
        for ( uint32_t c = node.FirstChild; c != UINode::Invalid; c = nodes[ c ].NextSibling )
        {
            if ( !nodes[ c ].Floating && !nodes[ c ].Hidden )
            {
                EmitNode( c );
            }
        }
        EmitGhosts( node.Id, true );
        PopClips( targetClips );
        state.CommandEnd = static_cast<uint32_t>( commands.size( ) );
        return;
    }

    const float s       = node.Scale;
    const bool  visible = UIMath::Intersects( node.Rect, node.Clip );
    const bool  layer   = node.Effect.Opacity < 1.0f || node.Overlay.A > 0;
    if ( layer )
    {
        UICommand &command = commands.emplace_back( );
        command.Type       = UICommandType::PushLayer;
        command.Rect       = node.Rect;
        command.Layer      = UIMath::OverlayLayer( node.Overlay, node.Effect.Opacity );
    }

    const DenOfIz_UICorners radius = ScaleCorners( node.Radius, s );
    const DenOfIz_UIEdges   border = ScaleEdges( desc.Style.BorderWidth, s );
    if ( visible )
    {
        PushFill( node.Rect, node.Background, radius );
    }

    const DenOfIz_UIImageDesc &image = desc.Image;
    if ( visible && DENOFIZ_HANDLE_IS_VALID( image.Texture ) )
    {
        DenOfIz_UIRect area = Inset( node.Rect, border );
        DenOfIz_Float4 uv   = image.UV;
        if ( uv.X == 0.0f && uv.Y == 0.0f && uv.Z == 0.0f && uv.W == 0.0f )
        {
            uv = DenOfIz_Float4{ 0.0f, 0.0f, 1.0f, 1.0f };
        }
        const float sourceW = std::fabs( uv.Z - uv.X ) * std::max( 1.0f, state.ImageWidth );
        const float sourceH = std::fabs( uv.W - uv.Y ) * std::max( 1.0f, state.ImageHeight );
        if ( image.Fit == DENOFIZ_UI_IMAGE_FIT_CONTAIN && sourceW > 0.0f && sourceH > 0.0f )
        {
            const float scale = std::min( area.Width / sourceW, area.Height / sourceH );
            const float w     = sourceW * scale;
            const float h     = sourceH * scale;
            area              = DenOfIz_UIRect{ area.X + ( area.Width - w ) * 0.5f, area.Y + ( area.Height - h ) * 0.5f, w, h };
        }
        else if ( image.Fit == DENOFIZ_UI_IMAGE_FIT_COVER && sourceW > 0.0f && sourceH > 0.0f && area.Width > 0.0f && area.Height > 0.0f )
        {
            const float scale = std::max( area.Width / sourceW, area.Height / sourceH );
            const float fx    = area.Width / ( sourceW * scale );
            const float fy    = area.Height / ( sourceH * scale );
            const float du    = ( uv.Z - uv.X ) * ( 1.0f - fx ) * 0.5f;
            const float dv    = ( uv.W - uv.Y ) * ( 1.0f - fy ) * 0.5f;
            uv                = DenOfIz_Float4{ uv.X + du, uv.Y + dv, uv.Z - du, uv.W - dv };
        }
        UICommand &command    = commands.emplace_back( );
        command.Type          = UICommandType::Image;
        command.Rect          = area;
        command.Image.Texture = image.Texture;
        command.Image.Tint    = image.Tint.A == 0 ? DenOfIz_UIColor{ 255, 255, 255, 255 } : image.Tint;
        command.Image.Radius  = InnerCorners( radius, border );
        command.Image.UV      = uv;
    }

    if ( node.Clips )
    {
        if ( node.OwnRoundClip != UINode::Invalid )
        {
            const UIRoundClip &entry = m_frame->RoundClips[ node.OwnRoundClip ];
            PushClip( entry.Rect, entry.Radius, true, false );
        }
        else
        {
            DenOfIz_UIRect shape = Inset( node.Rect, border );
            if ( desc.Scroll.OverflowX == DENOFIZ_UI_OVERFLOW_VISIBLE )
            {
                shape.X     = -UIUnbounded;
                shape.Width = UIUnbounded * 2.0f;
            }
            if ( desc.Scroll.OverflowY == DENOFIZ_UI_OVERFLOW_VISIBLE )
            {
                shape.Y      = -UIUnbounded;
                shape.Height = UIUnbounded * 2.0f;
            }
            PushClip( shape, DenOfIz_UICorners{ }, false, false );
        }
    }
    EmitGhosts( node.Id, false );
    for ( uint32_t c = node.FirstChild; c != UINode::Invalid; c = nodes[ c ].NextSibling )
    {
        if ( !nodes[ c ].Floating && !nodes[ c ].Hidden )
        {
            EmitNode( c );
        }
    }
    EmitGhosts( node.Id, true );
    if ( node.Clips )
    {
        commands.emplace_back( ).Type = UICommandType::PopClip;
    }

    if ( visible )
    {
        EmitScrollbars( node );
        EmitDividers( node, desc );
        PushBorder( node.Rect, node.BorderColor, border, radius );
    }
    if ( layer )
    {
        commands.emplace_back( ).Type = UICommandType::PopLayer;
    }
    PopClips( targetClips );
    state.CommandEnd = static_cast<uint32_t>( commands.size( ) );
}

void UIContext::EmitText( const UINode &node )
{
    const UIElementState &state = *node.State;
    const UITextLayout   *layout = state.Layout;
    if ( layout == nullptr )
    {
        return;
    }
    const DenOfIz_UITextDesc &desc     = m_frame->Texts[ node.Data ].Desc;
    const float               s        = node.Scale;
    const DenOfIz_UIRect     &rect     = node.Rect;
    const float               boxWidth = node.LayoutRect.Width;

    const bool rich = !layout->Run->Styles.empty( );
    if ( rich )
    {
        EmitTextRuns( node, false );
    }

    const DenOfIz_UITextSelection &selection = desc.Selection;
    if ( selection.Start != selection.End && selection.Color.A > 0 )
    {
        const uint32_t a = std::min( selection.Start, selection.End );
        const uint32_t b = std::max( selection.Start, selection.End );
        for ( uint32_t line = 0; line < layout->Lines.size( ); ++line )
        {
            const UITextLineLayout &l = layout->Lines[ line ];
            if ( b < l.ByteStart || a > l.ByteEnd || ( a == l.ByteEnd && line + 1 < layout->Lines.size( ) && layout->Lines[ line + 1 ].ByteStart == l.ByteEnd ) )
            {
                continue;
            }
            const float align = layout->AlignOffset( line, boxWidth, desc.Align );
            // Mixed directions split a logical range into several visual pieces.
            layout->SelectionSpans( line, a, b, l.Height * 0.3f, m_selectionSpans );
            for ( const DenOfIz_Float2 &span : m_selectionSpans )
            {
                if ( span.Y > span.X )
                {
                    PushFill( DenOfIz_UIRect{ rect.X + ( align + span.X ) * s, rect.Y + l.Top * s, ( span.Y - span.X ) * s, l.Height * s }, selection.Color, DenOfIz_UICorners{ } );
                }
            }
        }
    }

    if ( ( desc.Color.A > 0 || rich ) && UIMath::Intersects( rect, node.Clip ) )
    {
        UICommand &command     = m_frame->Commands.emplace_back( );
        command.Type           = UICommandType::Text;
        command.Rect           = rect;
        command.Text.Layout    = layout;
        command.Text.Color     = desc.Color;
        command.Text.Scale     = s;
        command.Text.BoxWidth  = boxWidth;
        command.Text.Align     = desc.Align;
    }
    if ( rich )
    {
        EmitTextRuns( node, true );
    }

    if ( desc.CaretColor.A > 0 )
    {
        const uint32_t line  = layout->LineOfOffset( desc.Caret );
        const float    width = ( desc.CaretWidth > 0.0f ? desc.CaretWidth : 1.5f ) * s;
        float          x     = rect.X + ( layout->AlignOffset( line, boxWidth, desc.Align ) + layout->CaretX( line, desc.Caret ) ) * s - width * 0.5f;
        x                    = std::round( x * m_pixelsPerPoint ) / m_pixelsPerPoint;
        PushFill( DenOfIz_UIRect{ x, rect.Y + layout->Lines[ line ].Top * s, width, layout->Lines[ line ].Height * s }, desc.CaretColor, DenOfIz_UICorners{ } );
    }
}

// Run backgrounds go under the glyphs, decorations and inline images over them, one rectangle per visual piece of the run on each line.
void UIContext::EmitTextRuns( const UINode &node, const bool foreground )
{
    const UITextLayout       &layout   = *node.State->Layout;
    const DenOfIz_UITextDesc &desc     = m_frame->Texts[ node.Data ].Desc;
    const float               s        = node.Scale;
    const DenOfIz_UIRect     &rect     = node.Rect;
    const float               boxWidth = node.LayoutRect.Width;
    const float               pixel    = 1.0f / m_pixelsPerPoint;
    auto                      isSet    = []( const DenOfIz_UIColor &c ) { return ( c.R | c.G | c.B | c.A ) != 0; };
    auto                      snap     = [ this ]( const float v ) { return std::round( v * m_pixelsPerPoint ) / m_pixelsPerPoint; };
    for ( const UIRunStyle &style : layout.Run->Styles )
    {
        const bool image  = style.Object && DENOFIZ_HANDLE_IS_VALID( style.Image );
        const bool wanted = foreground ? style.Decoration != 0 || image : style.Background.A > 0;
        const Font *font  = m_text.GetFace( style.FontId )->FontPtr;
        if ( !wanted || font == nullptr )
        {
            continue;
        }
        const DenOfIz_FontMetrics &m          = font->Metrics( );
        const DenOfIz_UIColor      color      = isSet( style.Color ) ? style.Color : desc.Color;
        const DenOfIz_UIColor      decoration = isSet( style.DecorationColor ) ? style.DecorationColor : color;
        for ( uint32_t line = 0; line < layout.Lines.size( ); ++line )
        {
            const UITextLineLayout &l = layout.Lines[ line ];
            if ( style.ByteEnd <= l.ByteStart || style.ByteStart >= l.ByteEnd )
            {
                continue;
            }
            const float align    = layout.AlignOffset( line, boxWidth, desc.Align );
            const float baseline = l.Baseline - style.Shift;
            layout.SelectionSpans( line, style.ByteStart, style.ByteEnd, 0.0f, m_selectionSpans );
            for ( const DenOfIz_Float2 &span : m_selectionSpans )
            {
                if ( span.Y <= span.X )
                {
                    continue;
                }
                const float x = rect.X + ( align + span.X ) * s;
                const float w = ( span.Y - span.X ) * s;
                if ( style.Object )
                {
                    const DenOfIz_UIRect box{ x, rect.Y + ( baseline - style.ObjectSize.Y ) * s, w, style.ObjectSize.Y * s };
                    if ( !foreground )
                    {
                        PushFill( box, style.Background, DenOfIz_UICorners{ } );
                    }
                    else if ( image )
                    {
                        UICommand &command    = m_frame->Commands.emplace_back( );
                        command.Type          = UICommandType::Image;
                        command.Rect          = box;
                        command.Image.Texture = style.Image;
                        command.Image.Tint    = DenOfIz_UIColor{ 255, 255, 255, 255 };
                        command.Image.Radius  = DenOfIz_UICorners{ };
                        command.Image.UV      = DenOfIz_Float4{ 0.0f, 0.0f, 1.0f, 1.0f };
                    }
                    continue;
                }
                if ( !foreground )
                {
                    PushFill( DenOfIz_UIRect{ x, rect.Y + ( baseline - m.Ascent * style.Size ) * s, w, ( m.Ascent + m.Descent ) * style.Size * s }, style.Background,
                              DenOfIz_UICorners{ } );
                    continue;
                }
                const float underline = std::max( m.UnderlineThickness * style.Size * s, pixel );
                const float strikeout = std::max( ( m.StrikeoutThickness > 0.0f ? m.StrikeoutThickness : m.UnderlineThickness ) * style.Size * s, pixel );
                auto        stroke    = [ & ]( const float y, const float thickness )
                { PushFill( DenOfIz_UIRect{ x, snap( rect.Y + y * s ), w, thickness }, decoration, DenOfIz_UICorners{ } ); };
                if ( style.Decoration & DENOFIZ_UI_TEXT_DECORATION_UNDERLINE_BIT )
                {
                    stroke( baseline + m.UnderlinePos * style.Size - underline * 0.5f / s, underline );
                }
                if ( style.Decoration & DENOFIZ_UI_TEXT_DECORATION_OVERLINE_BIT )
                {
                    stroke( baseline - m.Ascent * style.Size, underline );
                }
                if ( style.Decoration & DENOFIZ_UI_TEXT_DECORATION_LINE_THROUGH_BIT )
                {
                    stroke( baseline - m.StrikeoutPos * style.Size, strikeout );
                }
            }
        }
    }
}

void UIContext::EmitScrollbars( const UINode &node )
{
    const DenOfIz_UIElementDesc &desc = m_frame->Elements[ node.Data ];
    for ( int axis = 1; axis >= 0; --axis )
    {
        DenOfIz_UIRect track{ };
        DenOfIz_UIRect thumb{ };
        if ( !ScrollbarRects( *m_frame, node, axis, &track, &thumb ) )
        {
            continue;
        }
        DenOfIz_UIColor color  = desc.Scroll.ScrollbarColor.A > 0 ? desc.Scroll.ScrollbarColor : DenOfIz_UIColor{ 128, 128, 128, 130 };
        const bool      active = ( m_scrollbarDragId == node.Id && m_scrollbarAxis == axis ) ||
                                 UIMath::Contains( DenOfIz_UIRect{ track.X - ScrollbarInset, track.Y - ScrollbarInset, track.Width + ScrollbarInset * 2.0f,
                                                                   track.Height + ScrollbarInset * 2.0f },
                                                   m_pointer.X, m_pointer.Y );
        if ( active )
        {
            color.A = static_cast<uint8_t>( std::min( 255, color.A + 80 ) );
        }
        const float r = std::min( thumb.Width, thumb.Height ) * 0.5f;
        PushFill( thumb, color, DenOfIz_UICorners{ r, r, r, r } );
    }
}

void UIContext::CollectDividerChildren( const uint32_t nodeIndex )
{
    const auto &nodes = m_frame->Nodes;
    for ( uint32_t c = nodes[ nodeIndex ].FirstChild; c != UINode::Invalid; c = nodes[ c ].NextSibling )
    {
        const UINode &child = nodes[ c ];
        if ( child.Floating || child.Hidden )
        {
            continue;
        }
        if ( child.Contents )
        {
            CollectDividerChildren( c );
            continue;
        }
        if ( child.Kind == UINodeKind::Element && m_frame->Elements[ child.Data ].Layout.Position == DENOFIZ_UI_POSITION_ABSOLUTE )
        {
            continue;
        }
        m_dividerChildren.push_back( c );
    }
}

void UIContext::EmitDividers( const UINode &node, const DenOfIz_UIElementDesc &desc )
{
    const auto &style = desc.Style;
    if ( style.DividerWidth <= 0.0f || style.DividerColor.A == 0 )
    {
        return;
    }
    const auto     &nodes = m_frame->Nodes;
    const float     s     = node.Scale;
    const float     width = style.DividerWidth * s;
    const bool      row   = desc.Layout.Direction == DENOFIZ_UI_DIRECTION_ROW || desc.Layout.Direction == DENOFIZ_UI_DIRECTION_ROW_REVERSE;
    const YGNodeRef yn    = node.State->Node;
    const DenOfIz_UIEdges inner =
        ScaleEdges( DenOfIz_UIEdges{ style.BorderWidth.Left + YGNodeLayoutGetPadding( yn, YGEdgeLeft ), style.BorderWidth.Top + YGNodeLayoutGetPadding( yn, YGEdgeTop ),
                                     style.BorderWidth.Right + YGNodeLayoutGetPadding( yn, YGEdgeRight ), style.BorderWidth.Bottom + YGNodeLayoutGetPadding( yn, YGEdgeBottom ) },
                    s );
    const DenOfIz_UIRect content = Inset( node.Rect, inner );

    m_dividerChildren.clear( );
    CollectDividerChildren( static_cast<uint32_t>( &node - nodes.data( ) ) );
    for ( size_t i = 1; i < m_dividerChildren.size( ); ++i )
    {
        const DenOfIz_UIRect &a = nodes[ m_dividerChildren[ i - 1 ] ].Rect;
        const DenOfIz_UIRect &b = nodes[ m_dividerChildren[ i ] ].Rect;
        if ( row )
        {
            const float mid = b.X >= a.X ? ( a.X + a.Width + b.X ) * 0.5f : ( b.X + b.Width + a.X ) * 0.5f;
            PushFill( DenOfIz_UIRect{ mid - width * 0.5f, content.Y, width, content.Height }, style.DividerColor, DenOfIz_UICorners{ } );
        }
        else
        {
            const float mid = b.Y >= a.Y ? ( a.Y + a.Height + b.Y ) * 0.5f : ( b.Y + b.Height + a.Y ) * 0.5f;
            PushFill( DenOfIz_UIRect{ content.X, mid - width * 0.5f, content.Width, width }, style.DividerColor, DenOfIz_UICorners{ } );
        }
    }
}

void UIContext::EmitGhosts( const uint32_t parentId, const bool above )
{
    if ( !std::binary_search( m_ghostParents.begin( ), m_ghostParents.end( ), parentId ) )
    {
        return;
    }
    for ( const UIGhost &ghost : m_ghosts )
    {
        if ( ghost.Floating || ghost.ParentId != parentId )
        {
            continue;
        }
        const bool ghostAbove = ( ghost.Transition.Flags & DENOFIZ_UI_TRANSITION_EXIT_ABOVE_BIT ) != 0;
        if ( ghostAbove == above )
        {
            EmitGhost( ghost, false );
        }
    }
}

void UIContext::EmitGhost( const UIGhost &ghost, const bool restoreContext )
{
    const auto &exit     = ghost.Transition.Exit;
    const float progress = ghost.Transition.Duration > 0.0f ? std::clamp( ghost.Elapsed / ghost.Transition.Duration, 0.0f, 1.0f ) : 1.0f;
    const float t        = EaseExit( ghost.Transition.Easing, progress );

    DenOfIz_Float2 offset{ 0.0f, 0.0f };
    float          scale   = 1.0f;
    float          opacity = 1.0f;
    if ( exit.Flags & DENOFIZ_UI_TRANSITION_STATE_OFFSET_BIT )
    {
        offset = DenOfIz_Float2{ exit.Offset.X * t, exit.Offset.Y * t };
    }
    if ( exit.Flags & DENOFIZ_UI_TRANSITION_STATE_SCALE_BIT )
    {
        scale = UIMath::Lerp( 1.0f, exit.Scale, t );
    }
    if ( exit.Flags & DENOFIZ_UI_TRANSITION_STATE_OPACITY_BIT )
    {
        opacity = UIMath::Lerp( 1.0f, exit.Opacity, t );
    }
    DenOfIz_UIColor overlay{ };
    if ( exit.Flags & DENOFIZ_UI_TRANSITION_STATE_OVERLAY_COLOR_BIT )
    {
        overlay = UIMath::LerpColor( DenOfIz_UIColor{ exit.OverlayColor.R, exit.OverlayColor.G, exit.OverlayColor.B, 0 }, exit.OverlayColor, t );
    }

    DenOfIz_Float2  delta{ 0.0f, 0.0f };
    UIElementState *parent = m_states.Find( ghost.ParentId );
    if ( !ghost.Floating && parent != nullptr && parent->LastFrame == m_frameNumber )
    {
        const UINode &parentNode = m_frame->Nodes[ parent->NodeIndex ];
        delta                    = DenOfIz_Float2{ parentNode.Rect.X + ghost.LocalOffset.X - ghost.Rect.X, parentNode.Rect.Y + ghost.LocalOffset.Y - ghost.Rect.Y };
    }
    const float cx = ghost.Rect.X + ghost.Rect.Width * 0.5f + delta.X;
    const float cy = ghost.Rect.Y + ghost.Rect.Height * 0.5f + delta.Y;
    const float tx = scale * ( delta.X - cx ) + cx + offset.X;
    const float ty = scale * ( delta.Y - cy ) + cy + offset.Y;

    auto      &commands     = m_frame->Commands;
    const bool contextLayer = restoreContext && !UIMath::IsIdentityLayer( ghost.Layer );
    uint32_t   contextClips = 0;
    if ( contextLayer )
    {
        UICommand &command = commands.emplace_back( );
        command.Type       = UICommandType::PushLayer;
        command.Layer      = ghost.Layer;
    }
    if ( restoreContext )
    {
        for ( uint32_t i = 0; i < ghost.ChainCount; ++i )
        {
            PushClip( ghost.Chain[ i ].Rect, ghost.Chain[ i ].Radius, true, true );
        }
        PushClip( ghost.Clip, DenOfIz_UICorners{ }, false, true );
        contextClips = ghost.ChainCount + 1;
    }
    const bool layer = opacity < 1.0f || overlay.A > 0;
    if ( layer )
    {
        UICommand &command = commands.emplace_back( );
        command.Type       = UICommandType::PushLayer;
        command.Layer      = UIMath::OverlayLayer( overlay, opacity );
    }
    bool backgroundPending = ( exit.Flags & DENOFIZ_UI_TRANSITION_STATE_BACKGROUND_COLOR_BIT ) != 0;
    for ( const UICommand &source : ghost.Commands )
    {
        UICommand command = source;
        if ( command.Type != UICommandType::PushClip || !command.Clip.Fixed )
        {
            command.Rect = TransformRect( source.Rect, scale, tx, ty );
        }
        switch ( command.Type )
        {
        case UICommandType::Fill:
            command.Fill.Radius = ScaleCorners( command.Fill.Radius, scale );
            if ( backgroundPending && source.Rect.X == ghost.Rect.X && source.Rect.Y == ghost.Rect.Y && source.Rect.Width == ghost.Rect.Width &&
                 source.Rect.Height == ghost.Rect.Height )
            {
                command.Fill.Color = UIMath::LerpColor( command.Fill.Color, exit.BackgroundColor, t );
            }
            backgroundPending = false;
            break;
        case UICommandType::Border:
            command.Border.Radius = ScaleCorners( command.Border.Radius, scale );
            command.Border.Widths = ScaleEdges( command.Border.Widths, scale );
            break;
        case UICommandType::Image:
            command.Image.Radius = ScaleCorners( command.Image.Radius, scale );
            break;
        case UICommandType::Text:
            command.Text.Scale *= scale;
            command.Text.Layout->LastUsedFrame      = m_frameNumber;
            command.Text.Layout->Run->LastUsedFrame = m_frameNumber;
            break;
        case UICommandType::PushClip:
            if ( !command.Clip.Fixed )
            {
                command.Clip.Radius = ScaleCorners( command.Clip.Radius, scale );
            }
            break;
        default:
            break;
        }
        commands.push_back( command );
    }
    if ( layer )
    {
        commands.emplace_back( ).Type = UICommandType::PopLayer;
    }
    PopClips( contextClips );
    if ( contextLayer )
    {
        commands.emplace_back( ).Type = UICommandType::PopLayer;
    }
}

void UIContext::EmitDebug( )
{
    const auto &nodes = m_frame->Nodes;
    const float pixel = 1.0f / m_pixelsPerPoint;
    if ( m_debugFlags & DENOFIZ_UI_DEBUG_OUTLINES_BIT )
    {
        for ( size_t i = 1; i < nodes.size( ); ++i )
        {
            const UINode &node = nodes[ i ];
            if ( node.Hidden || node.Contents || !UIMath::Intersects( node.Rect, node.Clip ) )
            {
                continue;
            }
            const DenOfIz_UIColor color = node.Kind == UINodeKind::Text ? DenOfIz_UIColor{ 40, 170, 90, 160 } : DenOfIz_UIColor{ 230, 60, 160, 140 };
            PushBorder( node.Rect, color, DenOfIz_UIEdges{ pixel, pixel, pixel, pixel }, DenOfIz_UICorners{ } );
        }
    }
    if ( ( m_debugFlags & DENOFIZ_UI_DEBUG_HOVERED_BIT ) == 0 || m_hoveredId == 0 )
    {
        return;
    }
    UIElementState *it = m_states.Find( m_hoveredId );
    if ( it == nullptr || it->LastFrame != m_frameNumber )
    {
        return;
    }
    const UINode &node = nodes[ it->NodeIndex ];
    PushFill( node.Rect, DenOfIz_UIColor{ 60, 140, 255, 50 }, DenOfIz_UICorners{ } );
    PushBorder( node.Rect, DenOfIz_UIColor{ 60, 140, 255, 255 }, DenOfIz_UIEdges{ pixel * 2, pixel * 2, pixel * 2, pixel * 2 }, DenOfIz_UICorners{ } );

    char      label[ 128 ];
    const int written = std::snprintf( label, sizeof( label ), "#%08X  %.1f, %.1f  %.1f x %.1f", m_hoveredId, node.Rect.X, node.Rect.Y, node.Rect.Width, node.Rect.Height );
    const int length  = std::clamp( written, 0, static_cast<int>( sizeof( label ) ) - 1 );
    DenOfIz_UITextDesc textDesc{ };
    textDesc.FontSize           = 11.0f;
    textDesc.Wrap               = DENOFIZ_UI_TEXT_WRAP_NONE;
    const UITextParams  params  = UITextParams::From( textDesc );
    const UIShapedRun  *run     = m_text.Shape( 0, label, static_cast<uint32_t>( length ), UIText::Hash( label, static_cast<size_t>( length ) ) );
    const UITextLayout *layout  = m_text.Layout( run, params, -1.0f );
    const DenOfIz_Float2 viewport = GetViewportSize( );
    const float         padding = 4.0f;
    const float         w       = layout->Width + padding * 2.0f;
    const float         h       = layout->Height + padding * 2.0f;
    float               x       = std::clamp( node.Rect.X, 0.0f, std::max( 0.0f, viewport.X - w ) );
    float               y       = node.Rect.Y + node.Rect.Height + 2.0f;
    if ( y + h > viewport.Y )
    {
        y = std::max( 0.0f, node.Rect.Y - h - 2.0f );
    }
    PushFill( DenOfIz_UIRect{ x, y, w, h }, DenOfIz_UIColor{ 20, 24, 32, 230 }, DenOfIz_UICorners{ 3, 3, 3, 3 } );
    UICommand &command    = m_frame->Commands.emplace_back( );
    command.Type          = UICommandType::Text;
    command.Rect          = DenOfIz_UIRect{ x + padding, y + padding, layout->Width, layout->Height };
    command.Text.Layout   = layout;
    command.Text.Color    = DenOfIz_UIColor{ 240, 240, 240, 255 };
    command.Text.Scale    = 1.0f;
    command.Text.BoxWidth = layout->Width;
    command.Text.Align    = DENOFIZ_UI_TEXT_ALIGN_LEFT;
}
