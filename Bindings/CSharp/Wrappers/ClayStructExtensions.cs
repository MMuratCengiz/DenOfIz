using System.Numerics;
using System.Runtime.CompilerServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public partial struct ClaySizingAxis
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClaySizingAxis Fit(float min, float max)
        {
            return Methods.DenOfIz_ClaySizingAxis_Fit(min, max);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClaySizingAxis Grow(float min, float max)
        {
            return Methods.DenOfIz_ClaySizingAxis_Grow(min, max);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClaySizingAxis Fixed(float size)
        {
            return Methods.DenOfIz_ClaySizingAxis_Fixed(size);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClaySizingAxis Percent(float percent)
        {
            return Methods.DenOfIz_ClaySizingAxis_Percent(percent);
        }
    }

    public partial struct ClayColor
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClayColor Create(uint r, uint g, uint b, uint a)
        {
            return Methods.DenOfIz_ClayColor_Create(r, g, b, a);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public Vector4 ToFloat4()
        {
            return Methods.DenOfIz_ClayColor_ToFloat4(in this);
        }
    }

    public partial struct ClayBorderRadius
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClayBorderRadius Create(float topLeft, float topRight, float bottomLeft, float bottomRight)
        {
            return Methods.DenOfIz_ClayBorderRadius_Create(topLeft, topRight, bottomLeft, bottomRight);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClayBorderRadius CreateUniform(float radius)
        {
            return Methods.DenOfIz_ClayBorderRadius_CreateUniform(radius);
        }
    }

    public partial struct ClayBorderWidth
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClayBorderWidth Create(float left, float right, float top, float bottom, float betweenChildren)
        {
            return Methods.DenOfIz_ClayBorderWidth_Create(left, right, top, bottom, betweenChildren);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClayBorderWidth CreateUniform(float width)
        {
            return Methods.DenOfIz_ClayBorderWidth_CreateUniform(width);
        }
    }

    public partial struct ClayPadding
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClayPadding Create(float left, float right, float top, float bottom)
        {
            return Methods.DenOfIz_ClayPadding_Create(left, right, top, bottom);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClayPadding CreateUniform(float padding)
        {
            return Methods.DenOfIz_ClayPadding_CreateUniform(padding);
        }
    }

    public partial struct ClayElementDeclaration
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClayElementDeclaration Default()
        {
            return Methods.DenOfIz_ClayElementDeclaration_Default();
        }
    }

    public partial struct ClayTextDesc
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClayTextDesc Default()
        {
            return Methods.DenOfIz_ClayTextDesc_Default();
        }
    }

    public partial struct ClayClipDesc
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClayClipDesc Create(bool horizontal, bool vertical, Vector2 childOffset = default)
        {
            return new ClayClipDesc { Horizontal = horizontal, Vertical = vertical, ChildOffset = childOffset };
        }
    }

    public partial struct ClayTransitionStateDesc
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClayTransitionStateDesc FromOffset(Vector2 positionOffset)
        {
            return new ClayTransitionStateDesc { PositionOffset = positionOffset };
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClayTransitionStateDesc FromScale(float scale)
        {
            return new ClayTransitionStateDesc { Scale = scale };
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClayTransitionStateDesc FromOverlay(ClayColor overlayColor)
        {
            return new ClayTransitionStateDesc { OverlayColor = overlayColor };
        }

        public ClayTransitionStateDesc WithOffset(Vector2 positionOffset)
        {
            PositionOffset = positionOffset;
            return this;
        }

        public ClayTransitionStateDesc WithScale(float scale)
        {
            Scale = scale;
            return this;
        }

        public ClayTransitionStateDesc WithOverlay(ClayColor overlayColor)
        {
            OverlayColor = overlayColor;
            return this;
        }

        public ClayTransitionStateDesc WithBackgroundColor(ClayColor backgroundColor)
        {
            HasBackgroundColor = true;
            BackgroundColor = backgroundColor;
            return this;
        }

        public ClayTransitionStateDesc WithBorderColor(ClayColor borderColor)
        {
            HasBorderColor = true;
            BorderColor = borderColor;
            return this;
        }

        public ClayTransitionStateDesc WithBorderWidth(ClayBorderWidth borderWidth)
        {
            HasBorderWidth = true;
            BorderWidth = borderWidth;
            return this;
        }
    }

    public partial struct ClayTransitionDesc
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ClayTransitionDesc Default()
        {
            return Methods.DenOfIz_ClayTransitionDesc_Default();
        }

        public static ClayTransitionDesc Create(float duration, ClayTransitionPropertyFlagBits properties, ClayTransitionEasing easing = ClayTransitionEasing.EaseOut)
        {
            ClayTransitionDesc desc = Default();
            desc.Duration = duration;
            desc.Properties = (uint)properties;
            desc.Easing = easing;
            return desc;
        }

        public ClayTransitionPropertyFlagBits PropertyFlags
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => (ClayTransitionPropertyFlagBits)Properties;
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            set => Properties = (uint)value;
        }

        public ClayTransitionDesc WithEnter(ClayTransitionStateDesc state, ClayTransitionEnterTrigger trigger = ClayTransitionEnterTrigger.SkipOnFirstParentFrame)
        {
            Enter.Enabled = true;
            Enter.Trigger = trigger;
            Enter.State = state;
            return this;
        }

        public ClayTransitionDesc WithExit(ClayTransitionStateDesc state, ClayTransitionExitTrigger trigger = ClayTransitionExitTrigger.SkipWhenParentExits,
            ClayExitTransitionSiblingOrdering siblingOrdering = ClayExitTransitionSiblingOrdering.UnderneathSiblings)
        {
            Exit.Enabled = true;
            Exit.Trigger = trigger;
            Exit.SiblingOrdering = siblingOrdering;
            Exit.State = state;
            return this;
        }
    }
}
