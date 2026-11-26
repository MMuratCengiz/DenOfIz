/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <yoga/Yoga.h>
#include <array>
#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>
#include "DenOfIzGraphicsInternal/UI/UIText.h"
#include "DenOfIzGraphicsInternal/UI/UITypes.h"

namespace DenOfIz
{
    class UIContext;

    template <typename T>
    struct UITween
    {
        T     From{ };
        T     To{ };
        T     Current{ };
        float Elapsed     = 0.0f;
        bool  Initialized = false;
    };

    struct UIEffect
    {
        DenOfIz_Float2  Offset{ 0.0f, 0.0f };
        float           Scale   = 1.0f;
        float           Opacity = 1.0f;
        DenOfIz_UIColor Background{ };
        DenOfIz_UIColor Overlay{ };
        bool            HasBackground = false;
        bool            HasOverlay    = false;
    };

    struct UIElementState
    {
        uint32_t   Id    = 0;
        bool       Live  = false;
        UIContext *Owner = nullptr;
        YGNodeRef  Node  = nullptr;
        uint64_t   LastFrame    = 0;
        uint64_t   CreatedFrame = 0;
        float      IdleSeconds  = 0.0f;
        uint32_t   ParentId     = 0;
        uint32_t   FloatingTargetId = 0;
        uint32_t   NodeIndex    = 0;
        uint32_t   PrevNodeIndex = 0;
        uint64_t   PrevFrame     = 0;
        uint64_t   ComposedFrame = 0;
        bool       Appeared      = false;
        bool       ParentChanged = false;
        bool       TextChanged   = false;
        bool       IsText       = false;
        bool       NodeIsText   = false;
        bool       NodeIsImage  = false;
        bool       WasFloating  = false;
        bool       HasRect      = false;

        bool                 StyleApplied = false;
        DenOfIz_UILayoutDesc AppliedLayout{ };
        DenOfIz_UIEdges      AppliedBorder{ };
        uint32_t             AppliedContext = 0;

        const UIShapedRun  *Run    = nullptr;
        const UITextLayout *Layout = nullptr;
        uint64_t            TextEpoch = 0;
        uint64_t            TextHash  = 0;
        uint32_t            TextLength = 0;
        UITextParams        TextParams;
        float               TextMinWidth = 0.0f;

        DenOfIz_Texture ImageTexture = DENOFIZ_NULL_HANDLE;
        float           ImageWidth   = 0.0f;
        float           ImageHeight  = 0.0f;

        DenOfIz_UIRect Rect{ };
        DenOfIz_UIRect LayoutRect{ };

        bool           Scrollable = false;
        DenOfIz_Float2 ScrollOffset{ 0.0f, 0.0f };
        DenOfIz_Float2 ScrollTarget{ 0.0f, 0.0f };
        DenOfIz_Float2 MaxScroll{ 0.0f, 0.0f };
        DenOfIz_Float2 ContentSize{ 0.0f, 0.0f };
        DenOfIz_Float2 ViewportSize{ 0.0f, 0.0f };

        UITween<DenOfIz_Float2>    Position;
        UITween<DenOfIz_Float2>    Size;
        UITween<DenOfIz_UIColor>   Background;
        UITween<DenOfIz_UIColor>   BorderColor;
        UITween<DenOfIz_UIColor>   Overlay;
        UITween<DenOfIz_UICorners> Radius;
        uint64_t                   PositionTweenFrame = 0;
        float                      EnterElapsed       = -1.0f;

        bool                     HasExit = false;
        DenOfIz_UITransitionDesc Transition{ };
        uint32_t                 CommandBegin = 0;
        uint32_t                 CommandEnd   = 0;
        DenOfIz_Float2           LocalOffset{ 0.0f, 0.0f };
    };

    class UIStateMap
    {
        static constexpr uint32_t Empty = UINT32_MAX;
        static constexpr uint32_t Tomb  = UINT32_MAX - 1;

        mutable std::deque<UIElementState> m_pool;
        std::vector<uint32_t>              m_free;
        std::vector<uint32_t>              m_keys;
        std::vector<uint32_t>              m_slots;
        uint32_t                           m_mask = 0;
        uint32_t                           m_used = 0;
        uint32_t                           m_live = 0;

        uint32_t Probe( uint32_t id, bool *found ) const;
        void     Rehash( uint32_t capacity );

    public:
        UIStateMap( );

        UIElementState                   *Find( uint32_t id ) const;
        std::pair<UIElementState *, bool> TryEmplace( uint32_t id );
        void                              Erase( uint32_t id );

        template <typename F>
        void ForEach( F &&function )
        {
            for ( UIElementState &state : m_pool )
            {
                if ( state.Live )
                {
                    function( state );
                }
            }
        }
    };

    enum class UINodeKind : uint8_t
    {
        Root,
        Element,
        Text
    };

    struct UIRoundClip
    {
        DenOfIz_UIRect    Rect;
        DenOfIz_UICorners Radius;
        uint32_t          Parent;
    };

    struct UINode
    {
        static constexpr uint32_t Invalid = UINT32_MAX;

        uint32_t        Id          = 0;
        uint32_t        Parent      = Invalid;
        uint32_t        FirstChild  = Invalid;
        uint32_t        LastChild   = Invalid;
        uint32_t        NextSibling = Invalid;
        uint32_t        PrevSibling = Invalid;
        uint32_t        ChildCount  = 0;
        uint32_t        SubtreeEnd  = 0;
        uint32_t        Layer       = 0;
        uint32_t        Data        = 0;
        UIElementState *State       = nullptr;
        UINodeKind      Kind        = UINodeKind::Element;
        bool            Floating    = false;
        bool            Animating   = false;
        bool            Captures    = false;
        bool            Disabled    = false;
        bool            Focusable   = false;
        bool            Clips       = false;
        bool            Hidden      = false;
        bool            Contents    = false;
        bool            HasUnion    = false;
        DenOfIz_UIPointerMode Pointer = DENOFIZ_UI_POINTER_MODE_AUTO;
        DenOfIz_SystemCursor  Cursor  = DENOFIZ_SYSTEM_CURSOR_DEFAULT;

        DenOfIz_Float2    Local{ 0.0f, 0.0f };
        DenOfIz_Float2    Size{ 0.0f, 0.0f };
        DenOfIz_UIColor   Background{ };
        DenOfIz_UIColor   BorderColor{ };
        DenOfIz_UIColor   Overlay{ };
        DenOfIz_UICorners Radius{ };
        UIEffect          Effect;
        int32_t           ZIndex = 0;

        DenOfIz_UIRect LayoutRect{ };
        DenOfIz_UIRect Rect{ };
        DenOfIz_UIRect Clip{ };
        DenOfIz_UIRect ChildClip{ };
        DenOfIz_UIRect TargetClip{ };
        DenOfIz_UIRect Union{ };
        uint32_t       RoundClip       = Invalid;
        uint32_t       OwnRoundClip    = Invalid;
        uint32_t       TargetRoundClip = Invalid;
        float          Scale = 1.0f;
        float          TranslateX = 0.0f;
        float          TranslateY = 0.0f;
    };

    struct UITextNode
    {
        DenOfIz_UITextDesc Desc;
        uint32_t           Offset;
        uint32_t           Length;
        uint32_t           RunOffset = 0;
        uint32_t           RunCount  = 0;
    };

    struct UIFrame
    {
        std::vector<UINode>                Nodes;
        std::vector<DenOfIz_UIElementDesc> Elements;
        std::vector<UITextNode>            Texts;
        std::vector<char>                  TextArena;
        std::vector<DenOfIz_UITextRun>     TextRuns;
        std::vector<uint32_t>              Layers;
        std::vector<UICommand>             Commands;
        std::vector<UIRoundClip>           RoundClips;

        void Clear( );
    };

    struct UIGhost
    {
        uint32_t                 Id       = 0;
        uint32_t                 ParentId = 0;
        bool                     Floating = false;
        DenOfIz_UIRect           Rect{ };
        DenOfIz_Float2           LocalOffset{ 0.0f, 0.0f };
        DenOfIz_UITransitionDesc Transition{ };
        float                    Elapsed = 0.0f;
        std::vector<UICommand>   Commands;
        DenOfIz_UIRect           Clip{ };
        std::array<UIRoundClip, UIMaxRoundedClips> Chain{ };
        uint32_t                 ChainCount = 0;
        UILayerCommand           Layer{ 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    };

    struct UIPointerEvent
    {
        bool                Down;
        DenOfIz_MouseButton Button;
        float               X;
        float               Y;
        uint32_t            Clicks;
    };

    struct UIScrollRequest
    {
        uint32_t Id;
        bool     Caret;
        uint32_t ByteOffset;
    };

    class UIContext
    {
        static constexpr float    DragThreshold      = 4.0f;
        static constexpr float    WheelPoints        = 48.0f;
        static constexpr float    StateLifetime      = 5.0f;
        static constexpr uint64_t CacheAge           = 240;
        static constexpr float    DefaultScrollbar   = 6.0f;
        static constexpr float    ScrollbarInset     = 2.0f;
        static constexpr float    MinThumb           = 20.0f;

        UIText      m_text;
        YGConfigRef m_config   = nullptr;
        YGNodeRef   m_root     = nullptr;
        YGNodeRef   m_template = nullptr;

        UIStateMap                                   m_states;
        std::vector<uint32_t>                        m_expired;
        UIFrame                                      m_frames[ 2 ];
        UIFrame                                     *m_frame = &m_frames[ 0 ];
        UIFrame                                     *m_last  = &m_frames[ 1 ];
        std::vector<uint32_t>                        m_openStack;
        std::vector<uint32_t>                        m_floating;
        std::vector<UIGhost>                         m_ghosts;
        std::vector<uint32_t>                        m_ghostParents;
        std::vector<YGNodeRef>                       m_childScratch;
        std::vector<uint32_t>                        m_dividerChildren;
        std::vector<uint32_t>                        m_reconcile;
        std::vector<std::pair<uint32_t, uint32_t>>   m_reconcileRanges;
        std::vector<UIScrollRequest>                 m_scrollRequests;
        std::vector<DenOfIz_Float2>                  m_selectionSpans;
        std::vector<UIRunStyle>                      m_styles;

        uint64_t m_frameNumber   = 0;
        float    m_deltaTime     = 0.0f;
        bool     m_inFrame       = false;
        bool     m_animating     = false;
        bool     m_snapTransitions = true;
        float    m_viewportPixelWidth  = 1.0f;
        float    m_viewportPixelHeight = 1.0f;
        float    m_displayScale  = 1.0f;
        float    m_scale         = 1.0f;
        float    m_pixelsPerPoint = 1.0f;
        DenOfIz_UIDebugFlags m_debugFlags = 0;

        DenOfIz_Float2              m_pointer{ -1.0f, -1.0f };
        DenOfIz_Float2              m_wheel{ 0.0f, 0.0f };
        std::vector<UIPointerEvent> m_pointerEvents;
        std::vector<uint32_t>       m_hoverChain;
        std::vector<uint32_t>       m_prevHoverChain;
        std::vector<uint32_t>       m_pressChain;
        std::vector<uint32_t>       m_secondaryChain;
        std::vector<uint32_t>       m_middleChain;
        std::vector<uint32_t>       m_pressStarted;
        std::vector<uint32_t>       m_pressEnded;
        std::vector<uint32_t>       m_clicked;
        std::vector<uint32_t>       m_rightClicked;
        std::vector<uint32_t>       m_middleClicked;
        std::vector<uint32_t>       m_dragStarted;
        std::vector<uint32_t>       m_dragEnded;
        std::vector<uint32_t>       m_pressedThisFrame;
        std::vector<uint32_t>       m_scratchChain;
        bool                        m_anyPressThisFrame = false;
        bool                        m_primaryDown       = false;
        bool                        m_dragging          = false;
        bool                        m_doubleClick       = false;
        uint32_t                    m_clickCount        = 0;
        DenOfIz_Float2              m_pressPosition{ 0.0f, 0.0f };
        uint32_t                    m_focus             = 0;
        uint32_t                    m_focusGained       = 0;
        uint32_t                    m_focusLost         = 0;
        uint32_t                    m_pendingFocus      = 0;
        uint32_t                    m_focusVanished     = 0;
        bool                        m_hasPendingFocus   = false;
        bool                        m_pointerOverUI     = false;
        uint32_t                    m_hoveredId         = 0;
        DenOfIz_SystemCursor        m_cursor            = DENOFIZ_SYSTEM_CURSOR_DEFAULT;
        uint32_t                    m_scrollbarDragId   = 0;
        int32_t                     m_scrollbarAxis     = -1;
        float                       m_scrollbarGrab     = 0.0f;

    public:
        UIContext( );
        ~UIContext( );

        UIText &Text( );

        void           SetViewportPixels( float width, float height );
        DenOfIz_Float2 GetViewportSize( ) const;
        DenOfIz_Float2 GetViewportPixels( ) const;
        void           SetDisplayScale( float displayScale );
        void           SetScale( float scale );
        float          GetPixelsPerPoint( ) const;
        void           SetDebugFlags( DenOfIz_UIDebugFlags flags );
        void           FontsChanged( );

        void     HandlePointerMotion( float pixelX, float pixelY );
        void     HandlePointerButton( bool down, DenOfIz_MouseButton button, float pixelX, float pixelY, uint32_t clicks );
        void     HandleWheel( float x, float y );
        void     HandlePointerLeave( );

        void     BeginFrame( float deltaTime );
        uint32_t OpenElement( const DenOfIz_UIElementDesc &desc );
        void     CloseElement( );
        uint32_t AddText( const DenOfIz_StringView &text, const DenOfIz_UITextDesc &desc, const DenOfIz_UITextRunArray *runs = nullptr );
        void     EndFrame( );

        const std::vector<UICommand> &Commands( ) const;

        void                 GetInteraction( uint32_t id, DenOfIz_UIInteraction *out ) const;
        bool                 GetElementRect( uint32_t id, DenOfIz_UIRect *out ) const;
        bool                 IsPointerOverUI( ) const;
        uint32_t             GetHoveredId( ) const;
        DenOfIz_Float2       GetPointerPosition( ) const;
        DenOfIz_SystemCursor GetCursor( ) const;

        void     SetFocus( uint32_t id );
        uint32_t GetFocus( ) const;
        void     FocusNext( bool reverse );

        bool GetScrollState( uint32_t id, DenOfIz_UIScrollState *out ) const;
        void SetScrollOffset( uint32_t id, DenOfIz_Float2 offset );
        void ScrollIntoView( uint32_t id );
        void ScrollCaretIntoView( uint32_t textId, uint32_t byteOffset );

        DenOfIz_Float2 MeasureText( const DenOfIz_StringView &text, const DenOfIz_UITextDesc &desc, float maxWidth );
        bool           GetTextCaretRect( uint32_t textId, uint32_t byteOffset, DenOfIz_UIRect *out ) const;
        uint32_t       GetTextOffsetAtPoint( uint32_t textId, DenOfIz_Float2 point ) const;
        uint32_t       GetTextLineCount( uint32_t textId ) const;
        uint32_t       GetTextRunAtPoint( uint32_t textId, DenOfIz_Float2 point ) const;
        bool           GetTextLine( uint32_t textId, uint32_t lineIndex, DenOfIz_UITextLine *out ) const;

        uint32_t NodeCount( ) const;
        bool     GetNode( uint32_t index, DenOfIz_UINodeInfo *out ) const;
        uint32_t FindNode( uint32_t id ) const;
        uint32_t HitTestPoint( DenOfIz_Float2 point, DenOfIz_UInt32Array *outChain ) const;
        bool     IsAnimating( ) const;

    private:
        static YGSize MeasureTextNode( YGNodeConstRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode );
        static YGSize MeasureImageNode( YGNodeConstRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode );
        static float  TextBaseline( YGNodeConstRef node, float width, float height );

        void            UpdatePixelsPerPoint( );
        bool            DeclaredInLastFrame( const UIElementState &state ) const;
        UIElementState &AcquireState( uint32_t requested, uint32_t autoId, bool isText );
        uint32_t        AppendNode( UINodeKind kind, uint32_t requestedId, bool isText );

        void ResolveTexts( );
        void ResolveStyles( const UITextNode &text, const UITextParams &params );
        void ApplyStyles( );
        void ApplyElementStyle( uint32_t nodeIndex );
        void ApplyTextStyle( uint32_t nodeIndex );
        void ReconcileChildren( );
        void ReleaseStates( );
        void LayoutRoot( );
        void LayoutFloating( );
        void AnimateNode( uint32_t nodeIndex );
        void ComposeLayer( uint32_t layerRoot );
        void ComposeNode( uint32_t nodeIndex, float scale, float tx, float ty, const DenOfIz_UIRect &clip, uint32_t roundClip );
        uint32_t LayoutParent( uint32_t nodeIndex ) const;
        void     ExtendContentBounds( const UINode &node, float &right, float &bottom ) const;
        void     CollectDividerChildren( uint32_t nodeIndex );
        void           PositionFloating( uint32_t nodeIndex );
        DenOfIz_UIRect FloatingTarget( uint32_t nodeIndex );
        void UpdateScroll( UINode &node, const DenOfIz_UIElementDesc &desc );
        bool ProcessScrollRequests( );
        void UpdateGhosts( );
        void CreateGhost( UIElementState &state );
        void StoreResults( );

        const UINode *FindLastNode( uint32_t id ) const;
        bool          InsideRoundClips( const UIFrame &frame, uint32_t clip, float x, float y ) const;
        void          HitTest( const UIFrame &frame, float x, float y, std::vector<uint32_t> &outChain, bool *outCaptured, uint32_t *outDeepest ) const;
        int64_t       FindDeepest( const UIFrame &frame, uint32_t nodeIndex, float x, float y, bool passthrough ) const;
        void          TrimChainAtScrollbar( const UIFrame &frame, std::vector<uint32_t> &chain, float x, float y ) const;
        void          ProcessPointer( );
        void          ApplyWheel( );
        void          UpdateHover( );
        bool          ScrollbarRects( const UIFrame &frame, const UINode &node, int axis, DenOfIz_UIRect *track, DenOfIz_UIRect *thumb ) const;
        bool          BeginScrollbarDrag( float x, float y );
        void          UpdateScrollbarDrag( );
        void          SetFocusInternal( uint32_t id );

        void BuildCommands( );
        void EmitNode( uint32_t nodeIndex );
        void EmitText( const UINode &node );
        void EmitTextRuns( const UINode &node, bool foreground );
        void EmitGhosts( uint32_t parentId, bool above );
        void EmitGhost( const UIGhost &ghost, bool restoreContext );
        void EmitScrollbars( const UINode &node );
        void EmitDividers( const UINode &node, const DenOfIz_UIElementDesc &desc );
        void EmitDebug( );
        uint32_t PushClipChain( const UIFrame &frame, uint32_t roundClip, const DenOfIz_UIRect &clip );
        void     PushClip( const DenOfIz_UIRect &rect, const DenOfIz_UICorners &radius, bool rounded, bool fixed );
        void     PopClips( uint32_t count );
        void PushFill( const DenOfIz_UIRect &rect, const DenOfIz_UIColor &color, const DenOfIz_UICorners &radius );
        void PushBorder( const DenOfIz_UIRect &rect, const DenOfIz_UIColor &color, const DenOfIz_UIEdges &widths, const DenOfIz_UICorners &radius );
    };
} // namespace DenOfIz
