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

#include <DenOfIzGraphics/Utilities/InteropMath.h>
#include <DenOfIzGraphics/Vector2d/VGShapes.h>
#include <DenOfIzGraphics/Vector2d/VectorGraphics.h>
#include <tinyxml2.h>

namespace DenOfIz
{
    // SVG-specific structures that extend our current capabilities
    struct DZ_API SvgViewBox
    {
        float X      = 0.0f;
        float Y      = 0.0f;
        float Width  = 0.0f;
        float Height = 0.0f;
    };

    struct DZ_API SvgTransform
    {
        Float_4x4 Matrix;
        bool      HasTransform = false;
    };

    struct DZ_API SvgStyle
    {
        // Fill properties
        bool          HasFill     = false;
        Float_4       FillColor   = { 0.0f, 0.0f, 0.0f, 1.0f };
        float         FillOpacity = 1.0f;
        InteropString FillRule    = "nonzero";

        // Stroke properties
        bool          HasStroke        = false;
        Float_4       StrokeColor      = { 0.0f, 0.0f, 0.0f, 1.0f };
        float         StrokeWidth      = 1.0f;
        float         StrokeOpacity    = 1.0f;
        InteropString StrokeLineCap    = "butt";
        InteropString StrokeLineJoin   = "miter";
        float         StrokeMiterLimit = 4.0f;
        InteropString StrokeDashArray;
        float         StrokeDashOffset = 0.0f;

        // General properties
        float         Opacity = 1.0f;
        InteropString Display;
        InteropString Visibility = "visible";
    };

    struct DZ_API SvgGradientStop
    {
        float   Offset  = 0.0f;
        Float_4 Color   = { 0.0f, 0.0f, 0.0f, 1.0f };
        float   Opacity = 1.0f;
    };
    template class DZ_API InteropArray<SvgGradientStop>;

    struct DZ_API SvgLinearGradient
    {
        InteropString                 Id;
        Float_2                       Start         = { 0.0f, 0.0f };
        Float_2                       End           = { 1.0f, 0.0f };
        InteropString                 GradientUnits = "objectBoundingBox";
        SvgTransform                  GradientTransform;
        InteropArray<SvgGradientStop> Stops;
    };

    struct DZ_API SvgRadialGradient
    {
        InteropString                 Id;
        Float_2                       Center        = { 0.5f, 0.5f };
        Float_2                       FocalPoint    = { 0.5f, 0.5f };
        float                         Radius        = 0.5f;
        InteropString                 GradientUnits = "objectBoundingBox";
        SvgTransform                  GradientTransform;
        InteropArray<SvgGradientStop> Stops;
    };

    // SVG Document structure
    struct DZ_API SvgDocument
    {
        Float_2                         Size = { 100.0f, 100.0f };
        SvgViewBox                      ViewBox;
        bool                            HasViewBox = false;
        InteropArray<SvgLinearGradient> LinearGradients;
        InteropArray<SvgRadialGradient> RadialGradients;
    };

    // SVG Loading result
    enum class SvgLoadResult
    {
        Success,
        FileNotFound,
        InvalidXml,
        UnsupportedFeature,
        InvalidFormat
    };

    // SVG Loading options
    struct DZ_API SvgLoadOptions
    {
        // Removed all scaling options - use VectorGraphics transform API instead
        bool          LoadGradients         = true;
        bool          LoadText              = true;
        bool          ConvertTextToPaths    = false;
        float         TessellationTolerance = 1.0f;
        InteropString DefaultFontFamily     = "Arial";
        float         DefaultFontSize       = 12.0f;
    };

    class SvgLoader
    {
    public:
        DZ_API SvgLoader( );
        DZ_API ~SvgLoader( );

        SvgLoader( const SvgLoader & )            = delete;
        SvgLoader &operator=( const SvgLoader & ) = delete;
        SvgLoader( SvgLoader &&other ) noexcept;
        SvgLoader &operator=( SvgLoader &&other ) noexcept;

        DZ_API SvgLoadResult LoadFromFile( const InteropString &filePath, const SvgLoadOptions &options = { } );
        DZ_API SvgLoadResult LoadFromBinaryData( const InteropArray<Byte> &data, const SvgLoadOptions &options = { } );
        DZ_API SvgLoadResult LoadFromString( const InteropString &svgContent, const SvgLoadOptions &options = { } );

        DZ_API void RenderToVectorGraphics( VectorGraphics *vectorGraphics ) const;
        DZ_API void RenderElementById( VectorGraphics *vectorGraphics, const InteropString &elementId ) const;

        DZ_API const SvgDocument &GetDocument( ) const;
        DZ_API SvgViewBox         GetEffectiveViewBox( ) const;
        DZ_API Float_2            GetDocumentSize( ) const;
        // Removed CalculateTargetScale - use VectorGraphics transform API instead

        DZ_API InteropString GetLastError( ) const;
        DZ_API bool          HasErrors( ) const;

        DZ_API static Float_4      ParseColor( const InteropString &colorString );
        DZ_API static float        ParseLength( const InteropString &lengthString, float referenceValue = 100.0f );
        DZ_API static SvgTransform ParseTransform( const InteropString &transformString );
        DZ_API void                Clear( ) const;

    private:
        struct SvgRenderCommand
        {
            enum Type
            {
                RectCommand,
                CircleCommand,
                EllipseCommand,
                LineCommand,
                PolygonCommand,
                PathCommand,
                TextCommand
            };

            Type         Type;
            SvgStyle     Style;
            SvgTransform Transform;
            union
            {
                struct
                {
                    VGRect  Rect;
                    Float_4 CornerRadii; // rx, ry, rx, ry for SVG rect
                } RectData;

                struct
                {
                    VGCircle Circle;
                } CircleData;

                struct
                {
                    VGEllipse Ellipse;
                } EllipseData;

                struct
                {
                    VGLine Line;
                } LineData;

                struct
                {
                    VGPolygon Polygon;
                } PolygonData;

                struct
                {
                    VGPath2D *Path; // Owned by Impl
                } PathData;

                struct
                {
                    InteropString Text;
                    Float_2       Position;
                    float         FontSize;
                    InteropString FontFamily;
                } TextData;
            };

            SvgRenderCommand( );
            ~SvgRenderCommand( );

            SvgRenderCommand( const SvgRenderCommand &other );
            SvgRenderCommand( SvgRenderCommand &&other ) noexcept;
            SvgRenderCommand &operator=( const SvgRenderCommand &other );
            SvgRenderCommand &operator=( SvgRenderCommand &&other ) noexcept;

        private:
            void CopyUnionData( const SvgRenderCommand &other );
            void MoveUnionData( SvgRenderCommand &&other );
            void CleanupUnionData( );
        };

        class Impl;
        Impl *m_impl;

        bool     ParseDocument( tinyxml2::XMLElement *svgElement );
        bool     ParseElement( tinyxml2::XMLElement *element, const SvgStyle &parentStyle, const SvgTransform &parentTransform );
        void     ParseRect( const tinyxml2::XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const;
        void     ParseCircle( const tinyxml2::XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const;
        void     ParseEllipse( const tinyxml2::XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const;
        void     ParseLine( const tinyxml2::XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const;
        void     ParsePolyline( const tinyxml2::XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const;
        void     ParsePolygon( const tinyxml2::XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const;
        void     ParsePath( const tinyxml2::XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const;
        void     ParseText( const tinyxml2::XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const;
        void     ParseGroup( tinyxml2::XMLElement *element, const SvgStyle &parentStyle, const SvgTransform &parentTransform );
        void     ParseLinearGradient( tinyxml2::XMLElement *element ) const;
        void     ParseRadialGradient( tinyxml2::XMLElement *element ) const;
        void     ParseGradientStop( const tinyxml2::XMLElement *element, InteropArray<SvgGradientStop> &stops ) const;
        SvgStyle ParseElementStyle( tinyxml2::XMLElement *element, const SvgStyle &parentStyle ) const;
        SvgStyle ParseStyleAttribute( const InteropString &styleString, const SvgStyle &baseStyle ) const;
        void     ApplyGradientFill( VectorGraphics *vectorGraphics, const InteropString &gradientId ) const;
        VGPath2D ParsePathData( const InteropString &pathData ) const;
        bool     ParsePathCommand( const InteropString &command, float *data, uint32_t &dataIndex, uint32_t maxData ) const;

        void         ProcessPathCommand( const VGPath2D &path, char command, const float *data, uint32_t numValues, Float_2 &currentPos, Float_2 &lastControlPoint ) const;
        SvgTransform CombineTransforms( const SvgTransform &parent, const SvgTransform &child ) const;
        void         ApplyTransform( const VectorGraphics *vectorGraphics, const SvgTransform &transform ) const;
        Float_2      ViewBoxToPixel( const Float_2 &viewBoxCoord ) const;
        float        ViewBoxToPixelScale( ) const;

        InteropString GetAttributeString( const tinyxml2::XMLElement *element, const char *name, const InteropString &defaultValue = { } ) const;
        float         GetAttributeFloat( const tinyxml2::XMLElement *element, const char *name, float defaultValue = 0.0f ) const;
        bool          GetAttributeBool( const tinyxml2::XMLElement *element, const char *name, bool defaultValue = false ) const;

        void SetError( const InteropString &error ) const;
        void RenderCommand( VectorGraphics *vectorGraphics, const SvgRenderCommand &cmd ) const;
    };
} // namespace DenOfIz
