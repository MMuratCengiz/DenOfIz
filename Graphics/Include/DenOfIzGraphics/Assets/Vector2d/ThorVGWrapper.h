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

#pragma once

#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

namespace tvg
{
    class Paint;
    class Fill;
    class LinearGradient;
    class RadialGradient;
    class Shape;
    class Picture;
    class Scene;
    class SwCanvas;
} // namespace tvg

namespace DenOfIz
{
    class ThorVGPaint;
    class ThorVGGradient;
    class ThorVGShape;
    class ThorVGPicture;
    class ThorVGScene;
    class ThorVGLinearGradient;
    class ThorVGRadialGradient;
    class ThorVGCanvas;

    enum class ThorVGFillRule
    {
        Winding,
        EvenOdd
    };

    enum class ThorVGStrokeCap
    {
        Square,
        Round,
        Butt
    };

    enum class ThorVGStrokeJoin
    {
        Bevel,
        Round,
        Miter
    };

    enum class ThorVGCompositeMethod
    {
        ClipPath,
        AlphaMask,
        InvAlphaMask,
        LumaMask,
        InvLumaMask,
        AddMask,
        SubtractMask,
        IntersectMask,
        DifferenceMask,
        LightenMask,
        DarkenMask
    };

    enum class ThorVGBlendMethod
    {
        Normal,
        Add,
        Screen,
        Multiply,
        Overlay,
        Darken,
        Lighten,
        ColorDodge,
        ColorBurn,
        HardLight,
        SoftLight,
        Difference,
        Exclusion
    };

    enum class ThorVGSpreadMethod
    {
        Pad,
        Reflect,
        Repeat
    };

    struct DZ_API ThorVGColorStop
    {
        float   Offset;
        uint8_t R;
        uint8_t G;
        uint8_t B;
        uint8_t A;
    };

    struct DZ_API ThorVGColorStopArray
    {
        ThorVGColorStop *Elements;
        uint32_t         NumElements;
    };

    struct DZ_API ThorVGBounds
    {
        float X;
        float Y;
        float Width;
        float Height;
    };

    struct DZ_API ThorVGSize
    {
        float Width;
        float Height;
    };

    struct DZ_API ThorVGMatrix
    {
        float E11 = 1.0f, E12 = 0.0f, E13 = 0.0f;
        float E21 = 0.0f, E22 = 1.0f, E23 = 0.0f;
        float E31 = 0.0f, E32 = 0.0f, E33 = 1.0f;
    };

    class ThorVGPaint
    {
    public:
        DZ_API virtual ~ThorVGPaint( );

        DZ_API virtual void Transform( const ThorVGMatrix &m ) = 0;
        DZ_API virtual void Translate( float x, float y )      = 0;
        DZ_API virtual void Scale( float factor )              = 0;
        DZ_API virtual void Rotate( float degree )             = 0;

        DZ_API virtual void Opacity( uint8_t opacity )                                     = 0;
        DZ_API virtual void Composite( ThorVGPaint *target, ThorVGCompositeMethod method ) = 0;
        DZ_API virtual void Blend( ThorVGBlendMethod method )                              = 0;

        DZ_API virtual ThorVGBounds               GetBounds( bool transformed = false ) const = 0;
        DZ_API [[nodiscard]] virtual ThorVGPaint *Duplicate( ) const                          = 0;

    protected:
        ThorVGPaint( ) = default;

    private:
        virtual tvg::Paint *GetInternalPaint( ) = 0;

        friend class ThorVGShape;
        friend class ThorVGPicture;
        friend class ThorVGScene;
        friend class ThorVGCanvas;
        friend class ThorVGSwCanvas;
    };

    class ThorVGGradient
    {
    public:
        DZ_API virtual ~ThorVGGradient( );

        DZ_API virtual void ColorStops( const ThorVGColorStopArray &colorStops ) = 0;
        DZ_API virtual void Spread( ThorVGSpreadMethod spread )                           = 0;
        DZ_API virtual void Transform( const ThorVGMatrix &m )                            = 0;

    protected:
        ThorVGGradient( )                           = default;
        virtual tvg::Fill *GetInternalFill( ) const = 0;

    private:
        friend class ThorVGShape;
        friend class ThorVGPicture;
    };

    class ThorVGLinearGradient final : public ThorVGGradient
    {
        std::unique_ptr<tvg::LinearGradient> m_gradient;

    public:
        ThorVGLinearGradient( );
        ~ThorVGLinearGradient( ) override;

        DZ_API void Linear( float x1, float y1, float x2, float y2 ) const;
        DZ_API void ColorStops( const ThorVGColorStopArray &colorStops ) override;
        DZ_API void Spread( ThorVGSpreadMethod spread ) override;
        DZ_API void Transform( const ThorVGMatrix &m ) override;

    private:
        tvg::Fill *GetInternalFill( ) const override;
    };

    class ThorVGRadialGradient final : public ThorVGGradient
    {
        std::unique_ptr<tvg::RadialGradient> m_gradient;

    public:
        ThorVGRadialGradient( );
        ~ThorVGRadialGradient( ) override;

        DZ_API void Radial( float cx, float cy, float radius ) const;
        DZ_API void ColorStops( const ThorVGColorStopArray &colorStops ) override;
        DZ_API void Spread( ThorVGSpreadMethod spread ) override;
        DZ_API void Transform( const ThorVGMatrix &m ) override;

    private:
        tvg::Fill *GetInternalFill( ) const override;
    };

    class ThorVGShape final : public ThorVGPaint
    {
        std::unique_ptr<tvg::Shape> m_shape;

    public:
        ThorVGShape( );
        ~ThorVGShape( ) override;

        DZ_API void Reset( ) const;
        DZ_API void MoveTo( float x, float y ) const;
        DZ_API void LineTo( float x, float y ) const;
        DZ_API void CubicTo( float cx1, float cy1, float cx2, float cy2, float x, float y ) const;
        DZ_API void Close( ) const;

        DZ_API void AppendRect( float x, float y, float w, float h, float rx = 0, float ry = 0 ) const;
        DZ_API void AppendCircle( float cx, float cy, float rx, float ry ) const;
        DZ_API void AppendPath( const Float_2Array &points ) const;

        DZ_API void Fill( uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255 ) const;
        DZ_API void Fill( const ThorVGGradient *gradient ) const;

        DZ_API void Stroke( uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255 ) const;
        DZ_API void Stroke( float width ) const;
        DZ_API void Stroke( const ThorVGGradient *gradient ) const;
        DZ_API void StrokeCap( ThorVGStrokeCap cap ) const;
        DZ_API void StrokeJoin( ThorVGStrokeJoin join ) const;
        DZ_API void StrokeMiterlimit( float miterlimit ) const;
        DZ_API void StrokeDash( const FloatArray &pattern, float offset = 0 ) const;

        DZ_API void         Transform( const ThorVGMatrix &m ) override;
        DZ_API void         Translate( float x, float y ) override;
        DZ_API void         Scale( float factor ) override;
        DZ_API void         Rotate( float degree ) override;
        DZ_API void         Opacity( uint8_t opacity ) override;
        DZ_API void         Composite( ThorVGPaint *target, ThorVGCompositeMethod method ) override;
        DZ_API void         Blend( ThorVGBlendMethod method ) override;
        DZ_API ThorVGBounds GetBounds( bool transformed = false ) const override;
        DZ_API ThorVGPaint *Duplicate( ) const override;

    private:
        tvg::Paint *GetInternalPaint( ) override;
    };

    class ThorVGPicture final : public ThorVGPaint
    {
        std::unique_ptr<tvg::Picture> m_picture;

    public:
        DZ_API ThorVGPicture( );
        DZ_API ~ThorVGPicture( ) override;

        DZ_API void Load( const InteropString &path ) const;
        DZ_API void Load( const ByteArray &data, const InteropString &mimeType = nullptr, bool copy = true ) const;
        DZ_API void Load( uint32_t *data, uint32_t w, uint32_t h, bool premultiplied = true ) const;
        DZ_API void SetSize( float w, float h ) const;

        DZ_API [[nodiscard]] ThorVGSize GetSize( ) const;

        DZ_API void                       Transform( const ThorVGMatrix &m ) override;
        DZ_API void                       Translate( float x, float y ) override;
        DZ_API void                       Scale( float factor ) override;
        DZ_API void                       Rotate( float degree ) override;
        DZ_API void                       Opacity( uint8_t opacity ) override;
        DZ_API void                       Composite( ThorVGPaint *target, ThorVGCompositeMethod method ) override;
        DZ_API void                       Blend( ThorVGBlendMethod method ) override;
        DZ_API [[nodiscard]] ThorVGBounds GetBounds( bool transformed = false ) const override;
        DZ_API [[nodiscard]] ThorVGPaint *Duplicate( ) const override;

    private:
        tvg::Paint *GetInternalPaint( ) override;
    };

    class ThorVGScene final : public ThorVGPaint
    {
        std::unique_ptr<tvg::Scene> m_scene;

    public:
        DZ_API ThorVGScene( );
        DZ_API ~ThorVGScene( ) override;

        DZ_API void Push( ThorVGPaint *paint ) const;
        DZ_API void Clear( bool free = true ) const;

        DZ_API void                       Transform( const ThorVGMatrix &m ) override;
        DZ_API void                       Translate( float x, float y ) override;
        DZ_API void                       Scale( float factor ) override;
        DZ_API void                       Rotate( float degree ) override;
        DZ_API void                       Opacity( uint8_t opacity ) override;
        DZ_API void                       Composite( ThorVGPaint *target, ThorVGCompositeMethod method ) override;
        DZ_API void                       Blend( ThorVGBlendMethod method ) override;
        DZ_API [[nodiscard]] ThorVGBounds GetBounds( bool transformed = false ) const override;
        DZ_API [[nodiscard]] ThorVGPaint *Duplicate( ) const override;

    private:
        tvg::Paint *GetInternalPaint( ) override;
    };

    struct DZ_API ThorVGCanvasDesc
    {
        uint32_t Width;
        uint32_t Height;
    };

    class ThorVGCanvas
    {
        uint32_t                                  m_width{ };
        uint32_t                                  m_height{ };
        std::vector<uint32_t>                     m_rbgaData;
        mutable /*todo remove*/ std::vector<Byte> m_bytes;
        std::unique_ptr<tvg::SwCanvas>            m_canvas;

    public:
        DZ_API explicit ThorVGCanvas( const ThorVGCanvasDesc &desc );
        DZ_API ~ThorVGCanvas( );

        DZ_API void Push( ThorVGPaint *paint ) const;
        DZ_API void Clear( bool free = true );
        DZ_API void Update( ThorVGPaint *paint = nullptr ) const;
        DZ_API void Draw( ) const;
        DZ_API void Sync( ) const;
        DZ_API void Viewport( int32_t x, int32_t y, int32_t w, int32_t h ) const;

        DZ_API void Resize( uint32_t w, uint32_t h );

        DZ_API void            ResetData( );
        DZ_API UInt32ArrayView GetData( ) const;
        DZ_API ByteArrayView   GetDataAsBytes( ) const;
    };

    struct DZ_API ThorVGRendererDesc
    {
        ILogicalDevice *LogicalDevice = nullptr;
        uint32_t        Width         = 1024;
        uint32_t        Height        = 1024;
        uint32_t        ThreadCount   = 0;
        uint32_t        NumFrames     = 0;
    };
} // namespace DenOfIz
