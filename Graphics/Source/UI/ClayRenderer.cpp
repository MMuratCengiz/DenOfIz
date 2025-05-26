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
#include <DenOfIzGraphics/UI/Clay.h>
#include <DenOfIzGraphics/UI/ClayRenderer.h>
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

void ClayRenderer::Resize( const float width, const float height )
{
    if ( m_viewportWidth != width || m_viewportHeight != height )
    {
        m_viewportWidth  = width;
        m_viewportHeight = height;

        m_quadRenderer->SetCanvas( static_cast<uint32_t>( width ), static_cast<uint32_t>( height ) );
        m_desc.TextRenderer->SetViewport( Viewport{ 0, 0, width, height } );
        m_needsClear = true;
    }
}

void ClayRenderer::SetDpiScale( const float dpiScale )
{
    if ( m_dpiScale != dpiScale )
    {
        m_dpiScale   = dpiScale;
        m_needsClear = true;
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

void ClayRenderer::Render( ICommandList *commandList, const Clay_RenderCommandArray &commands, const uint32_t frameIndex )
{
    if ( commandList == nullptr )
    {
        LOG( ERROR ) << "ClayRenderer::Render: commandList is null";
        return;
    }

    if ( m_needsClear )
    {
        ClearCaches( );
        m_needsClear = false;
        return;
    }

    m_currentFrameQuadIndex     = 0;
    m_currentFrameMaterialIndex = 0;
    m_currentFrameIndex         = frameIndex;
    for ( int i = 0; i < commands.length; ++i )
    {
        const Clay_RenderCommand *cmd = &commands.internalArray[ i ];
        switch ( cmd->commandType )
        {
        case CLAY_RENDER_COMMAND_TYPE_NONE:
            break;
        case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
            if ( cmd->renderData.rectangle.cornerRadius.topLeft > 0 || cmd->renderData.rectangle.cornerRadius.topRight > 0 ||
                 cmd->renderData.rectangle.cornerRadius.bottomLeft > 0 || cmd->renderData.rectangle.cornerRadius.bottomRight > 0 )
            {
                RenderRoundedRectangle( cmd, frameIndex );
            }
            else
            {
                RenderRectangle( cmd, frameIndex );
            }
            break;
        case CLAY_RENDER_COMMAND_TYPE_BORDER:
            RenderBorder( cmd, frameIndex );
            break;
        case CLAY_RENDER_COMMAND_TYPE_TEXT:
            // Flush any pending quads before rendering text
            // if ( m_currentFrameQuadIndex > 0 )
            // {
            //     m_quadRenderer->Render( frameIndex, commandList );
            //     m_currentFrameQuadIndex = 0;
            // }
            // RenderText( cmd, frameIndex, commandList );
            break;
        case CLAY_RENDER_COMMAND_TYPE_IMAGE:
            RenderImage( cmd, frameIndex );
            break;
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
            commandList->BindScissorRect( cmd->boundingBox.x * m_dpiScale, cmd->boundingBox.y * m_dpiScale, cmd->boundingBox.width * m_dpiScale,
                                          cmd->boundingBox.height * m_dpiScale );
            break;
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
            commandList->BindScissorRect( 0, 0, m_viewportWidth * m_dpiScale, m_viewportHeight * m_dpiScale );
            break;
        case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
            break;
        }
    }

    if ( m_currentFrameQuadIndex > 0 )
    {
        m_quadRenderer->Render( frameIndex, commandList );
    }
    m_desc.TextRenderer->BeginBatch( );
    for ( int i = 0; i < commands.length; ++i )
    {
        const Clay_RenderCommand *cmd = &commands.internalArray[ i ];
        switch ( cmd->commandType )
        {
        case CLAY_RENDER_COMMAND_TYPE_TEXT:
            RenderText( cmd, frameIndex, commandList );
            break;
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
            commandList->BindScissorRect( cmd->boundingBox.x * m_dpiScale, cmd->boundingBox.y * m_dpiScale, cmd->boundingBox.width * m_dpiScale,
                                          cmd->boundingBox.height * m_dpiScale );
            break;
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
            commandList->BindScissorRect( 0, 0, m_viewportWidth * m_dpiScale, m_viewportHeight * m_dpiScale );
            break;
        default:
            break;
        }
    }
    m_desc.TextRenderer->EndBatch( commandList );
}

ClayDimensions ClayRenderer::MeasureText( const InteropString &text, const Clay_TextElementConfig &desc ) const
{
    if ( text.NumChars( ) == 0 )
    {
        return ClayDimensions{ 0, 0 };
    }

    TextRenderDesc textDesc;
    textDesc.Text          = text;
    textDesc.FontId        = desc.fontId;
    textDesc.FontSize      = desc.fontSize * m_dpiScale;
    textDesc.LetterSpacing = desc.letterSpacing * m_dpiScale;
    textDesc.LineHeight    = desc.lineHeight;

    const Float_2 size   = m_desc.TextRenderer->MeasureText( text, textDesc );
    float         height = size.Y;
    if ( desc.lineHeight > 0 )
    {
        height = static_cast<float>( desc.lineHeight );
    }
    return ClayDimensions{ size.X / m_dpiScale, height / m_dpiScale };
}

void ClayRenderer::RenderRectangle( const Clay_RenderCommand *command, const uint32_t frameIndex )
{
    const Clay_RectangleRenderData data   = command->renderData.rectangle;
    const Clay_BoundingBox         bounds = command->boundingBox;

    const auto     color      = data.backgroundColor;
    const uint32_t materialId = GetOrCreateMaterial( color, nullptr );
    const uint32_t quadId     = GetOrCreateQuad( bounds, materialId );

    QuadDataDesc quadDesc;
    quadDesc.QuadId     = quadId;
    quadDesc.Position   = Float_2( bounds.x, bounds.y );
    quadDesc.Size       = Float_2( bounds.width, bounds.height );
    quadDesc.MaterialId = materialId;
    quadDesc.Rotation   = 0.0f;
    quadDesc.Scale      = Float_2( 1.0f, 1.0f );
    quadDesc.UV0        = Float_2( 0.0f, 0.0f );
    quadDesc.UV1        = Float_2( 1.0f, 1.0f );

    m_quadRenderer->UpdateQuad( frameIndex, quadDesc );
}

void ClayRenderer::RenderRoundedRectangle( const Clay_RenderCommand *command, const uint32_t frameIndex )
{
    const Clay_RectangleRenderData data   = command->renderData.rectangle;
    const Clay_BoundingBox         bounds = command->boundingBox;

    ITextureResource *shapeTexture = GetOrCreateRoundedRectTexture( bounds, data );

    const auto     color      = data.backgroundColor;
    const uint32_t materialId = GetOrCreateMaterial( color, shapeTexture );
    const uint32_t quadId     = GetOrCreateQuad( bounds, materialId );

    QuadDataDesc quadDesc;
    quadDesc.QuadId     = quadId;
    quadDesc.Position   = Float_2( bounds.x, bounds.y );
    quadDesc.Size       = Float_2( bounds.width, bounds.height );
    quadDesc.MaterialId = materialId;

    m_quadRenderer->UpdateQuad( frameIndex, quadDesc );
}

void ClayRenderer::RenderBorder( const Clay_RenderCommand *command, const uint32_t frameIndex )
{
    const Clay_BoundingBox bounds = command->boundingBox;
    Clay_BorderRenderData  data   = command->renderData.border;

    if ( data.width.top > 0 )
    {
        Clay_BoundingBox topBorder;
        topBorder.x      = bounds.x;
        topBorder.y      = bounds.y;
        topBorder.width  = bounds.width;
        topBorder.height = static_cast<float>( data.width.top );

        Clay_RectangleRenderData rectData;
        rectData.backgroundColor = data.color;
        rectData.cornerRadius    = Clay_CornerRadius( 0, 0, 0, 0 );

        Clay_RenderCommand rectCommand{ };
        rectCommand.boundingBox          = topBorder;
        rectCommand.renderData.rectangle = rectData;
        rectCommand.commandType          = CLAY_RENDER_COMMAND_TYPE_RECTANGLE;
        RenderRectangle( &rectCommand, frameIndex );
    }

    if ( data.width.bottom > 0 )
    {
        Clay_BoundingBox bottomBorder;
        bottomBorder.x      = bounds.x;
        bottomBorder.y      = bounds.y + bounds.height - data.width.bottom;
        bottomBorder.width  = bounds.width;
        bottomBorder.height = static_cast<float>( data.width.bottom );

        Clay_RectangleRenderData rectData;
        rectData.backgroundColor = data.color;
        rectData.cornerRadius    = Clay_CornerRadius( 0, 0, 0, 0 );

        Clay_RenderCommand rectCommand{ };
        rectCommand.boundingBox          = bottomBorder;
        rectCommand.renderData.rectangle = rectData;
        rectCommand.commandType          = CLAY_RENDER_COMMAND_TYPE_RECTANGLE;
        RenderRectangle( &rectCommand, frameIndex );
    }

    if ( data.width.left > 0 )
    {
        Clay_BoundingBox leftBorder;
        leftBorder.x      = bounds.x;
        leftBorder.y      = bounds.y + data.width.top;
        leftBorder.width  = static_cast<float>( data.width.left );
        leftBorder.height = bounds.height - data.width.top - data.width.bottom;

        Clay_RectangleRenderData rectData;
        rectData.backgroundColor = data.color;
        rectData.cornerRadius    = Clay_CornerRadius( 0, 0, 0, 0 );

        Clay_RenderCommand rectCommand{ };
        rectCommand.boundingBox          = leftBorder;
        rectCommand.renderData.rectangle = rectData;
        rectCommand.commandType          = CLAY_RENDER_COMMAND_TYPE_RECTANGLE;
        RenderRectangle( &rectCommand, frameIndex );
    }

    if ( data.width.right > 0 )
    {
        Clay_BoundingBox rightBorder;
        rightBorder.x      = bounds.x + bounds.width - data.width.right;
        rightBorder.y      = bounds.y + data.width.top;
        rightBorder.width  = static_cast<float>( data.width.right );
        rightBorder.height = bounds.height - data.width.top - data.width.bottom;

        Clay_RectangleRenderData rectData;
        rectData.backgroundColor = data.color;
        rectData.cornerRadius    = Clay_CornerRadius( 0, 0, 0, 0 );

        Clay_RenderCommand rectCommand{ };
        rectCommand.boundingBox          = rightBorder;
        rectCommand.renderData.rectangle = rectData;
        rectCommand.commandType          = CLAY_RENDER_COMMAND_TYPE_RECTANGLE;
        RenderRectangle( &rectCommand, frameIndex );
    }
}

void ClayRenderer::RenderText( const Clay_RenderCommand *command, const uint32_t frameIndex, ICommandList *commandList ) const
{
    const Clay_TextRenderData data   = command->renderData.text;
    const Clay_BoundingBox    bounds = command->boundingBox;

    if ( data.stringContents.length == 0 )
    {
        return;
    }

    TextRenderDesc textDesc;
    textDesc.Text          = InteropString( data.stringContents.chars, data.stringContents.length );
    textDesc.X             = bounds.x * m_dpiScale;
    textDesc.Y             = bounds.y * m_dpiScale;
    textDesc.FontId        = data.fontId;
    textDesc.FontSize      = data.fontSize * m_dpiScale;
    textDesc.LetterSpacing = data.letterSpacing * m_dpiScale;
    textDesc.LineHeight    = data.lineHeight;
    textDesc.Color         = Float_4( data.textColor.r / 255.0f, data.textColor.g / 255.0f, data.textColor.b / 255.0f, data.textColor.a / 255.0f );
    m_desc.TextRenderer->AddText( textDesc );
}

void ClayRenderer::RenderImage( const Clay_RenderCommand *command, const uint32_t frameIndex )
{
    const Clay_ImageRenderData data   = command->renderData.image;
    const Clay_BoundingBox     bounds = command->boundingBox;

    const ITextureResource *texture = static_cast<ITextureResource *>( data.imageData );
    if ( !texture )
    {
        return;
    }

    const uint32_t materialId = GetOrCreateMaterial( data.backgroundColor, const_cast<ITextureResource *>( texture ) );
    const uint32_t quadId     = GetOrCreateQuad( bounds, materialId );

    // Update quad data for this frame
    QuadDataDesc quadDesc;
    quadDesc.QuadId     = quadId;
    quadDesc.Position   = Float_2( bounds.x, bounds.y );
    quadDesc.Size       = Float_2( bounds.width, bounds.height );
    quadDesc.MaterialId = materialId;
    quadDesc.Rotation   = 0.0f;
    quadDesc.Scale      = Float_2( 1.0f, 1.0f );
    quadDesc.UV0        = Float_2( 0.0f, 0.0f );
    quadDesc.UV1        = Float_2( 1.0f, 1.0f );

    m_quadRenderer->UpdateQuad( frameIndex, quadDesc );
}

ITextureResource *ClayRenderer::GetOrCreateRoundedRectTexture( const Clay_BoundingBox &bounds, const Clay_RectangleRenderData &data )
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

uint64_t ClayRenderer::GetShapeHash( const Clay_BoundingBox &bounds, const Clay_RectangleRenderData &data ) const
{
    constexpr std::hash<float> hasher;
    uint64_t                   hash = 0;
    hash ^= hasher( data.cornerRadius.topLeft ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    hash ^= hasher( data.cornerRadius.topRight ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    hash ^= hasher( data.cornerRadius.bottomLeft ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    hash ^= hasher( data.cornerRadius.bottomRight ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    hash ^= hasher( data.backgroundColor.r ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    hash ^= hasher( data.backgroundColor.g ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    hash ^= hasher( data.backgroundColor.b ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    hash ^= hasher( data.backgroundColor.a ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
    return hash;
}

void ClayRenderer::CreateVectorShape( const Clay_BoundingBox &bounds, const Clay_RectangleRenderData &data, ThorVGCanvas &canvas ) const
{
    ThorVGShape shape;

    const float width  = static_cast<float>( m_desc.Width );
    const float height = static_cast<float>( m_desc.Height );

    // Calculate corner radius as a percentage of the smaller dimension
    // This ensures corners scale properly when the texture is stretched
    const float minDim      = std::min( width, height );
    const float cornerScale = minDim / 100.0f; // Assume 100 units as reference

    const float scaledTL = data.cornerRadius.topLeft * cornerScale;
    const float scaledTR = data.cornerRadius.topRight * cornerScale;
    const float scaledBL = data.cornerRadius.bottomLeft * cornerScale;
    const float scaledBR = data.cornerRadius.bottomRight * cornerScale;

    shape.AppendRect( 0, 0, width, height, scaledTL, scaledTL );

    shape.Fill( static_cast<uint8_t>( data.backgroundColor.r ), static_cast<uint8_t>( data.backgroundColor.g ), static_cast<uint8_t>( data.backgroundColor.b ),
                static_cast<uint8_t>( data.backgroundColor.a ) );

    canvas.Push( &shape );
}

uint32_t ClayRenderer::GetOrCreateMaterial( const Clay_Color &color, ITextureResource *texture )
{
    const Float_4  dzColor    = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
    const uint32_t materialId = m_currentFrameMaterialIndex;
    if ( materialId >= m_nextMaterialId )
    {
        m_nextMaterialId = materialId + 1;

        QuadMaterialDesc materialDesc;
        materialDesc.MaterialId = materialId;
        materialDesc.Texture    = texture;
        materialDesc.Color      = dzColor;

        m_quadRenderer->AddMaterial( materialDesc );
    }
    else
    {
        // Update existing material
        QuadMaterialDesc materialDesc;
        materialDesc.MaterialId = materialId;
        materialDesc.Texture    = texture;
        materialDesc.Color      = dzColor;

        // Note: QuadRenderer doesn't have UpdateMaterial for all frames,
        // so materials are expected to be relatively stable
    }

    m_currentFrameMaterialIndex++;
    return materialId;
}

uint32_t ClayRenderer::GetOrCreateQuad( const Clay_BoundingBox &bounds, const uint32_t materialId )
{
    const uint32_t quadId = m_currentFrameQuadIndex;

    QuadDataDesc quadDesc;
    quadDesc.QuadId     = quadId;
    quadDesc.Position   = Float_2( bounds.x, bounds.y );
    quadDesc.Size       = Float_2( bounds.width, bounds.height );
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
