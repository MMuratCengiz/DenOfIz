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

namespace DenOfIz
{
    struct DZ_API VGLine
    {
        Float_2 StartPoint;
        Float_2 EndPoint;
        float   Thickness = 1.0f;
    };

    struct DZ_API VGQuad
    {
        Float_2 TopLeft;
        Float_2 TopRight;
        Float_2 BottomRight;
        Float_2 BottomLeft;
    };

    struct DZ_API VGRect
    {
        Float_2 TopLeft;
        Float_2 BottomRight;

        [[nodiscard]] float GetWidth( ) const
        {
            return BottomRight.X - TopLeft.X;
        }
        [[nodiscard]] float GetHeight( ) const
        {
            return BottomRight.Y - TopLeft.Y;
        }
        [[nodiscard]] Float_2 GetCenter( ) const
        {
            return { ( TopLeft.X + BottomRight.X ) * 0.5f, ( TopLeft.Y + BottomRight.Y ) * 0.5f };
        }
    };

    struct DZ_API VGRoundedRect
    {
        Float_2 TopLeft;
        Float_2 BottomRight;
        Float_4 CornerRadii; // TopLeft, TopRight, BottomRight, BottomLeft
    };

    struct DZ_API VGCircle
    {
        Float_2 Center;
        float   Radius;
    };

    struct DZ_API VGEllipse
    {
        Float_2 Center;
        Float_2 Radii;           // X = width radius, Y = height radius
        float   Rotation = 0.0f; // Rotation in radians
    };

    struct DZ_API VGPolygon
    {
        InteropArray<Float_2> Points;
        bool                  IsClosed = true;
    };

    // Path command types
    enum class VGPathCommandType
    {
        MoveTo,
        LineTo,
        HorizontalLineTo,
        VerticalLineTo,
        QuadraticCurveTo,
        SmoothQuadraticCurveTo,
        CubicCurveTo,
        SmoothCubicCurveTo,
        EllipticalArc,
        CircularArc,
        Close
    };

    // Individual path command structures
    struct DZ_API VGMoveToCommand
    {
        Float_2 Point;
        bool    IsRelative = false;
    };

    struct DZ_API VGLineToCommand
    {
        Float_2 Point;
        bool    IsRelative = false;
    };

    struct DZ_API VGHorizontalLineToCommand
    {
        float X;
        bool  IsRelative = false;
    };

    struct DZ_API VGVerticalLineToCommand
    {
        float Y;
        bool  IsRelative = false;
    };

    struct DZ_API VGQuadraticCurveToCommand
    {
        Float_2 ControlPoint;
        Float_2 EndPoint;
        bool    IsRelative = false;
    };

    struct DZ_API VGSmoothQuadraticCurveToCommand
    {
        Float_2 EndPoint;
        bool    IsRelative = false;
    };

    struct DZ_API VGCubicCurveToCommand
    {
        Float_2 ControlPoint1;
        Float_2 ControlPoint2;
        Float_2 EndPoint;
        bool    IsRelative = false;
    };

    struct DZ_API VGSmoothCubicCurveToCommand
    {
        Float_2 ControlPoint2;
        Float_2 EndPoint;
        bool    IsRelative = false;
    };

    struct DZ_API VGEllipticalArcCommand
    {
        Float_2 Radii;
        float   XAxisRotation; // In radians
        bool    LargeArcFlag;
        bool    SweepFlag; // true for clockwise
        Float_2 EndPoint;
        bool    IsRelative = false;
    };

    struct DZ_API VGCircularArcCommand
    {
        Float_2 Center;
        float   Radius;
        float   StartAngle; // In radians
        float   EndAngle;   // In radians
        bool    Clockwise = true;
    };

    struct DZ_API VGCloseCommand{
        // No data needed for close command
    };

    // Union-like structure for path commands
    struct DZ_API VGPathCommand
    {
        VGPathCommandType Type;

        union
        {
            VGMoveToCommand                 MoveTo;
            VGLineToCommand                 LineTo;
            VGHorizontalLineToCommand       HorizontalLineTo;
            VGVerticalLineToCommand         VerticalLineTo;
            VGQuadraticCurveToCommand       QuadraticCurveTo;
            VGSmoothQuadraticCurveToCommand SmoothQuadraticCurveTo;
            VGCubicCurveToCommand           CubicCurveTo;
            VGSmoothCubicCurveToCommand     SmoothCubicCurveTo;
            VGEllipticalArcCommand          EllipticalArc;
            VGCircularArcCommand            CircularArc;
            VGCloseCommand                  Close;
        };

        VGPathCommand( ) : Type( VGPathCommandType::MoveTo ), MoveTo{ }
        {
        }
        ~VGPathCommand( ) = default;
        VGPathCommand( const VGPathCommand &other );
        VGPathCommand &operator=( const VGPathCommand &other );
    };
    template class DZ_API InteropArray<VGPathCommand>;

    // Bounds information
    struct DZ_API VGBounds
    {
        Float_2 Min;
        Float_2 Max;

        [[nodiscard]] float GetWidth( ) const
        {
            return Max.X - Min.X;
        }
        [[nodiscard]] float GetHeight( ) const
        {
            return Max.Y - Min.Y;
        }
        [[nodiscard]] Float_2 GetCenter( ) const
        {
            return { ( Min.X + Max.X ) * 0.5f, ( Min.Y + Max.Y ) * 0.5f };
        }
        [[nodiscard]] bool IsEmpty( ) const
        {
            return Min.X >= Max.X || Min.Y >= Max.Y;
        }
    };

    // Path fill rules
    enum class VGFillRule
    {
        NonZero,
        EvenOdd
    };

    // Path line cap styles
    enum class VGLineCap
    {
        Butt,
        Round,
        Square
    };

    // Path line join styles
    enum class VGLineJoin
    {
        Miter,
        Round,
        Bevel
    };

    // Main Path2D class
    class VGPath2D
    {
        class Impl;
        Impl *m_impl;

    public:
        DZ_API VGPath2D( );
        DZ_API ~VGPath2D( );

        // Copy/move semantics
        DZ_API VGPath2D( const VGPath2D &other );
        DZ_API VGPath2D &operator=( const VGPath2D &other );
        DZ_API VGPath2D( VGPath2D &&other ) noexcept;
        DZ_API VGPath2D &operator=( VGPath2D &&other ) noexcept;

        // Basic path operations (absolute coordinates)
        DZ_API void Clear( ) const;
        DZ_API void MoveTo( const Float_2 &point ) const;
        DZ_API void LineTo( const Float_2 &point ) const;
        DZ_API void HorizontalLineTo( float x ) const;
        DZ_API void VerticalLineTo( float y ) const;
        DZ_API void Close( ) const;

        // Relative path operations
        DZ_API void RelativeMoveTo( const Float_2 &offset ) const;
        DZ_API void RelativeLineTo( const Float_2 &offset ) const;
        DZ_API void RelativeHorizontalLineTo( float dx ) const;
        DZ_API void RelativeVerticalLineTo( float dy ) const;

        // Curve operations (absolute)
        DZ_API void QuadraticCurveTo( const Float_2 &controlPoint, const Float_2 &endPoint ) const;
        DZ_API void SmoothQuadraticCurveTo( const Float_2 &endPoint ) const;
        DZ_API void CubicCurveTo( const Float_2 &controlPoint1, const Float_2 &controlPoint2, const Float_2 &endPoint ) const;
        DZ_API void SmoothCubicCurveTo( const Float_2 &controlPoint2, const Float_2 &endPoint ) const;

        // Curve operations (relative)
        DZ_API void RelativeQuadraticCurveTo( const Float_2 &controlOffset, const Float_2 &endOffset ) const;
        DZ_API void RelativeSmoothQuadraticCurveTo( const Float_2 &endOffset ) const;
        DZ_API void RelativeCubicCurveTo( const Float_2 &control1Offset, const Float_2 &control2Offset, const Float_2 &endOffset ) const;
        DZ_API void RelativeSmoothCubicCurveTo( const Float_2 &control2Offset, const Float_2 &endOffset ) const;

        // Arc operations (absolute)
        DZ_API void EllipticalArcTo( const Float_2 &radii, float xAxisRotation, bool largeArcFlag, bool sweepFlag, const Float_2 &endPoint ) const;
        DZ_API void CircularArcTo( const Float_2 &center, float radius, float startAngle, float endAngle, bool clockwise = true ) const;

        // Arc operations (relative)
        DZ_API void RelativeEllipticalArcTo( const Float_2 &radii, float xAxisRotation, bool largeArcFlag, bool sweepFlag, const Float_2 &endOffset ) const;

        // Convenience arc methods
        DZ_API void ArcTo( const Float_2 &center, float radius, float startAngle, float endAngle, bool clockwise = true ) const;
        DZ_API void ArcByCenter( const Float_2 &center, const Float_2 &radii, float startAngle, float endAngle, bool clockwise = true ) const;

        // Shape helpers
        DZ_API void AddRect( const VGRect &rect ) const;
        DZ_API void AddRoundedRect( const VGRoundedRect &roundedRect ) const;
        DZ_API void AddCircle( const VGCircle &circle ) const;
        DZ_API void AddEllipse( const VGEllipse &ellipse ) const;
        DZ_API void AddPolygon( const VGPolygon &polygon ) const;

        // Advanced shape helpers
        DZ_API void AddRectWithCorners( const Float_2 &topLeft, const Float_2 &bottomRight, float cornerRadius ) const;
        DZ_API void AddRectWithIndividualCorners( const Float_2 &topLeft, const Float_2 &bottomRight, float topLeftRadius, float topRightRadius, float bottomRightRadius,
                                           float bottomLeftRadius ) const;

        // Path queries
        DZ_API [[nodiscard]] bool     IsEmpty( ) const;
        DZ_API [[nodiscard]] bool     IsClosed( ) const;
        DZ_API [[nodiscard]] Float_2  GetCurrentPoint( ) const;
        DZ_API [[nodiscard]] Float_2  GetStartPoint( ) const;
        DZ_API [[nodiscard]] Float_2  GetLastControlPoint( ) const;
        DZ_API [[nodiscard]] bool     HasLastControlPoint( ) const;
        DZ_API [[nodiscard]] uint32_t GetCommandCount( ) const;
        DZ_API [[nodiscard]] VGBounds GetBounds( ) const;
        DZ_API [[nodiscard]] VGBounds GetTightBounds( ) const;

        // Path data access
        DZ_API [[nodiscard]] const InteropArray<VGPathCommand> &GetCommands( ) const;
        DZ_API [[nodiscard]] VGPathCommand                      GetCommand( uint32_t index ) const;

        // Path manipulation
        DZ_API void Reverse( ) const;
        DZ_API void Transform( const Float_4x4 &matrix ) const;
        DZ_API void Translate( const Float_2 &offset ) const;
        DZ_API void Scale( const Float_2 &scale ) const;
        DZ_API void Scale( float scale ) const;
        DZ_API void Rotate( float angleRadians, const Float_2 &center ) const;

        // Path combination
        DZ_API void AppendPath( const VGPath2D &other ) const;
        DZ_API void AppendPath( const VGPath2D &other, const Float_4x4 &transform ) const;

        // Tessellation and rendering hints
        DZ_API void                     SetTessellationTolerance( float tolerance ) const;
        DZ_API [[nodiscard]] float      GetTessellationTolerance( ) const;
        DZ_API void                     SetFillRule( VGFillRule fillRule ) const;
        DZ_API [[nodiscard]] VGFillRule GetFillRule( ) const;

        // Stroke properties (for path outlining)
        DZ_API void                     SetStrokeWidth( float width ) const;
        DZ_API [[nodiscard]] float      GetStrokeWidth( ) const;
        DZ_API void                     SetLineCap( VGLineCap cap ) const;
        DZ_API [[nodiscard]] VGLineCap  GetLineCap( ) const;
        DZ_API void                     SetLineJoin( VGLineJoin join ) const;
        DZ_API [[nodiscard]] VGLineJoin GetLineJoin( ) const;
        DZ_API void                     SetMiterLimit( float limit ) const;
        DZ_API [[nodiscard]] float      GetMiterLimit( ) const;

        // Dash pattern support
        DZ_API void                              SetDashPattern( const InteropArray<float> &pattern, float offset = 0.0f ) const;
        DZ_API [[nodiscard]] InteropArray<float> GetDashPattern( ) const;
        DZ_API [[nodiscard]] float               GetDashOffset( ) const;
        DZ_API void                              ClearDashPattern( ) const;
        DZ_API [[nodiscard]] bool                HasDashPattern( ) const;

        // Path analysis
        DZ_API [[nodiscard]] float   GetLength( ) const;
        DZ_API [[nodiscard]] Float_2 GetPointAtLength( float distance ) const;
        DZ_API [[nodiscard]] Float_2 GetTangentAtLength( float distance ) const;
        DZ_API [[nodiscard]] bool    ContainsPoint( const Float_2 &point ) const;
        DZ_API [[nodiscard]] bool    ContainsPoint( const Float_2 &point, VGFillRule fillRule ) const;

        // Validation
        DZ_API [[nodiscard]] bool                        IsValid( ) const;
        DZ_API [[nodiscard]] InteropArray<InteropString> GetValidationErrors( ) const;

    private:
        void AddCommand( const VGPathCommand &command ) const;
        void UpdateCurrentPoint( const Float_2 &point ) const;
        void UpdateLastControlPoint( const Float_2 &point ) const;
        void ClearLastControlPoint( ) const;
        void InvalidateBounds( ) const;
        void CalculateBounds( ) const;
        void CalculateTightBounds( ) const;
    };

    // Fluid interface
    class DZ_API VGPathBuilder
    {
        VGPath2D m_path;

    public:
        VGPathBuilder( ) = default;

        // Movement
        VGPathBuilder &MoveTo( const Float_2 &point )
        {
            m_path.MoveTo( point );
            return *this;
        }
        VGPathBuilder &RelativeMoveTo( const Float_2 &offset )
        {
            m_path.RelativeMoveTo( offset );
            return *this;
        }

        // Lines
        VGPathBuilder &LineTo( const Float_2 &point )
        {
            m_path.LineTo( point );
            return *this;
        }
        VGPathBuilder &RelativeLineTo( const Float_2 &offset )
        {
            m_path.RelativeLineTo( offset );
            return *this;
        }
        VGPathBuilder &HorizontalLineTo( float x )
        {
            m_path.HorizontalLineTo( x );
            return *this;
        }
        VGPathBuilder &VerticalLineTo( float y )
        {
            m_path.VerticalLineTo( y );
            return *this;
        }

        // Curves
        VGPathBuilder &QuadraticCurveTo( const Float_2 &control, const Float_2 &end )
        {
            m_path.QuadraticCurveTo( control, end );
            return *this;
        }
        VGPathBuilder &SmoothQuadraticCurveTo( const Float_2 &end )
        {
            m_path.SmoothQuadraticCurveTo( end );
            return *this;
        }
        VGPathBuilder &CubicCurveTo( const Float_2 &control1, const Float_2 &control2, const Float_2 &end )
        {
            m_path.CubicCurveTo( control1, control2, end );
            return *this;
        }
        VGPathBuilder &SmoothCubicCurveTo( const Float_2 &control2, const Float_2 &end )
        {
            m_path.SmoothCubicCurveTo( control2, end );
            return *this;
        }

        // Arcs
        VGPathBuilder &EllipticalArcTo( const Float_2 &radii, float rotation, bool large, bool sweep, const Float_2 &end )
        {
            m_path.EllipticalArcTo( radii, rotation, large, sweep, end );
            return *this;
        }
        VGPathBuilder &ArcTo( const Float_2 &center, float radius, float start, float end, bool clockwise = true )
        {
            m_path.ArcTo( center, radius, start, end, clockwise );
            return *this;
        }

        // Shapes
        VGPathBuilder &AddRect( const VGRect &rect )
        {
            m_path.AddRect( rect );
            return *this;
        }
        VGPathBuilder &AddCircle( const VGCircle &circle )
        {
            m_path.AddCircle( circle );
            return *this;
        }
        VGPathBuilder &AddEllipse( const VGEllipse &ellipse )
        {
            m_path.AddEllipse( ellipse );
            return *this;
        }

        // Control
        VGPathBuilder &Close( )
        {
            m_path.Close( );
            return *this;
        }

        [[nodiscard]] VGPath2D Build( ) const
        {
            return m_path;
        }
    };

} // namespace DenOfIz
