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

#include <DenOfIzGraphics/Assets/Vector2d/ThorVGWrapper.h>
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include <DenOfIzGraphics/Data/Texture.h>
#include <DenOfIzGraphics/Renderer/Sync/ResourceTracking.h>
#include <DenOfIzGraphics/Utilities/Common.h>

#include <thorvg.h>

#include <cstring>
#include <memory>

using namespace DenOfIz;

namespace DenOfIz
{
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

    bool ThorVGLinearGradient::Linear( const float x1, const float y1, const float x2, const float y2 ) const
    {
        return m_gradient->linear( x1, y1, x2, y2 ) == tvg::Result::Success;
    }

    bool ThorVGLinearGradient::ColorStops( const InteropArray<ThorVGColorStop> &colorStops )
    {
        std::vector<tvg::Fill::ColorStop> stops;
        stops.reserve( colorStops.NumElements( ) );

        for ( uint32_t i = 0; i < colorStops.NumElements( ); ++i )
        {
            const auto &stop = colorStops.GetElement( i );
            stops.push_back( { stop.Offset, stop.R, stop.G, stop.B, stop.A } );
        }

        return m_gradient->colorStops( stops.data( ), static_cast<uint32_t>( stops.size( ) ) ) == tvg::Result::Success;
    }

    bool ThorVGLinearGradient::Spread( const ThorVGSpreadMethod spread )
    {
        return m_gradient->spread( ToTvgSpread( spread ) ) == tvg::Result::Success;
    }

    bool ThorVGLinearGradient::Transform( const ThorVGMatrix &m )
    {
        return m_gradient->transform( ToTvgMatrix( m ) ) == tvg::Result::Success;
    }

    tvg::Fill *ThorVGLinearGradient::GetInternalFill( ) const
    {
        return m_gradient.get( );
    }

    ThorVGRadialGradient::ThorVGRadialGradient( )
    {
        m_gradient = tvg::RadialGradient::gen( );
    }

    bool ThorVGRadialGradient::Radial( const float cx, const float cy, const float radius ) const
    {
        return m_gradient->radial( cx, cy, radius ) == tvg::Result::Success;
    }

    bool ThorVGRadialGradient::ColorStops( const InteropArray<ThorVGColorStop> &colorStops )
    {
        if ( colorStops.NumElements( ) == 0 )
        {
            return false;
        }

        std::vector<tvg::Fill::ColorStop> stops;
        stops.reserve( colorStops.NumElements( ) );

        for ( uint32_t i = 0; i < colorStops.NumElements( ); ++i )
        {
            const auto &stop = colorStops.GetElement( i );
            stops.push_back( { stop.Offset, stop.R, stop.G, stop.B, stop.A } );
        }

        return m_gradient->colorStops( stops.data( ), static_cast<uint32_t>( stops.size( ) ) ) == tvg::Result::Success;
    }

    bool ThorVGRadialGradient::Spread( const ThorVGSpreadMethod spread )
    {
        return m_gradient->spread( ToTvgSpread( spread ) ) == tvg::Result::Success;
    }

    bool ThorVGRadialGradient::Transform( const ThorVGMatrix &m )
    {
        return m_gradient->transform( ToTvgMatrix( m ) ) == tvg::Result::Success;
    }

    tvg::Fill *ThorVGRadialGradient::GetInternalFill( ) const
    {
        return m_gradient.get( );
    }

    ThorVGShape::ThorVGShape( )
    {
        m_shape = tvg::Shape::gen( );
    }

    bool ThorVGShape::Reset( ) const
    {
        return m_shape->reset( ) == tvg::Result::Success;
    }

    bool ThorVGShape::MoveTo( const float x, const float y ) const
    {
        return m_shape->moveTo( x, y ) == tvg::Result::Success;
    }

    bool ThorVGShape::LineTo( const float x, const float y ) const
    {
        return m_shape->lineTo( x, y ) == tvg::Result::Success;
    }

    bool ThorVGShape::CubicTo( const float cx1, const float cy1, const float cx2, const float cy2, const float x, const float y ) const
    {
        return m_shape->cubicTo( cx1, cy1, cx2, cy2, x, y ) == tvg::Result::Success;
    }

    bool ThorVGShape::Close( ) const
    {
        return m_shape->close( ) == tvg::Result::Success;
    }

    bool ThorVGShape::AppendRect( const float x, const float y, const float w, const float h, const float rx, const float ry ) const
    {
        return m_shape->appendRect( x, y, w, h, rx, ry ) == tvg::Result::Success;
    }

    bool ThorVGShape::AppendCircle( const float cx, const float cy, const float rx, const float ry ) const
    {
        return m_shape->appendCircle( cx, cy, rx, ry ) == tvg::Result::Success;
    }

    bool ThorVGShape::AppendPath( const InteropArray<Float_2> &points ) const
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

        return m_shape->appendPath( commands.data( ), static_cast<uint32_t>( commands.size( ) ), pts.data( ), static_cast<uint32_t>( pts.size( ) ) ) == tvg::Result::Success;
    }

    bool ThorVGShape::Fill( const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a ) const
    {
        return m_shape->fill( r, g, b, a ) == tvg::Result::Success;
    }

    bool ThorVGShape::Fill( const ThorVGGradient *gradient ) const
    {
        if ( !gradient )
        {
            return false;
        }
        const auto fill = gradient->GetInternalFill( );
        if ( !fill )
        {
            return false;
        }

        const auto cloned = fill->duplicate( );
        return m_shape->fill( std::unique_ptr<tvg::Fill>( cloned ) ) == tvg::Result::Success;
    }

    bool ThorVGShape::Stroke( const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a ) const
    {
        return m_shape->stroke( r, g, b, a ) == tvg::Result::Success;
    }

    bool ThorVGShape::Stroke( const float width ) const
    {
        return m_shape->stroke( width ) == tvg::Result::Success;
    }

    bool ThorVGShape::Stroke( const ThorVGGradient *gradient ) const
    {
        if ( !gradient )
        {
            return false;
        }
        const auto fill = gradient->GetInternalFill( );
        if ( !fill )
        {
            return false;
        }

        const auto cloned = fill->duplicate( );
        return m_shape->stroke( std::unique_ptr<tvg::Fill>( cloned ) ) == tvg::Result::Success;
    }

    bool ThorVGShape::StrokeCap( const ThorVGStrokeCap cap ) const
    {
        return m_shape->stroke( ToTvgStrokeCap( cap ) ) == tvg::Result::Success;
    }

    bool ThorVGShape::StrokeJoin( const ThorVGStrokeJoin join ) const
    {
        return m_shape->stroke( ToTvgStrokeJoin( join ) ) == tvg::Result::Success;
    }

    bool ThorVGShape::StrokeMiterlimit( const float miterlimit ) const
    {
        return m_shape->stroke( miterlimit ) == tvg::Result::Success;
    }

    bool ThorVGShape::StrokeDash( const InteropArray<float> &pattern, float offset ) const
    {
        if ( pattern.NumElements( ) == 0 )
        {
            return m_shape->stroke( nullptr, 0 ) == tvg::Result::Success;
        }

        std::vector<float> dashPattern;
        dashPattern.reserve( pattern.NumElements( ) );

        for ( uint32_t i = 0; i < pattern.NumElements( ); ++i )
        {
            dashPattern.push_back( pattern.GetElement( i ) );
        }
        return m_shape->stroke( dashPattern.data( ), static_cast<uint32_t>( dashPattern.size( ) ) ) == tvg::Result::Success;
    }

    bool ThorVGShape::Transform( const ThorVGMatrix &m )
    {
        return m_shape->transform( ToTvgMatrix( m ) ) == tvg::Result::Success;
    }

    bool ThorVGShape::Translate( const float x, const float y )
    {
        return m_shape->translate( x, y ) == tvg::Result::Success;
    }

    bool ThorVGShape::Scale( const float factor )
    {
        return m_shape->scale( factor ) == tvg::Result::Success;
    }

    bool ThorVGShape::Rotate( const float degree )
    {
        return m_shape->rotate( degree ) == tvg::Result::Success;
    }

    bool ThorVGShape::Opacity( const uint8_t opacity )
    {
        return m_shape->opacity( opacity ) == tvg::Result::Success;
    }

    bool ThorVGShape::Composite( ThorVGPaint *target, const ThorVGCompositeMethod method )
    {
        if ( !target )
        {
            return false;
        }
        const auto paint = target->GetInternalPaint( );
        if ( !paint )
        {
            return false;
        }

        const auto cloned = paint->duplicate( );
        return m_shape->composite( std::unique_ptr<tvg::Paint>( cloned ), ToTvgCompositeMethod( method ) ) == tvg::Result::Success;
    }

    bool ThorVGShape::Blend( const ThorVGBlendMethod method )
    {
        return m_shape->blend( ToTvgBlendMethod( method ) ) == tvg::Result::Success;
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
        newShape->m_shape = std::unique_ptr<tvg::Shape>( dynamic_cast<tvg::Shape *>( m_shape->duplicate( ) ) );
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

    bool ThorVGPicture::Load( const char *path ) const
    {
        return m_picture->load( path ) == tvg::Result::Success;
    }

    bool ThorVGPicture::Load( const char *data, const uint32_t size, const char *mimeType, const bool copy ) const
    {
        return m_picture->load( data, size, mimeType, copy ) == tvg::Result::Success;
    }

    bool ThorVGPicture::Load( uint32_t *data, const uint32_t w, const uint32_t h, const bool premultiplied ) const
    {
        return m_picture->load( data, w, h, premultiplied ) == tvg::Result::Success;
    }

    bool ThorVGPicture::Size( const float w, const float h ) const
    {
        return m_picture->size( w, h ) == tvg::Result::Success;
    }

    ThorVGSize ThorVGPicture::GetSize( ) const
    {
        ThorVGSize size    = { };
        const auto picture = static_cast<const tvg::Picture *>( m_picture.get( ) );
        picture->size( &size.Width, &size.Height );
        return size;
    }

    bool ThorVGPicture::Transform( const ThorVGMatrix &m )
    {
        return m_picture->transform( ToTvgMatrix( m ) ) == tvg::Result::Success;
    }

    bool ThorVGPicture::Translate( const float x, const float y )
    {
        return m_picture->translate( x, y ) == tvg::Result::Success;
    }

    bool ThorVGPicture::Scale( const float factor )
    {
        return m_picture->scale( factor ) == tvg::Result::Success;
    }

    bool ThorVGPicture::Rotate( const float degree )
    {
        return m_picture->rotate( degree ) == tvg::Result::Success;
    }

    bool ThorVGPicture::Opacity( const uint8_t opacity )
    {
        return m_picture->opacity( opacity ) == tvg::Result::Success;
    }

    bool ThorVGPicture::Composite( ThorVGPaint *target, const ThorVGCompositeMethod method )
    {
        if ( !target )
        {
            return false;
        }
        const auto paint = target->GetInternalPaint( );
        if ( !paint )
        {
            return false;
        }

        const auto cloned = paint->duplicate( );
        return m_picture->composite( std::unique_ptr<tvg::Paint>( cloned ), ToTvgCompositeMethod( method ) ) == tvg::Result::Success;
    }

    bool ThorVGPicture::Blend( const ThorVGBlendMethod method )
    {
        return m_picture->blend( ToTvgBlendMethod( method ) ) == tvg::Result::Success;
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
        newPicture->m_picture = std::unique_ptr<tvg::Picture>( dynamic_cast<tvg::Picture *>( m_picture->duplicate( ) ) );
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

    bool ThorVGScene::Push( ThorVGPaint *paint ) const
    {
        if ( !paint )
        {
            return false;
        }
        const auto p = paint->GetInternalPaint( );
        if ( !p )
        {
            return false;
        }

        const auto cloned = p->duplicate( );
        return m_scene->push( std::unique_ptr<tvg::Paint>( cloned ) ) == tvg::Result::Success;
    }

    bool ThorVGScene::Clear( const bool free ) const
    {
        return m_scene->clear( free ) == tvg::Result::Success;
    }

    bool ThorVGScene::Transform( const ThorVGMatrix &m )
    {
        return m_scene->transform( ToTvgMatrix( m ) ) == tvg::Result::Success;
    }

    bool ThorVGScene::Translate( const float x, const float y )
    {
        return m_scene->translate( x, y ) == tvg::Result::Success;
    }

    bool ThorVGScene::Scale( const float factor )
    {
        return m_scene->scale( factor ) == tvg::Result::Success;
    }

    bool ThorVGScene::Rotate( const float degree )
    {
        return m_scene->rotate( degree ) == tvg::Result::Success;
    }

    bool ThorVGScene::Opacity( const uint8_t opacity )
    {
        return m_scene->opacity( opacity ) == tvg::Result::Success;
    }

    bool ThorVGScene::Composite( ThorVGPaint *target, const ThorVGCompositeMethod method )
    {
        if ( !target )
        {
            return false;
        }
        const auto paint = target->GetInternalPaint( );
        if ( !paint )
        {
            return false;
        }

        const auto cloned = paint->duplicate( );
        return m_scene->composite( std::unique_ptr<tvg::Paint>( cloned ), ToTvgCompositeMethod( method ) ) == tvg::Result::Success;
    }

    bool ThorVGScene::Blend( const ThorVGBlendMethod method )
    {
        return m_scene->blend( ToTvgBlendMethod( method ) ) == tvg::Result::Success;
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
        newScene->m_scene = std::unique_ptr<tvg::Scene>( dynamic_cast<tvg::Scene *>( m_scene->duplicate( ) ) );
        return newScene;
    }

    tvg::Paint *ThorVGScene::GetInternalPaint( )
    {
        return m_scene.get( );
    }

    ThorVGCanvas::ThorVGCanvas( const ThorVGCanvasDesc &desc )
    {
        m_canvas = tvg::SwCanvas::gen( );
        Resize( desc.Width, desc.Height );
    }

    bool ThorVGCanvas::Push( ThorVGPaint *paint ) const
    {
        if ( !paint || !m_canvas )
        {
            return false;
        }
        const auto p = paint->GetInternalPaint( );
        if ( !p )
        {
            return false;
        }
        const auto cloned = p->duplicate( );
        return m_canvas->push( std::unique_ptr<tvg::Paint>( cloned ) ) == tvg::Result::Success;
    }

    bool ThorVGCanvas::Clear( const bool free )
    {
        ResetData( );
        if ( !m_canvas )
        {
            return false;
        }
        return m_canvas->clear( free ) == tvg::Result::Success;
    }

    bool ThorVGCanvas::Update( ThorVGPaint *paint ) const
    {
        if ( !m_canvas )
        {
            return false;
        }
        if ( paint )
        {
            const auto p = paint->GetInternalPaint( );
            if ( !p )
            {
                return false;
            }
            return m_canvas->update( p ) == tvg::Result::Success;
        }
        return m_canvas->update( ) == tvg::Result::Success;
    }

    bool ThorVGCanvas::Draw( ) const
    {
        if ( !m_canvas )
        {
            return false;
        }
        return m_canvas->draw( ) == tvg::Result::Success;
    }

    bool ThorVGCanvas::Sync( ) const
    {
        if ( !m_canvas )
        {
            return false;
        }
        return m_canvas->sync( ) == tvg::Result::Success;
    }

    bool ThorVGCanvas::Viewport( const int32_t x, const int32_t y, const int32_t w, const int32_t h ) const
    {
        if ( !m_canvas )
        {
            return false;
        }
        return m_canvas->viewport( x, y, w, h ) == tvg::Result::Success;
    }

    bool ThorVGCanvas::Resize( const uint32_t w, const uint32_t h )
    {
        m_data.Resize( w * h * 4 );
        return m_canvas->target( m_data.Data( ), w, w, h, tvg::SwCanvas::ARGB8888 ) == tvg::Result::Success;
    }

    bool ThorVGCanvas::Mempool( ) const
    {
        return m_canvas->mempool( tvg::SwCanvas::MempoolPolicy::Individual ) == tvg::Result::Success;
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

    class ThorVGRenderer::Impl
    {
    public:
        ILogicalDevice                                *m_device = nullptr;
        std::unique_ptr<ThorVGCanvas>                  m_canvas;
        std::vector<std::unique_ptr<ITextureResource>> m_renderTargets;
        std::unique_ptr<IBufferResource>               m_stagingBuffer;
        ResourceTracking                               m_resourceTracking;
        void                                          *m_mappedStagingBuffer = nullptr;
        uint32_t                                       m_width               = 0;
        uint32_t                                       m_height              = 0;
        uint32_t                                       m_numFrames           = 3;

        explicit Impl( const ThorVGRendererDesc &desc ) : m_device( desc.LogicalDevice ), m_width( desc.Width ), m_height( desc.Height )
        {
            if ( desc.NumFrames > 0 )
            {
                m_numFrames = desc.NumFrames;
            }

            ThorVGCanvasDesc canvasDesc{ };
            canvasDesc.Width  = m_width;
            canvasDesc.Height = m_height;
            m_canvas          = std::make_unique<ThorVGCanvas>( canvasDesc );

            CreateRenderTargets( );
            CreateStagingBuffer( );
        }

        ~Impl( )
        {
            if ( m_mappedStagingBuffer && m_stagingBuffer )
            {
                m_stagingBuffer->UnmapMemory( );
                m_mappedStagingBuffer = nullptr;
            }
        }

        void CreateRenderTargets( )
        {
            m_renderTargets.clear( );
            m_renderTargets.reserve( m_numFrames );

            for ( uint32_t i = 0; i < m_numFrames; ++i )
            {
                TextureDesc desc{ };
                desc.Width      = m_width;
                desc.Height     = m_height;
                desc.Format     = Format::R8G8B8A8Unorm;
                desc.Descriptor = ResourceDescriptor::Texture;
                desc.Usages.Set( ResourceUsage::CopyDst );
                desc.Usages.Set( ResourceUsage::ShaderResource );
                desc.InitialUsage = ResourceUsage::Common;
                desc.DebugName    = InteropString( "ThorVG_RenderTarget_" ).Append( std::to_string( i ).c_str( ) );

                m_renderTargets.emplace_back( m_device->CreateTextureResource( desc ) );
                m_resourceTracking.TrackTexture( m_renderTargets[ i ].get( ), ResourceUsage::Common );
            }
        }

        void CreateStagingBuffer( )
        {
            if ( m_mappedStagingBuffer && m_stagingBuffer )
            {
                m_stagingBuffer->UnmapMemory( );
                m_mappedStagingBuffer = nullptr;
            }

            BufferDesc stagingDesc{ };
            stagingDesc.HeapType     = HeapType::CPU_GPU;
            stagingDesc.NumBytes     = m_canvas->GetData( ).NumElements( ) * sizeof( uint32_t );
            stagingDesc.InitialUsage = ResourceUsage::CopySrc;
            stagingDesc.Usages.Set( ResourceUsage::CopySrc );
            stagingDesc.DebugName = "ThorVG_StagingBuffer";

            m_stagingBuffer.reset( m_device->CreateBufferResource( stagingDesc ) );
            if ( m_stagingBuffer )
            {
                m_mappedStagingBuffer = m_stagingBuffer->MapMemory( );
            }
        }

        bool UpdateTexture( ICommandList *commandList, const uint32_t frameIndex )
        {
            if ( frameIndex >= m_renderTargets.size( ) || !m_renderTargets[ frameIndex ] )
            {
                return false;
            }

            const auto renderTarget = m_renderTargets[ frameIndex ].get( );

            BatchTransitionDesc batchTransitionDesc{ commandList };
            batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::CopyDst );
            m_resourceTracking.BatchTransition( batchTransitionDesc );
            if ( !m_stagingBuffer || !m_mappedStagingBuffer )
            {
                return false;
            }

            auto &data = m_canvas->GetData( );
            std::memcpy( m_mappedStagingBuffer, data.Data( ), data.NumElements( ) * sizeof( uint32_t ) );

            CopyBufferToTextureDesc copyDesc{ };
            copyDesc.DstTexture = renderTarget;
            copyDesc.SrcBuffer  = m_stagingBuffer.get( );
            copyDesc.SrcOffset  = 0;
            copyDesc.DstX       = 0;
            copyDesc.DstY       = 0;
            copyDesc.DstZ       = 0;
            copyDesc.Format     = Format::R8G8B8A8Unorm;
            copyDesc.MipLevel   = 0;
            copyDesc.ArrayLayer = 0;
            copyDesc.RowPitch   = m_width * sizeof( uint32_t );
            copyDesc.NumRows    = m_height;

            commandList->CopyBufferToTexture( copyDesc );

            batchTransitionDesc.Reset( commandList );
            batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::ShaderResource );
            m_resourceTracking.BatchTransition( batchTransitionDesc );
            return true;
        }
    };

    ThorVGRenderer::ThorVGRenderer( const ThorVGRendererDesc &desc ) : m_impl( std::make_unique<Impl>( desc ) )
    {
    }

    ThorVGRenderer::~ThorVGRenderer( ) = default;

    ThorVGCanvas *ThorVGRenderer::GetCanvas( ) const
    {
        return m_impl->m_canvas.get( );
    }

    bool ThorVGRenderer::BeginFrame( ) const
    {
        return m_impl->m_canvas->Clear( );
    }

    bool ThorVGRenderer::EndFrame( ) const
    {
        if ( !m_impl->m_canvas->Draw( ) )
        {
            return false;
        }
        return m_impl->m_canvas->Sync( );
    }

    bool ThorVGRenderer::RenderToTexture( ITextureResource *target, ICommandList *commandList ) const
    {
        if ( !target || !commandList )
        {
            return false;
        }
        if ( !m_impl->m_canvas->Sync( ) )
        {
            return false;
        }

        PipelineBarrierDesc barrier{ };
        barrier.TextureBarrier( TextureBarrierDesc{ target, ResourceUsage::Undefined, ResourceUsage::CopyDst } );
        commandList->PipelineBarrier( barrier );

        if ( !m_impl->m_stagingBuffer || !m_impl->m_mappedStagingBuffer )
        {
            return false;
        }

        auto &data = m_impl->m_canvas->GetData( );
        std::memcpy( m_impl->m_mappedStagingBuffer, data.Data( ), data.NumElements( ) * sizeof( uint32_t ) );

        CopyBufferToTextureDesc copyDesc{ };
        copyDesc.DstTexture = target;
        copyDesc.SrcBuffer  = m_impl->m_stagingBuffer.get( );
        copyDesc.SrcOffset  = 0;
        copyDesc.DstX       = 0;
        copyDesc.DstY       = 0;
        copyDesc.DstZ       = 0;
        copyDesc.Format     = Format::R8G8B8A8Unorm;
        copyDesc.MipLevel   = 0;
        copyDesc.ArrayLayer = 0;
        copyDesc.RowPitch   = m_impl->m_width * sizeof( uint32_t );
        copyDesc.NumRows    = m_impl->m_height;

        commandList->CopyBufferToTexture( copyDesc );

        barrier.Clear( );
        barrier.TextureBarrier( TextureBarrierDesc{ target, ResourceUsage::CopyDst, ResourceUsage::ShaderResource } );
        commandList->PipelineBarrier( barrier );

        return true;
    }

    bool ThorVGRenderer::UpdateRenderTarget( ICommandList *commandList, const uint32_t frameIndex ) const
    {
        if ( !commandList )
        {
            return false;
        }

        return m_impl->UpdateTexture( commandList, frameIndex );
    }

    ITextureResource *ThorVGRenderer::GetRenderTarget( const uint32_t frameIndex ) const
    {
        if ( frameIndex >= m_impl->m_renderTargets.size( ) )
        {
            return nullptr;
        }

        return m_impl->m_renderTargets[ frameIndex ].get( );
    }

    bool ThorVGRenderer::Resize( const uint32_t width, const uint32_t height ) const
    {
        if ( width == m_impl->m_width && height == m_impl->m_height )
        {
            return true;
        }

        m_impl->m_width  = width;
        m_impl->m_height = height;

        if ( !m_impl->m_canvas->Resize( width, height ) )
        {
            return false;
        }

        m_impl->CreateRenderTargets( );
        m_impl->CreateStagingBuffer( );
        return true;
    }
} // namespace DenOfIz
