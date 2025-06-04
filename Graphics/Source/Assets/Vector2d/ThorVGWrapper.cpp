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

#include "DenOfIzGraphics/Assets/Vector2d/ThorVGWrapper.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "DenOfIzGraphics/Data/Texture.h"

#include <thorvg.h>

#include <cstring>
#include <memory>

using namespace DenOfIz;

namespace DenOfIz
{
    static void ThorVGCheckResult( tvg::Result result )
    {
        if ( result != tvg::Result::Success )
        {
            LOG( ERROR ) << "ThorVG invalid result: " << static_cast<uint32_t>( result );
        }
    }

    static tvg::Matrix ToTvgMatrix( const ThorVGMatrix &m )
    {
        return { m.E11, m.E12, m.E13, m.E21, m.E22, m.E23, m.E31, m.E32, m.E33 };
    }

    static tvg::StrokeCap ToTvgStrokeCap( const ThorVGStrokeCap cap )
    {
        switch ( cap )
        {
        case ThorVGStrokeCap::Square:
            return tvg::StrokeCap::Square;
        case ThorVGStrokeCap::Round:
            return tvg::StrokeCap::Round;
        case ThorVGStrokeCap::Butt:
            return tvg::StrokeCap::Butt;
        default:
            return tvg::StrokeCap::Square;
        }
    }

    static tvg::StrokeJoin ToTvgStrokeJoin( const ThorVGStrokeJoin join )
    {
        switch ( join )
        {
        case ThorVGStrokeJoin::Bevel:
            return tvg::StrokeJoin::Bevel;
        case ThorVGStrokeJoin::Round:
            return tvg::StrokeJoin::Round;
        case ThorVGStrokeJoin::Miter:
            return tvg::StrokeJoin::Miter;
        default:
            return tvg::StrokeJoin::Bevel;
        }
    }

    static tvg::CompositeMethod ToTvgCompositeMethod( const ThorVGCompositeMethod method )
    {
        switch ( method )
        {
        case ThorVGCompositeMethod::ClipPath:
            return tvg::CompositeMethod::ClipPath;
        case ThorVGCompositeMethod::AlphaMask:
            return tvg::CompositeMethod::AlphaMask;
        case ThorVGCompositeMethod::InvAlphaMask:
            return tvg::CompositeMethod::InvAlphaMask;
        case ThorVGCompositeMethod::LumaMask:
            return tvg::CompositeMethod::LumaMask;
        case ThorVGCompositeMethod::InvLumaMask:
            return tvg::CompositeMethod::InvLumaMask;
        case ThorVGCompositeMethod::AddMask:
            return tvg::CompositeMethod::AddMask;
        case ThorVGCompositeMethod::SubtractMask:
            return tvg::CompositeMethod::SubtractMask;
        case ThorVGCompositeMethod::IntersectMask:
            return tvg::CompositeMethod::IntersectMask;
        case ThorVGCompositeMethod::DifferenceMask:
            return tvg::CompositeMethod::DifferenceMask;
        case ThorVGCompositeMethod::LightenMask:
            return tvg::CompositeMethod::LightenMask;
        case ThorVGCompositeMethod::DarkenMask:
            return tvg::CompositeMethod::DarkenMask;
        default:
            return tvg::CompositeMethod::ClipPath;
        }
    }

    static tvg::BlendMethod ToTvgBlendMethod( const ThorVGBlendMethod method )
    {
        switch ( method )
        {
        case ThorVGBlendMethod::Normal:
            return tvg::BlendMethod::Normal;
        case ThorVGBlendMethod::Add:
            return tvg::BlendMethod::Add;
        case ThorVGBlendMethod::Screen:
            return tvg::BlendMethod::Screen;
        case ThorVGBlendMethod::Multiply:
            return tvg::BlendMethod::Multiply;
        case ThorVGBlendMethod::Overlay:
            return tvg::BlendMethod::Overlay;
        case ThorVGBlendMethod::Darken:
            return tvg::BlendMethod::Darken;
        case ThorVGBlendMethod::Lighten:
            return tvg::BlendMethod::Lighten;
        case ThorVGBlendMethod::ColorDodge:
            return tvg::BlendMethod::ColorDodge;
        case ThorVGBlendMethod::ColorBurn:
            return tvg::BlendMethod::ColorBurn;
        case ThorVGBlendMethod::HardLight:
            return tvg::BlendMethod::HardLight;
        case ThorVGBlendMethod::SoftLight:
            return tvg::BlendMethod::SoftLight;
        case ThorVGBlendMethod::Difference:
            return tvg::BlendMethod::Difference;
        case ThorVGBlendMethod::Exclusion:
            return tvg::BlendMethod::Exclusion;
        default:
            return tvg::BlendMethod::Normal;
        }
    }

    static tvg::FillSpread ToTvgSpread( const ThorVGSpreadMethod spread )
    {
        switch ( spread )
        {
        case ThorVGSpreadMethod::Pad:
            return tvg::FillSpread::Pad;
        case ThorVGSpreadMethod::Reflect:
            return tvg::FillSpread::Reflect;
        case ThorVGSpreadMethod::Repeat:
            return tvg::FillSpread::Repeat;
        default:
            return tvg::FillSpread::Pad;
        }
    }

    ThorVGLinearGradient::ThorVGLinearGradient( )
    {
        m_gradient = tvg::LinearGradient::gen( );
    }

    void ThorVGLinearGradient::Linear( const float x1, const float y1, const float x2, const float y2 ) const
    {
        ThorVGCheckResult( m_gradient->linear( x1, y1, x2, y2 ) );
    }

    void ThorVGLinearGradient::ColorStops( const InteropArray<ThorVGColorStop> &colorStops )
    {
        std::vector<tvg::Fill::ColorStop> stops;
        stops.reserve( colorStops.NumElements( ) );

        for ( uint32_t i = 0; i < colorStops.NumElements( ); ++i )
        {
            const auto &stop = colorStops.GetElement( i );
            stops.push_back( { stop.Offset, stop.R, stop.G, stop.B, stop.A } );
        }

        ThorVGCheckResult( m_gradient->colorStops( stops.data( ), static_cast<uint32_t>( stops.size( ) ) ) );
    }

    void ThorVGLinearGradient::Spread( const ThorVGSpreadMethod spread )
    {
        ThorVGCheckResult( m_gradient->spread( ToTvgSpread( spread ) ) );
    }

    void ThorVGLinearGradient::Transform( const ThorVGMatrix &m )
    {
        ThorVGCheckResult( m_gradient->transform( ToTvgMatrix( m ) ) );
    }

    tvg::Fill *ThorVGLinearGradient::GetInternalFill( ) const
    {
        return m_gradient.get( );
    }

    ThorVGRadialGradient::ThorVGRadialGradient( )
    {
        m_gradient = tvg::RadialGradient::gen( );
    }

    void ThorVGRadialGradient::Radial( const float cx, const float cy, const float radius ) const
    {
        ThorVGCheckResult( m_gradient->radial( cx, cy, radius ) );
    }

    void ThorVGRadialGradient::ColorStops( const InteropArray<ThorVGColorStop> &colorStops )
    {
        std::vector<tvg::Fill::ColorStop> stops;
        stops.reserve( colorStops.NumElements( ) );

        for ( uint32_t i = 0; i < colorStops.NumElements( ); ++i )
        {
            const auto &stop = colorStops.GetElement( i );
            stops.push_back( { stop.Offset, stop.R, stop.G, stop.B, stop.A } );
        }

        ThorVGCheckResult( m_gradient->colorStops( stops.data( ), static_cast<uint32_t>( stops.size( ) ) ) );
    }

    void ThorVGRadialGradient::Spread( const ThorVGSpreadMethod spread )
    {
        ThorVGCheckResult( m_gradient->spread( ToTvgSpread( spread ) ) );
    }

    void ThorVGRadialGradient::Transform( const ThorVGMatrix &m )
    {
        ThorVGCheckResult( m_gradient->transform( ToTvgMatrix( m ) ) );
    }

    tvg::Fill *ThorVGRadialGradient::GetInternalFill( ) const
    {
        return m_gradient.get( );
    }

    ThorVGShape::ThorVGShape( )
    {
        m_shape = tvg::Shape::gen( );
    }

    void ThorVGShape::Reset( ) const
    {
        ThorVGCheckResult( m_shape->reset( ) );
    }

    void ThorVGShape::MoveTo( const float x, const float y ) const
    {
        ThorVGCheckResult( m_shape->moveTo( x, y ) );
    }

    void ThorVGShape::LineTo( const float x, const float y ) const
    {
        ThorVGCheckResult( m_shape->lineTo( x, y ) );
    }

    void ThorVGShape::CubicTo( const float cx1, const float cy1, const float cx2, const float cy2, const float x, const float y ) const
    {
        ThorVGCheckResult( m_shape->cubicTo( cx1, cy1, cx2, cy2, x, y ) );
    }

    void ThorVGShape::Close( ) const
    {
        ThorVGCheckResult( m_shape->close( ) );
    }

    void ThorVGShape::AppendRect( const float x, const float y, const float w, const float h, const float rx, const float ry ) const
    {
        ThorVGCheckResult( m_shape->appendRect( x, y, w, h, rx, ry ) );
    }

    void ThorVGShape::AppendCircle( const float cx, const float cy, const float rx, const float ry ) const
    {
        ThorVGCheckResult( m_shape->appendCircle( cx, cy, rx, ry ) );
    }

    void ThorVGShape::AppendPath( const InteropArray<Float_2> &points ) const
    {
        std::vector<tvg::PathCommand> commands;
        std::vector<tvg::Point>       pts;

        pts.push_back( { points.GetElement( 0 ).X, points.GetElement( 0 ).Y } );
        commands.push_back( tvg::PathCommand::MoveTo );

        for ( uint32_t i = 1; i < points.NumElements( ); ++i )
        {
            pts.push_back( { points.GetElement( i ).X, points.GetElement( i ).Y } );
            commands.push_back( tvg::PathCommand::LineTo );
        }

        ThorVGCheckResult( m_shape->appendPath( commands.data( ), static_cast<uint32_t>( commands.size( ) ), pts.data( ), static_cast<uint32_t>( pts.size( ) ) ) );
    }

    void ThorVGShape::Fill( const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a ) const
    {
        ThorVGCheckResult( m_shape->fill( r, g, b, a ) );
    }

    void ThorVGShape::Fill( const ThorVGGradient *gradient ) const
    {
        if ( !gradient )
        {
            LOG( ERROR ) << "ThorVGShape::Fill: gradient cannot be null";
            return;
        }
        const auto fill = gradient->GetInternalFill( );
        if ( !fill )
        {
            LOG( ERROR ) << "Specified gradient computed null internal fill";
            return;
        }

        const auto cloned = fill->duplicate( );
        ThorVGCheckResult( m_shape->fill( std::unique_ptr<tvg::Fill>( cloned ) ) );
    }

    void ThorVGShape::Stroke( const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a ) const
    {
        ThorVGCheckResult( m_shape->stroke( r, g, b, a ) );
    }

    void ThorVGShape::Stroke( const float width ) const
    {
        ThorVGCheckResult( m_shape->stroke( width ) );
    }

    void ThorVGShape::Stroke( const ThorVGGradient *gradient ) const
    {
        if ( !gradient )
        {
            LOG( ERROR ) << "ThorVGShape::Stroke: gradient cannot be null";
            return;
        }
        const auto fill = gradient->GetInternalFill( );
        if ( !fill )
        {
            LOG( ERROR ) << "Specified gradient computed null internal fill";
            return;
        }

        const auto cloned = fill->duplicate( );
        ThorVGCheckResult( m_shape->stroke( std::unique_ptr<tvg::Fill>( cloned ) ) );
    }

    void ThorVGShape::StrokeCap( const ThorVGStrokeCap cap ) const
    {
        ThorVGCheckResult( m_shape->stroke( ToTvgStrokeCap( cap ) ) );
    }

    void ThorVGShape::StrokeJoin( const ThorVGStrokeJoin join ) const
    {
        ThorVGCheckResult( m_shape->stroke( ToTvgStrokeJoin( join ) ) );
    }

    void ThorVGShape::StrokeMiterlimit( const float miterlimit ) const
    {
        ThorVGCheckResult( m_shape->stroke( miterlimit ) );
    }

    void ThorVGShape::StrokeDash( const InteropArray<float> &pattern, float offset ) const
    {
        if ( pattern.NumElements( ) == 0 )
        {
            ThorVGCheckResult( m_shape->stroke( nullptr, 0 ) );
        }

        std::vector<float> dashPattern;
        dashPattern.reserve( pattern.NumElements( ) );

        for ( uint32_t i = 0; i < pattern.NumElements( ); ++i )
        {
            dashPattern.push_back( pattern.GetElement( i ) );
        }
        ThorVGCheckResult( m_shape->stroke( dashPattern.data( ), static_cast<uint32_t>( dashPattern.size( ) ) ) );
    }

    void ThorVGShape::Transform( const ThorVGMatrix &m )
    {
        ThorVGCheckResult( m_shape->transform( ToTvgMatrix( m ) ) );
    }

    void ThorVGShape::Translate( const float x, const float y )
    {
        ThorVGCheckResult( m_shape->translate( x, y ) );
    }

    void ThorVGShape::Scale( const float factor )
    {
        ThorVGCheckResult( m_shape->scale( factor ) );
    }

    void ThorVGShape::Rotate( const float degree )
    {
        ThorVGCheckResult( m_shape->rotate( degree ) );
    }

    void ThorVGShape::Opacity( const uint8_t opacity )
    {
        ThorVGCheckResult( m_shape->opacity( opacity ) );
    }

    void ThorVGShape::Composite( ThorVGPaint *target, const ThorVGCompositeMethod method )
    {
        if ( !target )
        {
            LOG( ERROR ) << "ThorVGShape::Composite: target cannot be null";
            return;
        }
        const auto paint = target->GetInternalPaint( );
        if ( !paint )
        {
            LOG( ERROR ) << "Specified paint computed null internal paint";
            return;
        }

        const auto cloned = paint->duplicate( );
        ThorVGCheckResult( m_shape->composite( std::unique_ptr<tvg::Paint>( cloned ), ToTvgCompositeMethod( method ) ) );
    }

    void ThorVGShape::Blend( const ThorVGBlendMethod method )
    {
        ThorVGCheckResult( m_shape->blend( ToTvgBlendMethod( method ) ) );
    }

    ThorVGBounds ThorVGShape::GetBounds( const bool transformed ) const
    {
        ThorVGBounds bounds = { };
        m_shape->bounds( &bounds.X, &bounds.Y, &bounds.Width, &bounds.Height, transformed );
        return bounds;
    }

    ThorVGPaint *ThorVGShape::Duplicate( ) const
    {
        const auto newShape = new ThorVGShape( );
        newShape->m_shape   = std::unique_ptr<tvg::Shape>( static_cast<tvg::Shape *>( m_shape->duplicate( ) ) );
        return newShape;
    }

    tvg::Paint *ThorVGShape::GetInternalPaint( )
    {
        return m_shape.get( );
    }

    ThorVGPicture::ThorVGPicture( )
    {
        m_picture = tvg::Picture::gen( );
    }

    void ThorVGPicture::Load( const InteropString &path ) const
    {
        ThorVGCheckResult( m_picture->load( path.Get( ) ) );
    }

    void ThorVGPicture::Load( const InteropArray<Byte> &data, const InteropString &mimeType, const bool copy ) const
    {
        ThorVGCheckResult( m_picture->load( reinterpret_cast<const char *>( data.Data( ) ), static_cast<uint32_t>( data.NumElements( ) ), mimeType.Get( ), copy ) );
    }

    void ThorVGPicture::Load( uint32_t *data, const uint32_t w, const uint32_t h, const bool premultiplied ) const
    {
        ThorVGCheckResult( m_picture->load( data, w, h, premultiplied ) );
    }

    void ThorVGPicture::Size( const float w, const float h ) const
    {
        ThorVGCheckResult( m_picture->size( w, h ) );
    }

    ThorVGSize ThorVGPicture::GetSize( ) const
    {
        ThorVGSize size    = { };
        const auto picture = static_cast<const tvg::Picture *>( m_picture.get( ) );
        picture->size( &size.Width, &size.Height );
        return size;
    }

    void ThorVGPicture::Transform( const ThorVGMatrix &m )
    {
        ThorVGCheckResult( m_picture->transform( ToTvgMatrix( m ) ) );
    }

    void ThorVGPicture::Translate( const float x, const float y )
    {
        ThorVGCheckResult( m_picture->translate( x, y ) );
    }

    void ThorVGPicture::Scale( const float factor )
    {
        ThorVGCheckResult( m_picture->scale( factor ) );
    }

    void ThorVGPicture::Rotate( const float degree )
    {
        ThorVGCheckResult( m_picture->rotate( degree ) );
    }

    void ThorVGPicture::Opacity( const uint8_t opacity )
    {
        ThorVGCheckResult( m_picture->opacity( opacity ) );
    }

    void ThorVGPicture::Composite( ThorVGPaint *target, const ThorVGCompositeMethod method )
    {
        if ( !target )
        {
            LOG( ERROR ) << "ThorVGPicture::Composite: target cannot be null";
            return;
        }
        const auto paint = target->GetInternalPaint( );
        if ( !paint )
        {
            LOG( ERROR ) << "Specified paint computed null internal paint";
            return;
        }

        const auto cloned = paint->duplicate( );
        ThorVGCheckResult( m_picture->composite( std::unique_ptr<tvg::Paint>( cloned ), ToTvgCompositeMethod( method ) ) );
    }

    void ThorVGPicture::Blend( const ThorVGBlendMethod method )
    {
        ThorVGCheckResult( m_picture->blend( ToTvgBlendMethod( method ) ) );
    }

    ThorVGBounds ThorVGPicture::GetBounds( const bool transformed ) const
    {
        ThorVGBounds bounds = { };
        m_picture->bounds( &bounds.X, &bounds.Y, &bounds.Width, &bounds.Height, transformed );
        return bounds;
    }

    ThorVGPaint *ThorVGPicture::Duplicate( ) const
    {
        const auto newPicture = new ThorVGPicture( );
        newPicture->m_picture = std::unique_ptr<tvg::Picture>( static_cast<tvg::Picture *>( m_picture->duplicate( ) ) );
        return newPicture;
    }

    tvg::Paint *ThorVGPicture::GetInternalPaint( )
    {
        return m_picture.get( );
    }

    ThorVGScene::ThorVGScene( )
    {
        m_scene = tvg::Scene::gen( );
    }

    void ThorVGScene::Push( ThorVGPaint *paint ) const
    {
        if ( !paint )
        {
            LOG( ERROR ) << "Provided paint cannot be null";
            return;
        }
        const auto p = paint->GetInternalPaint( );
        if ( !p )
        {
            LOG( ERROR ) << "Provided paint computed null internal paint";
            return;
        }

        const auto cloned = p->duplicate( );
        ThorVGCheckResult( m_scene->push( std::unique_ptr<tvg::Paint>( cloned ) ) );
    }

    void ThorVGScene::Clear( const bool free ) const
    {
        ThorVGCheckResult( m_scene->clear( free ) );
    }

    void ThorVGScene::Transform( const ThorVGMatrix &m )
    {
        ThorVGCheckResult( m_scene->transform( ToTvgMatrix( m ) ) );
    }

    void ThorVGScene::Translate( const float x, const float y )
    {
        ThorVGCheckResult( m_scene->translate( x, y ) );
    }

    void ThorVGScene::Scale( const float factor )
    {
        ThorVGCheckResult( m_scene->scale( factor ) );
    }

    void ThorVGScene::Rotate( const float degree )
    {
        ThorVGCheckResult( m_scene->rotate( degree ) );
    }

    void ThorVGScene::Opacity( const uint8_t opacity )
    {
        ThorVGCheckResult( m_scene->opacity( opacity ) );
    }

    void ThorVGScene::Composite( ThorVGPaint *target, const ThorVGCompositeMethod method )
    {
        if ( !target )
        {
            LOG( ERROR ) << "ThorVGScene::Composite: target cannot be null";
            return;
        }
        const auto paint = target->GetInternalPaint( );
        if ( !paint )
        {
            LOG( ERROR ) << "Specified paint computed null internal paint";
            return;
        }

        const auto cloned = paint->duplicate( );
        ThorVGCheckResult( m_scene->composite( std::unique_ptr<tvg::Paint>( cloned ), ToTvgCompositeMethod( method ) ) );
    }

    void ThorVGScene::Blend( const ThorVGBlendMethod method )
    {
        ThorVGCheckResult( m_scene->blend( ToTvgBlendMethod( method ) ) );
    }

    ThorVGBounds ThorVGScene::GetBounds( const bool transformed ) const
    {
        ThorVGBounds bounds = { };
        m_scene->bounds( &bounds.X, &bounds.Y, &bounds.Width, &bounds.Height, transformed );
        return bounds;
    }

    ThorVGPaint *ThorVGScene::Duplicate( ) const
    {
        const auto newScene = new ThorVGScene( );
        newScene->m_scene   = std::unique_ptr<tvg::Scene>( static_cast<tvg::Scene *>( m_scene->duplicate( ) ) );
        return newScene;
    }

    tvg::Paint *ThorVGScene::GetInternalPaint( )
    {
        return m_scene.get( );
    }

    ThorVGCanvas::ThorVGCanvas( const ThorVGCanvasDesc &desc )
    {
        m_canvas = tvg::SwCanvas::gen( );
        m_canvas->mempool( tvg::SwCanvas::MempoolPolicy::Individual );
        Resize( desc.Width, desc.Height );
    }

    void ThorVGCanvas::Push( ThorVGPaint *paint ) const
    {
        if ( !paint )
        {
            LOG( WARNING ) << "Invalid paint or canvas";
            return;
        }
        const auto p = paint->GetInternalPaint( );
        if ( !p )
        {
            LOG( WARNING ) << "Invalid paint";
            return;
        }
        const auto cloned = p->duplicate( );
        ThorVGCheckResult( m_canvas->push( std::unique_ptr<tvg::Paint>( cloned ) ) );
    }

    void ThorVGCanvas::Clear( const bool free )
    {
        ResetData( );
        ThorVGCheckResult( m_canvas->clear( free ) );
    }

    void ThorVGCanvas::Update( ThorVGPaint *paint ) const
    {
        if ( paint )
        {
            const auto p = paint->GetInternalPaint( );
            if ( !p )
            {
                LOG( ERROR ) << "Specified paint computed null internal paint";
                return;
            }
            ThorVGCheckResult( m_canvas->update( p ) );
            return;
        }

        ThorVGCheckResult( m_canvas->update( ) );
    }

    void ThorVGCanvas::Draw( ) const
    {
        ThorVGCheckResult( m_canvas->draw( ) );
    }

    void ThorVGCanvas::Sync( ) const
    {
        ThorVGCheckResult( m_canvas->sync( ) );
    }

    void ThorVGCanvas::Viewport( const int32_t x, const int32_t y, const int32_t w, const int32_t h ) const
    {
        ThorVGCheckResult( m_canvas->viewport( x, y, w, h ) );
    }

    void ThorVGCanvas::Resize( const uint32_t w, const uint32_t h )
    {
        m_data.Resize( w * h * 4 );
        ThorVGCheckResult( m_canvas->target( m_data.Data( ), w, w, h, tvg::SwCanvas::ARGB8888 ) );
    }

    void ThorVGCanvas::ResetData( )
    {
        for ( int i = 0; i < m_data.NumElements( ); ++i )
        {
            m_data.SetElement( i, 0 );
        }
    }
    const InteropArray<uint32_t> &ThorVGCanvas::GetData( ) const
    {
        return m_data;
    }

    InteropArray<Byte> ThorVGCanvas::GetDataAsBytes( ) const
    {
        InteropArray<Byte> pixelData( m_width * m_height * 4 );
        for ( uint32_t i = 0; i < m_data.NumElements( ); ++i )
        {
            const uint32_t pixel = m_data.GetElement( i );
            const uint8_t  a     = pixel >> 24 & 0xFF;
            const uint8_t  r     = pixel >> 16 & 0xFF;
            const uint8_t  g     = pixel >> 8 & 0xFF;
            const uint8_t  b     = pixel & 0xFF;

            pixelData.AddElement( r );
            pixelData.AddElement( g );
            pixelData.AddElement( b );
            pixelData.AddElement( a );
        }
        return pixelData;
    }
} // namespace DenOfIz
