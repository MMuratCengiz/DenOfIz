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

ClayRenderer::ClayRenderer( const ClayRendererDesc &desc ) : m_desc( desc ), m_logicalDevice( desc.LogicalDevice ), m_clayContext( desc.ClayContext )
{
    if ( m_logicalDevice == nullptr )
    {
        LOG( ERROR ) << "ClayRenderer: LogicalDevice cannot be null";
        return;
    }

    if ( m_clayContext == nullptr )
    {
        LOG( ERROR ) << "ClayRenderer: ClayContext cannot be null";
        return;
    }
    m_clayText = m_clayContext->GetClayText( );

    m_viewportWidth  = desc.Width;
    m_viewportHeight = desc.Height;

    m_textures.resize( desc.MaxTextures, nullptr );

    CommandQueueDesc commandQueueDesc{ };
    commandQueueDesc.QueueType = QueueType::Graphics;
    m_commandQueue             = std::unique_ptr<ICommandQueue>( m_logicalDevice->CreateCommandQueue( commandQueueDesc ) );

    CommandListPoolDesc poolDesc{ };
    poolDesc.CommandQueue    = m_commandQueue.get( );
    poolDesc.NumCommandLists = desc.NumFrames;
    m_commandListPool        = std::unique_ptr<ICommandListPool>( m_logicalDevice->CreateCommandListPool( poolDesc ) );

    // Create command list pool for pipeline widgets
    if ( desc.MaxPipelineWidgets > 0 )
    {
        CommandListPoolDesc pipelinePoolDesc{ };
        pipelinePoolDesc.CommandQueue    = m_commandQueue.get( );
        pipelinePoolDesc.NumCommandLists = desc.MaxPipelineWidgets * desc.NumFrames;
        m_pipelineWidgetCommandListPool  = std::unique_ptr<ICommandListPool>( m_logicalDevice->CreateCommandListPool( pipelinePoolDesc ) );

        m_pipelineWidgetData.resize( desc.MaxPipelineWidgets * desc.NumFrames );
        auto pipelineCommandLists = m_pipelineWidgetCommandListPool->GetCommandLists( );
        for ( uint32_t i = 0; i < pipelineCommandLists.NumElements( ) && i < m_pipelineWidgetData.size( ); ++i )
        {
            m_pipelineWidgetData[ i ].CommandList = pipelineCommandLists.GetElement( i );
            m_pipelineWidgetData[ i ].Semaphore   = std::unique_ptr<ISemaphore>( m_logicalDevice->CreateSemaphore( ) );
        }
    }

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

void ClayRenderer::Resize( const float width, const float height )
{
    m_viewportWidth  = width;
    m_viewportHeight = height;
    CreateRenderTargets( );
    UpdateProjectionMatrix( );
    for ( const auto &widget : m_widgets | std::views::values )
    {
        if ( widget && widget->HasPipeline( ) )
        {
            widget->ResizeRenderResources( static_cast<uint32_t>( width ), static_cast<uint32_t>( height ) );
        }
    }
}

void ClayRenderer::SetDpiScale( const float dpiScale )
{
    m_dpiScale = dpiScale;
    if ( m_clayText )
    {
        m_clayText->SetDpiScale( dpiScale );
    }
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

    if ( m_clayText )
    {
        m_clayText->UpdateFrame( m_currentFrame );
    }

    if ( m_currentFrame % 6000 /*frame*/ == 0 )
    {
        m_shapeCache.Cleanup( m_currentFrame );
        if ( m_clayText )
        {
            m_clayText->CleanupCaches( 3000, 6000 );
        }
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
    if ( m_clayText )
    {
        for ( uint16_t fontId = 0; fontId < 100; ++fontId )
        {
            Font *font = m_clayText->GetFont( fontId );
            if ( font && font->Asset( ) )
            {
                atlasWidth  = static_cast<float>( font->Asset( )->AtlasWidth );
                atlasHeight = static_cast<float>( font->Asset( )->AtlasHeight );
                break;
            }
        }
    }

    // Font textures use indices 1 to MaxNumFonts
    // Regular textures use indices MaxNumFonts+1 and above
    tempUniforms.FontParams = XMFLOAT4( atlasWidth, atlasHeight, 12.0f, static_cast<float>( m_desc.MaxNumFonts + 1 ) );

    uint8_t *uniformLocation = reinterpret_cast<uint8_t *>( m_uniformBufferData ) + frameIndex * m_alignedUniformSize;
    memcpy( uniformLocation, &tempUniforms, sizeof( UIUniforms ) );

    const FrameData &frame = m_frameData[ frameIndex ];
    if ( m_texturesDirty )
    {
        SyncFontTexturesFromClayText( );
        for ( uint32_t i = 0; i < m_desc.NumFrames; ++i )
        {
            UpdateTextureBindings( i );
        }
        m_texturesDirty = false;
    }

    m_drawBatches.clear( );
    m_totalVertexCount = 0;
    m_totalIndexCount  = 0;
    m_currentDepth     = 0.9f;
    m_pipelineWidgetsToRender.clear( );

    uint32_t                   pipelineWidgetIndex = 0;
    InteropArray<ISemaphore *> pipelineWidgetSemaphores;
    for ( int32_t i = 0; i < commands.length; ++i )
    {
        const Clay_RenderCommand *cmd = Clay_RenderCommandArray_Get( &commands, i );
        if ( cmd->commandType != CLAY_RENDER_COMMAND_TYPE_CUSTOM )
        {
            continue;
        }
        const auto &data = cmd->renderData.custom;
        if ( data.customData == nullptr )
        {
            continue;
        }
        auto *widget = static_cast<Widget *>( data.customData );
        if ( widget == nullptr )
        {
            continue;
        }
        bool isRegisteredWidget = false;
        for ( const auto &widgetPtr : m_widgets | std::views::values )
        {
            if ( widgetPtr == widget )
            {
                isRegisteredWidget = true;
                break;
            }
        }

        if ( isRegisteredWidget && widget->HasPipeline( ) )
        {
            m_pipelineWidgetsToRender.push_back( widget );
            if ( pipelineWidgetIndex >= m_desc.MaxPipelineWidgets )
            {
                LOG( WARNING ) << "Exceeded maximum pipeline widgets";
                break;
            }

            uint32_t dataIndex = frameIndex * m_desc.MaxPipelineWidgets + pipelineWidgetIndex;
            if ( dataIndex >= m_pipelineWidgetData.size( ) )
            {
                LOG( ERROR ) << "Invalid pipeline widget data index";
                continue;
            }

            const PipelineWidgetData &widgetData = m_pipelineWidgetData[ dataIndex ];
            if ( !widgetData.CommandList || !widgetData.Semaphore )
            {
                continue;
            }

            WidgetExecutePipelineDesc context;
            context.CommandList = widgetData.CommandList;
            context.FrameIndex  = frameIndex;
            context.BoundingBox = { cmd->boundingBox.x, cmd->boundingBox.y, cmd->boundingBox.width, cmd->boundingBox.height };

            if ( !m_scissorStack.empty( ) )
            {
                const ScissorState &scissor = m_scissorStack.back( );
                context.ScissorRect.X       = scissor.X;
                context.ScissorRect.Y       = scissor.Y;
                context.ScissorRect.Width   = scissor.Width;
                context.ScissorRect.Height  = scissor.Height;
            }
            else
            {
                context.ScissorRect.X      = 0.0f;
                context.ScissorRect.Y      = 0.0f;
                context.ScissorRect.Width  = m_viewportWidth;
                context.ScissorRect.Height = m_viewportHeight;
            }

            widget->ExecuteCustomPipeline( context );
            const uint32_t textureIndex = widget->GetTextureIndex( );
            if ( textureIndex > 0 && textureIndex < m_textures.size( ) && widget->GetRenderTarget( frameIndex ) )
            {
                if ( textureIndex > m_desc.MaxNumFonts )
                {
                    m_textures[ textureIndex ] = widget->GetRenderTarget( frameIndex );
                    m_texturesDirty            = true;
                }
            }

            ExecuteCommandListsDesc widgetExecuteDesc{ };
            widgetExecuteDesc.CommandLists.AddElement( widgetData.CommandList );
            widgetExecuteDesc.SignalSemaphores.AddElement( widgetData.Semaphore.get( ) );
            m_commandQueue->ExecuteCommandLists( widgetExecuteDesc );

            pipelineWidgetSemaphores.AddElement( widgetData.Semaphore.get( ) );
            pipelineWidgetIndex++;
        }
    }

    if ( m_texturesDirty )
    {
        UpdateTextureBindings( frameIndex );
        m_texturesDirty = false;
    }

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
    for ( uint32_t i = 0; i < pipelineWidgetSemaphores.NumElements( ); ++i )
    {
        executeDesc.WaitSemaphores.AddElement( pipelineWidgetSemaphores.GetElement( i ) );
    }

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

    if ( !m_clayText )
    {
        LOG( WARNING ) << "ClayText not set in ClayRenderer";
        return;
    }

    const Font *font = m_clayText->GetFont( data.fontId );
    if ( font == nullptr )
    {
        LOG( WARNING ) << "Font not found for ID: " << data.fontId;
        return;
    }
    const float baseSize       = static_cast<float>( font->Asset( )->InitialFontSize );
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

        const float fontAscent        = static_cast<float>( font->Asset( )->Metrics.Ascent ) * effectiveScale;
        const float fontDescent       = static_cast<float>( font->Asset( )->Metrics.Descent ) * effectiveScale;
        const float defaultLineHeight = ( fontAscent + fontDescent ) * 1.2f;
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

                RenderSingleLineText( &lineCommand, data.fontId, effectiveScale, fontAscent );
            }
            currentY += lineHeight;
        }
    }
    else
    {
        const float fontAscent = static_cast<float>( font->Asset( )->Metrics.Ascent ) * effectiveScale;
        RenderSingleLineText( command, data.fontId, effectiveScale, fontAscent );
    }
}

void ClayRenderer::RenderSingleLineText( const Clay_RenderCommand *command, const uint16_t fontId, const float effectiveScale, const float fontAscent )
{
    const auto &data   = command->renderData.text;
    const auto &bounds = command->boundingBox;
    Font       *font   = m_clayText->GetFont( fontId );
    if ( !font )
    {
        return;
    }

    const TextLayout *textLayout = m_clayText->GetOrCreateShapedText( command, font );

    const float adjustedY = bounds.y * m_dpiScale + fontAscent;

    const TextVertexCacheKey vertexCacheKey = UITextVertexCache::CreateTextVertexKey( command, effectiveScale, adjustedY, m_dpiScale );
    CachedTextVertices      *cachedVertices = m_clayText->GetOrCreateTextVertices( vertexCacheKey );

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
                vertex.Position     = Float_3{ glyph.Position.X, glyph.Position.Y, 0.0f }; // Z will be set in AddVerticesWithDepth
                vertex.TexCoord     = glyph.UV;
                vertex.Color        = glyph.Color;
                vertex.TextureIndex = m_clayText->GetFontTextureIndex( fontId );
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

    if ( auto *widget = static_cast<Widget *>( data.customData ) )
    {
        bool isRegisteredWidget = false;
        for ( const auto &widgetPtr : m_widgets | std::views::values )
        {
            if ( widgetPtr == widget )
            {
                isRegisteredWidget = true;
                break;
            }
        }

        if ( isRegisteredWidget )
        {
            if ( widget->HasPipeline( ) )
            {
                uint32_t textureIndex = widget->GetTextureIndex( );

                // Check if texture index conflicts with fonts and needs re-registration
                if ( textureIndex > 0 && textureIndex <= m_desc.MaxNumFonts )
                {
                    // Re-register to get a new texture index
                    widget->SetTextureIndex( 0 );
                    textureIndex = RegisterTexture( nullptr );
                    widget->SetTextureIndex( textureIndex );
                }

                if ( widget->GetRenderTarget( m_currentFrameIndex ) && textureIndex > 0 )
                {

                    InteropArray<UIVertex> vertices;
                    InteropArray<uint32_t> indices;

                    UIShapes::GenerateRectangleDesc desc{ };
                    desc.Bounds       = command->boundingBox;
                    desc.Color        = Clay_Color{ 255, 255, 255, 255 }; // White to show texture
                    desc.TextureIndex = textureIndex;

                    UIShapes::GenerateRectangle( desc, &vertices, &indices, 0 );
                    if ( vertices.NumElements( ) > 0 && indices.NumElements( ) > 0 )
                    {
                        AddVerticesWithDepth( vertices, indices );
                    }
                }
            }
            else
            {
                ClayRenderBatch       renderBatch( this );
                const ClayBoundingBox boundingBox = { command->boundingBox.x, command->boundingBox.y, command->boundingBox.width, command->boundingBox.height };
                widget->Render( boundingBox, &renderBatch );
            }
        }
        else
        {
            LOG( WARNING ) << "Widget not registered, use ClayRenderer::RegisterWidget";
        }
    }
}

void ClayRenderer::SetScissor( const Clay_RenderCommand *command )
{
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
    // Start from MaxNumFonts+1 to skip font texture slots
    for ( uint32_t i = m_desc.MaxNumFonts + 1; i < m_textures.size( ); ++i )
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
    for ( size_t i = 0; i < m_textures.size( ); ++i )
    {
        ITextureResource *tex = m_textures[ i ];
        textureArray.AddElement( tex ? tex : m_nullTexture.get( ) );
    }
    frame.TextureBindGroup->BeginUpdate( )->SrvArray( 0, textureArray )->Sampler( 0, m_linearSampler.get( ) )->EndUpdate( );
}

void ClayRenderer::ClearCaches( )
{
    m_shapeCache.Clear( );
    if ( m_clayText )
    {
        m_clayText->ClearCaches( );
    }

    m_imageTextureIndices.clear( );
    bool anyTextureCleared = false;
    // Clear non-font textures
    for ( uint32_t i = m_desc.MaxNumFonts + 1; i < m_textures.size( ); ++i )
    {
        if ( m_textures[ i ] != nullptr )
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
    if ( !m_clayText )
    {
        return ClayDimensions{ 0, 0 };
    }

    return m_clayText->MeasureText( text, desc );
}

void ClayRenderer::AddVerticesWithDepth( const InteropArray<UIVertex> &vertices, const InteropArray<uint32_t> &indices )
{
    const uint32_t baseVertexIndex = m_batchedVertices.NumElements( );
    for ( uint32_t i = 0; i < vertices.NumElements( ); ++i )
    {
        UIVertex vertex   = vertices.GetElement( i );
        vertex.Position.Z = m_currentDepth;
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

void ClayRenderer::RegisterWidget( const uint32_t id, Widget *widget )
{
    m_widgets[ id ] = widget;
    if ( widget && widget->HasPipeline( ) )
    {
        const uint32_t currentIndex = widget->GetTextureIndex( );
        if ( currentIndex > 0 && currentIndex <= m_desc.MaxNumFonts )
        {
            widget->SetTextureIndex( 0 );
        }
        if ( widget->GetRenderTarget( 0 ) == nullptr )
        {
            widget->InitializeRenderResources( m_logicalDevice, static_cast<uint32_t>( m_viewportWidth ), static_cast<uint32_t>( m_viewportHeight ) );
        }
        if ( widget->GetTextureIndex( ) == 0 )
        {
            const uint32_t textureIndex = RegisterTexture( nullptr ); // Reserve a slot
            widget->SetTextureIndex( textureIndex );
        }
    }
}

void ClayRenderer::UnregisterWidget( const uint32_t id )
{
    m_widgets.erase( id );
}

uint32_t ClayRenderer::GetCurrentVertexCount( ) const
{
    return m_batchedVertices.NumElements( );
}

void ClayRenderBatch::AddVertices( const InteropArray<UIVertex> &vertices, const InteropArray<uint32_t> &indices )
{
    m_renderer->AddVerticesWithDepth( vertices, indices );
}

uint32_t ClayRenderBatch::GetCurrentVertexOffset( ) const
{
    return m_renderer->GetCurrentVertexCount( );
}

void ClayRenderer::SyncFontTexturesFromClayText( )
{
    for ( uint16_t fontId = 0; fontId < 100; ++fontId )
    {
        if ( ITextureResource *fontTexture = m_clayText->GetFontTexture( fontId ) )
        {
            const uint32_t textureIndex = m_clayText->GetFontTextureIndex( fontId );
            if ( textureIndex > 0 && textureIndex < m_textures.size( ) )
            {
                m_textures[ textureIndex ] = fontTexture;
            }
        }
    }
}
