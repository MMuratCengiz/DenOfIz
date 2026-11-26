/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include "DenOfIzGraphics/UI/UI.h"

namespace DenOfIz
{
    class UITextLayout;

    // Structured buffer elements shared with the UI shaders. Every member sits at its std430 alignment and the size is a multiple of 16, so the
    // DX layout the shaders are compiled with is also valid Vulkan std430 without scalar block layout.
    struct UIVertex
    {
        DenOfIz_Float4 Color;
        DenOfIz_Float3 Position;
        uint32_t       TextureIndex;
        DenOfIz_Float2 TexCoord;
        uint32_t       Glyph;
        uint32_t       Padding;
    };
    static_assert( sizeof( UIVertex ) == 48 && offsetof( UIVertex, Position ) == 16 && offsetof( UIVertex, TextureIndex ) == 28 && offsetof( UIVertex, TexCoord ) == 32 &&
                   offsetof( UIVertex, Glyph ) == 40 );

    // A rectangle the vertex shader expands into four vertices, drawn by the indices UIQuadIndexBit | quad * 4 + corner with the corners clockwise from
    // the top left. Shapes are drawn by indices of their UIVertex. Rect and UV are (left, top, right, bottom).
    struct UIQuad
    {
        DenOfIz_Float4 Rect;
        DenOfIz_Float4 UV;
        DenOfIz_Float4 Color;
        uint32_t       TextureIndex;
        uint32_t       Glyph;
        uint32_t       Padding[ 2 ];
    };
    static_assert( sizeof( UIQuad ) == 64 && offsetof( UIQuad, UV ) == 16 && offsetof( UIQuad, Color ) == 32 && offsetof( UIQuad, TextureIndex ) == 48 &&
                   offsetof( UIQuad, Glyph ) == 52 );

    // Below 2^24, the largest index value Vulkan guarantees without fullDrawIndexUint32.
    constexpr uint32_t UIQuadIndexBit = 1u << 23;
    constexpr uint32_t UIMaxQuads     = UIQuadIndexBit / 4;

    namespace UITextureSlots
    {
        constexpr uint32_t Solid       = 0;
        constexpr uint32_t Glyph       = 1;
        constexpr uint32_t ColourGlyph = 2;
        constexpr uint32_t FirstImage  = 3;
        constexpr uint32_t Count       = 128;
        constexpr uint32_t Bits        = 7;
        constexpr uint32_t Mask        = ( 1u << Bits ) - 1u;
    } // namespace UITextureSlots

    struct UIStateRecord
    {
        DenOfIz_Float4 A;
        DenOfIz_Float4 B;
        DenOfIz_Float4 C;
    };
    static_assert( sizeof( UIStateRecord ) == 48 && offsetof( UIStateRecord, B ) == 16 && offsetof( UIStateRecord, C ) == 32 );

    constexpr uint32_t UIMaxRoundedClips = DENOFIZ_UI_MAX_ROUNDED_CLIPS;
    constexpr float    UIUnbounded       = 1.0e7f;

    enum class UICommandType : uint8_t
    {
        Fill,
        Border,
        Image,
        Text,
        PushClip,
        PopClip,
        PushLayer,
        PopLayer
    };

    struct UIFillCommand
    {
        DenOfIz_UIColor   Color;
        DenOfIz_UICorners Radius;
    };

    struct UIBorderCommand
    {
        DenOfIz_UIColor   Color;
        DenOfIz_UIEdges   Widths;
        DenOfIz_UICorners Radius;
    };

    struct UIImageCommand
    {
        DenOfIz_Texture   Texture;
        DenOfIz_UIColor   Tint;
        DenOfIz_UICorners Radius;
        DenOfIz_Float4    UV;
    };

    struct UITextCommand
    {
        const UITextLayout *Layout;
        DenOfIz_UIColor     Color;
        float               Scale;
        float               BoxWidth;
        DenOfIz_UITextAlign Align;
    };

    struct UIClipCommand
    {
        DenOfIz_UICorners Radius;
        bool              Rounded;
        bool              Fixed;
    };

    struct UILayerCommand
    {
        float Scale;
        float BiasR;
        float BiasG;
        float BiasB;
        float Opacity;
    };

    struct UICommand
    {
        UICommandType  Type;
        DenOfIz_UIRect Rect;
        union
        {
            UIFillCommand   Fill;
            UIBorderCommand Border;
            UIImageCommand  Image;
            UITextCommand   Text;
            UIClipCommand   Clip;
            UILayerCommand  Layer;
        };
    };

    namespace UIMath
    {
        inline uint32_t Fmix32( uint32_t h )
        {
            h ^= h >> 16;
            h *= 0x85EBCA6Bu;
            h ^= h >> 13;
            h *= 0xC2B2AE35u;
            h ^= h >> 16;
            return h;
        }

        inline uint32_t HashId( const char *chars, const size_t length, const uint32_t seed )
        {
            uint32_t hash = 2166136261u ^ Fmix32( seed );
            for ( size_t i = 0; i < length; ++i )
            {
                hash ^= static_cast<uint8_t>( chars[ i ] );
                hash *= 16777619u;
            }
            hash = Fmix32( hash );
            return hash != 0 ? hash : 1;
        }

        inline uint32_t CombineId( const uint32_t id, const uint32_t value )
        {
            const uint32_t hash = Fmix32( id ^ Fmix32( value + 0x9E3779B9u ) );
            return hash != 0 ? hash : 1;
        }

        inline float Lerp( const float a, const float b, const float t )
        {
            return a + ( b - a ) * t;
        }

        inline DenOfIz_UIColor LerpColor( const DenOfIz_UIColor &a, const DenOfIz_UIColor &b, const float t )
        {
            const float s      = std::clamp( t, 0.0f, 1.0f );
            const float wa     = a.A * ( 1.0f - s );
            const float wb     = b.A * s;
            const float weight = wa + wb;
            if ( weight <= 0.0f )
            {
                return DenOfIz_UIColor{ b.R, b.G, b.B, 0 };
            }
            auto channel = [ & ]( const uint8_t x, const uint8_t y ) { return static_cast<uint8_t>( std::lround( std::clamp( ( x * wa + y * wb ) / weight, 0.0f, 255.0f ) ) ); };
            return DenOfIz_UIColor{ channel( a.R, b.R ), channel( a.G, b.G ), channel( a.B, b.B ), static_cast<uint8_t>( std::lround( weight ) ) };
        }

        inline bool ColorEquals( const DenOfIz_UIColor &a, const DenOfIz_UIColor &b )
        {
            return a.R == b.R && a.G == b.G && a.B == b.B && a.A == b.A;
        }

        inline bool HasCorners( const DenOfIz_UICorners &c )
        {
            return c.TopLeft > 0.0f || c.TopRight > 0.0f || c.BottomRight > 0.0f || c.BottomLeft > 0.0f;
        }

        inline bool HasEdges( const DenOfIz_UIEdges &e )
        {
            return e.Left > 0.0f || e.Top > 0.0f || e.Right > 0.0f || e.Bottom > 0.0f;
        }

        inline bool Intersects( const DenOfIz_UIRect &a, const DenOfIz_UIRect &b )
        {
            return a.X < b.X + b.Width && b.X < a.X + a.Width && a.Y < b.Y + b.Height && b.Y < a.Y + a.Height;
        }

        inline bool Contains( const DenOfIz_UIRect &r, const float x, const float y )
        {
            return x >= r.X && y >= r.Y && x < r.X + r.Width && y < r.Y + r.Height;
        }

        inline DenOfIz_UIRect Intersect( const DenOfIz_UIRect &a, const DenOfIz_UIRect &b )
        {
            const float x0 = std::max( a.X, b.X );
            const float y0 = std::max( a.Y, b.Y );
            const float x1 = std::min( a.X + a.Width, b.X + b.Width );
            const float y1 = std::min( a.Y + a.Height, b.Y + b.Height );
            return DenOfIz_UIRect{ x0, y0, std::max( 0.0f, x1 - x0 ), std::max( 0.0f, y1 - y0 ) };
        }

        inline DenOfIz_UIRect Union( const DenOfIz_UIRect &a, const DenOfIz_UIRect &b )
        {
            const float x0 = std::min( a.X, b.X );
            const float y0 = std::min( a.Y, b.Y );
            const float x1 = std::max( a.X + a.Width, b.X + b.Width );
            const float y1 = std::max( a.Y + a.Height, b.Y + b.Height );
            return DenOfIz_UIRect{ x0, y0, x1 - x0, y1 - y0 };
        }

        inline DenOfIz_UICorners ScaleCorners( const DenOfIz_UICorners &c, const float s )
        {
            return DenOfIz_UICorners{ c.TopLeft * s, c.TopRight * s, c.BottomRight * s, c.BottomLeft * s };
        }

        inline DenOfIz_UIEdges ScaleEdges( const DenOfIz_UIEdges &e, const float s )
        {
            return DenOfIz_UIEdges{ e.Left * s, e.Top * s, e.Right * s, e.Bottom * s };
        }

        inline DenOfIz_UICorners InnerCorners( const DenOfIz_UICorners &c, const DenOfIz_UIEdges &b )
        {
            return DenOfIz_UICorners{ std::max( 0.0f, c.TopLeft - std::max( b.Left, b.Top ) ), std::max( 0.0f, c.TopRight - std::max( b.Right, b.Top ) ),
                                      std::max( 0.0f, c.BottomRight - std::max( b.Right, b.Bottom ) ), std::max( 0.0f, c.BottomLeft - std::max( b.Left, b.Bottom ) ) };
        }

        inline DenOfIz_UICorners FitCorners( const float width, const float height, const DenOfIz_UICorners &c )
        {
            DenOfIz_UICorners r{ std::max( 0.0f, c.TopLeft ), std::max( 0.0f, c.TopRight ), std::max( 0.0f, c.BottomRight ), std::max( 0.0f, c.BottomLeft ) };
            float      scale = 1.0f;
            const auto limit = [ &scale ]( const float extent, const float a, const float b )
            {
                if ( a + b > extent )
                {
                    scale = std::min( scale, std::max( 0.0f, extent ) / ( a + b ) );
                }
            };
            limit( width, r.TopLeft, r.TopRight );
            limit( width, r.BottomLeft, r.BottomRight );
            limit( height, r.TopLeft, r.BottomLeft );
            limit( height, r.TopRight, r.BottomRight );
            return scale < 1.0f ? ScaleCorners( r, scale ) : r;
        }

        inline float RoundedRectDistance( const DenOfIz_UIRect &rect, const DenOfIz_UICorners &radius, const float x, const float y )
        {
            const float x0 = rect.X;
            const float y0 = rect.Y;
            const float x1 = rect.X + rect.Width;
            const float y1 = rect.Y + rect.Height;
            float       d  = std::max( std::max( x0 - x, x - x1 ), std::max( y0 - y, y - y1 ) );
            const auto  arc = [ &d, x, y ]( const float r, const float cx, const float cy, const bool inside )
            {
                if ( r > 0.0f && inside )
                {
                    d = std::max( d, std::hypot( x - cx, y - cy ) - r );
                }
            };
            arc( radius.TopLeft, x0 + radius.TopLeft, y0 + radius.TopLeft, x < x0 + radius.TopLeft && y < y0 + radius.TopLeft );
            arc( radius.TopRight, x1 - radius.TopRight, y0 + radius.TopRight, x > x1 - radius.TopRight && y < y0 + radius.TopRight );
            arc( radius.BottomRight, x1 - radius.BottomRight, y1 - radius.BottomRight, x > x1 - radius.BottomRight && y > y1 - radius.BottomRight );
            arc( radius.BottomLeft, x0 + radius.BottomLeft, y1 - radius.BottomLeft, x < x0 + radius.BottomLeft && y > y1 - radius.BottomLeft );
            return d;
        }

        inline UILayerCommand OverlayLayer( const DenOfIz_UIColor &overlay, const float opacity )
        {
            const float a = overlay.A / 255.0f;
            return UILayerCommand{ 1.0f - a, overlay.R / 255.0f * a, overlay.G / 255.0f * a, overlay.B / 255.0f * a, std::clamp( opacity, 0.0f, 1.0f ) };
        }

        inline UILayerCommand ComposeLayer( const UILayerCommand &outer, const UILayerCommand &inner )
        {
            return UILayerCommand{ inner.Scale * outer.Scale, inner.BiasR * outer.Scale + outer.BiasR, inner.BiasG * outer.Scale + outer.BiasG,
                                   inner.BiasB * outer.Scale + outer.BiasB, inner.Opacity * outer.Opacity };
        }

        inline bool IsIdentityLayer( const UILayerCommand &layer )
        {
            return layer.Scale == 1.0f && layer.BiasR == 0.0f && layer.BiasG == 0.0f && layer.BiasB == 0.0f && layer.Opacity == 1.0f;
        }
    } // namespace UIMath
} // namespace DenOfIz
