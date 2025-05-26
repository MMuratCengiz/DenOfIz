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

#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include <DenOfIzGraphics/UI/ClayRenderer.h>
#include <DenOfIzGraphics/UI/ClayWrapper.h>
#include <DenOfIzGraphics/Utilities/InteropMathConverter.h>
#include <DenOfIzGraphics/Utilities/Utilities.h>
#include <cstring>
#include <functional>

using namespace DenOfIz;
using namespace DirectX;

ClayRenderer::ClayRenderer( const ClayRendererDesc &desc ) : m_desc( desc )
{
    DZ_NOT_NULL( desc.LogicalDevice );
    DZ_NOT_NULL( desc.TextRenderer );

    QuadRendererDesc quadDesc;
    quadDesc.LogicalDevice      = m_desc.LogicalDevice;
    quadDesc.RenderTargetFormat = m_desc.RenderTargetFormat;
    quadDesc.NumFrames          = m_desc.NumFrames;
    quadDesc.BatchSize          = 1024;
    quadDesc.MaxNumMaterials    = m_desc.MaxNumMaterials;
    quadDesc.MaxNumQuads        = m_desc.MaxNumQuads;

    m_quadRenderer = std::make_unique<QuadRenderer>( quadDesc );

    ThorVGCanvasDesc canvasDesc;
    canvasDesc.Width  = m_desc.Width;
    canvasDesc.Height = m_desc.Height;
    m_vectorCanvas    = std::make_unique<ThorVGCanvas>( canvasDesc );
}

ClayRenderer::~ClayRenderer( )
{
    for ( auto &cache : m_shapeCache | std::views::values )
    {
        if ( cache.Texture )
        {
            delete cache.Texture;
            cache.Texture = nullptr;
        }
    }

    m_quadRenderer.reset( );
    m_vectorCanvas.reset( );
    m_shapeCache.clear( );
}

void ClayRenderer::SetViewportSize( const float width, const float height )
{
    if ( m_viewportWidth != width || m_viewportHeight != height )
    {
        m_quadRenderer->SetCanvas( static_cast<uint32_t>( width ), static_cast<uint32_t>( height ) );
        m_viewportWidth  = width;
        m_viewportHeight = height;
        m_needsClear     = true;
    }
}

void ClayRenderer::ClearCaches( )
{
    m_quadRenderer->ClearQuads( );
    m_quadRenderer->ClearMaterials( );

    for ( auto &cache : m_shapeCache | std::views::values )
    {
        if ( cache.Texture )
        {
            delete cache.Texture;
            cache.Texture = nullptr;
        }
    }
    m_shapeCache.clear( );
    m_nextMaterialId = 0;
    m_nextQuadId     = 0;
}

void ClayRenderer::InvalidateLayout( )
{
    m_needsClear = true;
}

void ClayRenderer::Render( ICommandList *commandList, const InteropArray<ClayRenderCommand> &commands, const uint32_t frameIndex )
{
    DZ_NOT_NULL( commandList );

    if ( m_needsClear )
    {
        ClearCaches( );
        m_needsClear = false;
    }

    // Reset frame indices to reuse existing quads/materials
    m_currentFrameQuadIndex     = 0;
    m_currentFrameMaterialIndex = 0;

    for ( int i = 0; i < commands.NumElements( ); ++i )
    {
        const auto &cmd = commands.GetElement( i );
        switch ( cmd.CommandType )
        {
        case ClayRenderCommandType::Rectangle:
            {
                if ( cmd.RenderData.Rectangle.CornerRadius.TopLeft > 0 || cmd.RenderData.Rectangle.CornerRadius.TopRight > 0 ||
                     cmd.RenderData.Rectangle.CornerRadius.BottomLeft > 0 || cmd.RenderData.Rectangle.CornerRadius.BottomRight > 0 )
                {
                    RenderRoundedRectangle( commandList, cmd.BoundingBox, cmd.RenderData.Rectangle, frameIndex );
                }
                else
                {
                    RenderRectangle( commandList, cmd.BoundingBox, cmd.RenderData.Rectangle, frameIndex );
                }
                break;
            }
        case ClayRenderCommandType::Border:
            {
                RenderBorder( commandList, cmd.BoundingBox, cmd.RenderData.Border, frameIndex );
                break;
            }
        case ClayRenderCommandType::Text:
            {
                RenderText( commandList, cmd.BoundingBox, cmd.RenderData.Text, frameIndex );
                break;
            }
        case ClayRenderCommandType::Image:
            {
                RenderImage( commandList, cmd.BoundingBox, cmd.RenderData.Image, frameIndex );
                break;
            }
        case ClayRenderCommandType::ScissorStart:
            {
                commandList->BindScissorRect( static_cast<int32_t>( cmd.BoundingBox.X ), static_cast<int32_t>( cmd.BoundingBox.Y ), static_cast<int32_t>( cmd.BoundingBox.Width ),
                                              static_cast<int32_t>( cmd.BoundingBox.Height ) );
                break;
            }
        case ClayRenderCommandType::ScissorEnd:
            {
                break;
            }
        default:
            break;
        }
    }

    m_quadRenderer->Render( frameIndex, commandList );
}

ClayDimensions ClayRenderer::MeasureText( const InteropString &text, const ClayTextDesc &desc ) const
{
    if ( !m_desc.TextRenderer )
    {
        return ClayDimensions{ 0, 0 };
    }

    TextRenderDesc textDesc;
    textDesc.Text          = text;
    textDesc.FontId        = desc.FontId;
    textDesc.FontSize      = desc.FontSize;
    textDesc.LetterSpacing = desc.LetterSpacing;
    textDesc.LineHeight    = desc.LineHeight;
    textDesc.Scale         = 1.0f; // Use font size instead of scale

    const Float_2 size = m_desc.TextRenderer->MeasureText( text, textDesc );
    return ClayDimensions{ size.X, size.Y };
}

void ClayRenderer::RenderRectangle( ICommandList *commandList, const ClayBoundingBox &bounds, const ClayRectangleRenderData &data, const uint32_t frameIndex )
{
    const Float_4  color      = data.BackgroundColor.ToFloat4( );
    const uint32_t materialId = GetOrCreateMaterial( color );
    const uint32_t quadId     = GetOrCreateQuad( bounds, materialId );

    QuadDataDesc quadDesc;
    quadDesc.QuadId     = quadId;
    quadDesc.Position   = Float_2( bounds.X, bounds.Y );
    quadDesc.Size       = Float_2( bounds.Width, bounds.Height );
    quadDesc.MaterialId = materialId;
    quadDesc.Rotation   = 0.0f;
    quadDesc.Scale      = Float_2( 1.0f, 1.0f );
    quadDesc.UV0        = Float_2( 0.0f, 0.0f );
    quadDesc.UV1        = Float_2( 1.0f, 1.0f );

    m_quadRenderer->UpdateQuad( frameIndex, quadDesc );
}

void ClayRenderer::RenderRoundedRectangle( ICommandList *commandList, const ClayBoundingBox &bounds, const ClayRectangleRenderData &data, const uint32_t frameIndex )
{
    ITextureResource *shapeTexture = GetOrCreateRoundedRectTexture( bounds, data );

    constexpr auto color      = Float_4( 1.0f, 1.0f, 1.0f, 1.0f ); // White tint since color is in texture
    const uint32_t materialId = GetOrCreateMaterial( color, shapeTexture );
    const uint32_t quadId     = GetOrCreateQuad( bounds, materialId );

    QuadDataDesc quadDesc;
    quadDesc.QuadId     = quadId;
    quadDesc.Position   = Float_2( bounds.X, bounds.Y );
    quadDesc.Size       = Float_2( bounds.Width, bounds.Height );
    quadDesc.MaterialId = materialId;
    quadDesc.Rotation   = 0.0f;
    quadDesc.Scale      = Float_2( 1.0f, 1.0f );
    quadDesc.UV0        = Float_2( 0.0f, 0.0f );
    quadDesc.UV1        = Float_2( 1.0f, 1.0f );

    m_quadRenderer->UpdateQuad( frameIndex, quadDesc );
}

void ClayRenderer::RenderBorder( ICommandList *commandList, const ClayBoundingBox &bounds, const ClayBorderRenderData &data, const uint32_t frameIndex )
{
    if ( data.Width.Top > 0 )
    {
        ClayBoundingBox topBorder;
        topBorder.X      = bounds.X;
        topBorder.Y      = bounds.Y;
        topBorder.Width  = bounds.Width;
        topBorder.Height = static_cast<float>( data.Width.Top );

        ClayRectangleRenderData rectData;
        rectData.BackgroundColor = data.Color;
        rectData.CornerRadius    = ClayCornerRadius( 0 );

        RenderRectangle( commandList, topBorder, rectData, frameIndex );
    }

    if ( data.Width.Bottom > 0 )
    {
        ClayBoundingBox bottomBorder;
        bottomBorder.X      = bounds.X;
        bottomBorder.Y      = bounds.Y + bounds.Height - data.Width.Bottom;
        bottomBorder.Width  = bounds.Width;
        bottomBorder.Height = static_cast<float>( data.Width.Bottom );

        ClayRectangleRenderData rectData;
        rectData.BackgroundColor = data.Color;
        rectData.CornerRadius    = ClayCornerRadius( 0 );

        RenderRectangle( commandList, bottomBorder, rectData, frameIndex );
    }

    if ( data.Width.Left > 0 )
    {
        ClayBoundingBox leftBorder;
        leftBorder.X      = bounds.X;
        leftBorder.Y      = bounds.Y + data.Width.Top;
        leftBorder.Width  = static_cast<float>( data.Width.Left );
        leftBorder.Height = bounds.Height - data.Width.Top - data.Width.Bottom;

        ClayRectangleRenderData rectData;
        rectData.BackgroundColor = data.Color;
        rectData.CornerRadius    = ClayCornerRadius( 0 );

        RenderRectangle( commandList, leftBorder, rectData, frameIndex );
    }

    if ( data.Width.Right > 0 )
    {
        ClayBoundingBox rightBorder;
        rightBorder.X      = bounds.X + bounds.Width - data.Width.Right;
        rightBorder.Y      = bounds.Y + data.Width.Top;
        rightBorder.Width  = static_cast<float>( data.Width.Right );
        rightBorder.Height = bounds.Height - data.Width.Top - data.Width.Bottom;

        ClayRectangleRenderData rectData;
        rectData.BackgroundColor = data.Color;
        rectData.CornerRadius    = ClayCornerRadius( 0 );

        RenderRectangle( commandList, rightBorder, rectData, frameIndex );
    }
}

void ClayRenderer::RenderText( ICommandList *commandList, const ClayBoundingBox &bounds, const ClayTextRenderData &data, const uint32_t frameIndex ) const
{
    if ( !m_desc.TextRenderer )
    {
        return;
    }

    // Begin text batch
    m_desc.TextRenderer->BeginBatch( );

    // Setup text rendering parameters using enhanced TextRenderDesc
    TextRenderDesc textDesc;
    textDesc.Text          = data.StringContents;
    textDesc.X             = bounds.X;
    textDesc.Y             = bounds.Y;
    textDesc.FontId        = data.FontId;
    textDesc.FontSize      = data.FontSize;
    textDesc.LetterSpacing = data.LetterSpacing;
    textDesc.LineHeight    = data.LineHeight;
    textDesc.Scale         = 1.0f; // Use font size instead of scale
    textDesc.Color         = Float_4( data.TextColor.R / 255.0f, data.TextColor.G / 255.0f, data.TextColor.B / 255.0f, data.TextColor.A / 255.0f );

    m_desc.TextRenderer->AddText( textDesc );
    m_desc.TextRenderer->EndBatch( commandList );
}

void ClayRenderer::RenderImage( ICommandList *commandList, const ClayBoundingBox &bounds, const ClayImageRenderData &data, const uint32_t frameIndex )
{
    const ITextureResource *texture = static_cast<ITextureResource *>( data.ImageData );
    if ( !texture )
    {
        return;
    }

    Float_4 tintColor = data.BackgroundColor.ToFloat4( );
    if ( tintColor.X == 0 && tintColor.Y == 0 && tintColor.Z == 0 && tintColor.W == 0 )
    {
        tintColor = Float_4( 1, 1, 1, 1 );
    }

    const uint32_t materialId = GetOrCreateMaterial( tintColor, const_cast<ITextureResource *>( texture ) );
    const uint32_t quadId     = GetOrCreateQuad( bounds, materialId );

    // Update quad data for this frame
    QuadDataDesc quadDesc;
    quadDesc.QuadId     = quadId;
    quadDesc.Position   = Float_2( bounds.X, bounds.Y );
    quadDesc.Size       = Float_2( bounds.Width, bounds.Height );
    quadDesc.MaterialId = materialId;
    quadDesc.Rotation   = 0.0f;
    quadDesc.Scale      = Float_2( 1.0f, 1.0f );
    quadDesc.UV0        = Float_2( 0.0f, 0.0f );
    quadDesc.UV1        = Float_2( 1.0f, 1.0f );

    m_quadRenderer->UpdateQuad( frameIndex, quadDesc );
}

ITextureResource *ClayRenderer::GetOrCreateRoundedRectTexture( const ClayBoundingBox &bounds, const ClayRectangleRenderData &data )
{
    const uint64_t hash = GetShapeHash( bounds, data );

    auto it = m_shapeCache.find( hash );
    if ( it != m_shapeCache.end( ) && it->second.Texture != nullptr )
    {
        return it->second.Texture;
    }

    m_vectorCanvas->Clear( );
    CreateVectorShape( bounds, data, *m_vectorCanvas );

    m_vectorCanvas->Draw( );
    m_vectorCanvas->Sync( );

    TextureDesc textureDesc;
    textureDesc.Width        = m_desc.Width;
    textureDesc.Height       = m_desc.Height;
    textureDesc.Format       = Format::R8G8B8A8Unorm;
    textureDesc.Descriptor   = BitSet( ResourceDescriptor::Texture );
    textureDesc.InitialUsage = ResourceUsage::ShaderResource;
    textureDesc.DebugName    = "Clay Rounded Rectangle Texture";

    std::unique_ptr<ITextureResource> texture( m_desc.LogicalDevice->CreateTextureResource( textureDesc ) );

    BatchResourceCopy batchCopy( m_desc.LogicalDevice );
    batchCopy.Begin( );

    CopyDataToTextureDesc copyDesc;
    copyDesc.Data       = m_vectorCanvas->GetDataAsBytes( );
    copyDesc.DstTexture = texture.get( );
    batchCopy.CopyDataToTexture( copyDesc );
    batchCopy.Submit( );

    ShapeCache &cache = m_shapeCache[ hash ];
    cache.Texture     = texture.release( );
    return cache.Texture;
}

uint64_t ClayRenderer::GetShapeHash( const ClayBoundingBox &bounds, const ClayRectangleRenderData &data ) const
{
    constexpr std::hash<float> hasher;
    uint64_t                   hash = 0;
    hash ^= hasher( data.CornerRadius.TopLeft ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    hash ^= hasher( data.CornerRadius.TopRight ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    hash ^= hasher( data.CornerRadius.BottomLeft ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    hash ^= hasher( data.CornerRadius.BottomRight ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    hash ^= hasher( data.BackgroundColor.R ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    hash ^= hasher( data.BackgroundColor.G ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    hash ^= hasher( data.BackgroundColor.B ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    hash ^= hasher( data.BackgroundColor.A ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    return hash;
}

void ClayRenderer::CreateVectorShape( const ClayBoundingBox &bounds, const ClayRectangleRenderData &data, ThorVGCanvas &canvas ) const
{
    ThorVGShape shape;

    const float width  = static_cast<float>( m_desc.Width );
    const float height = static_cast<float>( m_desc.Height );

    // Calculate corner radius as a percentage of the smaller dimension
    // This ensures corners scale properly when the texture is stretched
    const float minDim      = std::min( width, height );
    const float cornerScale = minDim / 100.0f; // Assume 100 units as reference

    const float scaledTL = data.CornerRadius.TopLeft * cornerScale;
    const float scaledTR = data.CornerRadius.TopRight * cornerScale;
    const float scaledBL = data.CornerRadius.BottomLeft * cornerScale;
    const float scaledBR = data.CornerRadius.BottomRight * cornerScale;

    shape.AppendRect( 0, 0, width, height, scaledTL, scaledTL );

    shape.Fill( static_cast<uint8_t>( data.BackgroundColor.R ), static_cast<uint8_t>( data.BackgroundColor.G ), static_cast<uint8_t>( data.BackgroundColor.B ),
                static_cast<uint8_t>( data.BackgroundColor.A ) );

    canvas.Push( &shape );
}

uint32_t ClayRenderer::GetOrCreateMaterial( const Float_4 &color, ITextureResource *texture )
{
    const uint32_t materialId = m_currentFrameMaterialIndex;
    if ( materialId >= m_nextMaterialId )
    {
        m_nextMaterialId = materialId + 1;

        QuadMaterialDesc materialDesc;
        materialDesc.MaterialId = materialId;
        materialDesc.Texture    = texture;
        materialDesc.Color      = color;

        m_quadRenderer->AddMaterial( materialDesc );
    }
    else
    {
        // Update existing material
        QuadMaterialDesc materialDesc;
        materialDesc.MaterialId = materialId;
        materialDesc.Texture    = texture;
        materialDesc.Color      = color;

        // Note: QuadRenderer doesn't have UpdateMaterial for all frames,
        // so materials are expected to be relatively stable
    }

    m_currentFrameMaterialIndex++;
    return materialId;
}

uint32_t ClayRenderer::GetOrCreateQuad( const ClayBoundingBox &bounds, const uint32_t materialId )
{
    const uint32_t quadId = m_currentFrameQuadIndex;

    QuadDataDesc quadDesc;
    quadDesc.QuadId     = quadId;
    quadDesc.Position   = Float_2( bounds.X, bounds.Y );
    quadDesc.Size       = Float_2( bounds.Width, bounds.Height );
    quadDesc.MaterialId = materialId;
    quadDesc.Rotation   = 0.0f;
    quadDesc.Scale      = Float_2( 1.0f, 1.0f );
    quadDesc.UV0        = Float_2( 0.0f, 0.0f );
    quadDesc.UV1        = Float_2( 1.0f, 1.0f );

    if ( quadId >= m_nextQuadId )
    {
        m_nextQuadId = quadId + 1;
        m_quadRenderer->AddQuad( quadDesc );
    }
    m_currentFrameQuadIndex++;
    return quadId;
}
