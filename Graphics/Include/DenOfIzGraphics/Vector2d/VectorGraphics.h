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

#include <DenOfIzGraphics/Assets/Font/TextRenderer.h>
#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include "VGPipeline.h"
#include "VGShapes.h"
#include "VGTransform.h"

#ifdef DrawText
#undef DrawText
#endif

namespace DenOfIz
{
    struct DZ_API VectorGraphicsDesc
    {
        ILogicalDevice *LogicalDevice                = nullptr;
        uint32_t        InitialVertexBufferSize      = 64 * 1024;
        uint32_t        InitialIndexBufferSize       = 32 * 1024;
        float           DefaultTessellationTolerance = 0.25f;
        TextRenderer   *TextRenderer                 = nullptr;
    };

    enum class VGGradientType
    {
        Linear,
        Radial,
        Conic
    };

    struct DZ_API VGGradientStop
    {
        Float_4 Color;    // RGBA
        float   Position; // 0.0 to 1.0
    };
    template class DZ_API InteropArray<VGGradientStop>;

    enum class VGPatternType
    {
        None,
        Texture,
        Procedural
    };

    enum class VGBlendMode
    {
        Normal,
        Multiply,
        Screen,
        Overlay,
        SoftLight,
        HardLight,
        ColorDodge,
        ColorBurn,
        Darken,
        Lighten,
        Difference,
        Exclusion
    };

    enum class VGFillType
    {
        None,
        Color,
        LinearGradient,
        RadialGradient,
        ConicGradient,
        Pattern
    };

    enum class VGAntialiasingMode
    {
        None,
        Geometric,
    };

    struct DZ_API VGStrokeStyle
    {
        Float_4             Color      = { 0.0f, 0.0f, 0.0f, 1.0f };
        float               Width      = 1.0f;
        VGLineCap           Cap        = VGLineCap::Butt;
        VGLineJoin          Join       = VGLineJoin::Miter;
        float               MiterLimit = 10.0f;
        InteropArray<float> DashPattern;
        float               DashOffset = 0.0f;
        bool                Enabled    = false;
    };

    struct DZ_API VGFillStyle
    {
        VGFillType                   Type           = VGFillType::Color;
        Float_4                      Color          = { 0.0f, 0.0f, 0.0f, 1.0f };
        VGGradientType               GradientType   = VGGradientType::Linear;
        Float_2                      GradientStart  = { 0.0f, 0.0f };
        Float_2                      GradientEnd    = { 1.0f, 1.0f };
        Float_2                      GradientCenter = { 0.5f, 0.5f };
        float                        GradientRadius = 1.0f;
        float                        GradientAngle  = 0.0f;
        InteropArray<VGGradientStop> GradientStops;
        ITextureResource            *PatternTexture = nullptr;
        Float_4x4                    PatternTransform;
        VGFillRule                   FillRule = VGFillRule::NonZero;
        bool                         Enabled  = true;
    };

    // Composite operation
    struct DZ_API VGCompositeStyle
    {
        VGBlendMode BlendMode = VGBlendMode::Normal;
        float       Alpha     = 1.0f;
    };

    // Complete styling state
    struct DZ_API VGStyle
    {
        VGFillStyle      Fill;
        VGStrokeStyle    Stroke;
        VGCompositeStyle Composite;
        // Transform is now managed by VGTransform
    };

    // Vertex data for rendering
    struct DZ_API VGVertex
    {
        Float_2 Position;
        Float_4 Color;
        Float_2 TexCoord;
        Float_4 GradientData; // For gradient calculations
    };

    // Rendering primitive types
    enum class VGPrimitiveType
    {
        Fill,
        Stroke,
        Gradient,
        Pattern,
        Text
    };

    // Render command for batching
    struct DZ_API VGRenderCommand
    {
        VGPrimitiveType   Type;
        VGStyle           Style;
        uint32_t          VertexOffset;
        uint32_t          VertexCount;
        uint32_t          IndexOffset;
        uint32_t          IndexCount;
        VGPath2D         *Path; // Todo who owns this?
        ITextureResource *Texture = nullptr;
    };

    class VectorGraphics
    {
        ICommandList *m_commandList = nullptr;
        VGPipeline   *m_pipeline    = nullptr;
        VGTransform  *m_transform   = nullptr;

        VGStyle m_currentStyle;

        std::vector<VGVertex>        m_vertices;
        std::vector<uint32_t>        m_indices;
        std::vector<VGRenderCommand> m_renderCommands;

        std::unique_ptr<IBufferResource> m_vertexBuffer;
        std::unique_ptr<IBufferResource> m_indexBuffer;
        uint32_t                         m_vertexBufferSize = 0;
        uint32_t                         m_indexBufferSize  = 0;
        ILogicalDevice                  *m_logicalDevice    = nullptr;

        float    m_tessellationTolerance = 0.5f;
        uint32_t m_frameIndex            = 0;

        VGAntialiasingMode m_antialiasingMode  = VGAntialiasingMode::None;
        float              m_antialiasingWidth = 1.0f;

        // Text rendering
        TextRenderer *m_textRenderer = nullptr;

        // Clipping state
        std::vector<VGRect> m_clipStack;
        bool                m_clippingEnabled = false;

    public:
        DZ_API explicit VectorGraphics( const VectorGraphicsDesc &desc );
        DZ_API ~VectorGraphics( );

        DZ_API void BeginBatch( ICommandList *commandList, uint32_t frameIndex = 0 );
        DZ_API void EndBatch( );
        DZ_API void Flush( ); // Immediate render of current batch

        // Fill styling
        DZ_API void SetFillColor( const Float_4 &color );
        DZ_API void SetFillEnabled( bool enabled );
        DZ_API void SetFillRule( VGFillRule rule );

        DZ_API void SetFillLinearGradient( const Float_2 &start, const Float_2 &end, const InteropArray<VGGradientStop> &stops );
        DZ_API void SetFillRadialGradient( const Float_2 &center, float radius, const InteropArray<VGGradientStop> &stops );
        DZ_API void SetFillConicGradient( const Float_2 &center, float angle, const InteropArray<VGGradientStop> &stops );
        DZ_API void SetFillPattern( ITextureResource *texture, const Float_4x4 &transform );

        // Stroke styling
        DZ_API void SetStrokeColor( const Float_4 &color );
        DZ_API void SetStrokeWidth( float width );
        DZ_API void SetStrokeLineCap( VGLineCap cap );
        DZ_API void SetStrokeLineJoin( VGLineJoin join );
        DZ_API void SetStrokeMiterLimit( float limit );
        DZ_API void SetStrokeDashPattern( const InteropArray<float> &pattern, float offset = 0.0f );
        DZ_API void SetStrokeEnabled( bool enabled );

        // Composite styling
        DZ_API void SetBlendMode( VGBlendMode mode );
        DZ_API void SetAlpha( float alpha );

        // Get current style
        DZ_API const VGStyle &GetCurrentStyle( ) const;
        DZ_API void           SetStyle( const VGStyle &style );

        // === Transform API ===
        DZ_API void Save( ) const;                                     // Push transform state
        DZ_API void Restore( ) const;                                  // Pop transform state
        DZ_API void PushTransform( const Float_4x4 &transform ) const; // Push and apply transform
        DZ_API void PopTransform( ) const;                             // Pop transform state
        DZ_API void ResetTransform( ) const;
        DZ_API void Transform( const Float_4x4 &matrix ) const;
        DZ_API void Translate( const Float_2 &offset ) const;
        DZ_API void Scale( const Float_2 &scale ) const;
        DZ_API void Scale( float scale ) const;
        DZ_API void Rotate( float angleRadians ) const;
        DZ_API void Rotate( float angleRadians, const Float_2 &center ) const;
        DZ_API void Skew( const Float_2 &skew ) const;

        // Path drawing
        DZ_API void DrawPath( const VGPath2D &path );
        DZ_API void FillPath( const VGPath2D &path );
        DZ_API void StrokePath( const VGPath2D &path );

        // Basic shapes
        DZ_API void DrawRect( const VGRect &rect );
        DZ_API void FillRect( const VGRect &rect );
        DZ_API void StrokeRect( const VGRect &rect );

        DZ_API void DrawRoundedRect( const VGRoundedRect &rect );
        DZ_API void FillRoundedRect( const VGRoundedRect &rect );
        DZ_API void StrokeRoundedRect( const VGRoundedRect &rect );

        DZ_API void DrawCircle( const VGCircle &circle );
        DZ_API void FillCircle( const VGCircle &circle );
        DZ_API void StrokeCircle( const VGCircle &circle );

        DZ_API void DrawEllipse( const VGEllipse &ellipse );
        DZ_API void FillEllipse( const VGEllipse &ellipse );
        DZ_API void StrokeEllipse( const VGEllipse &ellipse );

        DZ_API void DrawLine( const VGLine &line );
        DZ_API void DrawLines( const InteropArray<Float_2> &points, bool connected = true );

        DZ_API void DrawPolygon( const VGPolygon &polygon );
        DZ_API void FillPolygon( const VGPolygon &polygon );
        DZ_API void StrokePolygon( const VGPolygon &polygon );

        // Convenience methods
        DZ_API void DrawRect( const Float_2 &topLeft, const Float_2 &bottomRight );
        DZ_API void FillRect( const Float_2 &topLeft, const Float_2 &bottomRight );
        DZ_API void StrokeRect( const Float_2 &topLeft, const Float_2 &bottomRight );

        DZ_API void DrawCircle( const Float_2 &center, float radius );
        DZ_API void FillCircle( const Float_2 &center, float radius );
        DZ_API void StrokeCircle( const Float_2 &center, float radius );

        DZ_API void DrawLine( const Float_2 &start, const Float_2 &end, float thickness = 1.0f );

        // === Clipping API ===
        DZ_API void   ClipRect( const VGRect &rect );
        DZ_API void   ClipPath( const VGPath2D &path );
        DZ_API void   ResetClip( );
        DZ_API bool   IsClippingEnabled( ) const;
        DZ_API VGRect GetCurrentClipRect( ) const;

        // Text rendering
        DZ_API void   DrawText( const InteropString &text, const Float_2 &position, float scale = 1.0f ) const;
        DZ_API VGRect MeasureText( const InteropString &text, float scale = 36.0f ) const;

        // === Configuration ===
        DZ_API void  SetTessellationTolerance( float tolerance );
        DZ_API float GetTessellationTolerance( ) const;

        // Antialiasing configuration
        DZ_API void               SetAntialiasingMode( VGAntialiasingMode mode );
        DZ_API VGAntialiasingMode GetAntialiasingMode( ) const;
        DZ_API void               SetAntialiasingWidth( float width );
        DZ_API float              GetAntialiasingWidth( ) const;

        DZ_API void         SetPipeline( VGPipeline *pipeline );
        DZ_API void         SetTransform( VGTransform *transform );
        DZ_API VGPipeline  *GetPipeline( ) const;
        DZ_API VGTransform *GetTransform( ) const;

    private:
        // Tessellation and geometry generation
        void TessellatePath( const VGPath2D &path, bool forStroke = false );
        void TessellateRect( const VGRect &rect, bool forStroke = false );
        void TessellateRoundedRect( const VGRoundedRect &rect, bool forStroke = false );
        void TessellateCircle( const VGCircle &circle, bool forStroke = false );
        void TessellateEllipse( const VGEllipse &ellipse, bool forStroke = false );
        void TessellatePolygon( const VGPolygon &polygon, bool forStroke = false );
        void TessellateLine( const VGLine &line );

        // Stroke tessellation
        void GenerateStroke( const std::vector<Float_2> &points, bool closed );
        void GenerateLineCap( const Float_2 &point, const Float_2 &direction, bool isStart );
        void GenerateLineJoin( const Float_2 &point, const Float_2 &dir1, const Float_2 &dir2 );

        // Buffer management
        void EnsureVertexBufferCapacity( uint32_t vertexCount );
        void EnsureIndexBufferCapacity( uint32_t indexCount );
        void UpdateBuffers( );

        // Command generation
        void AddRenderCommand( VGPrimitiveType type, uint32_t vertexCount, uint32_t indexCount );

        // Vertex generation
        void AddVertex( const Float_2 &position, const Float_4 &color, const Float_2 &texCoord = { 0, 0 } );
        void AddVertex( const Float_2 &position, const Float_4 &color, const Float_2 &texCoord, float edgeDistance );
        void AddTriangle( uint32_t v0, uint32_t v1, uint32_t v2 );
        void AddQuad( uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3 );

        // Transform utilities
        Float_2   TransformPoint( const Float_2 &point ) const;
        Float_4x4 GetCurrentTransform( ) const;

        // Color utilities
        Float_4 ApplyAlpha( const Float_4 &color ) const;

        // Gradient utilities
        void SetupGradientVertexData( VGVertex &vertex, const Float_2 &position ) const;

        // Advanced tessellation algorithms
        void TessellateQuadraticBezier( const Float_2 &p0, const Float_2 &p1, const Float_2 &p2, std::vector<Float_2> &points );
        void TessellateCubicBezier( const Float_2 &p0, const Float_2 &p1, const Float_2 &p2, const Float_2 &p3, std::vector<Float_2> &points );
        void TessellateClosedPath( const std::vector<Float_2> &points );
        void TriangulatePolygon( const std::vector<Float_2> &points, std::vector<uint32_t> &indices ) const;
        void TessellateEllipticalArc( const Float_2 &start, const Float_2 &radii, float xAxisRotation, bool largeArcFlag, bool sweepFlag, const Float_2 &end,
                                      std::vector<Float_2> &points ) const;
        void TessellateCircularArc( const Float_2 &center, float radius, float startAngle, float endAngle, bool clockwise, std::vector<Float_2> &points ) const;

        // Geometric utilities
        static float DistancePointToLine( const Float_2 &point, const Float_2 &lineStart, const Float_2 &lineEnd );
        static bool  IsPointInTriangle( const Float_2 &point, const Float_2 &a, const Float_2 &b, const Float_2 &c );
        static float Cross2D( const Float_2 &a, const Float_2 &b );

        // Clipping utilities
        bool   IsPointInClipRect( const Float_2 &point ) const;
        VGRect IntersectRects( const VGRect &a, const VGRect &b ) const;

        // Rounded rectangle utilities
        void GenerateRoundedRectPath( float x1, float y1, float x2, float y2, float tlRadius, float trRadius, float blRadius, float brRadius, std::vector<Float_2> &path ) const;
        void TessellateStrokeFromPaths( const std::vector<Float_2> &outerPath, const std::vector<Float_2> &innerPath );

        // Clear state
        void ClearBatch( );
    };
} // namespace DenOfIz
