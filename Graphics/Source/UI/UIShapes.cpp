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

#include <DenOfIzGraphics/UI/UIShapes.h>
#include <cmath>

using namespace DenOfIz;
using namespace DirectX;

void UIShapes::GenerateRectangle( const GenerateRectangleDesc &desc, InteropArray<UIVertex> *outVertices, InteropArray<uint32_t> *outIndices, const uint32_t baseVertex )
{
    const auto  color  = ClayColorToFloat4( desc.Color );
    const auto &bounds = desc.Bounds;

    const uint32_t startVertex = outVertices->NumElements( );
    AddVertex( outVertices, bounds.x, bounds.y, 0.0f, 0.0f, 0.0f, color, desc.TextureIndex );
    AddVertex( outVertices, bounds.x + bounds.width, bounds.y, 0.0f, 1.0f, 0.0f, color, desc.TextureIndex );
    AddVertex( outVertices, bounds.x + bounds.width, bounds.y + bounds.height, 0.0f, 1.0f, 1.0f, color, desc.TextureIndex );
    AddVertex( outVertices, bounds.x, bounds.y + bounds.height, 0.0f, 0.0f, 1.0f, color, desc.TextureIndex );

    AddQuad( outIndices, baseVertex + startVertex, baseVertex + startVertex + 1, baseVertex + startVertex + 2, baseVertex + startVertex + 3 );
}

void UIShapes::GenerateRoundedRectangle( const GenerateRoundedRectangleDesc &desc, InteropArray<UIVertex> *outVertices, InteropArray<uint32_t> *outIndices,
                                         const uint32_t baseVertex )
{
    const auto  color        = ClayColorToFloat4( desc.Color );
    const auto &bounds       = desc.Bounds;
    const auto &cornerRadius = desc.CornerRadius;

    const float minDimension = std::min( bounds.width, bounds.height ) / 2.0f;
    const float radiusTL     = std::min( cornerRadius.topLeft, minDimension );
    const float radiusTR     = std::min( cornerRadius.topRight, minDimension );
    const float radiusBR     = std::min( cornerRadius.bottomRight, minDimension );
    const float radiusBL     = std::min( cornerRadius.bottomLeft, minDimension );

    const uint32_t segments = desc.SegmentsPerCorner;
    const uint32_t centerTL = outVertices->NumElements( );
    AddVertex( outVertices, bounds.x + radiusTL, bounds.y + radiusTL, 0.0f, 0.5f, 0.5f, color, desc.TextureIndex );

    const uint32_t centerTR = outVertices->NumElements( );
    AddVertex( outVertices, bounds.x + bounds.width - radiusTR, bounds.y + radiusTR, 0.0f, 0.5f, 0.5f, color, desc.TextureIndex );

    const uint32_t centerBR = outVertices->NumElements( );
    AddVertex( outVertices, bounds.x + bounds.width - radiusBR, bounds.y + bounds.height - radiusBR, 0.0f, 0.5f, 0.5f, color, desc.TextureIndex );

    const uint32_t centerBL = outVertices->NumElements( );
    AddVertex( outVertices, bounds.x + radiusBL, bounds.y + bounds.height - radiusBL, 0.0f, 0.5f, 0.5f, color, desc.TextureIndex );
    AddQuad( outIndices, baseVertex + centerTL, baseVertex + centerTR, baseVertex + centerBR, baseVertex + centerBL );

    const float angleStep = XM_PIDIV2 / static_cast<float>( segments );
    // Top-left corner
    if ( radiusTL > 0 )
    {
        for ( uint32_t i = 0; i <= segments; ++i )
        {
            const float angle = XM_PI + i * angleStep;
            const float x     = bounds.x + radiusTL + std::cos( angle ) * radiusTL;
            const float y     = bounds.y + radiusTL + std::sin( angle ) * radiusTL;
            AddVertex( outVertices, x, y, 0.0f, 0.5f, 0.5f, color, desc.TextureIndex );

            if ( i > 0 )
            {
                AddTriangle( outIndices, baseVertex + centerTL, baseVertex + outVertices->NumElements( ) - 2, baseVertex + outVertices->NumElements( ) - 1 );
            }
        }
    }
    if ( radiusTR > 0 )
    {
        for ( uint32_t i = 0; i <= segments; ++i )
        {
            const float angle = XM_PI + XM_PIDIV2 + i * angleStep;
            const float x     = bounds.x + bounds.width - radiusTR + std::cos( angle ) * radiusTR;
            const float y     = bounds.y + radiusTR + std::sin( angle ) * radiusTR;
            AddVertex( outVertices, x, y, 0.0f, 0.5f, 0.5f, color, desc.TextureIndex );

            if ( i > 0 )
            {
                AddTriangle( outIndices, baseVertex + centerTR, baseVertex + outVertices->NumElements( ) - 2, baseVertex + outVertices->NumElements( ) - 1 );
            }
        }
    }
    if ( radiusBR > 0 )
    {
        for ( uint32_t i = 0; i <= segments; ++i )
        {
            const float angle = i * angleStep;
            const float x     = bounds.x + bounds.width - radiusBR + std::cos( angle ) * radiusBR;
            const float y     = bounds.y + bounds.height - radiusBR + std::sin( angle ) * radiusBR;
            AddVertex( outVertices, x, y, 0.0f, 0.5f, 0.5f, color, desc.TextureIndex );

            if ( i > 0 )
            {
                AddTriangle( outIndices, baseVertex + centerBR, baseVertex + outVertices->NumElements( ) - 2, baseVertex + outVertices->NumElements( ) - 1 );
            }
        }
    }
    if ( radiusBL > 0 )
    {
        for ( uint32_t i = 0; i <= segments; ++i )
        {
            const float angle = XM_PIDIV2 + i * angleStep;
            const float x     = bounds.x + radiusBL + std::cos( angle ) * radiusBL;
            const float y     = bounds.y + bounds.height - radiusBL + std::sin( angle ) * radiusBL;
            AddVertex( outVertices, x, y, 0.0f, 0.5f, 0.5f, color, desc.TextureIndex );

            if ( i > 0 )
            {
                AddTriangle( outIndices, baseVertex + centerBL, baseVertex + outVertices->NumElements( ) - 2, baseVertex + outVertices->NumElements( ) - 1 );
            }
        }
    }

    if ( radiusTL > 0 || radiusTR > 0 )
    {
        const uint32_t tlEdge = outVertices->NumElements( );
        AddVertex( outVertices, bounds.x + radiusTL, bounds.y, 0.0f, 0.5f, 0.0f, color, desc.TextureIndex );
        const uint32_t trEdge = outVertices->NumElements( );
        AddVertex( outVertices, bounds.x + bounds.width - radiusTR, bounds.y, 0.0f, 0.5f, 0.0f, color, desc.TextureIndex );

        AddQuad( outIndices, baseVertex + tlEdge, baseVertex + trEdge, baseVertex + centerTR, baseVertex + centerTL );
    }
    if ( radiusTR > 0 || radiusBR > 0 )
    {
        const uint32_t rtEdge = outVertices->NumElements( );
        AddVertex( outVertices, bounds.x + bounds.width, bounds.y + radiusTR, 0.0f, 1.0f, 0.5f, color, desc.TextureIndex );
        const uint32_t rbEdge = outVertices->NumElements( );
        AddVertex( outVertices, bounds.x + bounds.width, bounds.y + bounds.height - radiusBR, 0.0f, 1.0f, 0.5f, color, desc.TextureIndex );

        AddQuad( outIndices, baseVertex + centerTR, baseVertex + rtEdge, baseVertex + rbEdge, baseVertex + centerBR );
    }
    if ( radiusBL > 0 || radiusBR > 0 )
    {
        const uint32_t brEdge = outVertices->NumElements( );
        AddVertex( outVertices, bounds.x + bounds.width - radiusBR, bounds.y + bounds.height, 0.0f, 0.5f, 1.0f, color, desc.TextureIndex );
        const uint32_t blEdge = outVertices->NumElements( );
        AddVertex( outVertices, bounds.x + radiusBL, bounds.y + bounds.height, 0.0f, 0.5f, 1.0f, color, desc.TextureIndex );

        AddQuad( outIndices, baseVertex + centerBR, baseVertex + brEdge, baseVertex + blEdge, baseVertex + centerBL );
    }
    if ( radiusTL > 0 || radiusBL > 0 )
    {
        const uint32_t lbEdge = outVertices->NumElements( );
        AddVertex( outVertices, bounds.x, bounds.y + bounds.height - radiusBL, 0.0f, 0.0f, 0.5f, color, desc.TextureIndex );
        const uint32_t ltEdge = outVertices->NumElements( );
        AddVertex( outVertices, bounds.x, bounds.y + radiusTL, 0.0f, 0.0f, 0.5f, color, desc.TextureIndex );

        AddQuad( outIndices, baseVertex + centerBL, baseVertex + lbEdge, baseVertex + ltEdge, baseVertex + centerTL );
    }
}

void UIShapes::GenerateBorder( const GenerateBorderDesc &desc, InteropArray<UIVertex> *outVertices, InteropArray<uint32_t> *outIndices, const uint32_t baseVertex )
{
    const auto  color        = ClayColorToFloat4( desc.Color );
    const auto &bounds       = desc.Bounds;
    const auto &borderWidth  = desc.BorderWidth;
    const auto &cornerRadius = desc.CornerRadius;

    const float minDimension = std::min( bounds.width, bounds.height ) / 2.0f;
    const float radiusTL     = std::min( cornerRadius.topLeft, minDimension );
    const float radiusTR     = std::min( cornerRadius.topRight, minDimension );
    const float radiusBR     = std::min( cornerRadius.bottomRight, minDimension );
    const float radiusBL     = std::min( cornerRadius.bottomLeft, minDimension );

    const uint32_t segments = desc.SegmentsPerCorner;
    auto generateArc = [ & ]( const float centerX, const float centerY, const float outerRadius, const float innerRadius, const float startAngle, const float endAngle ) -> void
    {
        const uint32_t startVertex = outVertices->NumElements( );
        for ( uint32_t i = 0; i <= segments; ++i )
        {
            const float angle = startAngle + ( endAngle - startAngle ) * ( i / static_cast<float>( segments ) );
            const float cosA  = std::cos( angle );
            const float sinA  = std::sin( angle );

            AddVertex( outVertices, centerX + cosA * outerRadius, centerY + sinA * outerRadius, 0.0f, 0.5f, 0.5f, color, 0 );
            AddVertex( outVertices, centerX + cosA * innerRadius, centerY + sinA * innerRadius, 0.0f, 0.5f, 0.5f, color, 0 );
        }

        for ( uint32_t i = 0; i < segments; ++i )
        {
            const uint32_t outerCurrent = startVertex + i * 2;
            const uint32_t innerCurrent = outerCurrent + 1;
            const uint32_t outerNext    = outerCurrent + 2;
            const uint32_t innerNext    = outerNext + 1;

            AddQuad( outIndices, baseVertex + outerCurrent, baseVertex + outerNext, baseVertex + innerNext, baseVertex + innerCurrent );
        }
    };

    if ( borderWidth.top > 0 )
    {
        const float leftX   = bounds.x + radiusTL;
        const float rightX  = bounds.x + bounds.width - radiusTR;
        const float topY    = bounds.y;
        const float bottomY = bounds.y + borderWidth.top;

        const uint32_t tl = outVertices->NumElements( );
        AddVertex( outVertices, leftX, topY, 0.0f, 0.5f, 0.0f, color, 0 );
        const uint32_t tr = outVertices->NumElements( );
        AddVertex( outVertices, rightX, topY, 0.0f, 0.5f, 0.0f, color, 0 );
        const uint32_t br = outVertices->NumElements( );
        AddVertex( outVertices, rightX, bottomY, 0.0f, 0.5f, 1.0f, color, 0 );
        const uint32_t bl = outVertices->NumElements( );
        AddVertex( outVertices, leftX, bottomY, 0.0f, 0.5f, 1.0f, color, 0 );

        AddQuad( outIndices, baseVertex + tl, baseVertex + tr, baseVertex + br, baseVertex + bl );
    }

    if ( borderWidth.right > 0 )
    {
        const float leftX   = bounds.x + bounds.width - borderWidth.right;
        const float rightX  = bounds.x + bounds.width;
        const float topY    = bounds.y + radiusTR;
        const float bottomY = bounds.y + bounds.height - radiusBR;

        const uint32_t tl = outVertices->NumElements( );
        AddVertex( outVertices, leftX, topY, 0.0f, 0.0f, 0.5f, color, 0 );
        const uint32_t tr = outVertices->NumElements( );
        AddVertex( outVertices, rightX, topY, 0.0f, 1.0f, 0.5f, color, 0 );
        const uint32_t br = outVertices->NumElements( );
        AddVertex( outVertices, rightX, bottomY, 0.0f, 1.0f, 0.5f, color, 0 );
        const uint32_t bl = outVertices->NumElements( );
        AddVertex( outVertices, leftX, bottomY, 0.0f, 0.0f, 0.5f, color, 0 );

        AddQuad( outIndices, baseVertex + tl, baseVertex + tr, baseVertex + br, baseVertex + bl );
    }

    if ( borderWidth.bottom > 0 )
    {
        const float leftX   = bounds.x + radiusBL;
        const float rightX  = bounds.x + bounds.width - radiusBR;
        const float topY    = bounds.y + bounds.height - borderWidth.bottom;
        const float bottomY = bounds.y + bounds.height;

        const uint32_t tl = outVertices->NumElements( );
        AddVertex( outVertices, leftX, topY, 0.0f, 0.5f, 0.0f, color, 0 );
        const uint32_t tr = outVertices->NumElements( );
        AddVertex( outVertices, rightX, topY, 0.0f, 0.5f, 0.0f, color, 0 );
        const uint32_t br = outVertices->NumElements( );
        AddVertex( outVertices, rightX, bottomY, 0.0f, 0.5f, 1.0f, color, 0 );
        const uint32_t bl = outVertices->NumElements( );
        AddVertex( outVertices, leftX, bottomY, 0.0f, 0.5f, 1.0f, color, 0 );

        AddQuad( outIndices, baseVertex + tl, baseVertex + tr, baseVertex + br, baseVertex + bl );
    }

    if ( borderWidth.left > 0 )
    {
        const float leftX   = bounds.x;
        const float rightX  = bounds.x + borderWidth.left;
        const float topY    = bounds.y + radiusTL;
        const float bottomY = bounds.y + bounds.height - radiusBL;

        const uint32_t tl = outVertices->NumElements( );
        AddVertex( outVertices, leftX, topY, 0.0f, 0.0f, 0.5f, color, 0 );
        const uint32_t tr = outVertices->NumElements( );
        AddVertex( outVertices, rightX, topY, 0.0f, 1.0f, 0.5f, color, 0 );
        const uint32_t br = outVertices->NumElements( );
        AddVertex( outVertices, rightX, bottomY, 0.0f, 1.0f, 0.5f, color, 0 );
        const uint32_t bl = outVertices->NumElements( );
        AddVertex( outVertices, leftX, bottomY, 0.0f, 0.0f, 0.5f, color, 0 );

        AddQuad( outIndices, baseVertex + tl, baseVertex + tr, baseVertex + br, baseVertex + bl );
    }

    if ( radiusTL > 0 && ( borderWidth.top > 0 || borderWidth.left > 0 ) )
    {
        const float thickness = std::max( borderWidth.top, borderWidth.left );
        generateArc( bounds.x + radiusTL, bounds.y + radiusTL, radiusTL, radiusTL - thickness, XM_PI, XM_PI + XM_PIDIV2 );
    }
    if ( radiusTR > 0 && ( borderWidth.top > 0 || borderWidth.right > 0 ) )
    {
        const float thickness = std::max( borderWidth.top, borderWidth.right );
        generateArc( bounds.x + bounds.width - radiusTR, bounds.y + radiusTR, radiusTR, radiusTR - thickness, XM_PI + XM_PIDIV2, XM_2PI );
    }
    if ( radiusBR > 0 && ( borderWidth.bottom > 0 || borderWidth.right > 0 ) )
    {
        const float thickness = std::max( borderWidth.bottom, borderWidth.right );
        generateArc( bounds.x + bounds.width - radiusBR, bounds.y + bounds.height - radiusBR, radiusBR, radiusBR - thickness, 0, XM_PIDIV2 );
    }
    if ( radiusBL > 0 && ( borderWidth.bottom > 0 || borderWidth.left > 0 ) )
    {
        const float thickness = std::max( borderWidth.bottom, borderWidth.left );
        generateArc( bounds.x + radiusBL, bounds.y + bounds.height - radiusBL, radiusBL, radiusBL - thickness, XM_PIDIV2, XM_PI );
    }
}

XMFLOAT4 UIShapes::ClayColorToFloat4( const Clay_Color &color )
{
    return XMFLOAT4( color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f );
}

void UIShapes::AddVertex( InteropArray<UIVertex> *vertices, const float x, const float y, const float z, const float u, const float v, const XMFLOAT4 &color,
                          const uint32_t textureIndex )
{
    UIVertex vertex;
    vertex.Position     = XMFLOAT3( x, y, z );
    vertex.TexCoord     = XMFLOAT2( u, v );
    vertex.Color        = color;
    vertex.TextureIndex = textureIndex;

    vertices->AddElement( vertex );
}

void UIShapes::AddTriangle( InteropArray<uint32_t> *indices, const uint32_t v0, const uint32_t v1, const uint32_t v2 )
{
    indices->AddElement( v0 );
    indices->AddElement( v1 );
    indices->AddElement( v2 );
}

void UIShapes::AddQuad( InteropArray<uint32_t> *indices, const uint32_t topLeft, const uint32_t topRight, const uint32_t bottomRight, const uint32_t bottomLeft )
{
    AddTriangle( indices, topLeft, topRight, bottomRight );
    AddTriangle( indices, topLeft, bottomRight, bottomLeft );
}
