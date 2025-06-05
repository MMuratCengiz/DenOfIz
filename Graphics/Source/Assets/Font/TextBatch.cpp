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

#include "DenOfIzGraphics/Assets/Font/TextBatch.h"
#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h"
#include "DenOfIzGraphics/Renderer/Sync/ResourceTracking.h"
#include "DenOfIzGraphicsInternal/Utilities/InteropMathConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

#include <DirectXMath.h>

using namespace DenOfIz;
using namespace DirectX;

TextBatch::TextBatch( const TextBatchDesc &desc ) : m_desc( desc )
{
    if ( desc.LogicalDevice == nullptr )
    {
        LOG( ERROR ) << "TextBatch::TextBatch LogicalDevice cannot be null";
        return;
    }
    if ( desc.Font == nullptr )
    {
        LOG( ERROR ) << "TextBatch::TextBatch LogicalDevice cannot be null";
        return;
    }
    m_font                                       = desc.Font;
    m_logicalDevice                              = desc.LogicalDevice;
    m_vertexBufferDesc                           = { };
    m_vertexBufferDesc.NumBytes                  = m_maxVertices * sizeof( GlyphVertex );
    m_vertexBufferDesc.Descriptor                = BitSet( ResourceDescriptor::VertexBuffer ) | ResourceDescriptor::StructuredBuffer;
    m_vertexBufferDesc.Usages                    = ResourceUsage::VertexAndConstantBuffer;
    m_vertexBufferDesc.HeapType                  = HeapType::CPU_GPU;
    m_vertexBufferDesc.DebugName                 = "Font Vertex Buffer";
    m_vertexBufferDesc.StructureDesc.NumElements = m_maxVertices;
    m_vertexBufferDesc.StructureDesc.Stride      = sizeof( GlyphVertex );
    m_vertexBuffer                               = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( m_vertexBufferDesc ) );
    m_vertexBufferMappedMemory                   = static_cast<Byte *>( m_vertexBuffer->MapMemory( ) );

    m_indexBufferDesc            = { };
    m_indexBufferDesc.NumBytes   = m_maxIndices * sizeof( uint32_t );
    m_indexBufferDesc.Descriptor = BitSet( ResourceDescriptor::IndexBuffer );
    m_indexBufferDesc.Usages     = ResourceUsage::IndexBuffer;
    m_indexBufferDesc.HeapType   = HeapType::CPU_GPU;
    m_indexBufferDesc.DebugName  = "Font Index Buffer";
    m_indexBuffer                = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( m_indexBufferDesc ) );
    m_indexBufferMappedMemory    = static_cast<Byte *>( m_indexBuffer->MapMemory( ) );

    BufferDesc uniformBufferDesc;
    uniformBufferDesc.NumBytes                  = 3 * sizeof( FontShaderUniforms );
    uniformBufferDesc.Descriptor                = BitSet( ResourceDescriptor::UniformBuffer );
    uniformBufferDesc.Usages                    = ResourceUsage::VertexAndConstantBuffer;
    uniformBufferDesc.HeapType                  = HeapType::CPU_GPU;
    uniformBufferDesc.DebugName                 = "Font Uniform Buffer";
    uniformBufferDesc.StructureDesc.NumElements = 1;
    uniformBufferDesc.StructureDesc.Stride      = sizeof( FontShaderUniforms );
    m_uniformBuffer                             = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( uniformBufferDesc ) );
    m_uniformBufferData                         = static_cast<FontShaderUniforms *>( m_uniformBuffer->MapMemory( ) );

    m_fontSampler = std::unique_ptr<ISampler>( m_logicalDevice->CreateSampler( { } ) );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_desc.RendererRootSignature;

    m_resourceBindGroup = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );
    InitializeAtlas( );
}

TextBatch::~TextBatch( )
{
    if ( m_uniformBufferData )
    {
        m_uniformBuffer->UnmapMemory( );
    }
    if ( m_vertexBufferMappedMemory )
    {
        m_vertexBuffer->UnmapMemory( );
    }
    if ( m_indexBufferMappedMemory )
    {
        m_indexBuffer->UnmapMemory( );
    }
}

void TextBatch::BeginBatch( )
{
    m_glyphVertices.Clear( );
    m_indexData.Clear( );
    m_currentVertexCount     = 0;
    m_currentIndexCount      = 0;
    m_currentTextLayoutIndex = 0;
}

void TextBatch::AddText( const AddTextDesc &desc )
{
    if ( m_textLayouts.size( ) <= m_currentTextLayoutIndex )
    {
        const size_t oldSize = m_textLayouts.size( );
        const size_t newSize = std::max<size_t>( m_currentTextLayoutIndex + 1, m_textLayouts.size( ) + 32 );
        m_textLayouts.resize( newSize );
        for ( size_t i = oldSize; i < newSize; ++i )
        {
            TextLayoutDesc textLayoutDesc{ m_font };
            m_textLayouts[ i ] = std::make_unique<TextLayout>( textLayoutDesc );
        }
    }

    const auto &textLayout = m_textLayouts[ m_currentTextLayoutIndex++ ];

    AddTextDesc modifiedParams = desc;

    const float baseSize       = static_cast<float>( m_font->Asset( )->InitialFontSize );
    const float targetSize     = desc.FontSize > 0 ? desc.FontSize : baseSize;
    const auto  effectiveScale = targetSize / baseSize;

    ShapeTextDesc shapeDesc{ };
    shapeDesc.Text      = desc.Text;
    shapeDesc.Direction = desc.Direction;
    shapeDesc.FontSize  = targetSize;

    textLayout->ShapeText( shapeDesc );

    if ( desc.HorizontalCenter || desc.VerticalCenter )
    {
        if ( desc.HorizontalCenter )
        {
            modifiedParams.X -= textLayout->GetTextWidth( ) / 2.0f;
        }

        if ( desc.VerticalCenter )
        {
            const auto &metrics    = m_font->Asset( )->Metrics;
            const float textHeight = static_cast<float>( metrics.Ascent + metrics.Descent ) * effectiveScale;
            modifiedParams.Y -= textHeight / 2.0f;
        }
    }

    const float fontAscent = static_cast<float>( m_font->Asset( )->Metrics.Ascent ) * effectiveScale;
    const float adjustedY  = modifiedParams.Y + fontAscent;

    GenerateTextVerticesDesc generateDesc{ };
    generateDesc.StartPosition = { modifiedParams.X, adjustedY };
    generateDesc.Color         = modifiedParams.Color;
    generateDesc.OutVertices   = &m_glyphVertices;
    generateDesc.OutIndices    = &m_indexData;
    generateDesc.Scale         = effectiveScale;
    generateDesc.LetterSpacing = desc.LetterSpacing;
    generateDesc.LineHeight    = desc.LineHeight;

    textLayout->GenerateTextVertices( generateDesc );

    m_currentVertexCount = m_glyphVertices.NumElements( );
    m_currentIndexCount  = m_indexData.NumElements( );

    // Resize buffers if needed
    if ( m_currentVertexCount > m_maxVertices || m_currentIndexCount > m_maxIndices )
    {
        m_maxVertices = std::max( m_maxVertices * 2, m_currentVertexCount );
        m_maxIndices  = std::max( m_maxIndices * 2, m_currentIndexCount );
    }
}

void TextBatch::EndBatch( ICommandList *commandList )
{
    if ( m_currentVertexCount == 0 || m_currentIndexCount == 0 || !m_font )
    {
        return; // Nothing to render
    }

    UpdateBuffers( );

    // Todo split this up into batch specific and text renderer specific
    FontShaderUniforms *uniforms = m_uniformBufferData;
    uniforms->Projection         = m_projectionMatrix;
    uniforms->TextColor          = Float_4{ 1.0f, 1.0f, 1.0f, 1.0f };

    const auto *fontAsset       = m_font->Asset( );
    uniforms->TextureSizeParams = Float_4{ static_cast<float>( fontAsset->AtlasWidth ), static_cast<float>( fontAsset->AtlasHeight ), Font::MsdfPixelRange,
                                           static_cast<float>( static_cast<uint32_t>( AntiAliasingMode::Grayscale ) ) };

    commandList->BindResourceGroup( m_resourceBindGroup.get( ) );
    commandList->BindVertexBuffer( m_vertexBuffer.get( ) );
    commandList->BindIndexBuffer( m_indexBuffer.get( ), IndexType::Uint32 );
    commandList->DrawIndexed( m_currentIndexCount, 1, 0, 0, 0 );
}

void TextBatch::SetProjectionMatrix( const Float_4x4 &projectionMatrix )
{
    m_projectionMatrix = projectionMatrix;
}

Float_2 TextBatch::MeasureText( const InteropString &text, const AddTextDesc &desc ) const
{
    const float baseSize       = static_cast<float>( m_font->Asset( )->InitialFontSize );
    const float targetSize     = desc.FontSize > 0 ? desc.FontSize : baseSize;
    const auto  effectiveScale = targetSize / baseSize;

    if ( text.NumChars( ) == 0 )
    {
        return Float_2{ 0.0f, 0.0f };
    }

    const TextLayoutDesc layoutDesc{ m_font };
    TextLayout           tempLayout( layoutDesc );

    ShapeTextDesc shapeDesc{ };
    shapeDesc.Text      = text;
    shapeDesc.Direction = desc.Direction;
    shapeDesc.FontSize  = targetSize;

    tempLayout.ShapeText( shapeDesc );

    float textWidth  = tempLayout.GetTextWidth( );
    float textHeight = tempLayout.GetTextHeight( );
    if ( desc.LetterSpacing > 0 && text.NumChars( ) > 0 )
    {
        textWidth += static_cast<float>( desc.LetterSpacing ) * ( text.NumChars( ) - 1 );
    }

    if ( desc.LineHeight > 0 )
    {
        textHeight = static_cast<float>( desc.LineHeight );
    }
    else
    {
        const auto &metrics = m_font->Asset( )->Metrics;
        textHeight          = static_cast<float>( metrics.Ascent + metrics.Descent ) * effectiveScale;
    }
    return Float_2{ textWidth, textHeight };
}

void TextBatch::UpdateBuffers( )
{
    // Check if we need to resize vertex buffer
    if ( m_vertexBufferDesc.NumBytes < m_glyphVertices.NumElements( ) * sizeof( GlyphVertex ) )
    {
        if ( m_vertexBufferMappedMemory )
        {
            m_vertexBuffer->UnmapMemory( );
            m_vertexBufferMappedMemory = nullptr;
        }

        m_vertexBufferDesc.NumBytes                  = m_maxVertices * sizeof( GlyphVertex );
        m_vertexBufferDesc.StructureDesc.NumElements = m_maxVertices;
        auto newBuffer                               = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( m_vertexBufferDesc ) );
        m_vertexBuffer                               = std::move( newBuffer );
        m_vertexBufferMappedMemory                   = static_cast<Byte *>( m_vertexBuffer->MapMemory( ) );
    }

    // Check if we need to resize index buffer
    if ( m_indexBufferDesc.NumBytes < m_indexData.NumElements( ) * sizeof( uint32_t ) )
    {
        if ( m_indexBufferMappedMemory )
        {
            m_indexBuffer->UnmapMemory( );
            m_indexBufferMappedMemory = nullptr;
        }

        m_indexBufferDesc.NumBytes = m_maxIndices * sizeof( uint32_t );
        auto newBuffer             = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( m_indexBufferDesc ) );
        m_indexBuffer              = std::move( newBuffer );
        m_indexBufferMappedMemory  = static_cast<Byte *>( m_indexBuffer->MapMemory( ) );
    }

    memcpy( m_vertexBufferMappedMemory, m_glyphVertices.Data( ), m_glyphVertices.NumElements( ) * sizeof( GlyphVertex ) );
    memcpy( m_indexBufferMappedMemory, m_indexData.Data( ), m_indexData.NumElements( ) * sizeof( uint32_t ) );
}

void TextBatch::InitializeAtlas( )
{
    CommandQueueDesc commandQueueDesc{ };
    commandQueueDesc.QueueType = QueueType::Graphics;

    auto commandQueue = std::unique_ptr<ICommandQueue>( m_logicalDevice->CreateCommandQueue( commandQueueDesc ) );

    CommandListPoolDesc commandListPoolDesc{ };
    commandListPoolDesc.CommandQueue    = commandQueue.get( );
    commandListPoolDesc.NumCommandLists = 1;

    auto commandListPool = std::unique_ptr<ICommandListPool>( m_logicalDevice->CreateCommandListPool( commandListPoolDesc ) );
    auto commandList     = commandListPool->GetCommandLists( ).GetElement( 0 );
    commandList->Begin( );

    const auto *fontAsset = m_desc.Font->Asset( );

    const auto alignedPitch = Utilities::Align( fontAsset->AtlasWidth * FontAsset::NumChannels, m_logicalDevice->DeviceInfo( ).Constants.BufferTextureRowAlignment );
    const auto alignedSlice = Utilities::Align( fontAsset->AtlasHeight, m_logicalDevice->DeviceInfo( ).Constants.BufferTextureAlignment );

    BufferDesc stagingDesc;
    stagingDesc.NumBytes          = alignedPitch * alignedSlice;
    stagingDesc.Descriptor        = BitSet( ResourceDescriptor::Buffer );
    stagingDesc.InitialUsage      = ResourceUsage::CopySrc;
    stagingDesc.DebugName         = "Font MSDF Atlas Staging Buffer";
    stagingDesc.HeapType          = HeapType::CPU;
    auto m_fontAtlasStagingBuffer = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( stagingDesc ) );

    TextureDesc textureDesc{ };
    textureDesc.Width        = fontAsset->AtlasWidth;
    textureDesc.Height       = fontAsset->AtlasHeight;
    textureDesc.Format       = Format::R8G8B8A8Unorm;
    textureDesc.Descriptor   = BitSet( ResourceDescriptor::Texture );
    textureDesc.InitialUsage = ResourceUsage::ShaderResource;
    textureDesc.DebugName    = "Font MTSDF Atlas Texture";
    m_atlas                  = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) );
    m_resourceTracking.TrackTexture( m_atlas.get( ), ResourceUsage::ShaderResource );

    m_resourceBindGroup->BeginUpdate( )->Cbv( 0, m_uniformBuffer.get( ) )->Srv( 0, m_atlas.get( ) )->Sampler( 0, m_fontSampler.get( ) )->EndUpdate( );
    m_resourceTracking.TrackBuffer( m_fontAtlasStagingBuffer.get( ), ResourceUsage::CopySrc );

    LoadAtlasIntoGpuTextureDesc loadDesc{ };
    loadDesc.Device        = m_logicalDevice;
    loadDesc.StagingBuffer = m_fontAtlasStagingBuffer.get( );
    loadDesc.CommandList   = commandList;
    loadDesc.Texture       = m_atlas.get( );
    FontAssetReader::LoadAtlasIntoGpuTexture( *fontAsset, loadDesc );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( m_atlas.get( ), ResourceUsage::CopyDst );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    CopyBufferToTextureDesc copyDesc{ };
    copyDesc.SrcBuffer  = m_fontAtlasStagingBuffer.get( );
    copyDesc.DstTexture = m_atlas.get( );
    copyDesc.RowPitch   = fontAsset->AtlasWidth * 4; // 4 bytes per pixel (RGBA)
    copyDesc.Format     = m_atlas->GetFormat( );

    commandList->CopyBufferToTexture( copyDesc );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( m_atlas.get( ), ResourceUsage::ShaderResource );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );
    ExecuteCommandListsDesc executeDesc{ };
    executeDesc.CommandLists = { commandList };
    commandQueue->ExecuteCommandLists( executeDesc );
    commandQueue->WaitIdle( );
}
