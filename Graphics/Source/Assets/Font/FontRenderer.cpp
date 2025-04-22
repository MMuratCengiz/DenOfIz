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
#include <DenOfIzGraphics/Assets/FileSystem/PathResolver.h>
#include <DenOfIzGraphics/Assets/Font/FontRenderer.h>
#include <DenOfIzGraphics/Utilities/Common_Asserts.h>
#include <directxmath.h>

using namespace DenOfIz;
using namespace DirectX;

FontRenderer::FontRenderer( GraphicsApi *graphicsApi, ILogicalDevice *logicalDevice ) : m_graphicsApi( graphicsApi ), m_logicalDevice( logicalDevice )
{
    XMStoreFloat4x4( &m_projectionMatrix, XMMatrixIdentity( ) );
}

FontRenderer::~FontRenderer( ) = default;

void FontRenderer::Initialize( )
{
    ShaderCompiler               shaderCompiler;
    std::vector<ShaderStageDesc> shaderStages;
    ShaderProgramDesc            programDesc;
    ShaderStageDesc             &vsDesc = programDesc.ShaderStages.EmplaceElement( );
    vsDesc.Stage                        = ShaderStage::Vertex;
    vsDesc.EntryPoint                   = "main";
    vsDesc.Path                         = "Assets/Shaders/FontShader.vs.hlsl";
    shaderStages.push_back( vsDesc );

    ShaderStageDesc &psDesc = programDesc.ShaderStages.EmplaceElement( );
    psDesc.Stage            = ShaderStage::Pixel;
    psDesc.EntryPoint       = "main";
    psDesc.Path             = "Assets/Shaders/FontShader.ps.hlsl";
    shaderStages.push_back( psDesc );

    m_fontShaderProgram           = std::make_unique<ShaderProgram>( programDesc );
    ShaderReflectDesc reflectDesc = m_fontShaderProgram->Reflect( );

    SamplerDesc samplerDesc;
    samplerDesc.AddressModeU  = SamplerAddressMode::ClampToEdge;
    samplerDesc.AddressModeV  = SamplerAddressMode::ClampToEdge;
    samplerDesc.MipLodBias    = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.CompareOp     = CompareOp::Never;
    samplerDesc.MinLod        = 0.0f;
    samplerDesc.MaxLod        = 0.0f;
    m_fontSampler             = std::unique_ptr<ISampler>( m_logicalDevice->CreateSampler( samplerDesc ) );

    m_fontAtlasTextureDesc              = { };
    m_fontAtlasTextureDesc.Width        = 512;
    m_fontAtlasTextureDesc.Height       = 512;
    m_fontAtlasTextureDesc.Format       = Format::R8Unorm;
    m_fontAtlasTextureDesc.Descriptor   = BitSet( ResourceDescriptor::Texture );
    m_fontAtlasTextureDesc.InitialUsage = ResourceUsage::ShaderResource;
    m_fontAtlasTextureDesc.DebugName    = "Font Atlas Texture";
    m_fontAtlasTexture                  = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( m_fontAtlasTextureDesc ) );
    m_resourceTracking.TrackTexture( m_fontAtlasTexture.get( ), ResourceUsage::ShaderResource );

    m_vertexBufferDesc            = { };
    m_vertexBufferDesc.NumBytes   = m_maxVertices * 8 * sizeof( float );
    m_vertexBufferDesc.Descriptor = BitSet( ResourceDescriptor::VertexBuffer );
    m_vertexBufferDesc.Usages     = ResourceUsage::VertexAndConstantBuffer;
    m_vertexBufferDesc.HeapType   = HeapType::CPU_GPU;
    m_vertexBufferDesc.DebugName  = "Font Vertex Buffer";
    m_vertexBuffer                = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( m_vertexBufferDesc ) );
    m_resourceTracking.TrackBuffer( m_vertexBuffer.get( ), ResourceUsage::VertexAndConstantBuffer );

    m_indexBufferDesc            = { };
    m_indexBufferDesc.NumBytes   = m_maxIndices * sizeof( uint32_t );
    m_indexBufferDesc.Descriptor = BitSet( ResourceDescriptor::IndexBuffer );
    m_indexBufferDesc.Usages     = ResourceUsage::IndexBuffer;
    m_indexBufferDesc.HeapType   = HeapType::CPU_GPU;
    m_indexBufferDesc.DebugName  = "Font Index Buffer";
    m_indexBuffer                = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( m_indexBufferDesc ) );
    m_resourceTracking.TrackBuffer( m_indexBuffer.get( ), ResourceUsage::IndexBuffer );

    BufferDesc uniformBufferDesc;
    uniformBufferDesc.NumBytes   = sizeof( FontShaderUniforms );
    uniformBufferDesc.Descriptor = BitSet( ResourceDescriptor::UniformBuffer );
    uniformBufferDesc.Usages     = ResourceUsage::VertexAndConstantBuffer;
    uniformBufferDesc.HeapType   = HeapType::CPU_GPU;
    uniformBufferDesc.DebugName  = "Font Uniform Buffer";
    m_uniformBuffer              = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( uniformBufferDesc ) );
    m_resourceTracking.TrackBuffer( m_uniformBuffer.get( ), ResourceUsage::VertexAndConstantBuffer );

    m_rootSignature = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflectDesc.RootSignature ) );
    m_inputLayout   = std::unique_ptr<IInputLayout>( m_logicalDevice->CreateInputLayout( reflectDesc.InputLayout ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.ShaderProgram     = m_fontShaderProgram.get( );
    pipelineDesc.RootSignature     = m_rootSignature.get( );
    pipelineDesc.InputLayout       = m_inputLayout.get( );
    pipelineDesc.Graphics.CullMode = CullMode::None;
    pipelineDesc.Graphics.FillMode = FillMode::Solid;

    RenderTargetDesc &renderTarget   = pipelineDesc.Graphics.RenderTargets.EmplaceElement( );
    renderTarget.Blend.Enable        = true;
    renderTarget.Blend.SrcBlend      = Blend::SrcAlpha;
    renderTarget.Blend.DstBlend      = Blend::InvSrcAlpha;
    renderTarget.Blend.BlendOp       = BlendOp::Add;
    renderTarget.Blend.SrcBlendAlpha = Blend::One;
    renderTarget.Blend.DstBlendAlpha = Blend::Zero;
    renderTarget.Blend.BlendOpAlpha  = BlendOp::Add;
    renderTarget.Format = renderTarget.Format = Format::R8G8B8A8Unorm;
    pipelineDesc.Graphics.PrimitiveTopology   = PrimitiveTopology::Triangle;

    m_fontPipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_rootSignature.get( );
    bindGroupDesc.RegisterSpace = 0;

    m_resourceBindGroup = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );
    m_resourceBindGroup->BeginUpdate( )->Cbv( 0, m_uniformBuffer.get( ) )->Srv( 0, m_fontAtlasTexture.get( ) )->Sampler( 0, m_fontSampler.get( ) )->EndUpdate( );
}

FontCache *FontRenderer::LoadFont( const InteropString &fontPath, const uint32_t pixelSize )
{
    const std::string path     = fontPath.Get( );
    const std::string cacheKey = path + "_" + std::to_string( pixelSize );
    if ( const auto it = m_loadedFonts.find( cacheKey ); it != m_loadedFonts.end( ) )
    {
        return it->second;
    }

    const auto fontCache = m_fontManager.LoadFont( fontPath, pixelSize );
    if ( fontCache )
    {
        m_loadedFonts[ cacheKey ] = fontCache;
        m_atlasNeedsUpdate        = true;
    }

    return fontCache;
}

void FontRenderer::SetFont( const InteropString &fontPath, const uint32_t pixelSize )
{
    m_currentFont = LoadFont( fontPath, pixelSize );
    if ( m_currentFont )
    {
        m_atlasNeedsUpdate = true;
    }
}

void FontRenderer::SetProjectionMatrix( const XMFLOAT4X4 &projectionMatrix )
{
    m_projectionMatrix = projectionMatrix;
}

void FontRenderer::BeginBatch( )
{
    m_vertexData.Clear( );
    m_indexData.Clear( );
    m_currentVertexCount = 0;
    m_currentIndexCount  = 0;
}

void FontRenderer::AddText( const TextRenderDesc &params )
{
    if ( !m_currentFont || params.Text.NumChars( ) == 0 )
    {
        return;
    }

    TextRenderDesc           modifiedParams = params;
    GenerateTextVerticesDesc generateTextDesc{ };
    generateTextDesc.Layout = m_fontManager.ShapeText( m_currentFont, params.Text, params.Direction );
    if ( params.HorizontalCenter || params.VerticalCenter )
    {
        if ( params.HorizontalCenter )
        {
            modifiedParams.X -= generateTextDesc.Layout.TotalWidth * params.Scale / 2.0f;
        }

        if ( params.VerticalCenter )
        {
            modifiedParams.Y -= generateTextDesc.Layout.TotalHeight * params.Scale / 2.0f;
        }
    }

    generateTextDesc.FontCache = m_currentFont;
    generateTextDesc.Text      = params.Text;
    generateTextDesc.X         = modifiedParams.X;
    generateTextDesc.Y         = modifiedParams.Y;
    generateTextDesc.Color     = modifiedParams.Color;
    generateTextDesc.Scale     = modifiedParams.Scale;

    m_fontManager.GenerateTextVertices( generateTextDesc, m_vertexData, m_indexData );

    m_currentVertexCount = m_vertexData.NumElements( ) / 8;
    m_currentIndexCount  = m_indexData.NumElements( );

    if ( m_currentVertexCount > m_maxVertices || m_currentIndexCount > m_maxIndices )
    {
        m_maxVertices = std::max( m_maxVertices * 2, m_currentVertexCount );
        m_maxIndices  = std::max( m_maxIndices * 2, m_currentIndexCount );
        LOG( INFO ) << "Font render buffers resized: vertices=" << m_maxVertices << ", indices=" << m_maxIndices;
    }
}

void FontRenderer::EndBatch( ICommandList *commandList )
{
    if ( m_currentVertexCount == 0 || m_currentIndexCount == 0 || !m_currentFont )
    {
        return; // Nothing to render
    }

    if ( m_atlasNeedsUpdate || m_currentFont->AtlasNeedsUpdate( ) )
    {
        UpdateAtlasTexture( commandList );
        m_atlasNeedsUpdate = false;
        m_currentFont->MarkAtlasUpdated( );
    }

    UpdateBuffers( );

    FontShaderUniforms uniforms{ };
    uniforms.Projection = m_projectionMatrix;
    uniforms.TextColor  = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );

    void *mappedData = m_uniformBuffer->MapMemory( );
    memcpy( mappedData, &uniforms, sizeof( FontShaderUniforms ) );
    m_uniformBuffer->UnmapMemory( );

    commandList->BindPipeline( m_fontPipeline.get( ) );
    commandList->BindResourceGroup( m_resourceBindGroup.get( ) );
    commandList->BindVertexBuffer( m_vertexBuffer.get( ) );
    commandList->BindIndexBuffer( m_indexBuffer.get( ), IndexType::Uint32 );
    commandList->DrawIndexed( m_currentIndexCount, 1, 0, 0, 0 );
}

void FontRenderer::UpdateAtlasTexture( ICommandList *commandList )
{
    const auto &fontAsset   = m_currentFont->GetFontAsset( );
    const auto &atlasBitmap = m_currentFont->GetAtlasBitmap( );

    if ( atlasBitmap.empty( ) )
    {
        return;
    }

    // Check if texture needs resizing
    if ( m_fontAtlasTextureDesc.Width != fontAsset.AtlasWidth || m_fontAtlasTextureDesc.Height != fontAsset.AtlasHeight )
    {
        TextureDesc newDesc = m_fontAtlasTextureDesc;
        newDesc.Width       = fontAsset.AtlasWidth;
        newDesc.Height      = fontAsset.AtlasHeight;

        auto newTexture = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( newDesc ) );
        m_resourceTracking.TrackTexture( newTexture.get( ), ResourceUsage::ShaderResource );
        m_fontAtlasTexture = std::move( newTexture );
        m_resourceBindGroup->BeginUpdate( )->Srv( 0, m_fontAtlasTexture.get( ) )->EndUpdate( );
    }

    BufferDesc stagingDesc;
    stagingDesc.NumBytes     = atlasBitmap.size( );
    stagingDesc.Descriptor   = BitSet( ResourceDescriptor::Buffer );
    stagingDesc.InitialUsage = ResourceUsage::CopySrc;
    stagingDesc.DebugName    = "Font Atlas Staging Buffer";
    stagingDesc.HeapType     = HeapType::CPU;

    const auto stagingBuffer = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( stagingDesc ) );
    m_resourceTracking.TrackBuffer( stagingBuffer.get( ), ResourceUsage::CopySrc );

    void *mappedData = stagingBuffer->MapMemory( );
    memcpy( mappedData, atlasBitmap.data( ), atlasBitmap.size( ) );
    stagingBuffer->UnmapMemory( );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( m_fontAtlasTexture.get( ), ResourceUsage::CopyDst );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    CopyBufferToTextureDesc copyDesc;
    copyDesc.SrcBuffer  = stagingBuffer.get( );
    copyDesc.DstTexture = m_fontAtlasTexture.get( );
    copyDesc.RowPitch   = fontAsset.AtlasWidth;
    copyDesc.Format     = m_fontAtlasTexture->GetFormat( );

    commandList->CopyBufferToTexture( copyDesc );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( m_fontAtlasTexture.get( ), ResourceUsage::ShaderResource );
    m_resourceTracking.BatchTransition( batchTransitionDesc );
}

void FontRenderer::UpdateBuffers( )
{
    // Check if we need to resize vertex buffer
    if ( m_vertexBufferDesc.NumBytes < m_vertexData.NumElements( ) * sizeof( float ) )
    {
        BufferDesc newDesc = m_vertexBufferDesc;
        newDesc.NumBytes   = m_maxVertices * 8 * sizeof( float );

        auto newBuffer = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( newDesc ) );
        m_resourceTracking.TrackBuffer( newBuffer.get( ), ResourceUsage::VertexAndConstantBuffer );
        m_vertexBuffer = std::move( newBuffer );
    }

    // Check if we need to resize index buffer
    if ( m_indexBufferDesc.NumBytes < m_indexData.NumElements( ) * sizeof( uint32_t ) )
    {
        BufferDesc newDesc = m_indexBufferDesc;
        newDesc.NumBytes   = m_maxIndices * sizeof( uint32_t );

        auto newBuffer = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( newDesc ) );
        m_resourceTracking.TrackBuffer( newBuffer.get( ), ResourceUsage::IndexBuffer );
        m_indexBuffer = std::move( newBuffer );
    }

    void *vertexData = m_vertexBuffer->MapMemory( );
    memcpy( vertexData, m_vertexData.Data( ), m_vertexData.NumElements( ) * sizeof( float ) );
    m_vertexBuffer->UnmapMemory( );

    void *indexData = m_indexBuffer->MapMemory( );
    memcpy( indexData, m_indexData.Data( ), m_indexData.NumElements( ) * sizeof( uint32_t ) );
    m_indexBuffer->UnmapMemory( );
}

void FontRenderer::CalculateCenteredPosition( const std::u32string &text, TextRenderDesc &params ) const
{
    if ( !m_currentFont || text.empty( ) )
    {
        return;
    }

    const FontAsset &fontAsset = m_currentFont->GetFontAsset( );

    float       textWidth  = 0.0f;
    const float textHeight = static_cast<float>( fontAsset.Metrics.LineHeight ) * params.Scale;

    // This is the legacy positioning method, still used as a fallback
    // The HarfBuzz-based positioning handles this more accurately now
    for ( const char32_t c : text )
    {
        if ( const GlyphMetrics *metrics = m_currentFont->GetGlyphMetrics( c ) )
        {
            textWidth += static_cast<float>( metrics->Advance ) * params.Scale;
        }
    }

    if ( params.HorizontalCenter )
    {
        params.X -= textWidth / 2.0f;
    }

    if ( params.VerticalCenter )
    {
        params.Y -= textHeight / 2.0f;
    }
}
