/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

/**
 * Immediate mode UI with flexbox layout.
 *
 * Every frame: BeginFrame, declare the tree with OpenElement/CloseElement/Text, EndFrame, then Render or RenderToTexture.
 * State (scroll offsets, transitions, focus, layout caches) is kept per element id between frames.
 *
 * Units: every position and size, including pointer positions and query results, is in points.
 * One point is GetPixelsPerPoint() pixels: the window's display scale times SetScale.
 * Rectangles and points exchanged with queries are in viewport space, the space of GetPointerPosition: scrolling and transitions applied.
 *
 * Layout is Yoga flexbox (https://www.yogalayout.dev) with its non web defaults: every DenOfIz_UILayoutDesc field maps one to one to
 * the Yoga style property of the same name, and there is no automatic minimum size (min-width: auto) except for text leaves, see
 * DenOfIz_UITextDesc. Top level elements are children of an implicit column the size of the viewport with AlignItems STRETCH.
 *
 * Ids: zero means automatic, derived from the parent id and the sibling index. Give an explicit id (HashId/CombineId) to any
 * element whose state must survive siblings being added or removed, or that is queried by id.
 *
 * Queries (GetInteraction, GetElementRect, GetScrollState, text queries) answer from the most recent EndFrame, so during the
 * declaration of a frame they describe the previous frame. Pointer button transitions are not lost between frames.
 *
 * Zero initialized descs are valid: every field's zero value is its default.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "DenOfIzGraphics/Assets/Font/Font.h"
#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Input/Event.h"
#include "DenOfIzGraphics/Input/InputData.h"
#include "DenOfIzGraphics/Input/Window.h"
#include "DenOfIzGraphics/Support/ResourceTracking.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_UI )

    typedef enum DenOfIz_UIDirection
    {
        DENOFIZ_UI_DIRECTION_ROW,
        DENOFIZ_UI_DIRECTION_COLUMN,
        DENOFIZ_UI_DIRECTION_ROW_REVERSE,
        DENOFIZ_UI_DIRECTION_COLUMN_REVERSE
    } DenOfIz_UIDirection;

    /** Distribution of children along the main axis. */
    typedef enum DenOfIz_UIJustify
    {
        DENOFIZ_UI_JUSTIFY_START,
        DENOFIZ_UI_JUSTIFY_CENTER,
        DENOFIZ_UI_JUSTIFY_END,
        DENOFIZ_UI_JUSTIFY_SPACE_BETWEEN,
        DENOFIZ_UI_JUSTIFY_SPACE_AROUND,
        DENOFIZ_UI_JUSTIFY_SPACE_EVENLY
    } DenOfIz_UIJustify;

    /** Alignment of children on the cross axis. */
    typedef enum DenOfIz_UIAlign
    {
        DENOFIZ_UI_ALIGN_STRETCH, /**< Children with an AUTO cross size fill the line, Yoga's default. */
        DENOFIZ_UI_ALIGN_START,
        DENOFIZ_UI_ALIGN_CENTER,
        DENOFIZ_UI_ALIGN_END,
        DENOFIZ_UI_ALIGN_BASELINE
    } DenOfIz_UIAlign;

    /** Overrides the parent's AlignItems for one child. */
    typedef enum DenOfIz_UIAlignSelf
    {
        DENOFIZ_UI_ALIGN_SELF_AUTO,
        DENOFIZ_UI_ALIGN_SELF_START,
        DENOFIZ_UI_ALIGN_SELF_CENTER,
        DENOFIZ_UI_ALIGN_SELF_END,
        DENOFIZ_UI_ALIGN_SELF_STRETCH,
        DENOFIZ_UI_ALIGN_SELF_BASELINE
    } DenOfIz_UIAlignSelf;

    /** Distribution of wrapped lines on the cross axis. */
    typedef enum DenOfIz_UIAlignContent
    {
        DENOFIZ_UI_ALIGN_CONTENT_START,
        DENOFIZ_UI_ALIGN_CONTENT_CENTER,
        DENOFIZ_UI_ALIGN_CONTENT_END,
        DENOFIZ_UI_ALIGN_CONTENT_STRETCH,
        DENOFIZ_UI_ALIGN_CONTENT_SPACE_BETWEEN,
        DENOFIZ_UI_ALIGN_CONTENT_SPACE_AROUND,
        DENOFIZ_UI_ALIGN_CONTENT_SPACE_EVENLY
    } DenOfIz_UIAlignContent;

    typedef enum DenOfIz_UIFlexWrap
    {
        DENOFIZ_UI_FLEX_WRAP_NO_WRAP,
        DENOFIZ_UI_FLEX_WRAP_WRAP,
        DENOFIZ_UI_FLEX_WRAP_WRAP_REVERSE
    } DenOfIz_UIFlexWrap;

    /** Type of a DenOfIz_UIValue. */
    typedef enum DenOfIz_UIValueType
    {
        DENOFIZ_UI_VALUE_TYPE_AUTO,    /**< Yoga auto. For MinWidth, MinHeight, MaxWidth, MaxHeight and Insets it means unset. */
        DENOFIZ_UI_VALUE_TYPE_POINTS,  /**< Value points. */
        DENOFIZ_UI_VALUE_TYPE_PERCENT  /**< Value is a fraction (0..1), Yoga percent divided by 100. */
    } DenOfIz_UIValueType;

    /** Width, Height, their minimum and maximum, Basis and Insets. */
    typedef struct DenOfIz_UIValue
    {
        DenOfIz_UIValueType Type;
        float               Value;
    } DenOfIz_UIValue;

    /** Type of a DenOfIz_UILength. */
    typedef enum DenOfIz_UILengthType
    {
        DENOFIZ_UI_LENGTH_TYPE_POINTS,  /**< Value points. */
        DENOFIZ_UI_LENGTH_TYPE_PERCENT, /**< Value is a fraction (0..1), Yoga percent divided by 100. */
        DENOFIZ_UI_LENGTH_TYPE_AUTO     /**< Yoga auto margin, takes free space. Padding, Gap and LineGap treat it as 0. */
    } DenOfIz_UILengthType;

    /** Margin, Padding, Gap and LineGap. */
    typedef struct DenOfIz_UILength
    {
        DenOfIz_UILengthType Type;
        float                Value;
    } DenOfIz_UILength;

    /** Yoga display. */
    typedef enum DenOfIz_UIDisplay
    {
        DENOFIZ_UI_DISPLAY_FLEX,
        /**
         * The element and its whole subtree, floating descendants included, are not laid out, drawn or hit tested. Per id state such as
         * scroll offsets and transition values is kept, a focused element inside is unfocused at the next BeginFrame like SetFocus(0).
         * Hiding is not disappearing: no exit transition plays and showing again plays no enter transition. GetElementRect and the text
         * queries answer false or zero for hidden elements, GetScrollState keeps answering with the kept offsets.
         */
        DENOFIZ_UI_DISPLAY_NONE,
        /**
         * The element has no box: its children are laid out as children of the nearest ancestor that is not CONTENTS. Style, Scroll,
         * Image, Transition, Cursor and every layout field except Display are ignored. The element itself is never hit but is part of the
         * hover and press chains of its descendants like any ancestor, and Pointer applies to its descendants. GetElementRect returns the
         * union of the rectangles of its laid out children, false when there are none. A floating element (Floating.Target set) treats
         * CONTENTS as FLEX.
         */
        DENOFIZ_UI_DISPLAY_CONTENTS
    } DenOfIz_UIDisplay;

    /** Yoga position type. */
    typedef enum DenOfIz_UIPosition
    {
        DENOFIZ_UI_POSITION_RELATIVE, /**< In the flow, Insets offset it from its laid out position. */
        DENOFIZ_UI_POSITION_ABSOLUTE, /**< Out of the flow, placed by Insets against the padding box of the nearest ancestor that is not STATIC. */
        DENOFIZ_UI_POSITION_STATIC    /**< In the flow, Insets are ignored and it is not the containing block of ABSOLUTE descendants. */
    } DenOfIz_UIPosition;

    /** Yoga box sizing: what Width, Height and their minimum and maximum measure. */
    typedef enum DenOfIz_UIBoxSizing
    {
        DENOFIZ_UI_BOX_SIZING_BORDER_BOX, /**< Border, padding and content. */
        DENOFIZ_UI_BOX_SIZING_CONTENT_BOX /**< Content only, padding and border are added. */
    } DenOfIz_UIBoxSizing;

    /**
     * Descendants of an element that clips on an axis are clipped to its padding box on that axis. When both axes clip and CornerRadius
     * is set, the clip is the rounded padding box: inner radius = max(0, radius - max(adjacent border widths)) per corner, scaled down
     * together like CornerRadius when they do not fit, with a one pixel anti aliased edge. Drawing evaluates the nearest
     * DENOFIZ_UI_MAX_ROUNDED_CLIPS rounded clips of an element, farther rounded ancestors clip to their padding box rectangle. Hit testing
     * follows what is drawn. Clipping also applies to text, images, scrollbars and nested clips of descendants.
     */
    typedef enum DenOfIz_UIOverflow
    {
        DENOFIZ_UI_OVERFLOW_VISIBLE,
        DENOFIZ_UI_OVERFLOW_CLIP,
        DENOFIZ_UI_OVERFLOW_SCROLL /**< Clips and scrolls with the mouse wheel, the scrollbar and SetScrollOffset. */
    } DenOfIz_UIOverflow;

#define DENOFIZ_UI_MAX_ROUNDED_CLIPS 4

    typedef enum DenOfIz_UIScrollbarMode
    {
        DENOFIZ_UI_SCROLLBAR_MODE_AUTO,  /**< Drawn and draggable while the content overflows a scrolling axis. Pressing it presses no element. */
        DENOFIZ_UI_SCROLLBAR_MODE_HIDDEN
    } DenOfIz_UIScrollbarMode;

    typedef enum DenOfIz_UIPointerMode
    {
        DENOFIZ_UI_POINTER_MODE_AUTO,       /**< Hit testable. Blocks elements beneath when it draws something (background, border, image or text) or scrolls. */
        DENOFIZ_UI_POINTER_MODE_BLOCK,      /**< Hit testable and always blocks elements beneath. */
        DENOFIZ_UI_POINTER_MODE_PASSTHROUGH /**< Ignored by hit testing, as are its descendants unless they set BLOCK. */
    } DenOfIz_UIPointerMode;

    typedef enum DenOfIz_UIAnchor
    {
        DENOFIZ_UI_ANCHOR_TOP_LEFT,
        DENOFIZ_UI_ANCHOR_TOP_CENTER,
        DENOFIZ_UI_ANCHOR_TOP_RIGHT,
        DENOFIZ_UI_ANCHOR_CENTER_LEFT,
        DENOFIZ_UI_ANCHOR_CENTER,
        DENOFIZ_UI_ANCHOR_CENTER_RIGHT,
        DENOFIZ_UI_ANCHOR_BOTTOM_LEFT,
        DENOFIZ_UI_ANCHOR_BOTTOM_CENTER,
        DENOFIZ_UI_ANCHOR_BOTTOM_RIGHT
    } DenOfIz_UIAnchor;

    typedef enum DenOfIz_UIFloatingTarget
    {
        DENOFIZ_UI_FLOATING_TARGET_NONE,    /**< Not floating. A floating element is drawn and hit tested as its own layer but stays a descendant of its parent for hover and press bubbling, declare popups at the root to keep them apart. */
        DENOFIZ_UI_FLOATING_TARGET_PARENT,  /**< Anchored to the parent element. */
        DENOFIZ_UI_FLOATING_TARGET_ELEMENT, /**< Anchored to the element with TargetId, the parent is used if it does not exist or is hidden. A CONTENTS target is the union of its children. */
        DENOFIZ_UI_FLOATING_TARGET_VIEWPORT
    } DenOfIz_UIFloatingTarget;

    typedef enum DenOfIz_UIFloatingFlagBits
    {
        DENOFIZ_UI_FLOATING_NONE_BIT             = 0,
        DENOFIZ_UI_FLOATING_CLIP_TO_TARGET_BIT   = 1 << 0, /**< Clipped like the target is drawn, rounded clips included. */
        DENOFIZ_UI_FLOATING_KEEP_IN_VIEWPORT_BIT = 1 << 1, /**< Flipped to the opposite side of the target, then shifted, to stay in the viewport. */
        DENOFIZ_UI_FLOATING_ABSOLUTE_Z_BIT       = 1 << 2  /**< ZIndex is absolute instead of relative to the enclosing floating element. */
    } DenOfIz_UIFloatingFlagBits;
    typedef uint32_t DenOfIz_UIFloatingFlags;

    typedef enum DenOfIz_UIElementFlagBits
    {
        DENOFIZ_UI_ELEMENT_NONE_BIT      = 0,
        DENOFIZ_UI_ELEMENT_FOCUSABLE_BIT = 1 << 0, /**< Takes focus when it or a descendant is pressed, pressing elsewhere clears the focus. Takes part in FocusNext. */
        DENOFIZ_UI_ELEMENT_DISABLED_BIT  = 1 << 1  /**< Still hit tested and blocking, reports no presses, clicks, drags or focus. Descendants are not affected. */
    } DenOfIz_UIElementFlagBits;
    typedef uint32_t DenOfIz_UIElementFlags;

    typedef enum DenOfIz_UIImageFit
    {
        DENOFIZ_UI_IMAGE_FIT_FILL,    /**< Stretched over the padding box. */
        DENOFIZ_UI_IMAGE_FIT_CONTAIN, /**< Largest size that fits, keeps the aspect ratio, centered. */
        DENOFIZ_UI_IMAGE_FIT_COVER    /**< Smallest size that covers, keeps the aspect ratio, centered and cropped. */
    } DenOfIz_UIImageFit;

    typedef enum DenOfIz_UITextWrap
    {
        DENOFIZ_UI_TEXT_WRAP_WORDS,    /**< Breaks at spaces, after hyphens, between CJK characters and at newlines. A word longer than the line is broken. */
        DENOFIZ_UI_TEXT_WRAP_ANYWHERE, /**< Like WORDS, but the minimum width is 0 instead of the longest word. */
        DENOFIZ_UI_TEXT_WRAP_NEWLINES, /**< Breaks only at newlines. */
        DENOFIZ_UI_TEXT_WRAP_NONE      /**< Single line, newlines are drawn as spaces. */
    } DenOfIz_UITextWrap;

    typedef enum DenOfIz_UITextAlign
    {
        DENOFIZ_UI_TEXT_ALIGN_START, /**< Left for left to right paragraphs, right for right to left ones. */
        DENOFIZ_UI_TEXT_ALIGN_END,
        DENOFIZ_UI_TEXT_ALIGN_LEFT,
        DENOFIZ_UI_TEXT_ALIGN_CENTER,
        DENOFIZ_UI_TEXT_ALIGN_RIGHT
    } DenOfIz_UITextAlign;

    typedef enum DenOfIz_UITextOverflow
    {
        DENOFIZ_UI_TEXT_OVERFLOW_CLIP,
        DENOFIZ_UI_TEXT_OVERFLOW_ELLIPSIS /**< The last visible line ends with an ellipsis when text is cut by MaxLines or by the width of a single line. */
    } DenOfIz_UITextOverflow;

    typedef enum DenOfIz_UIEasing
    {
        DENOFIZ_UI_EASING_EASE_OUT,
        DENOFIZ_UI_EASING_LINEAR,
        DENOFIZ_UI_EASING_EASE_IN,
        DENOFIZ_UI_EASING_EASE_IN_OUT,
        DENOFIZ_UI_EASING_EASE_OUT_BACK /**< Overshoots slightly, for pop-in effects. */
    } DenOfIz_UIEasing;

    /** Properties that animate towards new values when they change. */
    typedef enum DenOfIz_UITransitionPropertyFlagBits
    {
        DENOFIZ_UI_TRANSITION_PROPERTY_NONE_BIT             = 0,
        DENOFIZ_UI_TRANSITION_PROPERTY_POSITION_BIT         = 1 << 0, /**< Position relative to the parent, scrolling and parent movement apply immediately. */
        DENOFIZ_UI_TRANSITION_PROPERTY_SIZE_BIT             = 1 << 1,
        DENOFIZ_UI_TRANSITION_PROPERTY_BACKGROUND_COLOR_BIT = 1 << 2,
        DENOFIZ_UI_TRANSITION_PROPERTY_BORDER_COLOR_BIT     = 1 << 3,
        DENOFIZ_UI_TRANSITION_PROPERTY_OVERLAY_COLOR_BIT    = 1 << 4,
        DENOFIZ_UI_TRANSITION_PROPERTY_CORNER_RADIUS_BIT    = 1 << 5,
        DENOFIZ_UI_TRANSITION_PROPERTY_COLORS_BIT           = ( 1 << 2 ) | ( 1 << 3 ) | ( 1 << 4 ),
        DENOFIZ_UI_TRANSITION_PROPERTY_ALL_BIT              = 0x3F
    } DenOfIz_UITransitionPropertyFlagBits;
    typedef uint32_t DenOfIz_UITransitionPropertyFlags;

    /** Fields of a DenOfIz_UITransitionState that are applied. */
    typedef enum DenOfIz_UITransitionStateFlagBits
    {
        DENOFIZ_UI_TRANSITION_STATE_NONE_BIT             = 0,
        DENOFIZ_UI_TRANSITION_STATE_OFFSET_BIT           = 1 << 0,
        DENOFIZ_UI_TRANSITION_STATE_SCALE_BIT            = 1 << 1,
        DENOFIZ_UI_TRANSITION_STATE_OPACITY_BIT          = 1 << 2,
        DENOFIZ_UI_TRANSITION_STATE_BACKGROUND_COLOR_BIT = 1 << 3,
        DENOFIZ_UI_TRANSITION_STATE_OVERLAY_COLOR_BIT    = 1 << 4
    } DenOfIz_UITransitionStateFlagBits;
    typedef uint32_t DenOfIz_UITransitionStateFlags;

    typedef enum DenOfIz_UITransitionFlagBits
    {
        DENOFIZ_UI_TRANSITION_NONE_BIT             = 0,
        DENOFIZ_UI_TRANSITION_ENTER_WITH_PARENT_BIT = 1 << 0, /**< Plays the enter state even when the parent appeared in the same frame. */
        DENOFIZ_UI_TRANSITION_EXIT_WITH_PARENT_BIT  = 1 << 1, /**< Plays the exit state on its own even when the parent disappeared in the same frame. */
        DENOFIZ_UI_TRANSITION_EXIT_ABOVE_BIT        = 1 << 2  /**< A disappearing element is drawn above its former siblings instead of below. */
    } DenOfIz_UITransitionFlagBits;
    typedef uint32_t DenOfIz_UITransitionFlags;

    typedef enum DenOfIz_UIInteractionFlagBits
    {
        DENOFIZ_UI_INTERACTION_NONE_BIT           = 0,
        DENOFIZ_UI_INTERACTION_HOVERED_BIT        = 1 << 0,  /**< Under the pointer and not covered by a blocking element. */
        DENOFIZ_UI_INTERACTION_HOVER_STARTED_BIT  = 1 << 1,
        DENOFIZ_UI_INTERACTION_HOVER_ENDED_BIT    = 1 << 2,
        DENOFIZ_UI_INTERACTION_PRESSED_BIT        = 1 << 3,  /**< The primary button went down on it and is held, wherever the pointer is now. */
        DENOFIZ_UI_INTERACTION_PRESS_STARTED_BIT  = 1 << 4,
        DENOFIZ_UI_INTERACTION_PRESS_ENDED_BIT    = 1 << 5,
        DENOFIZ_UI_INTERACTION_CLICKED_BIT        = 1 << 6,  /**< Primary button pressed and released on it. */
        DENOFIZ_UI_INTERACTION_DOUBLE_CLICKED_BIT = 1 << 7,
        DENOFIZ_UI_INTERACTION_RIGHT_CLICKED_BIT  = 1 << 8,
        DENOFIZ_UI_INTERACTION_MIDDLE_CLICKED_BIT = 1 << 9,
        DENOFIZ_UI_INTERACTION_DRAGGING_BIT       = 1 << 10, /**< Pressed and moved past the drag threshold. */
        DENOFIZ_UI_INTERACTION_DRAG_STARTED_BIT   = 1 << 11,
        DENOFIZ_UI_INTERACTION_DRAG_ENDED_BIT     = 1 << 12,
        DENOFIZ_UI_INTERACTION_FOCUSED_BIT        = 1 << 13,
        DENOFIZ_UI_INTERACTION_FOCUS_GAINED_BIT   = 1 << 14,
        DENOFIZ_UI_INTERACTION_FOCUS_LOST_BIT     = 1 << 15,
        DENOFIZ_UI_INTERACTION_PRESSED_OUTSIDE_BIT = 1 << 16 /**< A button went down this frame outside the element, its descendants and, for a floating element, its target. */
    } DenOfIz_UIInteractionFlagBits;
    typedef uint32_t DenOfIz_UIInteractionFlags;

    typedef enum DenOfIz_UIDebugFlagBits
    {
        DENOFIZ_UI_DEBUG_NONE_BIT     = 0,
        DENOFIZ_UI_DEBUG_OUTLINES_BIT = 1 << 0, /**< Outlines every element. */
        DENOFIZ_UI_DEBUG_HOVERED_BIT  = 1 << 1  /**< Highlights the hovered element and prints its id and rectangle. */
    } DenOfIz_UIDebugFlagBits;
    typedef uint32_t DenOfIz_UIDebugFlags;

    typedef struct DenOfIz_UIColor
    {
        uint8_t R;
        uint8_t G;
        uint8_t B;
        uint8_t A;
    } DenOfIz_UIColor;

    typedef struct DenOfIz_UIRect
    {
        float X;
        float Y;
        float Width;
        float Height;
    } DenOfIz_UIRect;

    typedef struct DenOfIz_UIEdges
    {
        float Left;
        float Top;
        float Right;
        float Bottom;
    } DenOfIz_UIEdges;

    typedef struct DenOfIz_UICorners
    {
        float TopLeft;
        float TopRight;
        float BottomRight;
        float BottomLeft;
    } DenOfIz_UICorners;

    typedef struct DenOfIz_UILengthEdges
    {
        DenOfIz_UILength Left;
        DenOfIz_UILength Top;
        DenOfIz_UILength Right;
        DenOfIz_UILength Bottom;
    } DenOfIz_UILengthEdges;

    typedef struct DenOfIz_UIInsets
    {
        DenOfIz_UIValue Left;
        DenOfIz_UIValue Top;
        DenOfIz_UIValue Right;
        DenOfIz_UIValue Bottom;
    } DenOfIz_UIInsets;

    /**
     * Yoga style of an element, every field maps to the Yoga property of the same name. The zero value is a row with AlignItems STRETCH,
     * Justify START, no wrap, Grow 0, Shrink 0, Basis AUTO, Width and Height AUTO, no minimum or maximum, FLEX, RELATIVE and BORDER_BOX.
     * PERCENT values resolve like Yoga: sizes, Basis and RELATIVE Insets against the parent's content box on their axis, ABSOLUTE Insets
     * against the containing block's padding box, Padding and Margin against the parent's content box width, Gap and LineGap against the
     * element's own available content size on their axis (0 when it is unknown).
     *
     * A floating element (Floating.Target set) is the root of its own layout: PERCENT Width, Height, MinWidth, MinHeight, MaxWidth,
     * MaxHeight and Padding are fractions of the target's size (of its width for Padding). An AUTO Width or Height fits the content and
     * is at most the viewport's size on that axis unless MaxWidth or MaxHeight is set. Grow, Shrink, Basis, AlignSelf, Margin, Position
     * and Insets are ignored. Width PERCENT 1 matches the target's width.
     *
     * An image leaf (Image.Texture set, no children) is measured from the texture, one texel per point: AUTO axes take the texture size,
     * an axis sized by the parent (EXACTLY in Yoga terms) derives the other from the texture's aspect ratio, and maximum constraints
     * (AT_MOST) scale both axes down by the same factor. A derived axis is clamped by its own maximum. AspectRatio takes precedence.
     */
    typedef struct DenOfIz_UILayoutDesc
    {
        DenOfIz_UIDisplay      Display;
        DenOfIz_UIPosition     Position;
        DenOfIz_UIBoxSizing    BoxSizing;
        DenOfIz_UIDirection    Direction;
        DenOfIz_UIJustify      Justify;
        DenOfIz_UIAlign        AlignItems;
        DenOfIz_UIAlignSelf    AlignSelf;
        DenOfIz_UIAlignContent AlignContent;
        DenOfIz_UIFlexWrap     Wrap;
        float                  Grow;   /**< Yoga flexGrow, negative is 0. */
        float                  Shrink; /**< Yoga flexShrink, negative is 0. */
        DenOfIz_UIValue        Basis;  /**< Yoga flexBasis. */
        DenOfIz_UIValue        Width;
        DenOfIz_UIValue        Height;
        DenOfIz_UIValue        MinWidth;
        DenOfIz_UIValue        MinHeight;
        DenOfIz_UIValue        MaxWidth;
        DenOfIz_UIValue        MaxHeight;
        float                  AspectRatio; /**< Width over height, 0 is none. */
        DenOfIz_UILengthEdges  Padding;
        DenOfIz_UILengthEdges  Margin;
        DenOfIz_UILength       Gap;     /**< Between children on the main axis (Yoga column gap of a row, row gap of a column). */
        DenOfIz_UILength       LineGap; /**< Between wrapped lines (the other Yoga gap). */
        DenOfIz_UIInsets       Insets;  /**< Yoga position (left, top, right, bottom). */
    } DenOfIz_UILayoutDesc;

    typedef struct DenOfIz_UIStyleDesc
    {
        DenOfIz_UIColor   BackgroundColor;
        DenOfIz_UIColor   BorderColor;
        DenOfIz_UIEdges   BorderWidth; /**< Points, Yoga border: takes space inside the element like padding. */
        DenOfIz_UICorners CornerRadius;
        DenOfIz_UIColor   OverlayColor; /**< Blended by its alpha over every pixel the element and its descendants draw (rgb = rgb * (1 - a) + overlay * a), after texturing. Transparent texels stay transparent. */
        DenOfIz_UIColor   DividerColor; /**< Lines centered in the gaps between in-flow children, the children of CONTENTS children included. */
        float             DividerWidth;
    } DenOfIz_UIStyleDesc;

    typedef struct DenOfIz_UIScrollDesc
    {
        DenOfIz_UIOverflow      OverflowX;
        DenOfIz_UIOverflow      OverflowY;
        DenOfIz_UIScrollbarMode Scrollbar;
        DenOfIz_UIColor         ScrollbarColor; /**< Zero is a translucent grey. */
        float                   ScrollbarWidth; /**< Zero is 6 points. */
    } DenOfIz_UIScrollDesc;

    typedef struct DenOfIz_UIImageDesc
    {
        DenOfIz_Texture    Texture; /**< Drawn in the padding box with the inner corner radii. A leaf is measured from the texture, see DenOfIz_UILayoutDesc. */
        DenOfIz_UIImageFit Fit;
        DenOfIz_UIColor    Tint; /**< Multiplied with the texture, zero is untinted. */
        DenOfIz_Float4     UV;   /**< MinU, MinV, MaxU, MaxV, zero is the whole texture. */
    } DenOfIz_UIImageDesc;

    typedef struct DenOfIz_UIFloatingDesc
    {
        DenOfIz_UIFloatingTarget Target;
        uint32_t                 TargetId;
        DenOfIz_UIAnchor         Anchor;       /**< Point of this element placed on TargetAnchor. */
        DenOfIz_UIAnchor         TargetAnchor; /**< Point of the target. */
        DenOfIz_Float2           Offset;
        int32_t                  ZIndex; /**< Draw order among floating elements, relative to the enclosing floating element. Ties keep declaration order. */
        DenOfIz_UIFloatingFlags  Flags;
    } DenOfIz_UIFloatingDesc;

    typedef struct DenOfIz_UITransitionState
    {
        DenOfIz_UITransitionStateFlags Flags;
        DenOfIz_Float2                 Offset;
        float                          Scale; /**< Around the element's center. */
        float                          Opacity;
        DenOfIz_UIColor                BackgroundColor;
        DenOfIz_UIColor                OverlayColor;
    } DenOfIz_UITransitionState;

    typedef struct DenOfIz_UITransitionDesc
    {
        float                             Duration; /**< Seconds, zero disables transitions. */
        DenOfIz_UIEasing                  Easing;
        DenOfIz_UITransitionPropertyFlags Properties; /**< Properties that animate when they change, zero animates only Enter and Exit. */
        DenOfIz_UITransitionFlags         Flags;
        DenOfIz_UITransitionState         Enter; /**< Appearing elements animate from this state. */
        DenOfIz_UITransitionState         Exit;  /**< Disappearing elements keep being drawn while they animate to this state. */
    } DenOfIz_UITransitionDesc;

    typedef struct DenOfIz_UIElementDesc
    {
        uint32_t                 Id;
        DenOfIz_UIElementFlags   Flags;
        DenOfIz_UIPointerMode    Pointer;
        DenOfIz_SystemCursor     Cursor;  /**< Reported by GetCursor while hovered, DEFAULT inherits from the parent. */
        DenOfIz_UILayoutDesc     Layout;
        DenOfIz_UIStyleDesc      Style;
        DenOfIz_UIScrollDesc     Scroll;
        DenOfIz_UIImageDesc      Image;
        DenOfIz_UIFloatingDesc   Floating;
        DenOfIz_UITransitionDesc Transition;
    } DenOfIz_UIElementDesc;

    typedef struct DenOfIz_UITextSelection
    {
        uint32_t        Start; /**< Byte offsets, drawn when Start != End. */
        uint32_t        End;
        DenOfIz_UIColor Color;
    } DenOfIz_UITextSelection;

    /**
     * A text leaf is a Yoga node measured by the text. It has no layout desc: it has Grow 0 and Shrink 1, and its minimum size is its
     * min-content size, whatever the parent's direction:
     * - Minimum width: the widest unbreakable run of glyphs for WRAP_WORDS (measured from the same shaped glyphs and LetterSpacing as
     *   the drawn text), 0 for WRAP_ANYWHERE or when Overflow is ELLIPSIS, and the widest line for WRAP_NEWLINES and WRAP_NONE.
     * - Minimum height: its height at its laid out width, so it never shrinks on a column main axis.
     * It is stretched by the parent's AlignItems like any child. A parent narrower than the minimum width is overflowed.
     */
    typedef struct DenOfIz_UITextDesc
    {
        uint32_t                Id;
        DenOfIz_UIColor         Color;
        uint16_t                FontId;        /**< Zero is the built in font unless replaced with AddFont. */
        float                   FontSize;      /**< Points, zero is 14. */
        float                   LetterSpacing; /**< Points added after every character. */
        float                   LineHeight;    /**< Points, zero sizes each line to the tallest font drawing on it, fallbacks included. */
        DenOfIz_UITextWrap      Wrap;
        DenOfIz_UITextAlign     Align;
        DenOfIz_UITextOverflow  Overflow;
        uint32_t                MaxLines; /**< Zero is unlimited. */
        DenOfIz_UITextSelection Selection;
        uint32_t                Caret;      /**< Byte offset of the caret. */
        DenOfIz_UIColor         CaretColor; /**< The caret is drawn when the alpha is not zero. */
        float                   CaretWidth; /**< Points, zero is 1.5. */
    } DenOfIz_UITextDesc;

    typedef enum DenOfIz_UITextDecorationFlagBits
    {
        DENOFIZ_UI_TEXT_DECORATION_NONE_BIT         = 0,
        DENOFIZ_UI_TEXT_DECORATION_UNDERLINE_BIT    = 1 << 0,
        DENOFIZ_UI_TEXT_DECORATION_OVERLINE_BIT     = 1 << 1,
        DENOFIZ_UI_TEXT_DECORATION_LINE_THROUGH_BIT = 1 << 2
    } DenOfIz_UITextDecorationFlagBits;
    typedef uint32_t DenOfIz_UITextDecorationFlags;

    /**
     * A styled range of a rich text leaf. Runs follow each other from byte 0, each ending at ByteEnd; text past the last run takes the leaf's
     * font, size and colour. The whole text is shaped, bidi ordered and wrapped as one paragraph: runs only cut shaping where font or size change.
     * A run with a non zero ObjectSize is an inline box of that size in points instead of text (one U+FFFC by convention), its bottom on the
     * baseline, Image drawn in it when set. Decorations are drawn from the metrics of the run's font.
     */
    typedef struct DenOfIz_UITextRun
    {
        uint32_t                      ByteEnd;
        uint32_t                      Id;            /**< Reported by GetTextRunAtPoint, zero is none. */
        uint16_t                      FontId;
        float                         FontSize;      /**< Points, zero is the leaf's FontSize. */
        DenOfIz_UIColor               Color;         /**< Zero is the leaf's Color. */
        DenOfIz_UIColor               BackgroundColor;
        DenOfIz_UITextDecorationFlags Decoration;
        DenOfIz_UIColor               DecorationColor; /**< Zero is the run's colour. */
        float                         BaselineShift;   /**< Points the run is raised by, negative lowers it. */
        DenOfIz_Float2                ObjectSize;
        DenOfIz_Texture               Image;
    } DenOfIz_UITextRun;

    typedef struct DenOfIz_UITextRunArray
    {
        DenOfIz_UITextRun *Elements;
        size_t             NumElements;
    } DenOfIz_UITextRunArray;

    typedef struct DenOfIz_UIInteraction
    {
        DenOfIz_UIInteractionFlags Flags;
        uint32_t                   ClickCount;      /**< Consecutive clicks of the last primary press, 2 on a double click. */
        DenOfIz_Float2             PointerPosition; /**< Relative to the element's top left. */
        DenOfIz_Float2             DragDelta;       /**< Pointer movement since the press started. */
        DenOfIz_UIRect             Rect;
    } DenOfIz_UIInteraction;

    typedef struct DenOfIz_UIScrollState
    {
        DenOfIz_Float2 Offset; /**< Positive, 0 is the top left of the content. */
        DenOfIz_Float2 MaxOffset;
        DenOfIz_Float2 ViewportSize;
        DenOfIz_Float2 ContentSize;
    } DenOfIz_UIScrollState;

    typedef struct DenOfIz_UITextLine
    {
        uint32_t       StartOffset; /**< Byte offsets of the line, End excludes the line break. */
        uint32_t       EndOffset;
        DenOfIz_UIRect Rect;
    } DenOfIz_UITextLine;

    typedef struct DenOfIz_UIDesc
    {
        DenOfIz_LogicalDevice    LogicalDevice;
        DenOfIz_ResourceTracking ResourceTracking; /**< Optional. */
        DenOfIz_Window           Window;           /**< Optional. Its display scale and pixel size are followed. */
        DenOfIz_Format           RenderTargetFormat;
        DenOfIz_Format           DepthFormat; /**< Optional. Format of the depth attachment of passes Render records into. Depth is never tested. */
        uint32_t                 NumFrames;   /**< Frames in flight, the range of Render's frameIndex. Zero is 3. */
        uint32_t                 Width;       /**< Pixels. */
        uint32_t                 Height;
    } DenOfIz_UIDesc;

    typedef struct DenOfIz_UIRenderResult
    {
        DenOfIz_Texture   Texture;   /**< In the shader resource state. */
        DenOfIz_Semaphore Semaphore; /**< Signaled when the texture is ready. */
    } DenOfIz_UIRenderResult;

    DZ_API DenOfIz_UIColor       DenOfIz_UIColor_Rgba( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
    DZ_API DenOfIz_UIColor       DenOfIz_UIColor_Hex( uint32_t rgba );
    DZ_API DenOfIz_UIValue       DenOfIz_UIValue_Auto( );
    DZ_API DenOfIz_UIValue       DenOfIz_UIValue_Points( float points );
    DZ_API DenOfIz_UIValue       DenOfIz_UIValue_Percent( float fraction );
    DZ_API DenOfIz_UILength      DenOfIz_UILength_Points( float points );
    DZ_API DenOfIz_UILength      DenOfIz_UILength_Percent( float fraction );
    DZ_API DenOfIz_UILength      DenOfIz_UILength_Auto( );
    /** Every edge in points. */
    DZ_API DenOfIz_UILengthEdges DenOfIz_UILengthEdges_All( float points );
    /** Left and right, top and bottom in points. */
    DZ_API DenOfIz_UILengthEdges DenOfIz_UILengthEdges_Symmetric( float horizontal, float vertical );
    DZ_API DenOfIz_UIEdges       DenOfIz_UIEdges_All( float value );
    DZ_API DenOfIz_UIEdges       DenOfIz_UIEdges_Symmetric( float horizontal, float vertical );
    DZ_API DenOfIz_UICorners     DenOfIz_UICorners_All( float radius );

    /** Hashes a name to an element id, never zero. Seed composes ids, e.g. the id of a parent widget. */
    DZ_API uint32_t DenOfIz_UI_HashId( DenOfIz_StringView name, uint32_t seed );
    /** Derives an id from an id and a value such as a list index, never zero. */
    DZ_API uint32_t DenOfIz_UI_CombineId( uint32_t id, uint32_t value );

    DZ_API DenOfIz_UI DenOfIz_UI_Create( const DenOfIz_UIDesc *desc );
    DZ_API void       DenOfIz_UI_Destroy( DenOfIz_UI ui );

    /** Feeds pointer, wheel, keyboard focus and window events. Window resizes and display scale changes are followed for DenOfIz_UIDesc::Window. */
    DZ_API void           DenOfIz_UI_HandleEvent( DenOfIz_UI ui, const DenOfIz_Event *ev );
    DZ_API void           DenOfIz_UI_SetViewportSize( DenOfIz_UI ui, uint32_t pixelWidth, uint32_t pixelHeight );
    DZ_API DenOfIz_Float2 DenOfIz_UI_GetViewportSize( DenOfIz_UI ui );
    /** Overrides the display scale, zero or less follows the window again. */
    DZ_API void DenOfIz_UI_SetDisplayScale( DenOfIz_UI ui, float displayScale );
    /** Zoom applied on top of the display scale. */
    DZ_API void  DenOfIz_UI_SetScale( DenOfIz_UI ui, float scale );
    DZ_API float DenOfIz_UI_GetPixelsPerPoint( DenOfIz_UI ui );
    DZ_API void  DenOfIz_UI_SetDebugFlags( DenOfIz_UI ui, DenOfIz_UIDebugFlags flags );

    /**
     * Registers a font under an id below 64, replacing any font with the same id, including the built in font 0. The UI borrows the font:
     * keep it alive until the BeginFrame after RemoveFont or its replacement, or until the UI is destroyed.
     */
    DZ_API void DenOfIz_UI_AddFont( DenOfIz_UI ui, uint16_t fontId, DenOfIz_Font font );
    DZ_API void DenOfIz_UI_RemoveFont( DenOfIz_UI ui, uint16_t fontId );
    /** Font ids tried in order, without their own fallbacks, for characters the font lacks, before the system fonts. Replaces the previous list, the ids need not be registered yet. */
    DZ_API void DenOfIz_UI_SetFontFallbacks( DenOfIz_UI ui, uint16_t fontId, const DenOfIz_UInt16Array *fallbackIds );

    DZ_API void DenOfIz_UI_BeginFrame( DenOfIz_UI ui, float deltaTime );
    /** Returns the element's id, resolved when Id is zero. */
    DZ_API uint32_t DenOfIz_UI_OpenElement( DenOfIz_UI ui, const DenOfIz_UIElementDesc *desc );
    DZ_API void     DenOfIz_UI_CloseElement( DenOfIz_UI ui );
    /** Adds a text leaf to the open element and returns its id. The text is copied. */
    DZ_API uint32_t DenOfIz_UI_Text( DenOfIz_UI ui, DenOfIz_StringView text, const DenOfIz_UITextDesc *desc );
    /**
     * Adds a text leaf made of styled runs, see DenOfIz_UITextRun. The desc's font and size set the line's strut: a line is never shorter than a
     * line of plain text in them. Selection, caret and the text queries work across runs. The text and runs are copied.
     */
    DZ_API uint32_t DenOfIz_UI_RichText( DenOfIz_UI ui, DenOfIz_StringView text, const DenOfIz_UITextRunArray *runs, const DenOfIz_UITextDesc *desc );
    /** Lays out the declared tree and updates interaction, scrolling and transitions. */
    DZ_API void DenOfIz_UI_EndFrame( DenOfIz_UI ui );
    /** Records draws into a render pass the caller has begun on a target of RenderTargetFormat (and DepthFormat). */
    DZ_API void DenOfIz_UI_Render( DenOfIz_UI ui, uint32_t frameIndex, DenOfIz_CommandList commandList );
    /**
     * Renders into a texture owned by the UI and submits it on the UI's own queue. The texture is reused, or recreated after a resize, by
     * the next RenderToTexture with the same frameIndex. waitSemaphores is optional: semaphores of work producing textures the UI samples,
     * e.g. render targets shown as images.
     */
    DZ_API void DenOfIz_UI_RenderToTexture( DenOfIz_UI ui, uint32_t frameIndex, const DenOfIz_SemaphoreArray *waitSemaphores, DenOfIz_UIRenderResult *outResult );

    DZ_API void           DenOfIz_UI_GetInteraction( DenOfIz_UI ui, uint32_t id, DenOfIz_UIInteraction *outInteraction );
    DZ_API bool           DenOfIz_UI_GetElementRect( DenOfIz_UI ui, uint32_t id, DenOfIz_UIRect *outRect );
    DZ_API bool           DenOfIz_UI_IsPointerOverUI( DenOfIz_UI ui );
    DZ_API uint32_t       DenOfIz_UI_GetHoveredId( DenOfIz_UI ui );
    DZ_API DenOfIz_Float2 DenOfIz_UI_GetPointerPosition( DenOfIz_UI ui );
    /** The cursor requested by the hovered element, apply it with DenOfIz_InputSystem_SetCursor. */
    DZ_API DenOfIz_SystemCursor DenOfIz_UI_GetCursor( DenOfIz_UI ui );

    /** Focuses a focusable element, zero clears the focus. Takes effect at the next BeginFrame, which reports FOCUS_GAINED and FOCUS_LOST. */
    DZ_API void     DenOfIz_UI_SetFocus( DenOfIz_UI ui, uint32_t id );
    DZ_API uint32_t DenOfIz_UI_GetFocus( DenOfIz_UI ui );
    /** Moves the focus to the next or previous focusable element in declaration order, like SetFocus. */
    DZ_API void DenOfIz_UI_FocusNext( DenOfIz_UI ui, bool reverse );

    DZ_API bool DenOfIz_UI_GetScrollState( DenOfIz_UI ui, uint32_t id, DenOfIz_UIScrollState *outState );
    /** Sets the scroll offset, clamped at the next EndFrame. */
    DZ_API void DenOfIz_UI_SetScrollOffset( DenOfIz_UI ui, uint32_t id, DenOfIz_Float2 offset );
    /** Scrolls every scrolling ancestor of the element at the next EndFrame so the element is visible. */
    DZ_API void DenOfIz_UI_ScrollIntoView( DenOfIz_UI ui, uint32_t id );
    /** Scrolls every scrolling ancestor of a text element at the next EndFrame so the caret at a byte offset is visible. */
    DZ_API void DenOfIz_UI_ScrollCaretIntoView( DenOfIz_UI ui, uint32_t textId, uint32_t byteOffset );

    /** Size of text laid out with the desc, wrapped at maxWidth when it is greater than zero. */
    DZ_API DenOfIz_Float2 DenOfIz_UI_MeasureText( DenOfIz_UI ui, DenOfIz_StringView text, const DenOfIz_UITextDesc *desc, float maxWidth );
    /** Caret rectangle of a laid out text element at a byte offset. An offset at a soft line break is at the start of the following line. */
    DZ_API bool DenOfIz_UI_GetTextCaretRect( DenOfIz_UI ui, uint32_t textId, uint32_t byteOffset, DenOfIz_UIRect *outRect );
    /** Byte offset of the caret position closest to a point. */
    DZ_API uint32_t DenOfIz_UI_GetTextOffsetAtPoint( DenOfIz_UI ui, uint32_t textId, DenOfIz_Float2 point );
    /** Id of the run of a rich text element drawn under a point, zero when there is none. */
    DZ_API uint32_t DenOfIz_UI_GetTextRunAtPoint( DenOfIz_UI ui, uint32_t textId, DenOfIz_Float2 point );
    DZ_API uint32_t DenOfIz_UI_GetTextLineCount( DenOfIz_UI ui, uint32_t textId );
    DZ_API bool     DenOfIz_UI_GetTextLine( DenOfIz_UI ui, uint32_t textId, uint32_t lineIndex, DenOfIz_UITextLine *outLine );

#ifdef __cplusplus
}
#endif
