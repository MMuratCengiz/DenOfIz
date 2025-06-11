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

#include "DenOfIzGraphics/UI/Widgets/Widget.h"
#include "DenOfIzGraphicsInternal/UI/UIShapes.h"

using namespace DenOfIz;

Widget::Widget( IClayContext *clayContext, const uint32_t id ) : m_id( id ), m_clayContext( clayContext )
{
    m_renderTargets.resize( m_numFrames );
}

Widget::~Widget( )
{
}

bool Widget::HasPipeline( ) const
{
    return m_hasPipeline;
}
void Widget::InitializeRenderResources( ILogicalDevice *device, uint32_t width, uint32_t height )
{
}

void Widget::ResizeRenderResources( uint32_t width, uint32_t height )
{
}
void Widget::ExecuteCustomPipeline( const WidgetExecutePipelineDesc &context )
{
}

ITextureResource *Widget::GetRenderTarget( const uint32_t frameIndex ) const
{
    return m_renderTargets[ frameIndex ].get( );
}

void Widget::SetTextureIndex( const uint32_t index )
{
    m_textureIndex = index;
}
uint32_t Widget::GetTextureIndex( ) const
{
    return m_textureIndex;
}

uint32_t Widget::GetId( ) const
{
    return m_id;
}

bool Widget::IsHovered( ) const
{
    return m_isHovered;
}

bool Widget::IsFocused( ) const
{
    return m_isFocused;
}

void Widget::UpdateHoverState( const bool hovered )
{
    m_isHovered = hovered;
}

void Widget::UpdateHoverState( )
{
    m_isHovered = m_clayContext->PointerOver( m_id );
}

ClayBoundingBox Widget::GetBoundingBox( ) const
{
    return m_clayContext->GetElementBoundingBox( m_id );
}

void Widget::AddRectangle( IRenderBatch *renderBatch, const ClayBoundingBox &bounds, const ClayColor &color, const ClayCornerRadius &cornerRadius ) const
{
    std::vector<UIVertex> vertices;
    std::vector<uint32_t> indices;

    if ( cornerRadius.TopLeft == 0 && cornerRadius.TopRight == 0 && cornerRadius.BottomLeft == 0 && cornerRadius.BottomRight == 0 )
    {
        UIShapes::GenerateRectangleDesc desc;
        desc.Bounds       = Clay_BoundingBox{ bounds.X, bounds.Y, bounds.Width, bounds.Height };
        desc.Color        = Clay_Color{ color.R, color.G, color.B, color.A };
        desc.TextureIndex = 0;
        UIShapes::GenerateRectangle( desc, &vertices, &indices, 0 );
    }
    else
    {
        UIShapes::GenerateRoundedRectangleDesc desc;
        desc.Bounds       = Clay_BoundingBox{ bounds.X, bounds.Y, bounds.Width, bounds.Height };
        desc.Color        = Clay_Color{ color.R, color.G, color.B, color.A };
        desc.CornerRadius = Clay_CornerRadius{ cornerRadius.TopLeft, cornerRadius.TopRight, cornerRadius.BottomLeft, cornerRadius.BottomRight };
        desc.TextureIndex = 0;
        UIShapes::GenerateRoundedRectangle( desc, &vertices, &indices, 0 );
    }

    UIVertexArray vertexArray{ };
    vertexArray.Elements    = vertices.data( );
    vertexArray.NumElements = static_cast<uint32_t>( vertices.size( ) );

    UInt32Array indexArray{ };
    indexArray.Elements    = indices.data( );
    indexArray.NumElements = static_cast<uint32_t>( indices.size( ) );
    renderBatch->AddVertices( vertexArray, indexArray );
}

void Widget::AddBorder( IRenderBatch *renderBatch, const ClayBoundingBox &bounds, const ClayColor &color, const ClayBorderWidth &width, const ClayCornerRadius &cornerRadius ) const
{
    std::vector<UIVertex> vertices;
    std::vector<uint32_t> indices;

    UIShapes::GenerateBorderDesc desc;
    desc.Bounds       = Clay_BoundingBox{ bounds.X, bounds.Y, bounds.Width, bounds.Height };
    desc.Color        = Clay_Color{ color.R, color.G, color.B, color.A };
    desc.BorderWidth  = Clay_BorderWidth{ width.Left, width.Right, width.Top, width.Bottom, width.BetweenChildren };
    desc.CornerRadius = Clay_CornerRadius{ cornerRadius.TopLeft, cornerRadius.TopRight, cornerRadius.BottomLeft, cornerRadius.BottomRight };
    UIShapes::GenerateBorder( desc, &vertices, &indices, 0 );

    UIVertexArray vertexArray{ };
    vertexArray.Elements    = vertices.data( );
    vertexArray.NumElements = static_cast<uint32_t>( vertices.size( ) );

    UInt32Array indexArray{ };
    indexArray.Elements    = indices.data( );
    indexArray.NumElements = static_cast<uint32_t>( indices.size( ) );
    renderBatch->AddVertices( vertexArray, indexArray );
}
