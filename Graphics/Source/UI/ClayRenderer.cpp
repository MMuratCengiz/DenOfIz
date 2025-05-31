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

#include <DenOfIzGraphics/Assets/Font/Embedded/EmbeddedFonts.h>
#include <DenOfIzGraphics/Assets/Font/FontLibrary.h>
#include <DenOfIzGraphics/Assets/Import/ShaderImporter.h>
#include <DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h>
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include <DenOfIzGraphics/UI/ClayData.h>
#include <DenOfIzGraphics/UI/ClayRenderer.h>
#include <DenOfIzGraphics/UI/UIShaders.h>
#include <DenOfIzGraphics/UI/Widgets/DockableContainerWidget.h>
#include <DenOfIzGraphics/UI/Widgets/ResizableContainerWidget.h>
#include <DenOfIzGraphics/Utilities/Common.h>
#include <algorithm>
#include <cmath>

using namespace DenOfIz;
using namespace DirectX;

ClayRenderer::ClayRenderer( const ClayRendererDesc &desc ) : m_desc( desc ), m_logicalDevice( desc.LogicalDevice )
{
    if ( m_logicalDevice == nullptr )
    {
        LOG( ERROR ) << "ClayRenderer: LogicalDevice cannot be null";
        return;
    }

    m_viewportWidth  = desc.Width;
    m_viewportHeight = desc.Height;

    m_textures.resize( desc.MaxTextures, nullptr );
    m_textureFontFlags.resize( desc.MaxTextures, false );

    CommandQueueDesc commandQueueDesc{ };
    commandQueueDesc.QueueType = QueueType::Graphics;
    m_commandQueue             = std::unique_ptr<ICommandQueue>( m_logicalDevice->CreateCommandQueue( commandQueueDesc ) );

    CommandListPoolDesc poolDesc{ };
    poolDesc.CommandQueue    = m_commandQueue.get( );
    poolDesc.NumCommandLists = desc.NumFrames;
    m_commandListPool        = std::unique_ptr<ICommandListPool>( m_logicalDevice->CreateCommandListPool( poolDesc ) );

    CreateShaderProgram( );
    CreatePipeline( );
    CreateNullTexture( );
    CreateBuffers( );
    CreateRenderTargets( );
    UpdateProjectionMatrix( );

    auto commandLists = m_commandListPool->GetCommandLists( );
    for ( uint32_t i = 0; i < desc.NumFrames && i < commandLists.NumElements( ); ++i )
    {
        m_frameData[ i ].CommandList = commandLists.GetElement( i );
        m_frameData[ i ].FrameFence  = std::unique_ptr<IFence>( m_logicalDevice->CreateFence( ) );
    }

    FullscreenQuadPipelineDesc quadDesc{ };
    quadDesc.LogicalDevice = m_logicalDevice;
    quadDesc.OutputFormat  = desc.RenderTargetFormat;
    quadDesc.NumFrames     = desc.NumFrames;
    m_fullscreenQuad       = std::make_unique<FullscreenQuadPipeline>( quadDesc );

    static FontLibrary defaultFontLibrary;
    static auto        defaultFont = defaultFontLibrary.LoadFont( { EmbeddedFonts::GetInterVar( ) } );
    AddFont( 0, defaultFont );
}

ClayRenderer::~ClayRenderer( )
{
    if ( m_vertexBufferData )
    {
        m_vertexBuffer->UnmapMemory( );
    }
    if ( m_indexBufferData )
    {
        m_indexBuffer->UnmapMemory( );
    }
    if ( m_uniformBufferData )
    {
        m_uniformBuffer->UnmapMemory( );
    }
}

void ClayRenderer::CreateShaderProgram( )
{
    ShaderProgramDesc programDesc{ };

    ShaderStageDesc &vsDesc = programDesc.ShaderStages.EmplaceElement( );
    vsDesc.Stage            = ShaderStage::Vertex;
    vsDesc.EntryPoint       = InteropString( "main" );
    vsDesc.Data             = EmbeddedUIShaders::GetUIVertexShaderBytes( );

    ShaderStageDesc &psDesc = programDesc.ShaderStages.EmplaceElement( );
    psDesc.Stage            = ShaderStage::Pixel;
    psDesc.EntryPoint       = InteropString( "main" );
    psDesc.Data             = EmbeddedUIShaders::GetUIPixelShaderBytes( );

    psDesc.Bindless.MarkSrvAsBindlessArray( 0, 0, m_desc.MaxTextures );
    m_shaderProgram = std::make_unique<ShaderProgram>( programDesc );
}

void ClayRenderer::CreatePipeline( )
{
    const ShaderReflectDesc reflectDesc = m_shaderProgram->Reflect( );
    m_rootSignature                     = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflectDesc.RootSignature ) );
    m_inputLayout                       = std::unique_ptr<IInputLayout>( m_logicalDevice->CreateInputLayout( reflectDesc.InputLayout ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.RootSignature = m_rootSignature.get( );
    pipelineDesc.InputLayout   = m_inputLayout.get( );
    pipelineDesc.ShaderProgram = m_shaderProgram.get( );
    pipelineDesc.BindPoint     = BindPoint::Graphics;

    pipelineDesc.Graphics.PrimitiveTopology = PrimitiveTopology::Triangle;
    pipelineDesc.Graphics.CullMode          = CullMode::None;
    pipelineDesc.Graphics.FillMode          = FillMode::Solid;

    // Depth test used for z-ordering
    pipelineDesc.Graphics.DepthTest.Enable             = true;
    pipelineDesc.Graphics.DepthTest.CompareOp          = CompareOp::Less;
    pipelineDesc.Graphics.DepthTest.Write              = true;
    pipelineDesc.Graphics.DepthStencilAttachmentFormat = Format::D32Float;

    RenderTargetDesc &renderTarget   = pipelineDesc.Graphics.RenderTargets.EmplaceElement( );
    renderTarget.Format              = m_desc.RenderTargetFormat;
    renderTarget.Blend.Enable        = true;
    renderTarget.Blend.SrcBlend      = Blend::SrcAlpha;
    renderTarget.Blend.DstBlend      = Blend::InvSrcAlpha;
    renderTarget.Blend.BlendOp       = BlendOp::Add;
    renderTarget.Blend.SrcBlendAlpha = Blend::One;
    renderTarget.Blend.DstBlendAlpha = Blend::Zero;
    renderTarget.Blend.BlendOpAlpha  = BlendOp::Add;

    m_pipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );
}

void ClayRenderer::CreateBuffers( )
{
    BufferDesc vertexBufferDesc{ };
    vertexBufferDesc.NumBytes   = m_desc.MaxVertices * sizeof( UIVertex );
    vertexBufferDesc.Descriptor = ResourceDescriptor::VertexBuffer;
    vertexBufferDesc.Usages     = ResourceUsage::VertexAndConstantBuffer;
    vertexBufferDesc.HeapType   = HeapType::CPU_GPU;
    vertexBufferDesc.DebugName  = InteropString( "UI Vertex Buffer" );
    m_vertexBuffer              = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( vertexBufferDesc ) );
    m_vertexBufferData          = static_cast<uint8_t *>( m_vertexBuffer->MapMemory( ) );

    BufferDesc indexBufferDesc{ };
    indexBufferDesc.NumBytes   = m_desc.MaxIndices * sizeof( uint32_t );
    indexBufferDesc.Descriptor = ResourceDescriptor::IndexBuffer;
    indexBufferDesc.Usages     = ResourceUsage::IndexBuffer;
    indexBufferDesc.HeapType   = HeapType::CPU_GPU;
    indexBufferDesc.DebugName  = InteropString( "UI Index Buffer" );
    m_indexBuffer              = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( indexBufferDesc ) );
    m_indexBufferData          = static_cast<uint8_t *>( m_indexBuffer->MapMemory( ) );

    m_alignedUniformSize = Utilities::Align( sizeof( UIUniforms ), 256 );
    BufferDesc uniformBufferDesc{ };
    uniformBufferDesc.NumBytes   = m_desc.NumFrames * m_alignedUniformSize;
    uniformBufferDesc.Descriptor = ResourceDescriptor::UniformBuffer;
    uniformBufferDesc.Usages     = ResourceUsage::VertexAndConstantBuffer;
    uniformBufferDesc.HeapType   = HeapType::CPU_GPU;
    uniformBufferDesc.DebugName  = InteropString( "UI Uniform Buffer" );
    m_uniformBuffer              = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( uniformBufferDesc ) );
    m_uniformBufferData          = static_cast<UIUniforms *>( m_uniformBuffer->MapMemory( ) );

    SamplerDesc linearSamplerDesc{ };
    m_linearSampler = std::unique_ptr<ISampler>( m_logicalDevice->CreateSampler( SamplerDesc{ } ) );

    m_frameData.resize( m_desc.NumFrames );

    for ( uint32_t frameIdx = 0; frameIdx < m_desc.NumFrames; ++frameIdx )
    {
        FrameData &frame = m_frameData[ frameIdx ];

        ResourceBindGroupDesc constantGroupDesc{ };
        constantGroupDesc.RootSignature = m_rootSignature.get( );
        constantGroupDesc.RegisterSpace = 1;
        frame.ConstantsBindGroup        = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( constantGroupDesc ) );

        BindBufferDesc bindUniformsDesc{ };
        bindUniformsDesc.Resource       = m_uniformBuffer.get( );
        bindUniformsDesc.ResourceOffset = frameIdx * m_alignedUniformSize;

        frame.ConstantsBindGroup->BeginUpdate( )->Cbv( bindUniformsDesc )->EndUpdate( );

        ResourceBindGroupDesc textureGroupDesc{ };
        textureGroupDesc.RootSignature = m_rootSignature.get( );
        textureGroupDesc.RegisterSpace = 0; // Todo metal needs bindless arrays at 0
        frame.TextureBindGroup         = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( textureGroupDesc ) );
    }

    for ( uint32_t frameIdx = 0; frameIdx < m_desc.NumFrames; ++frameIdx )
    {
        UpdateTextureBindings( frameIdx );
    }
}

void ClayRenderer::CreateNullTexture( )
{
    TextureDesc textureDesc{ };
    textureDesc.Width      = 1;
    textureDesc.Height     = 1;
    textureDesc.Format     = Format::R8G8B8A8Unorm;
    textureDesc.Usages     = BitSet( ResourceUsage::ShaderResource );
    textureDesc.Descriptor = BitSet( ResourceDescriptor::Texture );
    textureDesc.HeapType   = HeapType::GPU;
    textureDesc.DebugName  = InteropString( "UI Null Texture" );

    m_nullTexture   = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) );
    m_textures[ 0 ] = m_nullTexture.get( );
}

void ClayRenderer::CreateRenderTargets( )
{
    for ( uint32_t frameIdx = 0; frameIdx < m_desc.NumFrames; ++frameIdx )
    {
        FrameData &frame = m_frameData[ frameIdx ];

        TextureDesc colorDesc{ };
        colorDesc.Width        = static_cast<uint32_t>( m_viewportWidth );
        colorDesc.Height       = static_cast<uint32_t>( m_viewportHeight );
        colorDesc.Format       = m_desc.RenderTargetFormat;
        colorDesc.Usages       = BitSet( ResourceUsage::RenderTarget ) | ResourceUsage::ShaderResource;
        colorDesc.InitialUsage = BitSet( ResourceUsage::RenderTarget );
        colorDesc.Descriptor   = BitSet( ResourceDescriptor::RenderTarget ) | ResourceDescriptor::Texture;
        colorDesc.HeapType     = HeapType::GPU;
        colorDesc.DebugName    = InteropString( "UI Color Target Frame " ).Append( std::to_string( frameIdx ).c_str( ) );

        frame.ColorTarget = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( colorDesc ) );

        TextureDesc depthDesc{ };
        depthDesc.Width        = static_cast<uint32_t>( m_viewportWidth );
        depthDesc.Height       = static_cast<uint32_t>( m_viewportHeight );
        depthDesc.Format       = Format::D32Float;
        depthDesc.Usages       = BitSet( ResourceUsage::DepthWrite ) | ResourceUsage::DepthRead;
        depthDesc.InitialUsage = BitSet( ResourceUsage::DepthWrite ) | ResourceUsage::DepthRead;
        depthDesc.Descriptor   = BitSet( ResourceDescriptor::DepthStencil ) | ResourceDescriptor::Texture;
        depthDesc.HeapType     = HeapType::GPU;
        depthDesc.DebugName    = InteropString( "UI Depth Buffer Frame " ).Append( std::to_string( frameIdx ).c_str( ) );

        frame.DepthBuffer = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( depthDesc ) );

        m_resourceTracking.TrackTexture( frame.ColorTarget.get( ), ResourceUsage::RenderTarget );
        m_resourceTracking.TrackTexture( frame.DepthBuffer.get( ), ResourceUsage::DepthWrite );
    }
}

void ClayRenderer::UpdateProjectionMatrix( )
{
    const XMMATRIX projection = XMMatrixOrthographicOffCenterLH( 0.0f, m_viewportWidth, m_viewportHeight, 0.0f, 0.0f, 1.0f );
    XMStoreFloat4x4( &m_projectionMatrix, projection );
}

void ClayRenderer::AddFont( const uint16_t fontId, Font *font )
{
    if ( font == nullptr )
    {
        LOG( ERROR ) << "ClayRenderer::AddFont: Font cannot be null";
        return;
    }

    auto &fontData   = m_fonts[ fontId ];
    fontData.FontPtr = font;
    InitializeFontAtlas( &fontData );
}

void ClayRenderer::RemoveFont( const uint16_t fontId )
{
    const auto it = m_fonts.find( fontId );
    if ( it != m_fonts.end( ) )
    {
        if ( it->second.TextureIndex > 0 && it->second.TextureIndex < m_textures.size( ) )
        {
            m_textures[ it->second.TextureIndex ] = nullptr;
            m_texturesDirty                       = true; // Mark textures as dirty when removing
        }
        m_fonts.erase( it );
    }
}

void ClayRenderer::Resize( const float width, const float height )
{
    m_viewportWidth  = width;
    m_viewportHeight = height;
    CreateRenderTargets( );
    UpdateProjectionMatrix( );
}

void ClayRenderer::SetDpiScale( const float dpiScale )
{
    m_dpiScale = dpiScale;
}

void ClayRenderer::SetDeltaTime( const float deltaTime )
{
    m_deltaTime = deltaTime;
}

void ClayRenderer::Render( ICommandList *commandList, const Clay_RenderCommandArray commands, const uint32_t frameIndex )
{
    if ( frameIndex >= m_frameData.size( ) )
    {
        LOG( ERROR ) << "ClayRenderer::Render: Invalid frame index " << frameIndex;
        return;
    }

    const FrameData &frame = m_frameData[ frameIndex ];
    frame.FrameFence->Wait( );

    RenderInternal( commandList, commands, frameIndex );
}

void ClayRenderer::RenderInternal( ICommandList *commandList, Clay_RenderCommandArray commands, uint32_t frameIndex )
{
    if ( commands.length == 0 )
    {
        return;
    }

    ++m_currentFrame;

    if ( m_currentFrame % 6000 /*frame*/ == 0 )
    {
        m_shapeCache.Cleanup( m_currentFrame );
        m_textVertexCache.Cleanup( m_currentFrame );
    }

    if ( m_currentFrame % 3000 /*frame*/ == 0 )
    {
        CleanupTextLayoutCache( );
    }

    m_batchedVertices.Clear( );
    m_batchedIndices.Clear( );

    m_batchedVertices.Reserve( commands.length * 6 );
    m_batchedIndices.Reserve( commands.length * 9 );

    m_currentDepth = 0.9f; // Depth starts from high goes low, lowest values are rendered

    UIUniforms tempUniforms;
    tempUniforms.Projection = m_projectionMatrix;
    tempUniforms.ScreenSize = XMFLOAT4( m_viewportWidth, m_viewportHeight, 0.0f, 0.0f );

    float atlasWidth  = 512.0f;
    float atlasHeight = 512.0f;
    for ( const auto &fontData : m_fonts | std::views::values )
    {
        // Todo we're using first one for now
        if ( fontData.FontPtr && fontData.FontPtr->Asset( ) )
        {
            atlasWidth  = static_cast<float>( fontData.FontPtr->Asset( )->AtlasWidth );
            atlasHeight = static_cast<float>( fontData.FontPtr->Asset( )->AtlasHeight );
            break;
        }
    }
    tempUniforms.FontParams = XMFLOAT4( atlasWidth, atlasHeight, Font::MsdfPixelRange, 0.0f );

    uint8_t *uniformLocation = reinterpret_cast<uint8_t *>( m_uniformBufferData ) + frameIndex * m_alignedUniformSize;
    memcpy( uniformLocation, &tempUniforms, sizeof( UIUniforms ) );

    const FrameData &frame = m_frameData[ frameIndex ];
    if ( m_texturesDirty )
    {
        for ( uint32_t i = 0; i < m_desc.NumFrames; ++i )
        {
            UpdateTextureBindings( i );
        }
        m_texturesDirty = false;
    }

    // Clear batches from previous frame
    m_drawBatches.clear( );
    m_totalVertexCount = 0;
    m_totalIndexCount  = 0;
    m_currentDepth     = 0.9f;

    // Generate vertices
    for ( int32_t i = 0; i < commands.length; ++i )
    {
        const Clay_RenderCommand *cmd = Clay_RenderCommandArray_Get( &commands, i );
        ProcessRenderCommand( cmd, commandList );
    }

    ICommandList *uiCmdList = frame.CommandList;
    uiCmdList->Begin( );

    BatchTransitionDesc batchTransitionDesc{ uiCmdList };
    batchTransitionDesc.TransitionTexture( frame.ColorTarget.get( ), ResourceUsage::RenderTarget );
    batchTransitionDesc.TransitionTexture( frame.DepthBuffer.get( ), ResourceUsage::DepthWrite );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    {
        RenderingDesc            renderingDesc{ };
        RenderingAttachmentDesc &colorAttachment = renderingDesc.RTAttachments.EmplaceElement( );
        colorAttachment.Resource                 = frame.ColorTarget.get( );
        colorAttachment.LoadOp                   = LoadOp::Clear;
        colorAttachment.StoreOp                  = StoreOp::Store;
        colorAttachment.SetClearColor( 0.0f, 0.0f, 0.0f, 1.0f ); // Clear to transparent

        renderingDesc.DepthAttachment.Resource = frame.DepthBuffer.get( );
        renderingDesc.DepthAttachment.LoadOp   = LoadOp::Clear;
        renderingDesc.DepthAttachment.StoreOp  = StoreOp::DontCare;
        renderingDesc.DepthAttachment.SetClearDepthStencil( 1.0f, 0.0f ); // Clear to far depth

        renderingDesc.RenderAreaWidth   = m_viewportWidth;
        renderingDesc.RenderAreaHeight  = m_viewportHeight;
        renderingDesc.RenderAreaOffsetX = 0.0f;
        renderingDesc.RenderAreaOffsetY = 0.0f;

        uiCmdList->BeginRendering( renderingDesc );
        uiCmdList->BindViewport( 0.0f, 0.0f, m_viewportWidth, m_viewportHeight );
        uiCmdList->BindScissorRect( 0.0f, 0.0f, m_viewportWidth, m_viewportHeight );
        uiCmdList->BindPipeline( m_pipeline.get( ) );
        uiCmdList->BindResourceGroup( frame.ConstantsBindGroup.get( ) );
        uiCmdList->BindResourceGroup( frame.TextureBindGroup.get( ) );

        FlushBatchedGeometry( uiCmdList );
        uiCmdList->EndRendering( );
    }

    batchTransitionDesc = BatchTransitionDesc{ uiCmdList };
    batchTransitionDesc.TransitionTexture( frame.ColorTarget.get( ), ResourceUsage::ShaderResource );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    uiCmdList->End( );

    ExecuteCommandListsDesc executeDesc{ };
    executeDesc.CommandLists.AddElement( uiCmdList );
    executeDesc.Signal = frame.FrameFence.get( );
    m_commandQueue->ExecuteCommandLists( executeDesc );

    m_fullscreenQuad->UpdateTarget( frameIndex, frame.ColorTarget.get( ) );
    m_fullscreenQuad->DrawTextureToScreen( commandList, frameIndex );
}

void ClayRenderer::ProcessRenderCommand( const Clay_RenderCommand *command, ICommandList *commandList )
{
    switch ( command->commandType )
    {
    case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
        RenderRectangle( command, commandList );
        break;

    case CLAY_RENDER_COMMAND_TYPE_BORDER:
        RenderBorder( command );
        break;

    case CLAY_RENDER_COMMAND_TYPE_TEXT:
        RenderText( command, commandList );
        break;

    case CLAY_RENDER_COMMAND_TYPE_IMAGE:
        RenderImage( command );
        break;

    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
        SetScissor( command );
        break;

    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
        ClearScissor( );
        break;

    case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
        RenderCustom( command, commandList );
        break;

    default:
        LOG( WARNING ) << "Unknown Clay render command type: " << command->commandType;
        break;
    }
}

void ClayRenderer::RenderRectangle( const Clay_RenderCommand *command, ICommandList *commandList )
{
    const auto &data   = command->renderData.rectangle;
    const auto &bounds = command->boundingBox;

    const ShapeCacheKey cacheKey = UIShapeCache::CreateRectangleKey( command );
    CachedShape        *cached   = m_shapeCache.GetOrCreateCachedShape( cacheKey, m_currentFrame );

    if ( cached->vertices.NumElements( ) == 0 )
    {
        constexpr uint32_t currentVertexCount = 0;

        if ( data.cornerRadius.topLeft > 0 || data.cornerRadius.topRight > 0 || data.cornerRadius.bottomLeft > 0 || data.cornerRadius.bottomRight > 0 )
        {
            UIShapes::GenerateRoundedRectangleDesc desc{ };
            desc.Bounds            = bounds;
            desc.Color             = data.backgroundColor;
            desc.CornerRadius      = data.cornerRadius;
            desc.TextureIndex      = 0; // Solid color
            desc.SegmentsPerCorner = 8;

            UIShapes::GenerateRoundedRectangle( desc, &cached->vertices, &cached->indices, currentVertexCount );
        }
        else
        {
            UIShapes::GenerateRectangleDesc desc{ };
            desc.Bounds       = bounds;
            desc.Color        = data.backgroundColor;
            desc.TextureIndex = 0;

            UIShapes::GenerateRectangle( desc, &cached->vertices, &cached->indices, currentVertexCount );
        }
    }

    if ( cached->vertices.NumElements( ) > 0 && cached->indices.NumElements( ) > 0 )
    {
        AddVerticesWithDepth( cached->vertices, cached->indices );
    }
}

void ClayRenderer::RenderBorder( const Clay_RenderCommand *command )
{
    const auto &data   = command->renderData.border;
    const auto &bounds = command->boundingBox;

    const ShapeCacheKey cacheKey = UIShapeCache::CreateBorderKey( command );
    CachedShape        *cached   = m_shapeCache.GetOrCreateCachedShape( cacheKey, m_currentFrame );

    if ( cached->vertices.NumElements( ) == 0 )
    {
        constexpr uint32_t currentVertexCount = 0;

        UIShapes::GenerateBorderDesc desc{ };
        desc.Bounds            = bounds;
        desc.Color             = data.color;
        desc.BorderWidth       = data.width;
        desc.CornerRadius      = data.cornerRadius;
        desc.SegmentsPerCorner = 8;

        UIShapes::GenerateBorder( desc, &cached->vertices, &cached->indices, currentVertexCount );
    }

    if ( cached->vertices.NumElements( ) > 0 && cached->indices.NumElements( ) > 0 )
    {
        AddVerticesWithDepth( cached->vertices, cached->indices );
    }
}

void ClayRenderer::RenderText( const Clay_RenderCommand *command, ICommandList *commandList )
{
    const auto &data   = command->renderData.text;
    const auto &bounds = command->boundingBox;

    const FontData *fontData = GetFontData( data.fontId );
    if ( fontData == nullptr || fontData->FontPtr == nullptr )
    {
        LOG( WARNING ) << "Font not found for ID: " << data.fontId;
        return;
    }
    const float baseSize       = static_cast<float>( fontData->FontPtr->Asset( )->InitialFontSize );
    const float targetSize     = data.fontSize > 0 ? data.fontSize * m_dpiScale : baseSize;
    const float effectiveScale = targetSize / baseSize;

    const std::string textStr( data.stringContents.chars, data.stringContents.length );
    if ( textStr.find( '\n' ) != std::string::npos )
    {
        std::vector<std::string> lines;
        size_t                   start = 0;
        size_t                   pos   = 0;

        while ( pos <= textStr.length( ) )
        {
            if ( pos == textStr.length( ) || textStr[ pos ] == '\n' )
            {
                lines.push_back( textStr.substr( start, pos - start ) );
                start = pos + 1;
            }
            pos++;
        }

        const float fontAscent        = static_cast<float>( fontData->FontPtr->Asset( )->Metrics.Ascent ) * effectiveScale;
        const float fontDescent       = static_cast<float>( fontData->FontPtr->Asset( )->Metrics.Descent ) * effectiveScale;
        const float defaultLineHeight = ( fontAscent + fontDescent ) * 1.2f; // Default 1.2x line spacing
        const float lineHeight        = data.lineHeight > 0 ? data.lineHeight : defaultLineHeight;

        float currentY = bounds.y * m_dpiScale + fontAscent;

        for ( const auto &line : lines )
        {
            if ( !line.empty( ) )
            {
                Clay_RenderCommand lineCommand                    = *command;
                lineCommand.renderData.text.stringContents.chars  = line.c_str( );
                lineCommand.renderData.text.stringContents.length = static_cast<int32_t>( line.length( ) );
                lineCommand.boundingBox.y                         = currentY / m_dpiScale - fontAscent / m_dpiScale;

                RenderSingleLineText( &lineCommand, fontData, effectiveScale, fontAscent );
            }
            currentY += lineHeight;
        }
    }
    else
    {
        const float fontAscent = static_cast<float>( fontData->FontPtr->Asset( )->Metrics.Ascent ) * effectiveScale;
        RenderSingleLineText( command, fontData, effectiveScale, fontAscent );
    }
}

void ClayRenderer::RenderSingleLineText( const Clay_RenderCommand *command, const FontData *fontData, float effectiveScale, float fontAscent )
{
    const auto &data   = command->renderData.text;
    const auto &bounds = command->boundingBox;

    const TextLayout *textLayout = GetOrCreateShapedText( command, fontData->FontPtr );

    const float adjustedY = bounds.y * m_dpiScale + fontAscent;

    const TextVertexCacheKey vertexCacheKey = UITextVertexCache::CreateTextVertexKey( command, effectiveScale, adjustedY, m_dpiScale );
    CachedTextVertices      *cachedVertices = m_textVertexCache.GetOrCreateCachedTextVertices( vertexCacheKey, m_currentFrame );

    if ( cachedVertices->vertices.NumElements( ) == 0 )
    {
        InteropArray<GlyphVertex> glyphVertices;
        InteropArray<uint32_t>    glyphIndices;

        GenerateTextVerticesDesc generateDesc{ };
        generateDesc.StartPosition = Float_2{ bounds.x * m_dpiScale, adjustedY };
        generateDesc.Color         = Float_4{ data.textColor.r / 255.0f, data.textColor.g / 255.0f, data.textColor.b / 255.0f, data.textColor.a / 255.0f };
        generateDesc.OutVertices   = &glyphVertices;
        generateDesc.OutIndices    = &glyphIndices;
        generateDesc.Scale         = effectiveScale;
        generateDesc.LetterSpacing = data.letterSpacing * m_dpiScale;
        generateDesc.LineHeight    = data.lineHeight;

        textLayout->GenerateTextVertices( generateDesc );

        if ( glyphVertices.NumElements( ) > 0 && glyphIndices.NumElements( ) > 0 )
        {
            for ( uint32_t i = 0; i < glyphVertices.NumElements( ); ++i )
            {
                const GlyphVertex glyph = glyphVertices.GetElement( i );
                UIVertex          vertex;
                vertex.Position     = XMFLOAT3( glyph.Position.X, glyph.Position.Y, 0.0f ); // Z will be set in AddVerticesWithDepth
                vertex.TexCoord     = XMFLOAT2( glyph.UV.X, glyph.UV.Y );
                vertex.Color        = XMFLOAT4( glyph.Color.X, glyph.Color.Y, glyph.Color.Z, glyph.Color.W );
                vertex.TextureIndex = fontData->TextureIndex;
                cachedVertices->vertices.AddElement( vertex );
            }

            for ( uint32_t i = 0; i < glyphIndices.NumElements( ); ++i )
            {
                cachedVertices->indices.AddElement( glyphIndices.GetElement( i ) );
            }
        }
    }

    if ( cachedVertices->vertices.NumElements( ) > 0 && cachedVertices->indices.NumElements( ) > 0 )
    {
        AddVerticesWithDepth( cachedVertices->vertices, cachedVertices->indices );
    }
}

void ClayRenderer::RenderImage( const Clay_RenderCommand *command )
{
    const auto &data   = command->renderData.image;
    const auto &bounds = command->boundingBox;

    uint32_t   textureIndex = 0;
    const auto it           = m_imageTextureIndices.find( data.imageData );
    if ( it != m_imageTextureIndices.end( ) )
    {
        textureIndex = it->second;
    }
    else
    {
        const auto texture                      = static_cast<ITextureResource *>( data.imageData );
        textureIndex                            = RegisterTexture( texture );
        m_imageTextureIndices[ data.imageData ] = textureIndex;
    }

    InteropArray<UIVertex> vertices;
    InteropArray<uint32_t> indices;

    UIShapes::GenerateRectangleDesc desc{ };
    desc.Bounds       = bounds;
    desc.Color        = Clay_Color{ 255, 255, 255, 255 }; // White to show texture colors
    desc.TextureIndex = textureIndex;

    UIShapes::GenerateRectangle( desc, &vertices, &indices, 0 );
    if ( vertices.NumElements( ) > 0 && indices.NumElements( ) > 0 )
    {
        AddVerticesWithDepth( vertices, indices );
    }
}

void ClayRenderer::RenderCustom( const Clay_RenderCommand *command, ICommandList *commandList )
{
    const auto &data = command->renderData.custom;
    if ( data.customData == nullptr )
    {
        return;
    }

    const auto *widgetData = static_cast<const ClayCustomWidgetData *>( data.customData );
    if ( widgetData == nullptr || widgetData->Data == nullptr )
    {
        LOG( WARNING ) << "Invalid custom widget data in RenderCustom";
        return;
    }

    switch ( widgetData->Type )
    {
    case ClayCustomWidgetType::TextField:
        {
            const auto *textFieldData = static_cast<const ClayTextFieldRenderData *>( widgetData->Data );
            RenderTextField( command, textFieldData, commandList );
            break;
        }
    case ClayCustomWidgetType::Checkbox:
        {
            const auto *checkboxData = static_cast<const ClayCheckboxRenderData *>( widgetData->Data );
            RenderCheckbox( command, checkboxData, commandList );
            break;
        }
    case ClayCustomWidgetType::Slider:
        {
            const auto *sliderData = static_cast<const ClaySliderRenderData *>( widgetData->Data );
            RenderSlider( command, sliderData, commandList );
            break;
        }
    case ClayCustomWidgetType::Dropdown:
        {
            const auto *dropdownData = static_cast<const ClayDropdownRenderData *>( widgetData->Data );
            RenderDropdown( command, dropdownData, commandList );
            break;
        }
    case ClayCustomWidgetType::ColorPicker:
        {
            const auto *colorPickerData = static_cast<const ClayColorPickerRenderData *>( widgetData->Data );
            RenderColorPicker( command, colorPickerData, commandList );
            break;
        }
    case ClayCustomWidgetType::ResizableContainer:
        {
            const auto *resizableData = static_cast<const ClayResizableContainerRenderData *>( widgetData->Data );
            RenderResizableContainer( command, resizableData, commandList );
            break;
        }
    case ClayCustomWidgetType::DockableContainer:
        {
            const auto *dockableData = static_cast<const ClayDockableContainerRenderData *>( widgetData->Data );
            RenderDockableContainer( command, dockableData, commandList );
            break;
        }
    default:
        LOG( WARNING ) << "Unknown custom widget type in RenderCustom: " << static_cast<uint32_t>( widgetData->Type );
        break;
    }
}

void ClayRenderer::RenderTextField( const Clay_RenderCommand *command, const ClayTextFieldRenderData *textFieldData, ICommandList *commandList )
{
    const auto &bounds = command->boundingBox;
    const auto *state  = textFieldData->State;
    const auto &desc   = textFieldData->Desc;

    constexpr float CURSOR_BLINK_PERIOD = 1.0f; // Blink every second

    const_cast<ClayTextFieldState *>( state )->CursorBlinkTime += m_deltaTime;
    if ( state->CursorBlinkTime >= CURSOR_BLINK_PERIOD )
    {
        const_cast<ClayTextFieldState *>( state )->CursorBlinkTime = 0.0f;
        const_cast<ClayTextFieldState *>( state )->CursorVisible   = !state->CursorVisible;
    }

    InteropArray<UIVertex> backgroundVertices;
    InteropArray<uint32_t> backgroundIndices;

    UIShapes::GenerateRectangleDesc backgroundDesc{ };
    backgroundDesc.Bounds       = bounds;
    backgroundDesc.Color        = Clay_Color{ desc.BackgroundColor.R, desc.BackgroundColor.G, desc.BackgroundColor.B, desc.BackgroundColor.A };
    backgroundDesc.TextureIndex = 0; // Solid color

    UIShapes::GenerateRectangle( backgroundDesc, &backgroundVertices, &backgroundIndices, 0 );
    if ( backgroundVertices.NumElements( ) > 0 && backgroundIndices.NumElements( ) > 0 )
    {
        AddVerticesWithDepth( backgroundVertices, backgroundIndices );
    }

    InteropArray<UIVertex> borderVertices;
    InteropArray<uint32_t> borderIndices;

    const ClayColor             &borderColor = state->IsFocused ? desc.FocusBorderColor : desc.BorderColor;
    UIShapes::GenerateBorderDesc borderDesc{ };
    borderDesc.Bounds       = bounds;
    borderDesc.Color        = Clay_Color{ borderColor.R, borderColor.G, borderColor.B, borderColor.A };
    borderDesc.BorderWidth  = Clay_BorderWidth{ 1, 1, 1, 1, 0 };
    borderDesc.CornerRadius = Clay_CornerRadius{ 0, 0, 0, 0 };

    UIShapes::GenerateBorder( borderDesc, &borderVertices, &borderIndices, 0 );
    if ( borderVertices.NumElements( ) > 0 && borderIndices.NumElements( ) > 0 )
    {
        AddVerticesWithDepth( borderVertices, borderIndices );
    }

    if ( state->HasSelection && state->SelectionStart != state->SelectionEnd )
    {
        size_t selStart = std::min( state->SelectionStart, state->SelectionEnd );
        size_t selEnd   = std::max( state->SelectionStart, state->SelectionEnd );
        selStart        = std::min( selStart, state->Text.NumChars( ) );
        selEnd          = std::min( selEnd, state->Text.NumChars( ) );

        std::string textStr = state->Text.Get( );

        if ( selStart < selEnd )
        {
            Clay_TextElementConfig measureConfig{ };
            measureConfig.fontId        = desc.FontId;
            measureConfig.fontSize      = desc.FontSize;
            measureConfig.textColor     = Clay_Color{ };
            measureConfig.wrapMode      = CLAY_TEXT_WRAP_NONE;
            measureConfig.textAlignment = CLAY_TEXT_ALIGN_LEFT;

            const ClayDimensions lineTextSize    = MeasureText( InteropString( "I" ), measureConfig );
            const float          lineHeight      = desc.LineHeight > 0 ? desc.LineHeight : lineTextSize.Height * 1.2f;
            const float          selectionHeight = lineTextSize.Height;

            if ( desc.Type == ClayTextFieldType::MultiLine )
            {
                const std::string textBeforeSelection = textStr.substr( 0, selStart );
                const std::string selectedText        = textStr.substr( selStart, selEnd - selStart );

                size_t startLine              = 0;
                size_t lastNewlineBeforeStart = 0;
                for ( size_t i = 0; i < textBeforeSelection.length( ); ++i )
                {
                    if ( textBeforeSelection[ i ] == '\n' )
                    {
                        startLine++;
                        lastNewlineBeforeStart = i + 1;
                    }
                }

                const std::string    textOnStartLine = textBeforeSelection.substr( lastNewlineBeforeStart );
                const ClayDimensions startLineSize   = MeasureText( InteropString( textOnStartLine.c_str( ) ), measureConfig );

                float currentY = bounds.y + desc.Padding.Top + startLine * lineHeight;
                float currentX = bounds.x + desc.Padding.Left + startLineSize.Width;

                size_t currentPos = 0;
                while ( currentPos < selectedText.length( ) )
                {
                    // Find next newline or end of selection
                    size_t nextNewline = selectedText.find( '\n', currentPos );
                    if ( nextNewline == std::string::npos )
                    {
                        nextNewline = selectedText.length( );
                    }

                    const std::string    lineText = selectedText.substr( currentPos, nextNewline - currentPos );
                    const ClayDimensions lineSize = MeasureText( InteropString( lineText.c_str( ) ), measureConfig );

                    InteropArray<UIVertex> selectionVertices;
                    InteropArray<uint32_t> selectionIndices;

                    Clay_BoundingBox selectionBounds;
                    selectionBounds.x      = currentX;
                    selectionBounds.y      = currentY;
                    selectionBounds.width  = lineSize.Width;
                    selectionBounds.height = selectionHeight;

                    UIShapes::GenerateRectangleDesc selectionDesc{ };
                    selectionDesc.Bounds       = selectionBounds;
                    selectionDesc.Color        = Clay_Color{ desc.SelectionColor.R, desc.SelectionColor.G, desc.SelectionColor.B, desc.SelectionColor.A };
                    selectionDesc.TextureIndex = 0;

                    UIShapes::GenerateRectangle( selectionDesc, &selectionVertices, &selectionIndices, 0 );
                    if ( selectionVertices.NumElements( ) > 0 && selectionIndices.NumElements( ) > 0 )
                    {
                        AddVerticesWithDepth( selectionVertices, selectionIndices );
                    }

                    if ( nextNewline < selectedText.length( ) )
                    {
                        currentY += lineHeight;
                        currentX   = bounds.x + desc.Padding.Left;
                        currentPos = nextNewline + 1;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                float selectionStartX = bounds.x + desc.Padding.Left;
                if ( selStart > 0 )
                {
                    const std::string    textBeforeSelection = textStr.substr( 0, selStart );
                    const ClayDimensions beforeSize          = MeasureText( InteropString( textBeforeSelection.c_str( ) ), measureConfig );
                    selectionStartX += beforeSize.Width;
                }

                const std::string    selectedText = textStr.substr( selStart, selEnd - selStart );
                const ClayDimensions selectedSize = MeasureText( InteropString( selectedText.c_str( ) ), measureConfig );

                InteropArray<UIVertex> selectionVertices;
                InteropArray<uint32_t> selectionIndices;

                Clay_BoundingBox selectionBounds;
                selectionBounds.x      = selectionStartX;
                selectionBounds.y      = bounds.y + desc.Padding.Top;
                selectionBounds.width  = selectedSize.Width;
                selectionBounds.height = selectionHeight;

                UIShapes::GenerateRectangleDesc selectionDesc{ };
                selectionDesc.Bounds       = selectionBounds;
                selectionDesc.Color        = Clay_Color{ desc.SelectionColor.R, desc.SelectionColor.G, desc.SelectionColor.B, desc.SelectionColor.A };
                selectionDesc.TextureIndex = 0;

                UIShapes::GenerateRectangle( selectionDesc, &selectionVertices, &selectionIndices, 0 );
                if ( selectionVertices.NumElements( ) > 0 && selectionIndices.NumElements( ) > 0 )
                {
                    AddVerticesWithDepth( selectionVertices, selectionIndices );
                }
            }
        }
    }

    const auto      &displayText = std::string( state->Text.NumChars( ) == 0 ? desc.PlaceholderText.Get( ) : state->Text.Get( ) );
    const ClayColor &textColor   = state->Text.NumChars( ) == 0 ? desc.PlaceholderColor : desc.TextColor;

    if ( !displayText.empty( ) )
    {
        Clay_RenderCommand tempTextCommand                    = *command;
        tempTextCommand.commandType                           = CLAY_RENDER_COMMAND_TYPE_TEXT;
        tempTextCommand.renderData.text.stringContents.chars  = displayText.c_str( );
        tempTextCommand.renderData.text.stringContents.length = static_cast<int32_t>( displayText.length( ) );
        tempTextCommand.renderData.text.textColor             = Clay_Color{ textColor.R, textColor.G, textColor.B, textColor.A };
        tempTextCommand.renderData.text.fontId                = desc.FontId;
        tempTextCommand.renderData.text.fontSize              = desc.FontSize;
        tempTextCommand.renderData.text.letterSpacing         = 0;
        tempTextCommand.renderData.text.lineHeight            = desc.LineHeight;

        tempTextCommand.boundingBox.x += desc.Padding.Left;
        tempTextCommand.boundingBox.y += desc.Padding.Top;
        tempTextCommand.boundingBox.width -= desc.Padding.Left + desc.Padding.Right;
        tempTextCommand.boundingBox.height -= desc.Padding.Top + desc.Padding.Bottom;

        RenderText( &tempTextCommand, commandList );
    }

    if ( state->IsFocused && state->CursorVisible && !desc.ReadOnly )
    {
        float cursorX = bounds.x + desc.Padding.Left;
        float cursorY = bounds.y + desc.Padding.Top;

        Clay_TextElementConfig cursorMeasureConfig{ };
        cursorMeasureConfig.fontId        = desc.FontId;
        cursorMeasureConfig.fontSize      = desc.FontSize;
        cursorMeasureConfig.textColor     = Clay_Color{ };
        cursorMeasureConfig.wrapMode      = CLAY_TEXT_WRAP_NONE;
        cursorMeasureConfig.textAlignment = CLAY_TEXT_ALIGN_LEFT;

        const ClayDimensions cursorTextSize = MeasureText( InteropString( "I" ), cursorMeasureConfig );
        const float          cursorHeight   = cursorTextSize.Height;
        const float          lineHeight     = desc.LineHeight > 0 ? desc.LineHeight : cursorTextSize.Height * 1.2f;

        if ( !state->Text.NumChars( ) == 0 && state->CursorPosition > 0 )
        {
            std::string       textStr          = state->Text.Get( );
            const std::string textBeforeCursor = textStr.substr( 0, std::min( state->CursorPosition, state->Text.NumChars( ) ) );

            if ( desc.Type == ClayTextFieldType::MultiLine )
            {
                // Count newlines before cursor to determine which line we're on
                size_t lineNumber     = 0;
                size_t lastNewlinePos = 0;

                for ( size_t i = 0; i < textBeforeCursor.length( ); ++i )
                {
                    if ( textBeforeCursor[ i ] == '\n' )
                    {
                        lineNumber++;
                        lastNewlinePos = i + 1;
                    }
                }

                const std::string textOnCurrentLine = textBeforeCursor.substr( lastNewlinePos );

                Clay_TextElementConfig measureConfig{ };
                measureConfig.fontId        = desc.FontId;
                measureConfig.fontSize      = desc.FontSize;
                measureConfig.textColor     = Clay_Color{ };
                measureConfig.wrapMode      = CLAY_TEXT_WRAP_NONE;
                measureConfig.textAlignment = CLAY_TEXT_ALIGN_LEFT;

                const ClayDimensions textSize = MeasureText( InteropString( textOnCurrentLine.c_str( ) ), measureConfig );
                cursorX += textSize.Width;
                cursorY += lineNumber * lineHeight;
            }
            else
            {
                Clay_TextElementConfig measureConfig{ };
                measureConfig.fontId        = desc.FontId;
                measureConfig.fontSize      = desc.FontSize;
                measureConfig.textColor     = Clay_Color{ };
                measureConfig.wrapMode      = CLAY_TEXT_WRAP_NONE;
                measureConfig.textAlignment = CLAY_TEXT_ALIGN_LEFT;

                const ClayDimensions textSize = MeasureText( InteropString( textBeforeCursor.c_str( ) ), measureConfig );
                cursorX += textSize.Width;
            }
        }

        InteropArray<UIVertex> cursorVertices;
        InteropArray<uint32_t> cursorIndices;

        Clay_BoundingBox cursorBounds;
        cursorBounds.x      = cursorX;
        cursorBounds.y      = cursorY;
        cursorBounds.width  = desc.CursorWidth;
        cursorBounds.height = cursorHeight;

        UIShapes::GenerateRectangleDesc cursorDesc{ };
        cursorDesc.Bounds       = cursorBounds;
        cursorDesc.Color        = Clay_Color{ desc.CursorColor.R, desc.CursorColor.G, desc.CursorColor.B, desc.CursorColor.A };
        cursorDesc.TextureIndex = 0; // Solid color

        UIShapes::GenerateRectangle( cursorDesc, &cursorVertices, &cursorIndices, 0 );
        if ( cursorVertices.NumElements( ) > 0 && cursorIndices.NumElements( ) > 0 )
        {
            AddVerticesWithDepth( cursorVertices, cursorIndices );
        }
    }
}

void ClayRenderer::RenderCheckbox( const Clay_RenderCommand *command, const ClayCheckboxRenderData *checkboxData, ICommandList *commandList )
{
    const auto &bounds = command->boundingBox;
    const auto *state  = checkboxData->State;
    const auto &desc   = checkboxData->Desc;

    if ( state->Checked )
    {
        InteropArray<UIVertex> checkVertices;
        InteropArray<uint32_t> checkIndices;

        const float checkSize    = desc.Size * 0.6f;
        const float checkOffsetX = bounds.x + ( desc.Size - checkSize ) * 0.5f;
        const float checkOffsetY = bounds.y + ( desc.Size - checkSize ) * 0.5f;

        Clay_BoundingBox checkBounds;
        checkBounds.x      = checkOffsetX;
        checkBounds.y      = checkOffsetY;
        checkBounds.width  = checkSize;
        checkBounds.height = checkSize;

        UIShapes::GenerateRectangleDesc checkDesc{ };
        checkDesc.Bounds       = checkBounds;
        checkDesc.Color        = Clay_Color{ desc.CheckColor.R, desc.CheckColor.G, desc.CheckColor.B, desc.CheckColor.A };
        checkDesc.TextureIndex = 0;

        UIShapes::GenerateRectangle( checkDesc, &checkVertices, &checkIndices, 0 );
        if ( checkVertices.NumElements( ) > 0 && checkIndices.NumElements( ) > 0 )
        {
            AddVerticesWithDepth( checkVertices, checkIndices );
        }
    }
}

void ClayRenderer::RenderSlider( const Clay_RenderCommand *command, const ClaySliderRenderData *sliderData, ICommandList *commandList )
{
    const auto &bounds = command->boundingBox;
    const auto *state  = sliderData->State;
    const auto &desc   = sliderData->Desc;

    const float trackY       = bounds.y + ( bounds.height - desc.Height ) * 0.5f;
    const float trackPadding = desc.KnobSize * 0.5f;
    const float trackWidth   = bounds.width - trackPadding * 2.0f;

    Clay_BoundingBox trackBounds;
    trackBounds.x      = bounds.x + trackPadding;
    trackBounds.y      = trackY;
    trackBounds.width  = trackWidth;
    trackBounds.height = desc.Height;

    InteropArray<UIVertex> trackVertices;
    InteropArray<uint32_t> trackIndices;

    UIShapes::GenerateRoundedRectangleDesc trackDesc{ };
    trackDesc.Bounds       = trackBounds;
    trackDesc.Color        = Clay_Color{ desc.BackgroundColor.R, desc.BackgroundColor.G, desc.BackgroundColor.B, desc.BackgroundColor.A };
    trackDesc.TextureIndex = 0;
    trackDesc.CornerRadius = Clay_CornerRadius{ desc.CornerRadius, desc.CornerRadius, desc.CornerRadius, desc.CornerRadius };

    UIShapes::GenerateRoundedRectangle( trackDesc, &trackVertices, &trackIndices, 0 );
    if ( trackVertices.NumElements( ) > 0 && trackIndices.NumElements( ) > 0 )
    {
        AddVerticesWithDepth( trackVertices, trackIndices );
    }

    const float normalizedValue = ( state->Value - desc.MinValue ) / ( desc.MaxValue - desc.MinValue );
    const float fillWidth       = trackWidth * normalizedValue;

    if ( fillWidth > 0 )
    {
        Clay_BoundingBox fillBounds;
        fillBounds.x      = trackBounds.x;
        fillBounds.y      = trackBounds.y;
        fillBounds.width  = fillWidth;
        fillBounds.height = trackBounds.height;

        InteropArray<UIVertex> fillVertices;
        InteropArray<uint32_t> fillIndices;

        UIShapes::GenerateRoundedRectangleDesc fillDesc{ };
        fillDesc.Bounds       = fillBounds;
        fillDesc.Color        = Clay_Color{ desc.FillColor.R, desc.FillColor.G, desc.FillColor.B, desc.FillColor.A };
        fillDesc.TextureIndex = 0;
        fillDesc.CornerRadius = Clay_CornerRadius{ desc.CornerRadius, desc.CornerRadius, desc.CornerRadius, desc.CornerRadius };

        UIShapes::GenerateRoundedRectangle( fillDesc, &fillVertices, &fillIndices, 0 );
        if ( fillVertices.NumElements( ) > 0 && fillIndices.NumElements( ) > 0 )
        {
            AddVerticesWithDepth( fillVertices, fillIndices );
        }
    }

    const float knobX = trackBounds.x + normalizedValue * trackWidth - desc.KnobSize * 0.5f;
    const float knobY = bounds.y + ( bounds.height - desc.KnobSize ) * 0.5f;

    Clay_BoundingBox knobBounds;
    knobBounds.x      = knobX;
    knobBounds.y      = knobY;
    knobBounds.width  = desc.KnobSize;
    knobBounds.height = desc.KnobSize;

    InteropArray<UIVertex> knobVertices;
    InteropArray<uint32_t> knobIndices;

    UIShapes::GenerateRoundedRectangleDesc knobDesc{ };
    knobDesc.Bounds       = knobBounds;
    knobDesc.Color        = Clay_Color{ desc.KnobColor.R, desc.KnobColor.G, desc.KnobColor.B, desc.KnobColor.A };
    knobDesc.TextureIndex = 0;
    knobDesc.CornerRadius = Clay_CornerRadius{ desc.KnobSize * 0.5f, desc.KnobSize * 0.5f, desc.KnobSize * 0.5f, desc.KnobSize * 0.5f };

    UIShapes::GenerateRoundedRectangle( knobDesc, &knobVertices, &knobIndices, 0 );
    if ( knobVertices.NumElements( ) > 0 && knobIndices.NumElements( ) > 0 )
    {
        AddVerticesWithDepth( knobVertices, knobIndices );
    }

    InteropArray<UIVertex> knobBorderVertices;
    InteropArray<uint32_t> knobBorderIndices;

    UIShapes::GenerateBorderDesc knobBorderDesc{ };
    knobBorderDesc.Bounds       = knobBounds;
    knobBorderDesc.Color        = Clay_Color{ desc.KnobBorderColor.R, desc.KnobBorderColor.G, desc.KnobBorderColor.B, desc.KnobBorderColor.A };
    knobBorderDesc.BorderWidth  = Clay_BorderWidth{ 1, 1, 1, 1, 0 };
    knobBorderDesc.CornerRadius = Clay_CornerRadius{ desc.KnobSize * 0.5f, desc.KnobSize * 0.5f, desc.KnobSize * 0.5f, desc.KnobSize * 0.5f };

    UIShapes::GenerateBorder( knobBorderDesc, &knobBorderVertices, &knobBorderIndices, 0 );
    if ( knobBorderVertices.NumElements( ) > 0 && knobBorderIndices.NumElements( ) > 0 )
    {
        AddVerticesWithDepth( knobBorderVertices, knobBorderIndices );
    }
}

void ClayRenderer::RenderDropdown( const Clay_RenderCommand *command, const ClayDropdownRenderData *dropdownData, ICommandList *commandList )
{
    const auto &bounds = command->boundingBox;
    const auto *state  = dropdownData->State;
    const auto &desc   = dropdownData->Desc;

    const std::string &displayText = state->SelectedIndex >= 0 ? std::string( state->SelectedText.Get( ) ) : std::string( desc.PlaceholderText.Get( ) );
    const ClayColor   &textColor   = state->SelectedIndex >= 0 ? desc.TextColor : desc.PlaceholderColor;

    if ( !displayText.empty( ) )
    {
        Clay_RenderCommand tempTextCommand                    = *command;
        tempTextCommand.commandType                           = CLAY_RENDER_COMMAND_TYPE_TEXT;
        tempTextCommand.renderData.text.stringContents.chars  = displayText.c_str( );
        tempTextCommand.renderData.text.stringContents.length = static_cast<int32_t>( displayText.length( ) );
        tempTextCommand.renderData.text.textColor             = Clay_Color{ textColor.R, textColor.G, textColor.B, textColor.A };
        tempTextCommand.renderData.text.fontId                = desc.FontId;
        tempTextCommand.renderData.text.fontSize              = desc.FontSize;
        tempTextCommand.renderData.text.letterSpacing         = 0;
        tempTextCommand.renderData.text.lineHeight            = 0;

        tempTextCommand.boundingBox.x += desc.Padding.Left;
        tempTextCommand.boundingBox.y += desc.Padding.Top;
        tempTextCommand.boundingBox.width -= desc.Padding.Left + desc.Padding.Right + 20;
        tempTextCommand.boundingBox.height -= desc.Padding.Top + desc.Padding.Bottom;

        RenderText( &tempTextCommand, commandList );
    }

    constexpr float arrowSize = 8.0f;
    const float     arrowX    = bounds.x + bounds.width - desc.Padding.Right - arrowSize;
    const float     arrowY    = bounds.y + ( bounds.height - arrowSize ) * 0.5f;

    InteropArray<UIVertex> arrowVertices;
    InteropArray<uint32_t> arrowIndices;

    Clay_BoundingBox arrowBounds;
    arrowBounds.x      = arrowX;
    arrowBounds.y      = arrowY;
    arrowBounds.width  = arrowSize;
    arrowBounds.height = arrowSize;

    UIShapes::GenerateRectangleDesc arrowDesc{ };
    arrowDesc.Bounds       = arrowBounds;
    arrowDesc.Color        = Clay_Color{ desc.TextColor.R, desc.TextColor.G, desc.TextColor.B, desc.TextColor.A };
    arrowDesc.TextureIndex = 0;

    UIShapes::GenerateRectangle( arrowDesc, &arrowVertices, &arrowIndices, 0 );
    if ( arrowVertices.NumElements( ) > 0 && arrowIndices.NumElements( ) > 0 )
    {
        AddVerticesWithDepth( arrowVertices, arrowIndices );
    }
}

void ClayRenderer::RenderColorPicker( const Clay_RenderCommand *command, const ClayColorPickerRenderData *colorPickerData, ICommandList *commandList )
{
    const auto &bounds = command->boundingBox;
    const auto *state  = colorPickerData->State;
    const auto &desc   = colorPickerData->Desc;

    if ( !state->IsExpanded )
    {
        InteropArray<UIVertex> colorVertices;
        InteropArray<uint32_t> colorIndices;

        UIShapes::GenerateRectangleDesc colorDesc{ };
        colorDesc.Bounds       = bounds;
        colorDesc.Color        = Clay_Color{ std::clamp( state->Rgb.X * 255.0f, 0.0f, 255.0f ), std::clamp( state->Rgb.Y * 255.0f, 0.0f, 255.0f ),
                                      std::clamp( state->Rgb.Z * 255.0f, 0.0f, 255.0f ), 255.0f };
        colorDesc.TextureIndex = 0;

        UIShapes::GenerateRectangle( colorDesc, &colorVertices, &colorIndices, 0 );
        if ( colorVertices.NumElements( ) > 0 && colorIndices.NumElements( ) > 0 )
        {
            AddVerticesWithDepth( colorVertices, colorIndices );
        }

        // Add border for visibility
        InteropArray<UIVertex> borderVertices;
        InteropArray<uint32_t> borderIndices;

        UIShapes::GenerateBorderDesc borderDesc{ };
        borderDesc.Bounds       = bounds;
        borderDesc.Color        = Clay_Color{ 128, 128, 128, 255 }; // Gray border
        borderDesc.BorderWidth  = Clay_BorderWidth{ 1, 1, 1, 1, 0 };
        borderDesc.CornerRadius = Clay_CornerRadius{ 4, 4, 4, 4 };

        UIShapes::GenerateBorder( borderDesc, &borderVertices, &borderIndices, 0 );
        if ( borderVertices.NumElements( ) > 0 && borderIndices.NumElements( ) > 0 )
        {
            AddVerticesWithDepth( borderVertices, borderIndices );
        }
    }
    else
    {
        const float colorWheelSize = desc.Size - desc.ValueBarWidth - 10.0f;

        Clay_BoundingBox wheelBounds;
        wheelBounds.x      = bounds.x;
        wheelBounds.y      = bounds.y;
        wheelBounds.width  = colorWheelSize;
        wheelBounds.height = colorWheelSize;

        InteropArray<UIVertex> wheelVertices;
        InteropArray<uint32_t> wheelIndices;

        // TODO: HSV color wheel rendering
        UIShapes::GenerateRoundedRectangleDesc wheelDesc{ };
        wheelDesc.Bounds       = wheelBounds;
        wheelDesc.Color        = Clay_Color{ std::clamp( state->Rgb.X * 255.0f, 0.0f, 255.0f ), std::clamp( state->Rgb.Y * 255.0f, 0.0f, 255.0f ),
                                      std::clamp( state->Rgb.Z * 255.0f, 0.0f, 255.0f ), 255.0f };
        wheelDesc.TextureIndex = 0;
        wheelDesc.CornerRadius = Clay_CornerRadius{ 4, 4, 4, 4 };

        UIShapes::GenerateRoundedRectangle( wheelDesc, &wheelVertices, &wheelIndices, 0 );
        if ( wheelVertices.NumElements( ) > 0 && wheelIndices.NumElements( ) > 0 )
        {
            AddVerticesWithDepth( wheelVertices, wheelIndices );
        }

        Clay_BoundingBox valueBounds;
        valueBounds.x      = bounds.x + colorWheelSize + 5.0f;
        valueBounds.y      = bounds.y;
        valueBounds.width  = desc.ValueBarWidth;
        valueBounds.height = colorWheelSize;

        InteropArray<UIVertex> valueVertices;
        InteropArray<uint32_t> valueIndices;

        UIShapes::GenerateRectangleDesc valueDesc{ };
        valueDesc.Bounds       = valueBounds;
        valueDesc.Color        = Clay_Color{ state->Hsv.Z * 255.0f, state->Hsv.Z * 255.0f, state->Hsv.Z * 255.0f, 255.0f };
        valueDesc.TextureIndex = 0;

        UIShapes::GenerateRectangle( valueDesc, &valueVertices, &valueIndices, 0 );
        if ( valueVertices.NumElements( ) > 0 && valueIndices.NumElements( ) > 0 )
        {
            AddVerticesWithDepth( valueVertices, valueIndices );
        }
    }
}

void ClayRenderer::SetScissor( const Clay_RenderCommand *command )
{
    // Flush current batch before changing scissor
    FlushCurrentBatch( );

    const auto &bounds = command->boundingBox;

    ScissorState state;
    state.Enabled = true;
    state.X       = bounds.x;
    state.Y       = bounds.y;
    state.Width   = bounds.width;
    state.Height  = bounds.height;

    m_scissorStack.push_back( state );
}

void ClayRenderer::ClearScissor( )
{
    FlushCurrentBatch( );
    if ( !m_scissorStack.empty( ) )
    {
        m_scissorStack.pop_back( );
    }
}

uint32_t ClayRenderer::RegisterTexture( ITextureResource *texture )
{
    if ( texture == nullptr )
    {
        return 0;
    }

    for ( uint32_t i = 1; i < m_textures.size( ); ++i )
    {
        if ( m_textures[ i ] == nullptr )
        {
            m_textures[ i ] = texture;
            m_texturesDirty = true;
            return i;
        }
    }

    LOG( ERROR ) << "ClayRenderer: Exceeded maximum texture count";
    return 0;
}

void ClayRenderer::UpdateTextureBindings( const uint32_t frameIndex ) const
{
    if ( frameIndex >= m_frameData.size( ) )
    {
        return;
    }

    const FrameData                 &frame = m_frameData[ frameIndex ];
    InteropArray<ITextureResource *> textureArray;
    for ( const auto &tex : m_textures )
    {
        textureArray.AddElement( tex ? tex : m_nullTexture.get( ) );
    }
    frame.TextureBindGroup->BeginUpdate( )->SrvArray( 0, textureArray )->Sampler( 0, m_linearSampler.get( ) )->EndUpdate( );
}

ClayRenderer::FontData *ClayRenderer::GetFontData( const uint16_t fontId )
{
    const auto it = m_fonts.find( fontId );
    if ( it != m_fonts.end( ) )
    {
        return &it->second;
    }
    return nullptr;
}

void ClayRenderer::InitializeFontAtlas( FontData *fontData )
{
    if ( fontData == nullptr || fontData->FontPtr == nullptr )
    {
        return;
    }

    const auto *fontAsset = fontData->FontPtr->Asset( );
    if ( fontAsset == nullptr )
    {
        LOG( ERROR ) << "Font asset is null";
        return;
    }

    TextureDesc textureDesc{ };
    textureDesc.Width        = fontAsset->AtlasWidth;
    textureDesc.Height       = fontAsset->AtlasHeight;
    textureDesc.Format       = Format::R8G8B8A8Unorm;
    textureDesc.Descriptor   = BitSet( ResourceDescriptor::Texture );
    textureDesc.Usages       = BitSet( ResourceUsage::ShaderResource );
    textureDesc.InitialUsage = ResourceUsage::ShaderResource;
    textureDesc.HeapType     = HeapType::GPU;
    textureDesc.DebugName    = "Font Atlas Texture";
    fontData->Atlas          = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) );

    if ( fontAsset->AtlasData.NumElements( ) > 0 )
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

        const auto alignedPitch = Utilities::Align( fontAsset->AtlasWidth * FontAsset::NumChannels, m_logicalDevice->DeviceInfo( ).Constants.BufferTextureRowAlignment );
        const auto alignedSlice = Utilities::Align( fontAsset->AtlasHeight, m_logicalDevice->DeviceInfo( ).Constants.BufferTextureAlignment );

        BufferDesc stagingDesc;
        stagingDesc.NumBytes          = alignedPitch * alignedSlice;
        stagingDesc.Descriptor        = BitSet( ResourceDescriptor::Buffer );
        stagingDesc.InitialUsage      = ResourceUsage::CopySrc;
        stagingDesc.DebugName         = "Font MSDF Atlas Staging Buffer";
        stagingDesc.HeapType          = HeapType::CPU;
        auto m_fontAtlasStagingBuffer = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( stagingDesc ) );

        m_resourceTracking.TrackTexture( fontData->Atlas.get( ), ResourceUsage::ShaderResource );
        m_resourceTracking.TrackBuffer( m_fontAtlasStagingBuffer.get( ), ResourceUsage::CopySrc );

        LoadAtlasIntoGpuTextureDesc loadDesc{ };
        loadDesc.Device        = m_logicalDevice;
        loadDesc.StagingBuffer = m_fontAtlasStagingBuffer.get( );
        loadDesc.CommandList   = commandList;
        loadDesc.Texture       = fontData->Atlas.get( );
        FontAssetReader::LoadAtlasIntoGpuTexture( *fontAsset, loadDesc );

        BatchTransitionDesc batchTransitionDesc{ commandList };
        batchTransitionDesc.TransitionTexture( fontData->Atlas.get( ), ResourceUsage::CopyDst );
        m_resourceTracking.BatchTransition( batchTransitionDesc );

        CopyBufferToTextureDesc copyDesc{ };
        copyDesc.SrcBuffer  = m_fontAtlasStagingBuffer.get( );
        copyDesc.DstTexture = fontData->Atlas.get( );
        copyDesc.RowPitch   = fontAsset->AtlasWidth * 4; // 4 bytes per pixel (RGBA)
        copyDesc.Format     = fontData->Atlas->GetFormat( );

        commandList->CopyBufferToTexture( copyDesc );

        batchTransitionDesc = BatchTransitionDesc{ commandList };
        batchTransitionDesc.TransitionTexture( fontData->Atlas.get( ), ResourceUsage::ShaderResource );
        m_resourceTracking.BatchTransition( batchTransitionDesc );

        commandList->End( );
        ExecuteCommandListsDesc executeDesc{ };
        executeDesc.CommandLists = { commandList };
        commandQueue->ExecuteCommandLists( executeDesc );
        commandQueue->WaitIdle( );
    }

    fontData->TextureIndex = RegisterTexture( fontData->Atlas.get( ) );
}

void ClayRenderer::ClearCaches( )
{
    m_textLayoutCache.Clear( );
    m_shapeCache.Clear( );
    m_textVertexCache.Clear( );
    for ( auto &val : m_fonts | std::views::values )
    {
        val.TextLayouts.Clear( );
        val.CurrentLayoutIndex = 0;
    }

    m_imageTextureIndices.clear( );
    bool anyTextureCleared = false;
    for ( uint32_t i = 1; i < m_textures.size( ); ++i )
    {
        bool isFontTexture = false;
        for ( const auto &val : m_fonts | std::views::values )
        {
            if ( val.TextureIndex == i )
            {
                isFontTexture = true;
                break;
            }
        }

        if ( !isFontTexture && m_textures[ i ] != nullptr )
        {
            m_textures[ i ]   = nullptr;
            anyTextureCleared = true;
        }
    }

    if ( anyTextureCleared )
    {
        m_texturesDirty = true;
    }
}

ClayDimensions ClayRenderer::MeasureText( const InteropString &text, const Clay_TextElementConfig &desc ) const
{
    ClayDimensions result{ };
    result.Width  = 0;
    result.Height = 0;

    const auto it = m_fonts.find( desc.fontId );
    if ( it == m_fonts.end( ) || it->second.FontPtr == nullptr )
    {
        return result;
    }

    const FontData &fontData = it->second;
    Font           *font     = fontData.FontPtr;

    const float baseSize   = static_cast<float>( font->Asset( )->InitialFontSize );
    const float targetSize = desc.fontSize > 0 ? desc.fontSize * m_dpiScale : baseSize;

    // Use the cached text layout system
    const TextLayout *layout = GetOrCreateShapedTextDirect( text.Get( ), text.NumChars( ), desc.fontId, static_cast<uint32_t>( targetSize ), font );

    const auto size = layout->GetTextSize( );
    result.Width    = size.X / m_dpiScale;
    result.Height   = size.Y / m_dpiScale;

    return result;
}

void ClayRenderer::AddVerticesWithDepth( const InteropArray<UIVertex> &vertices, const InteropArray<uint32_t> &indices )
{
    const uint32_t baseVertexIndex = m_batchedVertices.NumElements( );
    for ( uint32_t i = 0; i < vertices.NumElements( ); ++i )
    {
        UIVertex vertex   = vertices.GetElement( i );
        vertex.Position.z = m_currentDepth;
        m_batchedVertices.AddElement( vertex );
    }

    for ( uint32_t i = 0; i < indices.NumElements( ); ++i )
    {
        m_batchedIndices.AddElement( indices.GetElement( i ) + baseVertexIndex );
    }

    m_currentDepth += DEPTH_INCREMENT;
}

void ClayRenderer::FlushCurrentBatch( )
{
    // ReSharper disable once CppDFAConstantConditions
    if ( m_batchedVertices.NumElements( ) == 0 || m_batchedIndices.NumElements( ) == 0 )
    {
        return;
    }

    // ReSharper disable once CppDFAUnreachableCode
    constexpr uint32_t vertexAlignment = 256 / sizeof( UIVertex );
    constexpr uint32_t indexAlignment  = 256 / sizeof( uint32_t );

    const uint32_t alignedVertexOffset = ( m_totalVertexCount + vertexAlignment - 1 ) / vertexAlignment * vertexAlignment;
    const uint32_t alignedIndexOffset  = ( m_totalIndexCount + indexAlignment - 1 ) / indexAlignment * indexAlignment;

    const size_t vertexDataSize = m_batchedVertices.NumElements( ) * sizeof( UIVertex );
    const size_t indexDataSize  = m_batchedIndices.NumElements( ) * sizeof( uint32_t );

    if ( alignedVertexOffset + m_batchedVertices.NumElements( ) > m_desc.MaxVertices || alignedIndexOffset + m_batchedIndices.NumElements( ) > m_desc.MaxIndices )
    {
        LOG( ERROR ) << "ClayRenderer: Geometry exceeds buffer limits";
        return;
    }

    memcpy( m_vertexBufferData + alignedVertexOffset * sizeof( UIVertex ), m_batchedVertices.Data( ), vertexDataSize );

    const auto indexDst = reinterpret_cast<uint32_t *>( m_indexBufferData + alignedIndexOffset * sizeof( uint32_t ) );
    for ( uint32_t i = 0; i < m_batchedIndices.NumElements( ); ++i )
    {
        indexDst[ i ] = m_batchedIndices.GetElement( i ); // Don't add vertex offset here, use baseVertex instead
    }

    DrawBatch batch;
    batch.VertexOffset = alignedVertexOffset;
    batch.IndexOffset  = alignedIndexOffset;
    batch.IndexCount   = m_batchedIndices.NumElements( );

    if ( !m_scissorStack.empty( ) )
    {
        batch.Scissor = m_scissorStack.back( );
    }
    else
    {
        batch.Scissor.Enabled = false;
        batch.Scissor.X       = 0;
        batch.Scissor.Y       = 0;
        batch.Scissor.Width   = m_viewportWidth;
        batch.Scissor.Height  = m_viewportHeight;
    }

    m_drawBatches.push_back( batch );

    m_totalVertexCount = alignedVertexOffset + m_batchedVertices.NumElements( );
    m_totalIndexCount  = alignedIndexOffset + m_batchedIndices.NumElements( );

    m_batchedVertices.Clear( );
    m_batchedIndices.Clear( );
}

void ClayRenderer::ExecuteDrawBatches( ICommandList *commandList ) const
{
    if ( m_drawBatches.empty( ) )
    {
        return;
    }

    commandList->BindVertexBuffer( m_vertexBuffer.get( ) );
    commandList->BindIndexBuffer( m_indexBuffer.get( ), IndexType::Uint32 );

    for ( const auto &batch : m_drawBatches )
    {
        if ( batch.Scissor.Enabled )
        {
            commandList->BindScissorRect( batch.Scissor.X, batch.Scissor.Y, batch.Scissor.Width, batch.Scissor.Height );
        }
        else
        {
            commandList->BindScissorRect( 0, 0, m_viewportWidth, m_viewportHeight );
        }
        commandList->DrawIndexed( batch.IndexCount, 1, batch.IndexOffset, batch.VertexOffset, 0 );
    }
}

void ClayRenderer::FlushBatchedGeometry( ICommandList *commandList )
{
    FlushCurrentBatch( );
    ExecuteDrawBatches( commandList );
}

TextLayout *ClayRenderer::GetOrCreateShapedText( const Clay_RenderCommand *command, Font *font ) const
{
    const auto &data       = command->renderData.text;
    const float targetSize = data.fontSize > 0 ? data.fontSize * m_dpiScale : static_cast<float>( font->Asset( )->InitialFontSize );
    return GetOrCreateShapedTextDirect( data.stringContents.chars, data.stringContents.length, data.fontId, static_cast<uint32_t>( targetSize ), font );
}

TextLayout *ClayRenderer::GetOrCreateShapedTextDirect( const char *text, const size_t length, const uint16_t fontId, const uint32_t fontSize, Font *font ) const
{
    const uint64_t textHash = TextLayoutCache::HashString( text, length );
    return m_textLayoutCache.GetOrCreate( textHash, fontId, fontSize, font, text, length, m_currentFrame );
}

void ClayRenderer::CleanupTextLayoutCache( ) const
{
    m_textLayoutCache.Cleanup( m_currentFrame );
}

void ClayRenderer::RenderResizableContainer( const Clay_RenderCommand *command, const ClayResizableContainerRenderData *resizableData, ICommandList *commandList )
{
    const auto &bounds = command->boundingBox;
    const auto *state  = resizableData->State;
    const auto &desc   = resizableData->Desc;

    // Render resize handles if resizing is enabled
    if ( desc.EnableResize )
    {
        const float handleSize = desc.ResizeHandleSize;

        // Corner handles (higher priority for interaction)
        const std::vector<std::pair<float, float>> cornerOffsets = {
            { 0, 0 },                                                 // NorthWest
            { bounds.width - handleSize, 0 },                         // NorthEast
            { 0, bounds.height - handleSize },                        // SouthWest
            { bounds.width - handleSize, bounds.height - handleSize } // SouthEast
        };

        for ( const auto &[ offsetX, offsetY ] : cornerOffsets )
        {
            InteropArray<UIVertex> handleVertices;
            InteropArray<uint32_t> handleIndices;

            Clay_BoundingBox handleBounds;
            handleBounds.x      = bounds.x + offsetX;
            handleBounds.y      = bounds.y + offsetY;
            handleBounds.width  = handleSize;
            handleBounds.height = handleSize;

            UIShapes::GenerateRectangleDesc handleDesc{ };
            handleDesc.Bounds       = handleBounds;
            handleDesc.Color        = Clay_Color{ desc.HandleColor.R, desc.HandleColor.G, desc.HandleColor.B, desc.HandleColor.A };
            handleDesc.TextureIndex = 0;

            UIShapes::GenerateRectangle( handleDesc, &handleVertices, &handleIndices, 0 );
            if ( handleVertices.NumElements( ) > 0 && handleIndices.NumElements( ) > 0 )
            {
                AddVerticesWithDepth( handleVertices, handleIndices );
            }
        }

        // Edge handles
        const std::vector<std::tuple<float, float, float, float>> edgeHandles = {
            { handleSize, 0, bounds.width - 2 * handleSize, handleSize },                          // North
            { handleSize, bounds.height - handleSize, bounds.width - 2 * handleSize, handleSize }, // South
            { 0, handleSize, handleSize, bounds.height - 2 * handleSize },                         // West
            { bounds.width - handleSize, handleSize, handleSize, bounds.height - 2 * handleSize }  // East
        };

        for ( const auto &[ offsetX, offsetY, width, height ] : edgeHandles )
        {
            InteropArray<UIVertex> handleVertices;
            InteropArray<uint32_t> handleIndices;

            Clay_BoundingBox handleBounds;
            handleBounds.x      = bounds.x + offsetX;
            handleBounds.y      = bounds.y + offsetY;
            handleBounds.width  = width;
            handleBounds.height = height;

            UIShapes::GenerateRectangleDesc handleDesc{ };
            handleDesc.Bounds       = handleBounds;
            handleDesc.Color        = Clay_Color{ desc.HandleColor.R, desc.HandleColor.G, desc.HandleColor.B, desc.HandleColor.A };
            handleDesc.TextureIndex = 0;

            UIShapes::GenerateRectangle( handleDesc, &handleVertices, &handleIndices, 0 );
            if ( handleVertices.NumElements( ) > 0 && handleIndices.NumElements( ) > 0 )
            {
                AddVerticesWithDepth( handleVertices, handleIndices );
            }
        }
    }
}

void ClayRenderer::RenderDockableContainer( const Clay_RenderCommand *command, const ClayDockableContainerRenderData *dockableData, ICommandList *commandList )
{
    const auto &bounds = command->boundingBox;
    const auto *state  = dockableData->State;
    const auto &desc   = dockableData->Desc;

    if ( state->ShowDockZones )
    {
        if ( static_cast<DockingSide>( state->HoveredDockZone ) != DockingSide::None )
        {
            InteropArray<UIVertex> highlightVertices;
            InteropArray<uint32_t> highlightIndices;

            UIShapes::GenerateRectangleDesc highlightDesc{ };
            highlightDesc.Bounds       = bounds;
            highlightDesc.Color        = Clay_Color{ desc.DockZoneColor.R, desc.DockZoneColor.G, desc.DockZoneColor.B, 50 };
            highlightDesc.TextureIndex = 0;

            UIShapes::GenerateRectangle( highlightDesc, &highlightVertices, &highlightIndices, 0 );
            if ( highlightVertices.NumElements( ) > 0 && highlightIndices.NumElements( ) > 0 )
            {
                AddVerticesWithDepth( highlightVertices, highlightIndices );
            }
        }
    }

    if ( state->IsDragging )
    {
        InteropArray<UIVertex> dragVertices;
        InteropArray<uint32_t> dragIndices;

        UIShapes::GenerateRectangleDesc dragDesc{ };
        dragDesc.Bounds       = bounds;
        dragDesc.Color        = Clay_Color{ 100, 100, 100, 100 };
        dragDesc.TextureIndex = 0;

        UIShapes::GenerateRectangle( dragDesc, &dragVertices, &dragIndices, 0 );
        if ( dragVertices.NumElements( ) > 0 && dragIndices.NumElements( ) > 0 )
        {
            AddVerticesWithDepth( dragVertices, dragIndices );
        }
    }
}
