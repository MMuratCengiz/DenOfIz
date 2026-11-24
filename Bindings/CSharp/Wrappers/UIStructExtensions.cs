using System.Numerics;
using System.Runtime.CompilerServices;

namespace DenOfIz
{
    public partial struct UIColor
    {
        public UIColor(byte r, byte g, byte b, byte a = 255)
        {
            R = r;
            G = g;
            B = b;
            A = a;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static UIColor Rgba(byte r, byte g, byte b, byte a = 255) => new(r, g, b, a);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static UIColor Hex(uint rgba) => new((byte)(rgba >> 24), (byte)(rgba >> 16), (byte)(rgba >> 8), (byte)rgba);

        public readonly UIColor WithAlpha(byte a) => new(R, G, B, a);

        public static UIColor Transparent => default;
        public static UIColor White => new(255, 255, 255);
        public static UIColor Black => new(0, 0, 0);
    }

    public partial struct UIValue
    {
        public UIValue(UIValueType type, float value)
        {
            Type = type;
            Value = value;
        }

        public static UIValue Auto => default;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static UIValue Points(float points) => new(UIValueType.Points, points);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static UIValue Percent(float fraction) => new(UIValueType.Percent, fraction);

        public static implicit operator UIValue(float points) => Points(points);
    }

    public partial struct UILength
    {
        public UILength(UILengthType type, float value)
        {
            Type = type;
            Value = value;
        }

        public static UILength Auto => new(UILengthType.Auto, 0);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static UILength Points(float points) => new(UILengthType.Points, points);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static UILength Percent(float fraction) => new(UILengthType.Percent, fraction);

        public static implicit operator UILength(float points) => Points(points);
    }

    public partial struct UILengthEdges
    {
        public UILengthEdges(UILength left, UILength top, UILength right, UILength bottom)
        {
            Left = left;
            Top = top;
            Right = right;
            Bottom = bottom;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static UILengthEdges All(UILength value) => new(value, value, value, value);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static UILengthEdges Symmetric(UILength horizontal, UILength vertical) => new(horizontal, vertical, horizontal, vertical);
    }

    public partial struct UIEdges
    {
        public UIEdges(float left, float top, float right, float bottom)
        {
            Left = left;
            Top = top;
            Right = right;
            Bottom = bottom;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static UIEdges All(float value) => new(value, value, value, value);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static UIEdges Symmetric(float horizontal, float vertical) => new(horizontal, vertical, horizontal, vertical);
    }

    public partial struct UICorners
    {
        public UICorners(float topLeft, float topRight, float bottomRight, float bottomLeft)
        {
            TopLeft = topLeft;
            TopRight = topRight;
            BottomRight = bottomRight;
            BottomLeft = bottomLeft;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static UICorners All(float radius) => new(radius, radius, radius, radius);
    }

    public partial struct UIInsets
    {
        public UIInsets(UIValue left, UIValue top, UIValue right, UIValue bottom)
        {
            Left = left;
            Top = top;
            Right = right;
            Bottom = bottom;
        }
    }

    public partial struct UIRect
    {
        public readonly Vector2 Position => new(X, Y);
        public readonly Vector2 Size => new(Width, Height);
        public readonly bool Contains(Vector2 point) => point.X >= X && point.Y >= Y && point.X < X + Width && point.Y < Y + Height;
    }

    public partial struct UITransitionState
    {
        public static UITransitionState FromOffset(Vector2 offset) => new() { Flags = (uint)UITransitionStateFlagBits.Offset, Offset = offset };
        public static UITransitionState FromScale(float scale) => new() { Flags = (uint)UITransitionStateFlagBits.Scale, Scale = scale };
        public static UITransitionState FromOpacity(float opacity) => new() { Flags = (uint)UITransitionStateFlagBits.Opacity, Opacity = opacity };
        public static UITransitionState FromBackground(UIColor color) => new() { Flags = (uint)UITransitionStateFlagBits.BackgroundColor, BackgroundColor = color };
        public static UITransitionState FromOverlay(UIColor color) => new() { Flags = (uint)UITransitionStateFlagBits.OverlayColor, OverlayColor = color };

        public readonly UITransitionState WithOffset(Vector2 offset) => this with { Flags = Flags | (uint)UITransitionStateFlagBits.Offset, Offset = offset };
        public readonly UITransitionState WithScale(float scale) => this with { Flags = Flags | (uint)UITransitionStateFlagBits.Scale, Scale = scale };
        public readonly UITransitionState WithOpacity(float opacity) => this with { Flags = Flags | (uint)UITransitionStateFlagBits.Opacity, Opacity = opacity };
    }

    public partial struct UIInteraction
    {
        private readonly bool Has(UIInteractionFlagBits bit) => (Flags & (uint)bit) != 0;

        public readonly bool Hovered => Has(UIInteractionFlagBits.Hovered);
        public readonly bool HoverStarted => Has(UIInteractionFlagBits.HoverStarted);
        public readonly bool HoverEnded => Has(UIInteractionFlagBits.HoverEnded);
        public readonly bool Pressed => Has(UIInteractionFlagBits.Pressed);
        public readonly bool PressStarted => Has(UIInteractionFlagBits.PressStarted);
        public readonly bool PressEnded => Has(UIInteractionFlagBits.PressEnded);
        public readonly bool Clicked => Has(UIInteractionFlagBits.Clicked);
        public readonly bool DoubleClicked => Has(UIInteractionFlagBits.DoubleClicked);
        public readonly bool RightClicked => Has(UIInteractionFlagBits.RightClicked);
        public readonly bool MiddleClicked => Has(UIInteractionFlagBits.MiddleClicked);
        public readonly bool Dragging => Has(UIInteractionFlagBits.Dragging);
        public readonly bool DragStarted => Has(UIInteractionFlagBits.DragStarted);
        public readonly bool DragEnded => Has(UIInteractionFlagBits.DragEnded);
        public readonly bool Focused => Has(UIInteractionFlagBits.Focused);
        public readonly bool FocusGained => Has(UIInteractionFlagBits.FocusGained);
        public readonly bool FocusLost => Has(UIInteractionFlagBits.FocusLost);
        public readonly bool PressedOutside => Has(UIInteractionFlagBits.PressedOutside);
    }
}
