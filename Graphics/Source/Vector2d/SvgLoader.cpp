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

#include <DenOfIzGraphics/Utilities/Common.h>
#include <DenOfIzGraphics/Vector2d/SvgLoader.h>
#include <DirectXMath.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace
{
    void SafeSubstringCopy( char *dest, const char *src, const size_t srcLen, const size_t destSize )
    {
        DZ_RETURN_IF( destSize == 0 );
        const size_t copyLen = std::min( srcLen, destSize - 1 );

        std::memcpy( dest, src, copyLen );
        dest[ copyLen ] = '\0';
    }
} // namespace

using namespace DenOfIz;
using namespace tinyxml2;
using namespace DirectX;

class SvgLoader::Impl
{
public:
    tinyxml2::XMLDocument xmlDoc;
    SvgDocument           document;
    SvgLoadDesc           options;
    InteropString         lastError;
    bool                  hasErrors = false;

    std::vector<std::unique_ptr<VGPath2D>> pathStorage;
    std::vector<SvgRenderCommand>          renderCommands;
    std::vector<SvgTransform>              transformStack;

    ~Impl( )
    {
        renderCommands.clear( );
        pathStorage.clear( );
        transformStack.clear( );
    }

    void Clear( )
    {
        document = { };
        renderCommands.clear( );
        pathStorage.clear( );
        transformStack.clear( );
        lastError = InteropString( );
        hasErrors = false;
    }

    void SetError( const InteropString &error )
    {
        lastError = error;
        hasErrors = true;
    }
};

SvgLoader::SvgLoader( ) : m_impl( new Impl( ) )
{
}

SvgLoader::~SvgLoader( )
{
    delete m_impl;
}

SvgLoader::SvgLoader( SvgLoader &&other ) noexcept : m_impl( other.m_impl )
{
    other.m_impl = nullptr;
}

SvgLoader &SvgLoader::operator=( SvgLoader &&other ) noexcept
{
    if ( this != &other )
    {
        delete m_impl;
        m_impl       = other.m_impl;
        other.m_impl = nullptr;
    }
    return *this;
}

SvgLoadResult SvgLoader::LoadFromFile( const InteropString &filePath, const SvgLoadDesc &options )
{
    if ( !m_impl )
    {
        return SvgLoadResult::InvalidFormat;
    }

    m_impl->Clear( );
    m_impl->options = options;

    const std::ifstream file( filePath.Get( ) );
    if ( !file.is_open( ) )
    {
        m_impl->SetError( InteropString( "Failed to open file: " ).Append( filePath.Get( ) ) );
        return SvgLoadResult::FileNotFound;
    }

    std::stringstream buffer;
    buffer << file.rdbuf( );
    const std::string content = buffer.str( );

    return LoadFromString( content.c_str( ), options );
}

SvgLoadResult SvgLoader::LoadFromBinaryData( const InteropArray<Byte> &data, const SvgLoadDesc &options )
{
    if ( !m_impl || data.NumElements( ) == 0 )
    {
        return SvgLoadResult::InvalidFormat;
    }

    InteropArray<char> charData( data.NumElements( ) + 1 );
    std::memcpy( charData.Data( ), data.Data( ), data.NumElements( ) );
    charData.SetElement( data.NumElements( ), '\0' );

    const InteropString content( charData.Data( ) );
    return LoadFromString( content, options );
}

SvgLoadResult SvgLoader::LoadFromString( const InteropString &svgContent, const SvgLoadDesc &options )
{
    if ( !m_impl )
    {
        return SvgLoadResult::InvalidFormat;
    }

    m_impl->Clear( );
    m_impl->options = options;

    const XMLError result = m_impl->xmlDoc.Parse( svgContent.Get( ) );
    if ( result != XML_SUCCESS )
    {
        m_impl->SetError( "Failed to parse XML" );
        return SvgLoadResult::InvalidXml;
    }

    XMLElement *svgElement = m_impl->xmlDoc.FirstChildElement( "svg" );
    if ( !svgElement )
    {
        m_impl->SetError( "No SVG root element found" );
        return SvgLoadResult::InvalidFormat;
    }

    if ( !ParseDocument( svgElement ) )
    {
        return SvgLoadResult::InvalidFormat;
    }

    return SvgLoadResult::Success;
}

void SvgLoader::RenderToVectorGraphics( VectorGraphics *vectorGraphics ) const
{
    if ( !m_impl || !vectorGraphics )
    {
        return;
    }
    for ( const auto &cmd : m_impl->renderCommands )
    {
        RenderCommand( vectorGraphics, cmd );
    }
}

void SvgLoader::RenderElementById( VectorGraphics *vectorGraphics, const InteropString &elementId ) const
{
    // TODO: Implement element ID filtering
    // For now, just render everything
    RenderToVectorGraphics( vectorGraphics );
}

const SvgDocument &SvgLoader::GetDocument( ) const
{
    return m_impl->document;
}

SvgViewBox SvgLoader::GetEffectiveViewBox( ) const
{
    if ( m_impl->document.HasViewBox )
    {
        return m_impl->document.ViewBox;
    }

    SvgViewBox viewBox;
    viewBox.X      = 0.0f;
    viewBox.Y      = 0.0f;
    viewBox.Width  = m_impl->document.Size.X;
    viewBox.Height = m_impl->document.Size.Y;
    return viewBox;
}

Float_2 SvgLoader::GetDocumentSize( ) const
{
    return m_impl->document.Size;
}

InteropString SvgLoader::GetLastError( ) const
{
    return m_impl->lastError;
}

bool SvgLoader::HasErrors( ) const
{
    return m_impl->hasErrors;
}

void SvgLoader::Clear( ) const
{
    if ( m_impl )
    {
        m_impl->Clear( );
    }
}

static bool StringStartsWith( const char *str, const char *prefix )
{
    return std::strncmp( str, prefix, std::strlen( prefix ) ) == 0;
}

static bool StringEndsWith( const char *str, const char *suffix )
{
    const size_t strLen    = std::strlen( str );
    const size_t suffixLen = std::strlen( suffix );
    if ( suffixLen > strLen )
    {
        return false;
    }
    return std::strcmp( str + strLen - suffixLen, suffix ) == 0;
}

static bool StringContains( const char *str, const char *substr )
{
    if ( !str )
    {
        return false;
    }
    return std::strstr( str, substr ) != nullptr;
}

Float_4 SvgLoader::ParseColor( const InteropString &colorString )
{
    if ( colorString.IsEmpty( ) || colorString.Equals( InteropString( "none" ) ) )
    {
        return { 0.0f, 0.0f, 0.0f, 0.0f };
    }

    const InteropString lowerColorString = colorString.ToLower( );
    const std::string   color            = lowerColorString.Get( );
    const char         *colorStr         = color.c_str( );

    if ( std::strcmp( colorStr, "black" ) == 0 )
    {
        return { 0.0f, 0.0f, 0.0f, 1.0f };
    }
    if ( std::strcmp( colorStr, "white" ) == 0 )
    {
        return { 1.0f, 1.0f, 1.0f, 1.0f };
    }
    if ( std::strcmp( colorStr, "red" ) == 0 )
    {
        return { 1.0f, 0.0f, 0.0f, 1.0f };
    }
    if ( std::strcmp( colorStr, "green" ) == 0 )
    {
        return { 0.0f, 0.5f, 0.0f, 1.0f };
    }
    if ( std::strcmp( colorStr, "blue" ) == 0 )
    {
        return { 0.0f, 0.0f, 1.0f, 1.0f };
    }
    if ( std::strcmp( colorStr, "yellow" ) == 0 )
    {
        return { 1.0f, 1.0f, 0.0f, 1.0f };
    }
    if ( std::strcmp( colorStr, "cyan" ) == 0 )
    {
        return { 0.0f, 1.0f, 1.0f, 1.0f };
    }
    if ( std::strcmp( colorStr, "magenta" ) == 0 )
    {
        return { 1.0f, 0.0f, 1.0f, 1.0f };
    }
    if ( std::strcmp( colorStr, "orange" ) == 0 )
    {
        return { 1.0f, 0.647f, 0.0f, 1.0f };
    }
    if ( std::strcmp( colorStr, "purple" ) == 0 )
    {
        return { 0.5f, 0.0f, 0.5f, 1.0f };
    }
    if ( std::strcmp( colorStr, "brown" ) == 0 )
    {
        return { 0.647f, 0.165f, 0.165f, 1.0f };
    }
    if ( std::strcmp( colorStr, "gray" ) == 0 || std::strcmp( colorStr, "grey" ) == 0 )
    {
        return { 0.5f, 0.5f, 0.5f, 1.0f };
    }

    if ( StringStartsWith( colorStr, "#" ) )
    {
        const char  *hexStr = colorStr + 1; // Skip the '#'
        const size_t hexLen = std::strlen( hexStr );

        if ( hexLen == 3 )
        {
            // Short form: #RGB -> #RRGGBB
            char expandedHex[ 7 ];
            expandedHex[ 0 ] = hexStr[ 0 ];
            expandedHex[ 1 ] = hexStr[ 0 ];
            expandedHex[ 2 ] = hexStr[ 1 ];
            expandedHex[ 3 ] = hexStr[ 1 ];
            expandedHex[ 4 ] = hexStr[ 2 ];
            expandedHex[ 5 ] = hexStr[ 2 ];
            expandedHex[ 6 ] = '\0';

            const uint32_t hexValue = std::strtoul( expandedHex, nullptr, 16 );
            const float    r        = ( hexValue >> 16 & 0xFF ) / 255.0f;
            const float    g        = ( hexValue >> 8 & 0xFF ) / 255.0f;
            const float    b        = ( hexValue & 0xFF ) / 255.0f;
            return { r, g, b, 1.0f };
        }
        if ( hexLen == 6 )
        {
            const uint32_t hexValue = std::strtoul( hexStr, nullptr, 16 );
            const float    r        = ( hexValue >> 16 & 0xFF ) / 255.0f;
            const float    g        = ( hexValue >> 8 & 0xFF ) / 255.0f;
            const float    b        = ( hexValue & 0xFF ) / 255.0f;
            return { r, g, b, 1.0f };
        }
    }

    if ( StringStartsWith( colorStr, "rgb(" ) && StringEndsWith( colorStr, ")" ) )
    {
        const char  *start    = colorStr + 4; // Skip "rgb("
        const size_t totalLen = std::strlen( colorStr );

        const size_t valuesLen = totalLen - 5; // Remove "rgb(" and ")"
        const auto   valueStr  = new char[ valuesLen + 1 ];
        SafeSubstringCopy( valueStr, start, valuesLen, valuesLen + 1 );

        // Parse comma-separated values
        const char *ptr = valueStr;
        float       r = 0.0f, g = 0.0f, b = 0.0f;

        r = std::strtof( ptr, const_cast<char **>( &ptr ) );
        while ( *ptr && ( *ptr == ',' || *ptr == ' ' ) )
            ptr++;
        g = std::strtof( ptr, const_cast<char **>( &ptr ) );
        while ( *ptr && ( *ptr == ',' || *ptr == ' ' ) )
            ptr++;
        b = std::strtof( ptr, const_cast<char **>( &ptr ) );

        const bool isPercentage = StringContains( valueStr, "%" );
        delete[] valueStr;
        if ( isPercentage )
        {
            r /= 100.0f;
            g /= 100.0f;
            b /= 100.0f;
        }
        else
        {
            r /= 255.0f;
            g /= 255.0f;
            b /= 255.0f;
        }

        return { r, g, b, 1.0f };
    }

    return { 0.0f, 0.0f, 0.0f, 1.0f };
}

float SvgLoader::ParseLength( const InteropString &lengthString, const float referenceValue )
{
    if ( lengthString.IsEmpty( ) )
    {
        return 0.0f;
    }

    const char  *str = lengthString.Get( );
    const size_t len = std::strlen( str );

    if ( StringEndsWith( str, "%" ) )
    {
        // Create substring without '%'
        const auto numStr = new char[ len ];
        SafeSubstringCopy( numStr, str, len - 1, len );

        const float percent = std::strtof( numStr, nullptr );
        delete[] numStr;
        return percent / 100.0f * referenceValue;
    }

    // Handle units(Todo we treat these all the same)
    if ( StringEndsWith( str, "px" ) || StringEndsWith( str, "pt" ) || StringEndsWith( str, "em" ) || StringEndsWith( str, "mm" ) || StringEndsWith( str, "cm" ) ||
         StringEndsWith( str, "in" ) )
    {
        // Create substring without the last 2 characters
        const auto numStr = new char[ len - 1 ];
        SafeSubstringCopy( numStr, str, len - 2, len - 1 );

        const float result = std::strtof( numStr, nullptr );
        delete[] numStr;
        return result;
    }

    return std::strtof( str, nullptr );
}

SvgTransform SvgLoader::ParseTransform( const InteropString &transformString )
{
    SvgTransform result;
    result.HasTransform = false;

    if ( transformString.IsEmpty( ) )
    {
        return result;
    }

    result.Matrix       = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    result.HasTransform = true;

    InteropString lowerStr = transformString.ToLower( );
    const char   *str      = lowerStr.Get( );
    if ( StringContains( str, "translate" ) )
    {
        if ( const char *start = std::strstr( str, "translate(" ) )
        {
            start += 10; // Skip "translate("
            if ( const char *end = std::strchr( start, ')' ) )
            {
                const size_t paramLen = end - start;
                auto         paramStr = new char[ paramLen + 1 ];
                SafeSubstringCopy( paramStr, start, paramLen, paramLen + 1 );

                const char *ptr = paramStr;
                const float tx  = std::strtof( ptr, const_cast<char **>( &ptr ) );
                while ( *ptr && ( *ptr == ',' || *ptr == ' ' ) )
                {
                    ptr++;
                }
                const float ty = *ptr ? std::strtof( ptr, nullptr ) : 0.0f;

                result.Matrix._41 = tx;
                result.Matrix._42 = ty;

                delete[] paramStr;
            }
        }
    }

    if ( StringContains( str, "scale" ) )
    {
        if ( const char *start = std::strstr( str, "scale(" ) )
        {
            start += 6; // Skip "scale("
            if ( const char *end = std::strchr( start, ')' ) )
            {
                const size_t paramLen = end - start;
                auto         paramStr = new char[ paramLen + 1 ];
                SafeSubstringCopy( paramStr, start, paramLen, paramLen + 1 );

                const char *ptr = paramStr;
                const float sx  = std::strtof( ptr, const_cast<char **>( &ptr ) );
                while ( *ptr && ( *ptr == ',' || *ptr == ' ' ) )
                {
                    ptr++;
                }
                const float sy = *ptr ? std::strtof( ptr, nullptr ) : sx;

                result.Matrix._11 *= sx;
                result.Matrix._22 *= sy;
                delete[] paramStr;
            }
        }
    }

    if ( StringContains( str, "rotate" ) )
    {
        if ( const char *start = std::strstr( str, "rotate(" ) )
        {
            start += 7; // Skip "rotate("
            if ( const char *end = std::strchr( start, ')' ) )
            {
                // Extract parameters
                const size_t paramLen = end - start;
                auto         paramStr = new char[ paramLen + 1 ];
                SafeSubstringCopy( paramStr, start, paramLen, paramLen + 1 );

                const float angle    = std::strtof( paramStr, nullptr );
                const float radians  = angle * static_cast<float>( M_PI ) / 180.0f;
                const float cosAngle = std::cos( radians );
                const float sinAngle = std::sin( radians );

                const Float_4x4 rotMatrix = { cosAngle, -sinAngle, 0.0f, 0.0f, sinAngle, cosAngle, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
                Float_4x4       newMatrix; // 3d -> 2d
                newMatrix._11 = result.Matrix._11 * cosAngle - result.Matrix._12 * sinAngle;
                newMatrix._12 = result.Matrix._11 * sinAngle + result.Matrix._12 * cosAngle;
                newMatrix._21 = result.Matrix._21 * cosAngle - result.Matrix._22 * sinAngle;
                newMatrix._22 = result.Matrix._21 * sinAngle + result.Matrix._22 * cosAngle;
                newMatrix._41 = result.Matrix._41;
                newMatrix._42 = result.Matrix._42;
                newMatrix._13 = newMatrix._14 = newMatrix._23 = newMatrix._24 = 0.0f;
                newMatrix._31 = newMatrix._32 = newMatrix._34 = newMatrix._43 = 0.0f;
                newMatrix._33 = newMatrix._44 = 1.0f;

                result.Matrix = newMatrix;

                delete[] paramStr;
            }
        }
    }

    return result;
}

SvgLoader::SvgRenderCommand::SvgRenderCommand( ) : Type( RectCommand )
{
    std::memset( &RectData, 0, sizeof( RectData ) );
}

SvgLoader::SvgRenderCommand::~SvgRenderCommand( )
{
    CleanupUnionData( );
}

SvgLoader::SvgRenderCommand::SvgRenderCommand( const SvgRenderCommand &other ) : Type( other.Type ), Style( other.Style ), Transform( other.Transform )
{
    CopyUnionData( other );
}

SvgLoader::SvgRenderCommand::SvgRenderCommand( SvgRenderCommand &&other ) noexcept :
    Type( other.Type ), Style( std::move( other.Style ) ), Transform( std::move( other.Transform ) )
{
    MoveUnionData( std::move( other ) );
}

SvgLoader::SvgRenderCommand &SvgLoader::SvgRenderCommand::operator=( const SvgRenderCommand &other )
{
    if ( this != &other )
    {
        CleanupUnionData( );
        Type      = other.Type;
        Style     = other.Style;
        Transform = other.Transform;
        CopyUnionData( other );
    }
    return *this;
}

SvgLoader::SvgRenderCommand &SvgLoader::SvgRenderCommand::operator=( SvgRenderCommand &&other ) noexcept
{
    if ( this != &other )
    {
        CleanupUnionData( );
        Type      = other.Type;
        Style     = std::move( other.Style );
        Transform = std::move( other.Transform );
        MoveUnionData( std::move( other ) );
    }
    return *this;
}

void SvgLoader::SvgRenderCommand::CopyUnionData( const SvgRenderCommand &other )
{
    switch ( Type )
    {
    case RectCommand:
        RectData = other.RectData;
        break;
    case CircleCommand:
        CircleData = other.CircleData;
        break;
    case EllipseCommand:
        EllipseData = other.EllipseData;
        break;
    case LineCommand:
        LineData = other.LineData;
        break;
    case PolygonCommand:
        new ( &PolygonData ) auto( other.PolygonData );
        break;
    case PathCommand:
        PathData = other.PathData;
        break;
    case TextCommand:
        new ( &TextData ) auto( other.TextData );
        break;
    }
}

void SvgLoader::SvgRenderCommand::MoveUnionData( SvgRenderCommand &&other )
{
    switch ( Type )
    {
    case RectCommand:
        RectData = other.RectData;
        break;
    case CircleCommand:
        CircleData = other.CircleData;
        break;
    case EllipseCommand:
        EllipseData = other.EllipseData;
        break;
    case LineCommand:
        LineData = other.LineData;
        break;
    case PolygonCommand:
        new ( &PolygonData ) auto( std::move( other.PolygonData ) );
        break;
    case PathCommand:
        PathData.Path       = other.PathData.Path;
        other.PathData.Path = nullptr;
        break;
    case TextCommand:
        new ( &TextData ) auto( std::move( other.TextData ) );
        break;
    }
}

void SvgLoader::SvgRenderCommand::CleanupUnionData( )
{
    switch ( Type )
    {
    case PolygonCommand:
        PolygonData.~decltype( PolygonData )( );
        break;
    case TextCommand:
        TextData.~decltype( TextData )( );
        break;
    case PathCommand:
        PathData.Path = nullptr;
        break;
    default:
        break;
    }
}

bool SvgLoader::ParseDocument( XMLElement *svgElement )
{
    if ( !svgElement )
    {
        return false;
    }

    const InteropString widthStr  = GetAttributeString( svgElement, "width", "100" );
    const InteropString heightStr = GetAttributeString( svgElement, "height", "100" );

    m_impl->document.Size.X = ParseLength( widthStr, 100.0f );
    m_impl->document.Size.Y = ParseLength( heightStr, 100.0f );

    const InteropString viewBoxStr = GetAttributeString( svgElement, "viewBox" );
    if ( !viewBoxStr.IsEmpty( ) )
    {
        const char *ptr            = viewBoxStr.Get( );
        m_impl->document.ViewBox.X = std::strtof( ptr, const_cast<char **>( &ptr ) );
        while ( *ptr && ( *ptr == ',' || *ptr == ' ' ) )
        {
            ptr++;
        }
        m_impl->document.ViewBox.Y = std::strtof( ptr, const_cast<char **>( &ptr ) );
        while ( *ptr && ( *ptr == ',' || *ptr == ' ' ) )
        {
            ptr++;
        }
        m_impl->document.ViewBox.Width = std::strtof( ptr, const_cast<char **>( &ptr ) );
        while ( *ptr && ( *ptr == ',' || *ptr == ' ' ) )
        {
            ptr++;
        }
        m_impl->document.ViewBox.Height = std::strtof( ptr, nullptr );
        m_impl->document.HasViewBox     = true;
    }

    // Parse defs for gradients first
    if ( XMLElement *defs = svgElement->FirstChildElement( "defs" ) )
    {
        for ( XMLElement *element = defs->FirstChildElement( ); element; element = element->NextSiblingElement( ) )
        {
            const std::string tagName = element->Name( );
            if ( tagName == "linearGradient" )
            {
                ParseLinearGradient( element );
            }
            else if ( tagName == "radialGradient" )
            {
                ParseRadialGradient( element );
            }
        }
    }

    SvgTransform defaultTransform;
    defaultTransform.HasTransform = false;
    for ( XMLElement *element = svgElement->FirstChildElement( ); element; element = element->NextSiblingElement( ) )
    {
        const SvgStyle defaultStyle;
        ParseElement( element, defaultStyle, defaultTransform );
    }

    return true;
}

bool SvgLoader::ParseElement( XMLElement *element, const SvgStyle &parentStyle, const SvgTransform &parentTransform )
{
    if ( !element )
    {
        return false;
    }

    const std::string tagName = element->Name( );
    // Skip defs as they were already processed
    if ( tagName == "defs" )
    {
        return true;
    }

    const SvgStyle     style             = ParseElementStyle( element, parentStyle );
    const SvgTransform elementTransform  = ParseTransform( GetAttributeString( element, "transform" ) );
    const SvgTransform combinedTransform = CombineTransforms( parentTransform, elementTransform );

    if ( tagName == "rect" )
    {
        ParseRect( element, style, combinedTransform );
    }
    else if ( tagName == "circle" )
    {
        ParseCircle( element, style, combinedTransform );
    }
    else if ( tagName == "ellipse" )
    {
        ParseEllipse( element, style, combinedTransform );
    }
    else if ( tagName == "line" )
    {
        ParseLine( element, style, combinedTransform );
    }
    else if ( tagName == "polyline" )
    {
        ParsePolyline( element, style, combinedTransform );
    }
    else if ( tagName == "polygon" )
    {
        ParsePolygon( element, style, combinedTransform );
    }
    else if ( tagName == "path" )
    {
        ParsePath( element, style, combinedTransform );
    }
    else if ( tagName == "text" && m_impl->options.LoadText )
    {
        ParseText( element, style, combinedTransform );
    }
    else if ( tagName == "g" )
    {
        ParseGroup( element, style, combinedTransform );
    }
    else if ( tagName == "svg" )
    {
        // Nested SVG - treat as group
        ParseGroup( element, style, combinedTransform );
    }

    return true;
}

void SvgLoader::ParseRect( const XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const
{
    m_impl->renderCommands.emplace_back( );
    SvgRenderCommand &cmd = m_impl->renderCommands.back( );
    cmd.Type              = SvgRenderCommand::RectCommand;
    cmd.Style             = style;
    cmd.Transform         = transform;

    const float x      = GetAttributeFloat( element, "x", 0.0f );
    const float y      = GetAttributeFloat( element, "y", 0.0f );
    const float width  = GetAttributeFloat( element, "width", 0.0f );
    const float height = GetAttributeFloat( element, "height", 0.0f );
    const float rx     = GetAttributeFloat( element, "rx", 0.0f );
    const float ry     = GetAttributeFloat( element, "ry", rx );

    cmd.RectData.Rect.TopLeft     = { x, y };
    cmd.RectData.Rect.BottomRight = { x + width, y + height };
    cmd.RectData.CornerRadii      = { rx, ry, rx, ry };
}

void SvgLoader::ParseCircle( const XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const
{
    m_impl->renderCommands.emplace_back( );
    SvgRenderCommand &cmd = m_impl->renderCommands.back( );
    cmd.Type              = SvgRenderCommand::CircleCommand;
    cmd.Style             = style;
    cmd.Transform         = transform;

    const float cx = GetAttributeFloat( element, "cx", 0.0f );
    const float cy = GetAttributeFloat( element, "cy", 0.0f );
    const float r  = GetAttributeFloat( element, "r", 0.0f );

    cmd.CircleData.Circle.Center = { cx, cy };
    cmd.CircleData.Circle.Radius = r;
}

void SvgLoader::ParseEllipse( const XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const
{
    m_impl->renderCommands.emplace_back( );
    SvgRenderCommand &cmd = m_impl->renderCommands.back( );
    cmd.Type              = SvgRenderCommand::EllipseCommand;
    cmd.Style             = style;
    cmd.Transform         = transform;

    const float cx = GetAttributeFloat( element, "cx", 0.0f );
    const float cy = GetAttributeFloat( element, "cy", 0.0f );
    const float rx = GetAttributeFloat( element, "rx", 0.0f );
    const float ry = GetAttributeFloat( element, "ry", 0.0f );

    cmd.EllipseData.Ellipse.Center   = { cx, cy };
    cmd.EllipseData.Ellipse.Radii    = { rx, ry };
    cmd.EllipseData.Ellipse.Rotation = 0.0f;
}

void SvgLoader::ParseLine( const XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const
{
    m_impl->renderCommands.emplace_back( );
    SvgRenderCommand &cmd = m_impl->renderCommands.back( );
    cmd.Type              = SvgRenderCommand::LineCommand;
    cmd.Style             = style;
    cmd.Transform         = transform;

    const float x1 = GetAttributeFloat( element, "x1", 0.0f );
    const float y1 = GetAttributeFloat( element, "y1", 0.0f );
    const float x2 = GetAttributeFloat( element, "x2", 0.0f );
    const float y2 = GetAttributeFloat( element, "y2", 0.0f );

    cmd.LineData.Line.StartPoint = { x1, y1 };
    cmd.LineData.Line.EndPoint   = { x2, y2 };
    cmd.LineData.Line.Thickness  = style.StrokeWidth;
}

void SvgLoader::ParsePolyline( const XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const
{
    m_impl->renderCommands.emplace_back( );
    SvgRenderCommand &cmd = m_impl->renderCommands.back( );
    cmd.Type              = SvgRenderCommand::PolygonCommand;
    cmd.Style             = style;
    cmd.Transform         = transform;

    const InteropString pointsStr = GetAttributeString( element, "points" );
    if ( !pointsStr.IsEmpty( ) )
    {
        cmd.PolygonData.Polygon.IsClosed = false;
        const char *ptr                  = pointsStr.Get( );
        while ( *ptr )
        {
            while ( *ptr && ( std::isspace( *ptr ) || *ptr == ',' ) )
            {
                ptr++;
            }
            if ( !*ptr )
            {
                break;
            }

            const float x = std::strtof( ptr, const_cast<char **>( &ptr ) );
            while ( *ptr && ( std::isspace( *ptr ) || *ptr == ',' ) )
            {
                ptr++;
            }
            if ( !*ptr )
            {
                break;
            }

            const float y = std::strtof( ptr, const_cast<char **>( &ptr ) );
            cmd.PolygonData.Polygon.Points.AddElement( { x, y } );
        }
    }
}

void SvgLoader::ParsePolygon( const XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const
{
    ParsePolyline( element, style, transform );
    if ( !m_impl->renderCommands.empty( ) )
    {
        SvgRenderCommand &cmd            = m_impl->renderCommands.back( );
        cmd.PolygonData.Polygon.IsClosed = true;
    }
}

void SvgLoader::ParsePath( const XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const
{
    const InteropString pathData = GetAttributeString( element, "d" );
    if ( pathData.IsEmpty( ) )
    {
        return;
    }

    m_impl->renderCommands.emplace_back( );
    SvgRenderCommand &cmd = m_impl->renderCommands.back( );
    cmd.Type              = SvgRenderCommand::PathCommand;
    cmd.Style             = style;
    cmd.Transform         = transform;

    VGPath2D parsedPath = ParsePathData( pathData );
    auto     pathPtr    = std::make_unique<VGPath2D>( std::move( parsedPath ) );
    pathPtr->SetTessellationTolerance( m_impl->options.TessellationTolerance );
    VGPath2D *rawPtr = pathPtr.get( );
    m_impl->pathStorage.push_back( std::move( pathPtr ) );
    cmd.PathData.Path = rawPtr;
}

void SvgLoader::ParseText( const XMLElement *element, const SvgStyle &style, const SvgTransform &transform ) const
{
    if ( !m_impl->options.LoadText )
    {
        return;
    }

    m_impl->renderCommands.emplace_back( );
    SvgRenderCommand &cmd = m_impl->renderCommands.back( );
    cmd.Type              = SvgRenderCommand::TextCommand;
    cmd.Style             = style;
    cmd.Transform         = transform;

    const float         x          = GetAttributeFloat( element, "x", 0.0f );
    const float         y          = GetAttributeFloat( element, "y", 0.0f );
    const InteropString fontFamily = GetAttributeString( element, "font-family", m_impl->options.DefaultFontFamily );
    const float         fontSize   = GetAttributeFloat( element, "font-size", m_impl->options.DefaultFontSize );

    cmd.TextData.Position   = { x, y };
    cmd.TextData.FontSize   = fontSize;
    cmd.TextData.FontFamily = fontFamily;

    const char *textContent = element->GetText( );
    cmd.TextData.Text       = textContent ? textContent : "";
}

void SvgLoader::ParseGroup( XMLElement *element, const SvgStyle &parentStyle, const SvgTransform &parentTransform )
{
    const SvgStyle     groupStyle        = ParseElementStyle( element, parentStyle );
    const SvgTransform elementTransform  = ParseTransform( GetAttributeString( element, "transform" ) );
    const SvgTransform combinedTransform = CombineTransforms( parentTransform, elementTransform );
    for ( XMLElement *child = element->FirstChildElement( ); child; child = child->NextSiblingElement( ) )
    {
        ParseElement( child, groupStyle, combinedTransform );
    }
}

void SvgLoader::ParseLinearGradient( XMLElement *element ) const
{
    SvgLinearGradient &gradient = m_impl->document.LinearGradients.EmplaceElement( );
    gradient.Id                 = GetAttributeString( element, "id" );

    gradient.Start.X = GetAttributeFloat( element, "x1", 0.0f );
    gradient.Start.Y = GetAttributeFloat( element, "y1", 0.0f );
    gradient.End.X   = GetAttributeFloat( element, "x2", 1.0f );
    gradient.End.Y   = GetAttributeFloat( element, "y2", 0.0f );

    gradient.GradientUnits     = GetAttributeString( element, "gradientUnits", "objectBoundingBox" );
    gradient.GradientTransform = ParseTransform( GetAttributeString( element, "gradientTransform" ) );

    for ( XMLElement *stop = element->FirstChildElement( "stop" ); stop; stop = stop->NextSiblingElement( "stop" ) )
    {
        ParseGradientStop( stop, gradient.Stops );
    }
}

void SvgLoader::ParseRadialGradient( XMLElement *element ) const
{
    SvgRadialGradient &gradient = m_impl->document.RadialGradients.EmplaceElement( );
    gradient.Id                 = GetAttributeString( element, "id" );

    gradient.Center.X     = GetAttributeFloat( element, "cx", 0.5f );
    gradient.Center.Y     = GetAttributeFloat( element, "cy", 0.5f );
    gradient.FocalPoint.X = GetAttributeFloat( element, "fx", gradient.Center.X );
    gradient.FocalPoint.Y = GetAttributeFloat( element, "fy", gradient.Center.Y );
    gradient.Radius       = GetAttributeFloat( element, "r", 0.5f );

    gradient.GradientUnits     = GetAttributeString( element, "gradientUnits", "objectBoundingBox" );
    gradient.GradientTransform = ParseTransform( GetAttributeString( element, "gradientTransform" ) );

    for ( XMLElement *stop = element->FirstChildElement( "stop" ); stop; stop = stop->NextSiblingElement( "stop" ) )
    {
        ParseGradientStop( stop, gradient.Stops );
    }
}

void SvgLoader::ParseGradientStop( const XMLElement *element, InteropArray<SvgGradientStop> &stops ) const
{
    SvgGradientStop &stop = stops.EmplaceElement( );
    stop.Offset           = GetAttributeFloat( element, "offset", 0.0f );

    const InteropString offsetStr    = GetAttributeString( element, "offset", "0" );
    const std::string   offsetStrStr = offsetStr.Get( );
    if ( offsetStrStr.ends_with( "%" ) )
    {
        stop.Offset /= 100.0f;
    }

    const InteropString stopColor = GetAttributeString( element, "stop-color", "black" );
    stop.Color                    = ParseColor( stopColor );
    stop.Opacity                  = GetAttributeFloat( element, "stop-opacity", 1.0f );
    stop.Color.W *= stop.Opacity; // Apply opacity to alpha
}

SvgStyle SvgLoader::ParseElementStyle( XMLElement *element, const SvgStyle &parentStyle ) const
{
    SvgStyle style = parentStyle;

    const InteropString fill = GetAttributeString( element, "fill" );
    if ( !fill.IsEmpty( ) )
    {
        if ( fill.Equals( InteropString( "none" ) ) )
        {
            style.HasFill = false;
        }
        else
        {
            style.HasFill = true;
            if ( IsGradientUrl( fill ) )
            {
                style.FillGradientId = ExtractGradientId( fill );
                style.FillColor      = { 0.0f, 0.0f, 0.0f, 1.0f }; // Default fallback color
            }
            else
            {
                style.FillColor = ParseColor( fill );
            }
        }
    }

    const InteropString stroke = GetAttributeString( element, "stroke" );
    if ( !stroke.IsEmpty( ) )
    {
        if ( stroke.Equals( InteropString( "none" ) ) )
        {
            style.HasStroke = false;
        }
        else
        {
            style.HasStroke = true;
            if ( IsGradientUrl( stroke ) )
            {
                style.StrokeGradientId = ExtractGradientId( stroke );
                style.StrokeColor      = { 0.0f, 0.0f, 0.0f, 1.0f }; // Default fallback color
            }
            else
            {
                style.StrokeColor = ParseColor( stroke );
            }
        }
    }

    style.StrokeWidth   = GetAttributeFloat( element, "stroke-width", style.StrokeWidth );
    style.FillOpacity   = GetAttributeFloat( element, "fill-opacity", style.FillOpacity );
    style.StrokeOpacity = GetAttributeFloat( element, "stroke-opacity", style.StrokeOpacity );
    style.Opacity       = GetAttributeFloat( element, "opacity", style.Opacity );

    style.StrokeLineCap    = GetAttributeString( element, "stroke-linecap", style.StrokeLineCap );
    style.StrokeLineJoin   = GetAttributeString( element, "stroke-linejoin", style.StrokeLineJoin );
    style.StrokeMiterLimit = GetAttributeFloat( element, "stroke-miterlimit", style.StrokeMiterLimit );
    style.StrokeDashArray  = GetAttributeString( element, "stroke-dasharray", style.StrokeDashArray );
    style.StrokeDashOffset = GetAttributeFloat( element, "stroke-dashoffset", style.StrokeDashOffset );

    style.FillRule   = GetAttributeString( element, "fill-rule", style.FillRule );
    style.Display    = GetAttributeString( element, "display", style.Display );
    style.Visibility = GetAttributeString( element, "visibility", style.Visibility );

    const InteropString styleAttr = GetAttributeString( element, "style" );
    if ( !styleAttr.IsEmpty( ) )
    {
        style = ParseStyleAttribute( styleAttr, style );
    }

    style.FillColor.W *= style.FillOpacity * style.Opacity;
    style.StrokeColor.W *= style.StrokeOpacity * style.Opacity;
    return style;
}

SvgStyle SvgLoader::ParseStyleAttribute( const InteropString &styleString, const SvgStyle &baseStyle ) const
{
    SvgStyle style = baseStyle;
    // Parse CSS-style declarations separated by semicolons
    const InteropString str = styleString;
    const char         *ptr = str.Get( );
    while ( *ptr )
    {
        // Find property name
        const char *propStart = ptr;
        while ( *ptr && *ptr != ':' && *ptr != ';' )
        {
            ptr++;
        }
        if ( *ptr != ':' )
        {
            break;
        }

        const size_t propLen = ptr - propStart;
        const auto   propStr = new char[ propLen + 1 ];
        SafeSubstringCopy( propStr, propStart, propLen, propLen + 1 );

        // Trim whitespace
        char *trimmedProp = propStr;
        while ( *trimmedProp && std::isspace( *trimmedProp ) )
        {
            trimmedProp++;
        }
        char *propEnd = trimmedProp + std::strlen( trimmedProp ) - 1;
        while ( propEnd > trimmedProp && std::isspace( *propEnd ) )
        {
            *propEnd-- = '\0';
        }

        InteropString property( trimmedProp );
        delete[] propStr;
        ptr++; // Skip ':'

        // Find property value
        const char *valueStart = ptr;
        while ( *ptr && *ptr != ';' )
        {
            ptr++;
        }

        // Create value string
        const size_t valueLen = ptr - valueStart;
        const auto   valueStr = new char[ valueLen + 1 ];
        SafeSubstringCopy( valueStr, valueStart, valueLen, valueLen + 1 );

        // Trim whitespace
        char *trimmedValue = valueStr;
        while ( *trimmedValue && std::isspace( *trimmedValue ) )
        {
            trimmedValue++;
        }
        char *valueEnd = trimmedValue + std::strlen( trimmedValue ) - 1;
        while ( valueEnd > trimmedValue && std::isspace( *valueEnd ) )
        {
            *valueEnd-- = '\0';
        }

        InteropString value( trimmedValue );
        delete[] valueStr;

        if ( *ptr == ';' )
        {
            ptr++; // Skip ';'
        }
        // Apply property
        if ( property.Equals( InteropString( "fill" ) ) )
        {
            if ( value.Equals( InteropString( "none" ) ) )
            {
                style.HasFill = false;
            }
            else
            {
                style.HasFill = true;
                if ( IsGradientUrl( value ) )
                {
                    style.FillGradientId = ExtractGradientId( value );
                    style.FillColor      = { 0.0f, 0.0f, 0.0f, 1.0f }; // Default fallback color
                }
                else
                {
                    style.FillColor = ParseColor( value );
                }
            }
        }
        else if ( property.Equals( InteropString( "stroke" ) ) )
        {
            if ( value.Equals( InteropString( "none" ) ) )
            {
                style.HasStroke = false;
            }
            else
            {
                style.HasStroke = true;
                if ( IsGradientUrl( value ) )
                {
                    style.StrokeGradientId = ExtractGradientId( value );
                    style.StrokeColor      = { 0.0f, 0.0f, 0.0f, 1.0f }; // Default fallback color
                }
                else
                {
                    style.StrokeColor = ParseColor( value );
                }
            }
        }
        else if ( property.Equals( InteropString( "stroke-width" ) ) )
        {
            style.StrokeWidth = ParseLength( value );
        }
        else if ( property.Equals( InteropString( "fill-opacity" ) ) )
        {
            style.FillOpacity = std::strtof( value.Get( ), nullptr );
        }
        else if ( property.Equals( InteropString( "stroke-opacity" ) ) )
        {
            style.StrokeOpacity = std::strtof( value.Get( ), nullptr );
        }
        else if ( property.Equals( InteropString( "opacity" ) ) )
        {
            style.Opacity = std::strtof( value.Get( ), nullptr );
        }
    }

    return style;
}

VGPath2D SvgLoader::ParsePathData( const InteropString &pathData ) const
{
    VGPath2D path;
    if ( pathData.IsEmpty( ) )
    {
        return path;
    }

    const char *ptr              = pathData.Get( );
    Float_2     currentPos       = { 0.0f, 0.0f };
    Float_2     lastControlPoint = { 0.0f, 0.0f };

    while ( *ptr )
    {
        while ( *ptr && std::isspace( *ptr ) )
        {
            ptr++;
        }
        if ( !*ptr )
        {
            break;
        }
        const char command = *ptr++;

        float    data[ 6 ] = { };
        uint32_t dataIndex = 0;
        while ( *ptr && dataIndex < 6 )
        {
            while ( *ptr && ( std::isspace( *ptr ) || *ptr == ',' ) )
            {
                ptr++;
            }
            if ( !*ptr || std::isalpha( *ptr ) )
            {
                break;
            }
            data[ dataIndex++ ] = std::strtof( ptr, const_cast<char **>( &ptr ) );
        }

        ProcessPathCommand( path, command, data, dataIndex, currentPos, lastControlPoint );
    }

    return path;
}

void SvgLoader::ProcessPathCommand( const VGPath2D &path, const char command, const float *data, const uint32_t numValues, Float_2 &currentPos, Float_2 &lastControlPoint ) const
{
    switch ( command )
    {
    case 'M': // Move to (absolute)
        if ( numValues >= 2 )
        {
            currentPos = { data[ 0 ], data[ 1 ] };
            path.MoveTo( currentPos );
        }
        break;

    case 'm': // Move to (relative)
        if ( numValues >= 2 )
        {
            currentPos.X += data[ 0 ];
            currentPos.Y += data[ 1 ];
            path.MoveTo( currentPos );
        }
        break;

    case 'L': // Line to (absolute)
        if ( numValues >= 2 )
        {
            currentPos = { data[ 0 ], data[ 1 ] };
            path.LineTo( currentPos );
        }
        break;

    case 'l': // Line to (relative)
        if ( numValues >= 2 )
        {
            currentPos.X += data[ 0 ];
            currentPos.Y += data[ 1 ];
            path.LineTo( currentPos );
        }
        break;

    case 'H': // Horizontal line to (absolute)
        if ( numValues >= 1 )
        {
            currentPos.X = data[ 0 ];
            path.LineTo( currentPos );
        }
        break;

    case 'h': // Horizontal line to (relative)
        if ( numValues >= 1 )
        {
            currentPos.X += data[ 0 ];
            path.LineTo( currentPos );
        }
        break;

    case 'V': // Vertical line to (absolute)
        if ( numValues >= 1 )
        {
            currentPos.Y = data[ 0 ];
            path.LineTo( currentPos );
        }
        break;

    case 'v': // Vertical line to (relative)
        if ( numValues >= 1 )
        {
            currentPos.Y += data[ 0 ];
            path.LineTo( currentPos );
        }
        break;

    case 'Q': // Quadratic curve to (absolute)
        if ( numValues >= 4 )
        {
            const Float_2 controlPoint = { data[ 0 ], data[ 1 ] };
            currentPos                 = { data[ 2 ], data[ 3 ] };
            lastControlPoint           = controlPoint;
            path.QuadraticCurveTo( controlPoint, currentPos );
        }
        break;

    case 'q': // Quadratic curve to (relative)
        if ( numValues >= 4 )
        {
            const Float_2 controlPoint = { currentPos.X + data[ 0 ], currentPos.Y + data[ 1 ] };
            currentPos.X += data[ 2 ];
            currentPos.Y += data[ 3 ];
            lastControlPoint = controlPoint;
            path.QuadraticCurveTo( controlPoint, currentPos );
        }
        break;

    case 'T': // Smooth quadratic curve to (absolute)
        if ( numValues >= 2 )
        {
            currentPos = { data[ 0 ], data[ 1 ] };
            path.SmoothQuadraticCurveTo( currentPos );
        }
        break;

    case 't': // Smooth quadratic curve to (relative)
        if ( numValues >= 2 )
        {
            currentPos.X += data[ 0 ];
            currentPos.Y += data[ 1 ];
            path.SmoothQuadraticCurveTo( currentPos );
        }
        break;

    case 'C': // Cubic curve to (absolute)
        if ( numValues >= 6 )
        {
            const Float_2 controlPoint1 = { data[ 0 ], data[ 1 ] };
            const Float_2 controlPoint2 = { data[ 2 ], data[ 3 ] };
            currentPos                  = { data[ 4 ], data[ 5 ] };
            lastControlPoint            = controlPoint2;
            path.CubicCurveTo( controlPoint1, controlPoint2, currentPos );
        }
        break;

    case 'c': // Cubic curve to (relative)
        if ( numValues >= 6 )
        {
            const Float_2 controlPoint1 = { currentPos.X + data[ 0 ], currentPos.Y + data[ 1 ] };
            const Float_2 controlPoint2 = { currentPos.X + data[ 2 ], currentPos.Y + data[ 3 ] };
            currentPos.X += data[ 4 ];
            currentPos.Y += data[ 5 ];
            lastControlPoint = controlPoint2;
            path.CubicCurveTo( controlPoint1, controlPoint2, currentPos );
        }
        break;

    case 'S': // Smooth cubic curve to (absolute)
        if ( numValues >= 4 )
        {
            const Float_2 controlPoint2 = { data[ 0 ], data[ 1 ] };
            currentPos                  = { data[ 2 ], data[ 3 ] };
            lastControlPoint            = controlPoint2;
            path.SmoothCubicCurveTo( controlPoint2, currentPos );
        }
        break;

    case 's': // Smooth cubic curve to (relative)
        if ( numValues >= 4 )
        {
            const Float_2 controlPoint2 = { currentPos.X + data[ 0 ], currentPos.Y + data[ 1 ] };
            currentPos.X += data[ 2 ];
            currentPos.Y += data[ 3 ];
            lastControlPoint = controlPoint2;
            path.SmoothCubicCurveTo( controlPoint2, currentPos );
        }
        break;

    case 'A': // Elliptical arc to (absolute)
        if ( numValues >= 7 )
        {
            const Float_2 radii         = { data[ 0 ], data[ 1 ] };
            const float   xAxisRotation = data[ 2 ] * static_cast<float>( M_PI ) / 180.0f;
            const bool    largeArcFlag  = data[ 3 ] != 0.0f;
            const bool    sweepFlag     = data[ 4 ] != 0.0f;
            currentPos                  = { data[ 5 ], data[ 6 ] };
            path.EllipticalArcTo( radii, xAxisRotation, largeArcFlag, sweepFlag, currentPos );
        }
        break;

    case 'a': // Elliptical arc to (relative)
        if ( numValues >= 7 )
        {
            const Float_2 radii         = { data[ 0 ], data[ 1 ] };
            const float   xAxisRotation = data[ 2 ] * static_cast<float>( M_PI ) / 180.0f;
            const bool    largeArcFlag  = data[ 3 ] != 0.0f;
            const bool    sweepFlag     = data[ 4 ] != 0.0f;
            const Float_2 endOffset     = { data[ 5 ], data[ 6 ] };
            currentPos.X += endOffset.X;
            currentPos.Y += endOffset.Y;
            path.RelativeEllipticalArcTo( radii, xAxisRotation, largeArcFlag, sweepFlag, endOffset );
        }
        break;

    case 'Z':
    case 'z': // Close path
        path.Close( );
        break;
    default:
        break;
    }
}

SvgTransform SvgLoader::CombineTransforms( const SvgTransform &parent, const SvgTransform &child ) const
{
    if ( !parent.HasTransform && !child.HasTransform )
    {
        SvgTransform result;
        result.HasTransform = false;
        return result;
    }

    if ( !parent.HasTransform )
    {
        return child;
    }

    if ( !child.HasTransform )
    {
        return parent;
    }

    SvgTransform result;
    result.HasTransform = true;

    const XMMATRIX parentMatrix   = XMLoadFloat4x4( reinterpret_cast<const XMFLOAT4X4 *>( &parent.Matrix ) );
    const XMMATRIX childMatrix    = XMLoadFloat4x4( reinterpret_cast<const XMFLOAT4X4 *>( &child.Matrix ) );
    const XMMATRIX combinedMatrix = XMMatrixMultiply( childMatrix, parentMatrix );
    XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &result.Matrix ), combinedMatrix );
    return result;
}

void SvgLoader::ApplyTransform( const VectorGraphics *vectorGraphics, const SvgTransform &transform ) const
{
    if ( transform.HasTransform )
    {
        vectorGraphics->Transform( transform.Matrix );
    }
}

void SvgLoader::RenderCommand( VectorGraphics *vectorGraphics, const SvgRenderCommand &cmd ) const
{
    vectorGraphics->Save( );
    ApplyTransform( vectorGraphics, cmd.Transform );

    if ( cmd.Style.HasFill && cmd.Style.Visibility.Equals( InteropString( "visible" ) ) && !cmd.Style.Display.Equals( InteropString( "none" ) ) )
    {
        vectorGraphics->SetFillEnabled( true );
        if ( !cmd.Style.FillGradientId.IsEmpty( ) )
        {
            ApplyGradientFill( vectorGraphics, cmd.Style.FillGradientId );
        }
        else
        {
            vectorGraphics->SetFillColor( cmd.Style.FillColor );
        }

        if ( cmd.Style.FillRule.Equals( InteropString( "evenodd" ) ) )
        {
            vectorGraphics->SetFillRule( VGFillRule::EvenOdd );
        }
        else
        {
            vectorGraphics->SetFillRule( VGFillRule::NonZero );
        }
    }
    else
    {
        vectorGraphics->SetFillEnabled( false );
    }

    if ( cmd.Style.HasStroke && cmd.Style.Visibility.Equals( InteropString( "visible" ) ) && !cmd.Style.Display.Equals( InteropString( "none" ) ) )
    {
        vectorGraphics->SetStrokeEnabled( true );
        vectorGraphics->SetStrokeColor( cmd.Style.StrokeColor );

        // Stroke width scaling is now handled by VectorGraphics transform system
        vectorGraphics->SetStrokeWidth( cmd.Style.StrokeWidth );

        if ( cmd.Style.StrokeLineCap.Equals( InteropString( "round" ) ) )
        {
            vectorGraphics->SetStrokeLineCap( VGLineCap::Round );
        }
        else if ( cmd.Style.StrokeLineCap.Equals( InteropString( "square" ) ) )
        {
            vectorGraphics->SetStrokeLineCap( VGLineCap::Square );
        }
        else
        {
            vectorGraphics->SetStrokeLineCap( VGLineCap::Butt );
        }

        if ( cmd.Style.StrokeLineJoin.Equals( InteropString( "round" ) ) )
        {
            vectorGraphics->SetStrokeLineJoin( VGLineJoin::Round );
        }
        else if ( cmd.Style.StrokeLineJoin.Equals( InteropString( "bevel" ) ) )
        {
            vectorGraphics->SetStrokeLineJoin( VGLineJoin::Bevel );
        }
        else
        {
            vectorGraphics->SetStrokeLineJoin( VGLineJoin::Miter );
        }

        vectorGraphics->SetStrokeMiterLimit( cmd.Style.StrokeMiterLimit );
        if ( !cmd.Style.StrokeDashArray.IsEmpty( ) )
        {
            InteropArray<float> dashPattern;
            const char         *ptr = cmd.Style.StrokeDashArray.Get( );
            while ( *ptr )
            {
                while ( *ptr && ( std::isspace( *ptr ) || *ptr == ',' ) )
                {
                    ptr++;
                }
                if ( !*ptr )
                {
                    break;
                }

                const float value = std::strtof( ptr, const_cast<char **>( &ptr ) );
                if ( value > 0.0f ) // Only add positive values
                {
                    dashPattern.AddElement( value );
                }
            }
            if ( dashPattern.NumElements( ) > 0 )
            {
                vectorGraphics->SetStrokeDashPattern( dashPattern, cmd.Style.StrokeDashOffset );
            }
        }
    }
    else
    {
        vectorGraphics->SetStrokeEnabled( false );
    }

    switch ( cmd.Type )
    {
    case SvgRenderCommand::RectCommand:
        {
            if ( cmd.RectData.CornerRadii.X > 0.0f || cmd.RectData.CornerRadii.Y > 0.0f )
            {
                VGRoundedRect roundedRect;
                roundedRect.TopLeft     = cmd.RectData.Rect.TopLeft;
                roundedRect.BottomRight = cmd.RectData.Rect.BottomRight;
                roundedRect.CornerRadii = cmd.RectData.CornerRadii;
                vectorGraphics->DrawRoundedRect( roundedRect );
            }
            else
            {
                vectorGraphics->DrawRect( cmd.RectData.Rect );
            }
        }
        break;

    case SvgRenderCommand::CircleCommand:
        {
            vectorGraphics->DrawCircle( cmd.CircleData.Circle );
        }
        break;

    case SvgRenderCommand::EllipseCommand:
        {
            vectorGraphics->DrawEllipse( cmd.EllipseData.Ellipse );
        }
        break;

    case SvgRenderCommand::LineCommand:
        {
            vectorGraphics->DrawLine( cmd.LineData.Line );
        }
        break;

    case SvgRenderCommand::PolygonCommand:
        {
            vectorGraphics->DrawPolygon( cmd.PolygonData.Polygon );
        }
        break;

    case SvgRenderCommand::PathCommand:
        if ( cmd.PathData.Path )
        {
            vectorGraphics->DrawPath( *cmd.PathData.Path );
        }
        break;

    case SvgRenderCommand::TextCommand:
        if ( m_impl->options.LoadText )
        {
            vectorGraphics->DrawText( cmd.TextData.Text, cmd.TextData.Position, cmd.TextData.FontSize );
        }
        break;
    }

    vectorGraphics->Restore( );
}

InteropString SvgLoader::GetAttributeString( const XMLElement *element, const char *name, const InteropString &defaultValue ) const
{
    if ( !element )
    {
        return defaultValue;
    }

    const char *value = element->Attribute( name );
    return value ? InteropString( value ) : defaultValue;
}

float SvgLoader::GetAttributeFloat( const XMLElement *element, const char *name, const float defaultValue ) const
{
    if ( !element )
    {
        return defaultValue;
    }

    const char *value = element->Attribute( name );
    return value ? std::strtof( value, nullptr ) : defaultValue;
}

bool SvgLoader::GetAttributeBool( const XMLElement *element, const char *name, const bool defaultValue ) const
{
    if ( !element )
    {
        return defaultValue;
    }

    const char *value = element->Attribute( name );
    if ( !value )
    {
        return defaultValue;
    }

    InteropString str( value );
    str = str.ToLower( );
    return str.Equals( InteropString( "true" ) ) || str.Equals( InteropString( "1" ) ) || str.Equals( InteropString( "yes" ) );
}

void SvgLoader::ApplyGradientFill( VectorGraphics *vectorGraphics, const InteropString &gradientId ) const
{
    if ( !vectorGraphics || gradientId.IsEmpty( ) )
    {
        return;
    }

    // Search for linear gradient first
    for ( uint32_t i = 0; i < m_impl->document.LinearGradients.NumElements( ); ++i )
    {
        const SvgLinearGradient &gradient = m_impl->document.LinearGradients.GetElement( i );
        if ( gradient.Id.Equals( gradientId ) )
        {
            InteropArray<VGGradientStop> stops;
            for ( uint32_t j = 0; j < gradient.Stops.NumElements( ); ++j )
            {
                const SvgGradientStop &svgStop = gradient.Stops.GetElement( j );
                VGGradientStop        &vgStop  = stops.EmplaceElement( );
                vgStop.Position                = svgStop.Offset; // Use Position instead of Offset
                vgStop.Color                   = svgStop.Color;
            }
            vectorGraphics->SetFillLinearGradient( gradient.Start, gradient.End, stops );
            return;
        }
    }

    // Search for radial gradient
    for ( uint32_t i = 0; i < m_impl->document.RadialGradients.NumElements( ); ++i )
    {
        const SvgRadialGradient &gradient = m_impl->document.RadialGradients.GetElement( i );
        if ( gradient.Id.Equals( gradientId ) )
        {
            InteropArray<VGGradientStop> stops;
            for ( uint32_t j = 0; j < gradient.Stops.NumElements( ); ++j )
            {
                const SvgGradientStop &svgStop = gradient.Stops.GetElement( j );
                VGGradientStop        &vgStop  = stops.EmplaceElement( );
                vgStop.Position                = svgStop.Offset; // Use Position instead of Offset
                vgStop.Color                   = svgStop.Color;
            }
            vectorGraphics->SetFillRadialGradient( gradient.Center, gradient.Radius, stops );
            return;
        }
    }
    // Gradient not found - fall back to solid color
    vectorGraphics->SetFillColor( { 0.0f, 0.0f, 0.0f, 1.0f } );
}

bool SvgLoader::ParsePathCommand( const InteropString &command, float *data, uint32_t &dataIndex, const uint32_t maxData ) const
{
    if ( command.IsEmpty( ) || !data || dataIndex >= maxData )
    {
        return false;
    }

    const char *ptr = command.Get( );
    while ( *ptr && std::isspace( *ptr ) )
    {
        ptr++;
    }

    if ( !*ptr )
    {
        return false;
    }

    *ptr++; // Parse the command character
    while ( *ptr && dataIndex < maxData )
    {
        while ( *ptr && ( std::isspace( *ptr ) || *ptr == ',' ) )
        {
            ptr++;
        }
        if ( !*ptr || std::isalpha( *ptr ) )
        {
            break;
        }

        char *endPtr;
        data[ dataIndex ] = std::strtof( ptr, &endPtr );

        if ( endPtr == ptr )
        {
            break; // No number found
        }
        ptr = endPtr;
        dataIndex++;
    }
    return true;
}

Float_2 SvgLoader::ViewBoxToPixel( const Float_2 &viewBoxCoord ) const
{
    if ( !m_impl->document.HasViewBox )
    {
        return viewBoxCoord;
    }

    const SvgViewBox &viewBox = m_impl->document.ViewBox;
    const Float_2    &docSize = m_impl->document.Size;
    if ( viewBox.Width <= 0.001f || viewBox.Height <= 0.001f )
    {
        return viewBoxCoord;
    }

    const float scaleX = docSize.X / viewBox.Width;
    const float scaleY = docSize.Y / viewBox.Height;

    Float_2 pixelCoord;
    pixelCoord.X = ( viewBoxCoord.X - viewBox.X ) * scaleX;
    pixelCoord.Y = ( viewBoxCoord.Y - viewBox.Y ) * scaleY;
    return pixelCoord;
}

float SvgLoader::ViewBoxToPixelScale( ) const
{
    if ( !m_impl->document.HasViewBox )
    {
        return 1.0f;
    }

    const SvgViewBox &viewBox = m_impl->document.ViewBox;
    const Float_2    &docSize = m_impl->document.Size;

    if ( viewBox.Width <= 0.001f || viewBox.Height <= 0.001f )
    {
        return 1.0f;
    }

    const float scaleX = docSize.X / viewBox.Width;
    const float scaleY = docSize.Y / viewBox.Height;
    return std::min( scaleX, scaleY );
}

void SvgLoader::SetError( const InteropString &error ) const
{
    if ( m_impl )
    {
        m_impl->SetError( error );
    }
}

bool SvgLoader::IsGradientUrl( const InteropString &value ) const
{
    if ( value.IsEmpty( ) )
    {
        return false;
    }

    const char *str = value.Get( );
    return StringStartsWith( str, "url(#" );
}

InteropString SvgLoader::ExtractGradientId( const InteropString &url ) const
{
    if ( !IsGradientUrl( url ) )
    {
        return InteropString( );
    }

    const char *str   = url.Get( );
    const char *start = str + 5; // Skip "url(#"
    const char *end   = std::strchr( start, ')' );

    if ( !end )
    {
        return InteropString( ); // Invalid format
    }

    const size_t idLen = end - start;
    if ( idLen == 0 )
    {
        return InteropString( ); // Empty ID
    }

    const auto idStr = new char[ idLen + 1 ];
    SafeSubstringCopy( idStr, start, idLen, idLen + 1 );

    InteropString result( idStr );
    delete[] idStr;

    return result;
}
