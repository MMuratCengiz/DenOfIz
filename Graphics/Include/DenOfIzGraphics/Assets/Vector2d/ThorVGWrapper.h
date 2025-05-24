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

#include <thorvg.h>

#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzGraphics/Utilities/InteropMath.h>

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
    template class DZ_API InteropArray<ThorVGColorStop>;

    struct DZ_API ThorVGMatrix
    {
        float E11 = 1.0f, E12 = 0.0f, E13 = 0.0f;
        float E21 = 0.0f, E22 = 1.0f, E23 = 0.0f;
        float E31 = 0.0f, E32 = 0.0f, E33 = 1.0f;
    };

    class ThorVGPaint
    {
    public:
        DZ_API virtual ~ThorVGPaint( ) = default;

        DZ_API virtual bool Transform( const ThorVGMatrix &m ) = 0;
        DZ_API virtual bool Translate( float x, float y )      = 0;
        DZ_API virtual bool Scale( float factor )              = 0;
        DZ_API virtual bool Rotate( float degree )             = 0;

        DZ_API virtual bool Opacity( uint8_t opacity )                                     = 0;
        DZ_API virtual bool Composite( ThorVGPaint *target, ThorVGCompositeMethod method ) = 0;
        DZ_API virtual bool Blend( ThorVGBlendMethod method )                              = 0;

        DZ_API [[nodiscard]] virtual ThorVGBounds GetBounds( bool transformed = false ) const = 0;
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
        DZ_API virtual ~ThorVGGradient( ) = default;

        DZ_API virtual bool ColorStops( const InteropArray<ThorVGColorStop> &colorStops ) = 0;
        DZ_API virtual bool Spread( ThorVGSpreadMethod spread )                           = 0;
        DZ_API virtual bool Transform( const ThorVGMatrix &m )                            = 0;

    protected:
        ThorVGGradient( )                           = default;
        virtual tvg::Fill *GetInternalFill( ) const = 0;

    private:
        friend class ThorVGShape;
        friend class ThorVGPicture;
    };

    class ThorVGLinearGradient final : public ThorVGGradient
    {
        std::unique_ptr<tvg::LinearGradient> m_gradient = nullptr;

    public:
        ThorVGLinearGradient( );
        ~ThorVGLinearGradient( ) override = default;

        DZ_API [[nodiscard]] bool Linear( float x1, float y1, float x2, float y2 ) const;
        DZ_API [[nodiscard]] bool ColorStops( const InteropArray<ThorVGColorStop> &colorStops ) override;
        DZ_API [[nodiscard]] bool Spread( ThorVGSpreadMethod spread ) override;
        DZ_API [[nodiscard]] bool Transform( const ThorVGMatrix &m ) override;

    private:
        tvg::Fill *GetInternalFill( ) const override;
    };

    class ThorVGRadialGradient final : public ThorVGGradient
    {
        std::unique_ptr<tvg::RadialGradient> m_gradient = nullptr;

    public:
        ThorVGRadialGradient( );
        ~ThorVGRadialGradient( ) override = default;

        DZ_API [[nodiscard]] bool Radial( float cx, float cy, float radius ) const;
        DZ_API [[nodiscard]] bool ColorStops( const InteropArray<ThorVGColorStop> &colorStops ) override;
        DZ_API [[nodiscard]] bool Spread( ThorVGSpreadMethod spread ) override;
        DZ_API [[nodiscard]] bool Transform( const ThorVGMatrix &m ) override;

    private:
        tvg::Fill *GetInternalFill( ) const override;
    };

    class ThorVGShape final : public ThorVGPaint
    {
        std::unique_ptr<tvg::Shape> m_shape = nullptr;

    public:
        ThorVGShape( );
        ~ThorVGShape( ) override = default;

        DZ_API [[nodiscard]] bool Reset( ) const;
        DZ_API [[nodiscard]] bool MoveTo( float x, float y ) const;
        DZ_API [[nodiscard]] bool LineTo( float x, float y ) const;
        DZ_API [[nodiscard]] bool CubicTo( float cx1, float cy1, float cx2, float cy2, float x, float y ) const;
        DZ_API [[nodiscard]] bool Close( ) const;

        DZ_API [[nodiscard]] bool AppendRect( float x, float y, float w, float h, float rx = 0, float ry = 0 ) const;
        DZ_API [[nodiscard]] bool AppendCircle( float cx, float cy, float rx, float ry ) const;
        DZ_API [[nodiscard]] bool AppendPath( const InteropArray<Float_2> &points ) const;

        DZ_API [[nodiscard]] bool Fill( uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255 ) const;
        DZ_API [[nodiscard]] bool Fill( const ThorVGGradient *gradient ) const;

        DZ_API [[nodiscard]] bool Stroke( uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255 ) const;
        DZ_API [[nodiscard]] bool Stroke( float width ) const;
        DZ_API [[nodiscard]] bool Stroke( const ThorVGGradient *gradient ) const;
        DZ_API [[nodiscard]] bool StrokeCap( ThorVGStrokeCap cap ) const;
        DZ_API [[nodiscard]] bool StrokeJoin( ThorVGStrokeJoin join ) const;
        DZ_API [[nodiscard]] bool StrokeMiterlimit( float miterlimit ) const;
        DZ_API [[nodiscard]] bool StrokeDash( const InteropArray<float> &pattern, float offset = 0 ) const;

        DZ_API [[nodiscard]] bool         Transform( const ThorVGMatrix &m ) override;
        DZ_API [[nodiscard]] bool         Translate( float x, float y ) override;
        DZ_API [[nodiscard]] bool         Scale( float factor ) override;
        DZ_API [[nodiscard]] bool         Rotate( float degree ) override;
        DZ_API [[nodiscard]] bool         Opacity( uint8_t opacity ) override;
        DZ_API [[nodiscard]] bool         Composite( ThorVGPaint *target, ThorVGCompositeMethod method ) override;
        DZ_API [[nodiscard]] bool         Blend( ThorVGBlendMethod method ) override;
        DZ_API [[nodiscard]] ThorVGBounds GetBounds( bool transformed = false ) const override;
        DZ_API [[nodiscard]] ThorVGPaint *Duplicate( ) const override;

    private:
        tvg::Paint *GetInternalPaint( ) override;
    };

    class ThorVGPicture final : public ThorVGPaint
    {
        std::unique_ptr<tvg::Picture> m_picture = nullptr;

    public:
        DZ_API ThorVGPicture( );
        DZ_API ~ThorVGPicture( ) override = default;

        DZ_API bool Load( const char *path ) const;
        DZ_API bool Load( const char *data, uint32_t size, const char *mimeType = nullptr, bool copy = true ) const;
        DZ_API bool Load( uint32_t *data, uint32_t w, uint32_t h, bool premultiplied = true ) const;

        DZ_API [[nodiscard]] bool       Size( float w, float h ) const;
        DZ_API [[nodiscard]] ThorVGSize GetSize( ) const;

        DZ_API bool                       Transform( const ThorVGMatrix &m ) override;
        DZ_API bool                       Translate( float x, float y ) override;
        DZ_API bool                       Scale( float factor ) override;
        DZ_API bool                       Rotate( float degree ) override;
        DZ_API bool                       Opacity( uint8_t opacity ) override;
        DZ_API bool                       Composite( ThorVGPaint *target, ThorVGCompositeMethod method ) override;
        DZ_API bool                       Blend( ThorVGBlendMethod method ) override;
        DZ_API [[nodiscard]] ThorVGBounds GetBounds( bool transformed = false ) const override;
        DZ_API [[nodiscard]] ThorVGPaint *Duplicate( ) const override;

    private:
        tvg::Paint *GetInternalPaint( ) override;
    };

    class ThorVGScene final : public ThorVGPaint
    {
        std::unique_ptr<tvg::Scene> m_scene = nullptr;

    public:
        DZ_API ThorVGScene( );
        DZ_API ~ThorVGScene( ) override = default;

        DZ_API bool               Push( ThorVGPaint *paint ) const;
        DZ_API [[nodiscard]] bool Clear( bool free = true ) const;

        DZ_API bool                       Transform( const ThorVGMatrix &m ) override;
        DZ_API bool                       Translate( float x, float y ) override;
        DZ_API bool                       Scale( float factor ) override;
        DZ_API bool                       Rotate( float degree ) override;
        DZ_API bool                       Opacity( uint8_t opacity ) override;
        DZ_API bool                       Composite( ThorVGPaint *target, ThorVGCompositeMethod method ) override;
        DZ_API bool                       Blend( ThorVGBlendMethod method ) override;
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
        InteropArray<uint32_t>         m_data;
        std::unique_ptr<tvg::SwCanvas> m_canvas = nullptr;

    public:
        DZ_API explicit ThorVGCanvas( const ThorVGCanvasDesc &desc );
        DZ_API ~ThorVGCanvas( ) = default;

        DZ_API bool Push( ThorVGPaint *paint ) const;
        DZ_API bool Clear( bool free = true );
        DZ_API bool Update( ThorVGPaint *paint = nullptr ) const;
        DZ_API bool Draw( ) const;
        DZ_API bool Sync( ) const;
        DZ_API bool Viewport( int32_t x, int32_t y, int32_t w, int32_t h ) const;

        DZ_API bool Resize( uint32_t w, uint32_t h );
        DZ_API bool Mempool( ) const;

        DZ_API void  ResetData( );
        DZ_API const InteropArray<uint32_t> &GetData( ) const;
    };

    struct DZ_API ThorVGRendererDesc
    {
        ILogicalDevice *LogicalDevice = nullptr;
        uint32_t        Width         = 1024;
        uint32_t        Height        = 1024;
        uint32_t        ThreadCount   = 0;
        uint32_t        NumFrames     = 0;
    };

    class ThorVGRenderer
    {
    public:
        DZ_API explicit ThorVGRenderer( const ThorVGRendererDesc &desc );
        DZ_API ~ThorVGRenderer( );

        DZ_API ThorVGCanvas *GetCanvas( ) const;

        DZ_API bool BeginFrame( ) const;
        DZ_API bool EndFrame( ) const;
        DZ_API bool RenderToTexture( ITextureResource *target, ICommandList *commandList ) const;

        DZ_API bool              UpdateRenderTarget( ICommandList *commandList, uint32_t frameIndex ) const;
        DZ_API ITextureResource *GetRenderTarget( uint32_t frameIndex ) const;

        DZ_API bool Resize( uint32_t width, uint32_t height ) const;
    private:
        class Impl;
        std::unique_ptr<Impl> m_impl;
    };
} // namespace DenOfIz
