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

#include "DenOfIzGraphicsInternal/UI/ClayRenderer.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/EmbeddedShaders.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

// Include last due to DirectXMath
#include "DenOfIzGraphicsInternal/Utilities/InteropMathConverter.h"

using namespace DenOfIz;
using namespace DirectX;

ClayRenderer::ClayRenderer( const ClayRendererDesc &desc ) : m_desc( desc ), m_logicalDevice( desc.LogicalDevice ), m_clayContext( desc.Context )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( m_logicalDevice ) )
    {
        spdlog::error( "ClayRenderer: LogicalDevice cannot be null" );
        return;
    }

    if ( m_clayContext == nullptr )
    {
        spdlog::error( "ClayRenderer: ClayContext cannot be null" );
        return;
    }
    m_clayText = m_clayContext->GetClayText( );

    DenOfIz_PhysicalDevice deviceInfo{};
    DenOfIz_LogicalDevice_DeviceInfo( m_logicalDevice, &deviceInfo );
    m_supportsSrvArray = deviceInfo.Capabilities.SrvArray;
    spdlog::info( "ClayRenderer: SrvArray support: {}", m_supportsSrvArray ? "yes" : "no" );

    if ( DENOFIZ_HANDLE_IS_VALID( desc.ResourceTracking ) )
    {
        m_resourceTracking     = desc.ResourceTracking;
        m_ownsResourceTracking = false;
    }
    else
    {
        DenOfIz_ResourceTracking_Create( &m_resourceTracking );
        m_ownsResourceTracking = true;
    }

    m_viewportWidth  = desc.Width;
    m_viewportHeight = desc.Height;

    DenOfIz_CommandQueueDesc commandQueueDesc{};
    commandQueueDesc.QueueType = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    DenOfIz_LogicalDevice_CreateCommandQueue( m_logicalDevice, &commandQueueDesc, &m_commandQueue );

    DenOfIz_CommandListPoolDesc poolDesc{};
    poolDesc.CommandQueue    = m_commandQueue;
    poolDesc.NumCommandLists = desc.NumFrames;
    DenOfIz_LogicalDevice_CreateCommandListPool( m_logicalDevice, &poolDesc, &m_commandListPool );

    m_frameData.resize( m_desc.NumFrames );
    for ( uint32_t i = 0; i < m_desc.NumFrames; ++i )
    {
        m_frameData[ i ].Textures.resize( desc.MaxTextures, DENOFIZ_NULL_HANDLE );
    }
    CreateShaderProgram( );
    CreatePipeline( );
    CreateNullTexture( );
    CreateBuffers( );
    CreateRenderTargets( );
    UpdateProjectionMatrix( );

    DenOfIz_CommandListArray commandLists{};
    DenOfIz_CommandListPool_GetCommandLists( m_commandListPool, &commandLists );
    for ( uint32_t i = 0; i < desc.NumFrames && i < commandLists.NumElements; ++i )
    {
        m_frameData[ i ].CommandList = commandLists.Elements[ i ];
        DenOfIz_LogicalDevice_CreateFence( m_logicalDevice, &m_frameData[ i ].FrameFence );
        DenOfIz_LogicalDevice_CreateSemaphore( m_logicalDevice, &m_frameData[ i ].FrameSemaphore );
    }
}

ClayRenderer::~ClayRenderer( )
{
    if ( m_ownsResourceTracking )
    {
        DenOfIz_ResourceTracking_Destroy( m_resourceTracking );
    }
    if ( m_vertexBufferData && DENOFIZ_HANDLE_IS_VALID( m_vertexBuffer ) )
    {
        DenOfIz_Buffer_UnmapMemory( m_vertexBuffer );
    }
    if ( m_indexBufferData && DENOFIZ_HANDLE_IS_VALID( m_indexBuffer ) )
    {
        DenOfIz_Buffer_UnmapMemory( m_indexBuffer );
    }
    if ( m_uniformBufferData && DENOFIZ_HANDLE_IS_VALID( m_uniformBuffer ) )
    {
        DenOfIz_Buffer_UnmapMemory( m_uniformBuffer );
    }

    for ( auto &bindGroup : m_perTextureBindGroups )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            DenOfIz_BindGroup_Destroy( bindGroup );
        }
    }
    m_perTextureBindGroups.clear( );

    for ( auto &frame : m_frameData )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( frame.ConstantsBindGroup ) )
        {
            DenOfIz_BindGroup_Destroy( frame.ConstantsBindGroup );
        }
        if ( m_supportsSrvArray && DENOFIZ_HANDLE_IS_VALID( frame.TextureBindGroup ) )
        {
            DenOfIz_BindGroup_Destroy( frame.TextureBindGroup );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( frame.ColorTarget ) )
        {
            DenOfIz_TextureResource_Destroy( frame.ColorTarget );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( frame.DepthBuffer ) )
        {
            DenOfIz_TextureResource_Destroy( frame.DepthBuffer );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( frame.FrameFence ) )
        {
            DenOfIz_Fence_Destroy( frame.FrameFence );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( frame.FrameSemaphore ) )
        {
            DenOfIz_Semaphore_Destroy( frame.FrameSemaphore );
        }
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_linearSampler ) )
    {
        DenOfIz_Sampler_Destroy( m_linearSampler );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_nullTexture ) )
    {
        DenOfIz_TextureResource_Destroy( m_nullTexture );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_uniformBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_uniformBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_indexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_indexBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_vertexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_vertexBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_pipeline ) )
    {
        DenOfIz_Pipeline_Destroy( m_pipeline );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_rootSignature ) )
    {
        DenOfIz_RootSignature_Destroy( m_rootSignature );
    }
    for ( auto &layout : m_bindGroupLayouts )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( layout ) )
        {
            DenOfIz_BindGroupLayout_Destroy( layout );
        }
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_inputLayout ) )
    {
        DenOfIz_InputLayout_Destroy( m_inputLayout );
    }
    // m_shaderProgram comes from EmbeddedShaders static cache
    m_shaderProgram = DENOFIZ_NULL_HANDLE;
    if ( DENOFIZ_HANDLE_IS_VALID( m_commandListPool ) )
    {
        DenOfIz_CommandListPool_Destroy( m_commandListPool );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_commandQueue ) )
    {
        DenOfIz_CommandQueue_Destroy( m_commandQueue );
    }
}

void ClayRenderer::CreateShaderProgram( )
{
    if ( m_supportsSrvArray )
    {
        m_shaderProgram = EmbeddedShaders::GetUIArrayShaderProgram( );
    }
    else
    {
        m_shaderProgram = EmbeddedShaders::GetUISingleShaderProgram( );
    }
}

void ClayRenderer::CreatePipeline( )
{
    DenOfIz_ShaderReflectDesc reflectDesc{};
    DenOfIz_ShaderProgram_Reflect( m_shaderProgram, &reflectDesc );

    m_bindGroupLayouts.resize( reflectDesc.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < reflectDesc.BindGroupLayouts.NumElements; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroupLayout( m_logicalDevice, &reflectDesc.BindGroupLayouts.Elements[ i ], &m_bindGroupLayouts[ i ] );
    }

    DenOfIz_RootSignatureDesc rootSigDesc{};
    rootSigDesc.BindGroupLayouts.Elements    = m_bindGroupLayouts.data( );
    rootSigDesc.BindGroupLayouts.NumElements = m_bindGroupLayouts.size( );
    rootSigDesc.RootConstants                = reflectDesc.RootConstants;
    DenOfIz_LogicalDevice_CreateRootSignature( m_logicalDevice, &rootSigDesc, &m_rootSignature );
    DenOfIz_LogicalDevice_CreateInputLayout( m_logicalDevice, &reflectDesc.InputLayout, &m_inputLayout );

    DenOfIz_PipelineDesc pipelineDesc{};
    pipelineDesc.RootSignature = m_rootSignature;
    pipelineDesc.InputLayout   = m_inputLayout;
    pipelineDesc.ShaderProgram = m_shaderProgram;
    pipelineDesc.BindPoint     = DENOFIZ_BIND_POINT_GRAPHICS;

    pipelineDesc.Graphics.PrimitiveTopology = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
    pipelineDesc.Graphics.CullMode          = DENOFIZ_CULL_MODE_NONE;
    pipelineDesc.Graphics.FillMode          = DENOFIZ_FILL_MODE_SOLID;

    pipelineDesc.Graphics.DepthTest.Enable             = true;
    pipelineDesc.Graphics.DepthTest.CompareOp          = DENOFIZ_COMPARE_OP_LESS;
    pipelineDesc.Graphics.DepthTest.Write              = true;
    pipelineDesc.Graphics.DepthStencilAttachmentFormat = DENOFIZ_FORMAT_D32_FLOAT;

    DenOfIz_RenderTargetDesc renderTarget{};
    renderTarget.Format                      = m_desc.RenderTargetFormat;
    renderTarget.Blend.Enable                = true;
    renderTarget.Blend.SrcBlend              = DENOFIZ_BLEND_SRC_ALPHA;
    renderTarget.Blend.DstBlend              = DENOFIZ_BLEND_INV_SRC_ALPHA;
    renderTarget.Blend.BlendOp               = DENOFIZ_BLEND_OP_ADD;
    renderTarget.Blend.SrcBlendAlpha         = DENOFIZ_BLEND_SRC_ALPHA;
    renderTarget.Blend.DstBlendAlpha         = DENOFIZ_BLEND_INV_SRC_ALPHA;
    renderTarget.Blend.BlendOpAlpha          = DENOFIZ_BLEND_OP_ADD;
    renderTarget.Blend.RenderTargetWriteMask = 0x0F;

    pipelineDesc.Graphics.RenderTargets.Elements    = &renderTarget;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;

    DenOfIz_LogicalDevice_CreatePipeline( m_logicalDevice, &pipelineDesc, &m_pipeline );
}

void ClayRenderer::CreateBuffers( )
{
    DenOfIz_BufferDesc vertexBufferDesc{};
    vertexBufferDesc.NumBytes  = m_desc.MaxVertices * sizeof( UIVertex );
    vertexBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_VERTEX_BIT;
    vertexBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    vertexBufferDesc.DebugName = DENOFIZ_STRING( "UI Vertex Buffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &vertexBufferDesc, &m_vertexBuffer );
    DenOfIz_Buffer_MapMemory( m_vertexBuffer, (void **)&m_vertexBufferData );

    DenOfIz_BufferDesc indexBufferDesc{};
    indexBufferDesc.NumBytes  = m_desc.MaxIndices * sizeof( uint32_t );
    indexBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_INDEX_BIT;
    indexBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    indexBufferDesc.DebugName = DENOFIZ_STRING( "UI Index Buffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &indexBufferDesc, &m_indexBuffer );
    DenOfIz_Buffer_MapMemory( m_indexBuffer, (void **)&m_indexBufferData );

    m_alignedUniformSize = Utilities::Align( sizeof( UIUniforms ), 256 );
    DenOfIz_BufferDesc uniformBufferDesc{};
    uniformBufferDesc.NumBytes  = m_desc.NumFrames * m_alignedUniformSize;
    uniformBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    uniformBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    uniformBufferDesc.DebugName = DENOFIZ_STRING( "UI Uniform Buffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &uniformBufferDesc, &m_uniformBuffer );
    DenOfIz_Buffer_MapMemory( m_uniformBuffer, (void **)&m_uniformBufferData );

    DenOfIz_SamplerDesc samplerDesc{};
    samplerDesc.MagFilter    = DENOFIZ_FILTER_LINEAR;
    samplerDesc.MinFilter    = DENOFIZ_FILTER_LINEAR;
    samplerDesc.MipmapMode   = DENOFIZ_MIPMAP_MODE_LINEAR;
    samplerDesc.AddressModeU = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.AddressModeV = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.AddressModeW = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.CompareOp    = DENOFIZ_COMPARE_OP_ALWAYS;
    DenOfIz_LogicalDevice_CreateSampler( m_logicalDevice, &samplerDesc, &m_linearSampler );

    for ( uint32_t frameIdx = 0; frameIdx < m_desc.NumFrames; ++frameIdx )
    {
        FrameData &frame = m_frameData[ frameIdx ];

        DenOfIz_BindGroupDesc constantGroupDesc{};
        constantGroupDesc.Layout = m_bindGroupLayouts[ 1 ];
        DenOfIz_LogicalDevice_CreateBindGroup( m_logicalDevice, &constantGroupDesc, &frame.ConstantsBindGroup );

        DenOfIz_BindBufferDesc bindUniformsDesc{};
        bindUniformsDesc.Resource       = m_uniformBuffer;
        bindUniformsDesc.ResourceOffset = frameIdx * m_alignedUniformSize;

        DenOfIz_BindGroup_BeginUpdate( frame.ConstantsBindGroup );
        DenOfIz_BindGroup_CbvWithDesc( frame.ConstantsBindGroup, &bindUniformsDesc );
        DenOfIz_BindGroup_EndUpdate( frame.ConstantsBindGroup );

        if ( m_supportsSrvArray )
        {
            DenOfIz_BindGroupDesc textureGroupDesc{};
            textureGroupDesc.Layout = m_bindGroupLayouts[ 0 ];
            DenOfIz_LogicalDevice_CreateBindGroup( m_logicalDevice, &textureGroupDesc, &frame.TextureBindGroup );
        }
    }

    if ( !m_supportsSrvArray )
    {
        m_perTextureBindGroups.resize( m_desc.MaxTextures );
        for ( uint32_t i = 0; i < m_desc.MaxTextures; ++i )
        {
            DenOfIz_BindGroupDesc perTextureGroupDesc{};
            perTextureGroupDesc.Layout = m_bindGroupLayouts[ 0 ];
            DenOfIz_LogicalDevice_CreateBindGroup( m_logicalDevice, &perTextureGroupDesc, &m_perTextureBindGroups[ i ] );
        }
    }

    for ( uint32_t frameIdx = 0; frameIdx < m_desc.NumFrames; ++frameIdx )
    {
        UpdateTextureBindings( frameIdx );
    }
}

void ClayRenderer::CreateNullTexture( )
{
    DenOfIz_TextureDesc textureDesc{};
    textureDesc.Width     = 1;
    textureDesc.Height    = 1;
    textureDesc.Depth     = 1;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format    = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Usage     = DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT;
    textureDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    textureDesc.DebugName = DENOFIZ_STRING( "UI Null Texture" );

    DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &textureDesc, &m_nullTexture );
    for ( uint32_t frameIdx = 0; frameIdx < m_desc.NumFrames; ++frameIdx )
    {
        FrameData &frame    = m_frameData[ frameIdx ];
        frame.Textures[ 0 ] = m_nullTexture;
    }
}

void ClayRenderer::CreateRenderTargets( )
{
    for ( uint32_t frameIdx = 0; frameIdx < m_desc.NumFrames; ++frameIdx )
    {
        FrameData &frame = m_frameData[ frameIdx ];

        if ( DENOFIZ_HANDLE_IS_VALID( frame.ColorTarget ) )
        {
            DenOfIz_ResourceTracking_UntrackTexture( m_resourceTracking, frame.ColorTarget );
            DenOfIz_TextureResource_Destroy( frame.ColorTarget );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( frame.DepthBuffer ) )
        {
            DenOfIz_ResourceTracking_UntrackTexture( m_resourceTracking, frame.DepthBuffer );
            DenOfIz_TextureResource_Destroy( frame.DepthBuffer );
        }

        DenOfIz_TextureDesc colorDesc{};
        colorDesc.Width             = static_cast<uint32_t>( m_viewportWidth );
        colorDesc.Height            = static_cast<uint32_t>( m_viewportHeight );
        colorDesc.Depth             = 1;
        colorDesc.MipLevels         = 1;
        colorDesc.ArraySize         = 1;
        colorDesc.Format            = m_desc.RenderTargetFormat;
        colorDesc.Usage             = DENOFIZ_TEXTURE_USAGE_COPY_SRC_BIT | DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT | DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT;
        colorDesc.HeapType          = DENOFIZ_HEAP_TYPE_GPU;
        const std::string colorName = "UI Color Target Frame " + std::to_string( frameIdx );
        colorDesc.DebugName         = DenOfIz_StringView( colorName.c_str( ), static_cast<uint32_t>( colorName.size( ) ) );

        DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &colorDesc, &frame.ColorTarget );

        DenOfIz_TextureDesc depthDesc{};
        depthDesc.Width                 = static_cast<uint32_t>( m_viewportWidth );
        depthDesc.Height                = static_cast<uint32_t>( m_viewportHeight );
        depthDesc.Depth                 = 1;
        depthDesc.MipLevels             = 1;
        depthDesc.ArraySize             = 1;
        depthDesc.Format                = DENOFIZ_FORMAT_D32_FLOAT;
        depthDesc.Usage                 = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;
        depthDesc.HeapType              = DENOFIZ_HEAP_TYPE_GPU;
        depthDesc.ClearDepthStencilHint = { 1.0f, 0.0f };
        const std::string depthName     = "UI Depth Buffer Frame " + std::to_string( frameIdx );
        depthDesc.DebugName             = DenOfIz_StringView( depthName.c_str( ), static_cast<uint32_t>( depthName.size( ) ) );

        DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &depthDesc, &frame.DepthBuffer );

        DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, frame.ColorTarget, DENOFIZ_QUEUE_TYPE_GRAPHICS );
        DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, frame.DepthBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    }
}

void ClayRenderer::UpdateProjectionMatrix( )
{
    const XMMATRIX projection = XMMatrixOrthographicOffCenterLH( 0.0f, m_viewportWidth, m_viewportHeight, 0.0f, 0.0f, 1.0f );
    XMFLOAT4X4     projMatrix;
    XMStoreFloat4x4( &projMatrix, projection );
    m_projectionMatrix = InteropMathConverter::Float_4x4FromXMFLOAT4X4( projMatrix );
}

void ClayRenderer::Resize( const float width, const float height )
{
    if ( m_viewportWidth == width && m_viewportHeight == height )
    {
        return;
    }
    m_viewportWidth  = width;
    m_viewportHeight = height;
    CreateRenderTargets( );
    UpdateProjectionMatrix( );
}

void ClayRenderer::SetDeltaTime( const float deltaTime )
{
    m_deltaTime = deltaTime;
}

void ClayRenderer::Render( const Clay_RenderCommandArray commands, const uint32_t frameIndex, DenOfIz_Texture *outTexture, DenOfIz_Semaphore *outSemaphore )
{
    if ( frameIndex >= m_frameData.size( ) )
    {
        spdlog::error( "ClayRenderer::Render: Invalid frame index {}", frameIndex );
        return;
    }

    const FrameData &frame = m_frameData[ frameIndex ];
    DenOfIz_Fence_Wait( frame.FrameFence );

    RenderInternal( commands, frameIndex, outTexture, outSemaphore );
}

void ClayRenderer::RenderToCommandList( Clay_RenderCommandArray commands, const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    if ( frameIndex >= m_frameData.size( ) )
    {
        spdlog::error( "ClayRenderer::RenderToCommandList: Invalid frame index {}", frameIndex );
        return;
    }

    if ( commands.length == 0 )
    {
        return;
    }

    FrameData &frame = m_frameData[ frameIndex ];

    ++m_currentFrame;

    if ( m_clayText )
    {
        m_clayText->UpdateFrame( m_currentFrame );
    }

    if ( m_currentFrame % 6000 == 0 )
    {
        m_shapeCache.Cleanup( m_currentFrame );
    }

    m_batchedVertices.clear( );
    m_batchedIndices.clear( );

    m_batchedVertices.reserve( commands.length * 6 );
    m_batchedIndices.reserve( commands.length * 9 );

    m_currentDepth = 0.9f;

    UIUniforms tempUniforms;
    tempUniforms.Projection = m_projectionMatrix;
    tempUniforms.ScreenSize = DenOfIz_Float4{ m_viewportWidth, m_viewportHeight, 0.0f, 0.0f };

    float atlasWidth  = 512.0f;
    float atlasHeight = 512.0f;
    if ( m_clayText != nullptr )
    {
        for ( uint16_t fontId = 0; fontId < 100; ++fontId )
        {
            Font *font = m_clayText->GetFont( fontId );
            if ( font && DENOFIZ_HANDLE_IS_VALID( font->Asset( ) ) )
            {
                atlasWidth  = static_cast<float>( DenOfIz_FontAsset_AtlasWidth( font->Asset( ) ) );
                atlasHeight = static_cast<float>( DenOfIz_FontAsset_AtlasHeight( font->Asset( ) ) );
                break;
            }
        }
    }

    tempUniforms.FontParams = DenOfIz_Float4{ atlasWidth, atlasHeight, DENOFIZ_FONT_MSDF_PIXEL_RANGE, static_cast<float>( m_desc.MaxNumFonts + 1 ) };

    uint8_t *uniformLocation = reinterpret_cast<uint8_t *>( m_uniformBufferData ) + frameIndex * m_alignedUniformSize;
    memcpy( uniformLocation, &tempUniforms, sizeof( UIUniforms ) );

    m_drawBatches.clear( );
    m_overlayColorStack.clear( );
    m_totalVertexCount         = 0;
    m_totalIndexCount          = 0;
    m_currentDepth             = 0.9f;
    m_currentBatchTextureIndex = 0;

    for ( int32_t i = 0; i < commands.length; ++i )
    {
        const Clay_RenderCommand *cmd = Clay_RenderCommandArray_Get( &commands, i );
        ProcessRenderCommand( cmd, commandList, frameIndex );
    }

    if ( m_frameData[ frameIndex ].AreTexturesDirty )
    {
        SyncFontTexturesFromClayText( );
        UpdateTextureBindings( frameIndex );
        m_frameData[ frameIndex ].AreTexturesDirty = false;
    }

    DenOfIz_CommandList_BindViewport( commandList, 0.0f, 0.0f, m_viewportWidth, m_viewportHeight );
    DenOfIz_CommandList_BindScissorRect( commandList, 0.0f, 0.0f, m_viewportWidth, m_viewportHeight );
    DenOfIz_CommandList_BindPipeline( commandList, m_pipeline );
    DenOfIz_CommandList_BindGroup( commandList, frame.ConstantsBindGroup );
    if ( m_supportsSrvArray )
    {
        DenOfIz_CommandList_BindGroup( commandList, frame.TextureBindGroup );
    }

    FlushBatchedGeometry( commandList );
}

void ClayRenderer::RenderInternal( Clay_RenderCommandArray commands, uint32_t frameIndex, DenOfIz_Texture *outTexture, DenOfIz_Semaphore *outSemaphore )
{
    if ( commands.length == 0 )
    {
        return;
    }

    FrameData &frame = m_frameData[ frameIndex ];

    ++m_currentFrame;

    if ( m_clayText )
    {
        m_clayText->UpdateFrame( m_currentFrame );
    }

    if ( m_currentFrame % 6000 == 0 )
    {
        m_shapeCache.Cleanup( m_currentFrame );
    }

    m_batchedVertices.clear( );
    m_batchedIndices.clear( );

    m_batchedVertices.reserve( commands.length * 6 );
    m_batchedIndices.reserve( commands.length * 9 );

    m_currentDepth = 0.9f;

    UIUniforms tempUniforms;
    tempUniforms.Projection = m_projectionMatrix;
    tempUniforms.ScreenSize = DenOfIz_Float4{ m_viewportWidth, m_viewportHeight, 0.0f, 0.0f };

    float atlasWidth  = 512.0f;
    float atlasHeight = 512.0f;
    if ( m_clayText != nullptr )
    {
        for ( uint16_t fontId = 0; fontId < 100; ++fontId )
        {
            Font *font = m_clayText->GetFont( fontId );
            if ( font && DENOFIZ_HANDLE_IS_VALID( font->Asset( ) ) )
            {
                atlasWidth  = static_cast<float>( DenOfIz_FontAsset_AtlasWidth( font->Asset( ) ) );
                atlasHeight = static_cast<float>( DenOfIz_FontAsset_AtlasHeight( font->Asset( ) ) );
                break;
            }
        }
    }

    tempUniforms.FontParams = DenOfIz_Float4{ atlasWidth, atlasHeight, DENOFIZ_FONT_MSDF_PIXEL_RANGE, static_cast<float>( m_desc.MaxNumFonts + 1 ) };

    uint8_t *uniformLocation = reinterpret_cast<uint8_t *>( m_uniformBufferData ) + frameIndex * m_alignedUniformSize;
    memcpy( uniformLocation, &tempUniforms, sizeof( UIUniforms ) );

    m_drawBatches.clear( );
    m_overlayColorStack.clear( );
    m_totalVertexCount         = 0;
    m_totalIndexCount          = 0;
    m_currentDepth             = 0.9f;
    m_currentBatchTextureIndex = 0;

    DenOfIz_CommandList uiCmdList = frame.CommandList;
    for ( int32_t i = 0; i < commands.length; ++i )
    {
        const Clay_RenderCommand *cmd = Clay_RenderCommandArray_Get( &commands, i );
        ProcessRenderCommand( cmd, uiCmdList, frameIndex );
    }

    if ( m_frameData[ frameIndex ].AreTexturesDirty )
    {
        SyncFontTexturesFromClayText( );
        UpdateTextureBindings( frameIndex );
        m_frameData[ frameIndex ].AreTexturesDirty = false;
    }

    DenOfIz_CommandList_Begin( uiCmdList );

    std::array textureTransitions = { DenOfIz_TransitionTextureDesc{ frame.ColorTarget, DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS },
                                      DenOfIz_TransitionTextureDesc{ frame.DepthBuffer, DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS } };

    DenOfIz_BatchTransitionDesc batchTransitionDesc{};
    batchTransitionDesc.Textures = { .Elements = textureTransitions.data( ), .NumElements = textureTransitions.size( ) };
    DenOfIz_ResourceTracking_BatchTransition( m_resourceTracking, uiCmdList, &batchTransitionDesc );

    {
        DenOfIz_RenderingDesc           renderingDesc{};
        DenOfIz_RenderingAttachmentDesc colorAttachment{};
        colorAttachment.Resource = frame.ColorTarget;
        colorAttachment.LoadOp   = DENOFIZ_LOAD_OP_CLEAR;
        colorAttachment.StoreOp  = DENOFIZ_STORE_OP_STORE;

        renderingDesc.RTAttachments.Elements    = &colorAttachment;
        renderingDesc.RTAttachments.NumElements = 1;
        renderingDesc.NumLayers                 = 1;

        renderingDesc.DepthAttachment.Resource          = frame.DepthBuffer;
        renderingDesc.DepthAttachment.LoadOp            = DENOFIZ_LOAD_OP_CLEAR;
        renderingDesc.DepthAttachment.StoreOp           = DENOFIZ_STORE_OP_DONT_CARE;
        renderingDesc.DepthAttachment.ClearDepthStencil = { 1.0f, 0 };

        renderingDesc.RenderAreaWidth   = m_viewportWidth;
        renderingDesc.RenderAreaHeight  = m_viewportHeight;
        renderingDesc.RenderAreaOffsetX = 0.0f;
        renderingDesc.RenderAreaOffsetY = 0.0f;

        DenOfIz_CommandList_BeginRendering( uiCmdList, &renderingDesc );
        DenOfIz_CommandList_BindViewport( uiCmdList, 0.0f, 0.0f, m_viewportWidth, m_viewportHeight );
        DenOfIz_CommandList_BindScissorRect( uiCmdList, 0.0f, 0.0f, m_viewportWidth, m_viewportHeight );
        DenOfIz_CommandList_BindPipeline( uiCmdList, m_pipeline );
        DenOfIz_CommandList_BindGroup( uiCmdList, frame.ConstantsBindGroup );
        if ( m_supportsSrvArray )
        {
            DenOfIz_CommandList_BindGroup( uiCmdList, frame.TextureBindGroup );
        }

        FlushBatchedGeometry( uiCmdList );
        DenOfIz_CommandList_EndRendering( uiCmdList );
    }

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, uiCmdList, frame.ColorTarget, DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( uiCmdList );

    DenOfIz_ExecuteCommandListsDesc executeDesc{};
    executeDesc.CommandLists.Elements        = &uiCmdList;
    executeDesc.CommandLists.NumElements     = 1;
    executeDesc.Signal                       = frame.FrameFence;
    executeDesc.SignalSemaphores.Elements    = &( frame.FrameSemaphore );
    executeDesc.SignalSemaphores.NumElements = 1;

    DenOfIz_CommandQueue_ExecuteCommandLists( m_commandQueue, &executeDesc );

    if ( outTexture != nullptr )
    {
        *outTexture = frame.ColorTarget;
    }
    if ( outSemaphore != nullptr )
    {
        *outSemaphore = frame.FrameSemaphore;
    }
}

void ClayRenderer::ProcessRenderCommand( const Clay_RenderCommand *command, DenOfIz_CommandList commandList, const uint32_t frameIndex )
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
        RenderImage( command, frameIndex );
        break;

    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
        SetScissor( command );
        break;

    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
        ClearScissor( );
        break;

    case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_START:
        PushOverlayColor( command );
        break;

    case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_END:
        PopOverlayColor( );
        break;

    case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
        RenderCustom( command, frameIndex );
        break;

    default:
        spdlog::warn( "Unknown Clay render command type: {}", static_cast<int>( command->commandType ) );
        break;
    }
}

void ClayRenderer::RenderRectangle( const Clay_RenderCommand *command, DenOfIz_CommandList commandList )
{
    const auto &data   = command->renderData.rectangle;
    const auto &bounds = command->boundingBox;

    const ShapeCacheKey cacheKey = UIShapeCache::CreateRectangleKey( command );
    CachedShape        *cached   = m_shapeCache.GetOrCreateCachedShape( cacheKey, m_currentFrame );

    if ( cached->vertices.size( ) == 0 )
    {
        constexpr uint32_t currentVertexCount = 0;

        if ( data.cornerRadius.topLeft > 0 || data.cornerRadius.topRight > 0 || data.cornerRadius.bottomLeft > 0 || data.cornerRadius.bottomRight > 0 )
        {
            UIShapes::GenerateRoundedRectangleDesc desc{};
            desc.Bounds            = bounds;
            desc.Color             = data.backgroundColor;
            desc.CornerRadius      = data.cornerRadius;
            desc.TextureIndex      = 0; // Solid color
            desc.SegmentsPerCorner = 8;

            UIShapes::GenerateRoundedRectangle( desc, &cached->vertices, &cached->indices, currentVertexCount );
        }
        else
        {
            UIShapes::GenerateRectangleDesc desc{};
            desc.Bounds       = bounds;
            desc.Color        = data.backgroundColor;
            desc.TextureIndex = 0;

            UIShapes::GenerateRectangle( desc, &cached->vertices, &cached->indices, currentVertexCount );
        }
    }

    if ( cached->vertices.size( ) > 0 && cached->indices.size( ) > 0 )
    {
        AddVerticesWithDepthVec( cached->vertices, cached->indices );
    }
}

void ClayRenderer::RenderBorder( const Clay_RenderCommand *command )
{
    const auto &data   = command->renderData.border;
    const auto &bounds = command->boundingBox;

    const ShapeCacheKey cacheKey = UIShapeCache::CreateBorderKey( command );
    CachedShape        *cached   = m_shapeCache.GetOrCreateCachedShape( cacheKey, m_currentFrame );

    if ( cached->vertices.size( ) == 0 )
    {
        constexpr uint32_t currentVertexCount = 0;

        UIShapes::GenerateBorderDesc desc{};
        desc.Bounds            = bounds;
        desc.Color             = data.color;
        desc.BorderWidth       = data.width;
        desc.CornerRadius      = data.cornerRadius;
        desc.SegmentsPerCorner = 8;

        UIShapes::GenerateBorder( desc, &cached->vertices, &cached->indices, currentVertexCount );
    }

    if ( cached->vertices.size( ) > 0 && cached->indices.size( ) > 0 )
    {
        AddVerticesWithDepthVec( cached->vertices, cached->indices );
    }
}

void ClayRenderer::RenderText( const Clay_RenderCommand *command, DenOfIz_CommandList commandList )
{
    const auto &data   = command->renderData.text;
    const auto &bounds = command->boundingBox;

    if ( !m_clayText )
    {
        spdlog::warn( "ClayText not set in ClayRenderer" );
        return;
    }

    const Font *font = m_clayText->GetFont( data.fontId );
    if ( font == nullptr )
    {
        spdlog::warn( "Font not found for ID: {}", data.fontId );
        return;
    }
    const float baseSize       = static_cast<float>( DenOfIz_FontAsset_InitialFontSize( font->Asset( ) ) );
    const float targetSize     = data.fontSize > 0 ? data.fontSize : baseSize;
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

        const DenOfIz_FontMetrics fontMetrics       = DenOfIz_FontAsset_GetMetrics( font->Asset( ) );
        const float               fontAscent        = fontMetrics.Ascent * effectiveScale;
        const float               fontDescent       = fontMetrics.Descent * effectiveScale;
        const float               defaultLineHeight = ( fontAscent + fontDescent ) * 1.2f;
        const float               lineHeight        = data.lineHeight > 0 ? data.lineHeight : defaultLineHeight;

        float currentY = bounds.y + fontAscent;

        for ( const auto &line : lines )
        {
            if ( !line.empty( ) )
            {
                Clay_RenderCommand lineCommand                    = *command;
                lineCommand.renderData.text.stringContents.chars  = line.c_str( );
                lineCommand.renderData.text.stringContents.length = static_cast<int32_t>( line.length( ) );
                lineCommand.boundingBox.y                         = currentY - fontAscent;

                RenderSingleLineText( &lineCommand, data.fontId, effectiveScale, fontAscent );
            }
            currentY += lineHeight;
        }
    }
    else
    {
        const float fontAscent = DenOfIz_FontAsset_GetMetrics( font->Asset( ) ).Ascent * effectiveScale;
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

    const float adjustedY = bounds.y + fontAscent;

    const TextVertexCacheKey vertexCacheKey = UITextVertexCache::CreateTextVertexKey( command, effectiveScale, adjustedY );
    CachedTextVertices      *cachedVertices = m_clayText->GetOrCreateTextVertices( vertexCacheKey );

    if ( cachedVertices->vertices.size( ) == 0 )
    {
        const TextVertexAllocationInfo allocInfo = textLayout->GetVertexAllocationInfo( );
        if ( allocInfo.VertexCount > 0 && allocInfo.IndexCount > 0 )
        {
            std::vector<GlyphVertex> glyphVertices( allocInfo.VertexCount );
            std::vector<uint32_t>    glyphIndices( allocInfo.IndexCount );

            GenerateTextVerticesDesc generateDesc{};
            generateDesc.StartPosition   = DenOfIz_Float2{ bounds.x, adjustedY };
            generateDesc.Color           = DenOfIz_Float4{ data.textColor.r / 255.0f, data.textColor.g / 255.0f, data.textColor.b / 255.0f, data.textColor.a / 255.0f };
            generateDesc.OutVertices     = glyphVertices.data( );
            generateDesc.OutIndices      = glyphIndices.data( );
            generateDesc.BaseVertexIndex = 0;
            generateDesc.BaseIndexOffset = 0;
            generateDesc.Scale           = effectiveScale;
            generateDesc.LetterSpacing   = data.letterSpacing;
            generateDesc.LineHeight      = data.lineHeight;

            textLayout->GenerateTextVertices( generateDesc );

            cachedVertices->vertices.reserve( allocInfo.VertexCount );
            for ( uint32_t i = 0; i < allocInfo.VertexCount; ++i )
            {
                const GlyphVertex &glyph = glyphVertices[ i ];
                UIVertex           vertex{};
                vertex.Position     = DenOfIz_Float3{ glyph.Position.X, glyph.Position.Y, 0.0f }; // Z will be set in AddVerticesWithDepth
                vertex.TexCoord     = glyph.UV;
                vertex.Color        = glyph.Color;
                vertex.TextureIndex = m_clayText->GetFontTextureIndex( fontId );
                cachedVertices->vertices.push_back( vertex );
            }

            cachedVertices->indices.reserve( allocInfo.IndexCount );
            for ( uint32_t i = 0; i < allocInfo.IndexCount; ++i )
            {
                cachedVertices->indices.push_back( glyphIndices[ i ] );
            }
        }
    }

    if ( cachedVertices->vertices.size( ) > 0 && cachedVertices->indices.size( ) > 0 )
    {
        AddVerticesWithDepthVec( cachedVertices->vertices, cachedVertices->indices );
    }
}

void ClayRenderer::RenderImage( const Clay_RenderCommand *command, const uint32_t frameIndex )
{
    const auto &data   = command->renderData.image;
    const auto &bounds = command->boundingBox;

    FrameData &frame        = m_frameData[ frameIndex ];
    uint32_t   textureIndex = 0;
    const auto it           = frame.ImageTextureIndices.find( data.imageData );
    if ( it != frame.ImageTextureIndices.end( ) )
    {
        textureIndex = it->second;
    }
    else
    {
        const auto texture                          = reinterpret_cast<DenOfIz_Texture>( data.imageData );
        textureIndex                                = RegisterTexture( texture, frameIndex );
        frame.ImageTextureIndices[ data.imageData ] = textureIndex;
    }

    std::vector<UIVertex> vertices;
    std::vector<uint32_t> indices;

    UIShapes::GenerateRectangleDesc desc{};
    desc.Bounds       = bounds;
    desc.Color        = Clay_Color{ 255, 255, 255, 255 };
    desc.TextureIndex = textureIndex;

    UIShapes::GenerateRectangle( desc, &vertices, &indices, 0 );
    if ( vertices.size( ) > 0 && indices.size( ) > 0 )
    {
        AddVerticesWithDepthVec( vertices, indices );
    }
}

void ClayRenderer::RenderCustom( const Clay_RenderCommand *command, const uint32_t frameIndex )
{
    const auto &data = command->renderData.custom;
    if ( data.customData == nullptr )
    {
        return;
    }

    auto      *customPtr = data.customData;
    const auto texture   = reinterpret_cast<DenOfIz_Texture>( customPtr );
    if ( DENOFIZ_HANDLE_IS_VALID( texture ) )
    {
        Clay_RenderCommand imageCommand         = *command;
        imageCommand.commandType                = CLAY_RENDER_COMMAND_TYPE_IMAGE;
        imageCommand.renderData.image.imageData = reinterpret_cast<void *>( texture );
        RenderImage( &imageCommand, frameIndex );
    }
}

void ClayRenderer::SetScissor( const Clay_RenderCommand *command )
{
    FlushCurrentBatch( );

    const auto &bounds = command->boundingBox;

    float x0 = std::floor( bounds.x );
    float y0 = std::floor( bounds.y );
    float x1 = std::ceil( bounds.x + bounds.width );
    float y1 = std::ceil( bounds.y + bounds.height );

    if ( !m_scissorStack.empty( ) )
    {
        const ScissorState &parent = m_scissorStack.back( );
        x0 = std::max( x0, parent.X );
        y0 = std::max( y0, parent.Y );
        x1 = std::min( x1, parent.X + parent.Width );
        y1 = std::min( y1, parent.Y + parent.Height );
    }

    ScissorState state;
    state.Enabled = true;
    state.X       = x0;
    state.Y       = y0;
    state.Width   = std::max( 0.0f, x1 - x0 );
    state.Height  = std::max( 0.0f, y1 - y0 );

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

void ClayRenderer::PushOverlayColor( const Clay_RenderCommand *command )
{
    m_overlayColorStack.push_back( command->renderData.overlayColor.color );
}

void ClayRenderer::PopOverlayColor( )
{
    if ( !m_overlayColorStack.empty( ) )
    {
        m_overlayColorStack.pop_back( );
    }
}

uint32_t ClayRenderer::RegisterTexture( DenOfIz_Texture texture, const uint32_t frameIndex )
{
    FrameData &frameData = m_frameData[ frameIndex ];
    for ( uint32_t i = m_desc.MaxNumFonts + 1; i < frameData.Textures.size( ); ++i )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( frameData.Textures[ i ] ) )
        {
            frameData.Textures[ i ]    = texture;
            frameData.AreTexturesDirty = true;
            return i;
        }
    }

    spdlog::error( "ClayRenderer: Exceeded maximum texture count" );
    return 0;
}

void ClayRenderer::UpdateTextureBindings( const uint32_t frameIndex ) const
{
    if ( frameIndex >= m_frameData.size( ) )
    {
        return;
    }

    const FrameData &frame = m_frameData[ frameIndex ];

    if ( m_supportsSrvArray )
    {
        std::vector<DenOfIz_Texture> textureVector;
        textureVector.reserve( frame.Textures.size( ) );
        for ( size_t i = 0; i < frame.Textures.size( ); ++i )
        {
            DenOfIz_Texture tex = frame.Textures[ i ];
            textureVector.push_back( DENOFIZ_HANDLE_IS_VALID( tex ) ? tex : m_nullTexture );
        }

        DenOfIz_TextureArray textureArray{};
        textureArray.Elements    = textureVector.data( );
        textureArray.NumElements = static_cast<uint32_t>( textureVector.size( ) );

        DenOfIz_BindGroup_BeginUpdate( frame.TextureBindGroup );
        DenOfIz_BindGroup_SrvArray( frame.TextureBindGroup, 0, &textureArray );
        DenOfIz_BindGroup_Sampler( frame.TextureBindGroup, 0, m_linearSampler );
        DenOfIz_BindGroup_EndUpdate( frame.TextureBindGroup );
    }
    else
    {
        for ( size_t i = 0; i < frame.Textures.size( ) && i < m_perTextureBindGroups.size( ); ++i )
        {
            DenOfIz_Texture tex = DENOFIZ_HANDLE_IS_VALID( frame.Textures[ i ] ) ? frame.Textures[ i ] : m_nullTexture;

            DenOfIz_BindGroup_BeginUpdate( m_perTextureBindGroups[ i ] );
            DenOfIz_BindGroup_SrvTexture( m_perTextureBindGroups[ i ], 0, tex );
            DenOfIz_BindGroup_Sampler( m_perTextureBindGroups[ i ], 0, m_linearSampler );
            DenOfIz_BindGroup_EndUpdate( m_perTextureBindGroups[ i ] );
        }
    }
}

void ClayRenderer::ClearCaches( )
{
    m_shapeCache.Clear( );
    if ( m_clayText )
    {
        m_clayText->ClearCaches( );
    }

    bool anyTextureCleared = false;
    for ( size_t frameIndex = 0; frameIndex < m_frameData.size( ); ++frameIndex )
    {
        FrameData &frameData = m_frameData[ frameIndex ];
        for ( uint32_t i = m_desc.MaxNumFonts + 1; i < frameData.Textures.size( ); ++i )
        {
            if ( DENOFIZ_HANDLE_IS_VALID( frameData.Textures[ i ] ) )
            {
                frameData.Textures[ i ] = DENOFIZ_NULL_HANDLE;
                anyTextureCleared       = true;
            }
        }
        frameData.ImageTextureIndices.clear( );
    }

    if ( anyTextureCleared )
    {
        m_texturesDirty = true;
    }
}

DenOfIz_ClayDimensions ClayRenderer::MeasureText( const DenOfIz_StringView &text, const Clay_TextElementConfig &desc ) const
{
    if ( !m_clayText )
    {
        return DenOfIz_ClayDimensions{ 0, 0 };
    }

    return m_clayText->MeasureText( text, desc );
}

void ClayRenderer::AddVerticesWithDepthVec( std::vector<UIVertex> &vertices, std::vector<uint32_t> &indices )
{
    UIVertexArray vertexArray{};
    vertexArray.Elements    = vertices.data( );
    vertexArray.NumElements = static_cast<uint32_t>( vertices.size( ) );

    DenOfIz_UInt32Array indexArray{};
    indexArray.Elements    = indices.data( );
    indexArray.NumElements = static_cast<uint32_t>( indices.size( ) );
    AddVerticesWithDepth( vertexArray, indexArray );
}

void ClayRenderer::AddVerticesWithDepth( const UIVertexArray &vertices, const DenOfIz_UInt32Array &indices )
{
    if ( vertices.NumElements == 0 )
    {
        return;
    }

    const uint32_t textureIndex = vertices.Elements[ 0 ].TextureIndex;

    if ( !m_supportsSrvArray && m_batchedVertices.size( ) > 0 && textureIndex != m_currentBatchTextureIndex )
    {
        FlushCurrentBatch( );
    }
    m_currentBatchTextureIndex = textureIndex;

    float overlayMix = 0.0f;
    float overlayR   = 0.0f;
    float overlayG   = 0.0f;
    float overlayB   = 0.0f;
    if ( !m_overlayColorStack.empty( ) )
    {
        const Clay_Color &overlay = m_overlayColorStack.back( );
        overlayMix                = overlay.a / 255.0f;
        overlayR                  = overlay.r / 255.0f;
        overlayG                  = overlay.g / 255.0f;
        overlayB                  = overlay.b / 255.0f;
    }

    const uint32_t baseVertexIndex = m_batchedVertices.size( );
    for ( uint32_t i = 0; i < vertices.NumElements; ++i )
    {
        UIVertex vertex   = vertices.Elements[ i ];
        vertex.Position.Z = m_currentDepth;

        if ( overlayMix > 0.0f )
        {
            vertex.Color.X += ( overlayR - vertex.Color.X ) * overlayMix;
            vertex.Color.Y += ( overlayG - vertex.Color.Y ) * overlayMix;
            vertex.Color.Z += ( overlayB - vertex.Color.Z ) * overlayMix;
        }

        if ( !m_supportsSrvArray && textureIndex > 0 )
        {
            if ( textureIndex < m_desc.MaxNumFonts + 1 )
            {
                vertex.TextureIndex = 1;
            }
            else
            {
                vertex.TextureIndex = 2;
            }
        }

        m_batchedVertices.push_back( vertex );
    }

    for ( uint32_t i = 0; i < indices.NumElements; ++i )
    {
        m_batchedIndices.push_back( indices.Elements[ i ] + baseVertexIndex );
    }

    m_currentDepth += DEPTH_INCREMENT;
}

void ClayRenderer::FlushCurrentBatch( )
{
    // ReSharper disable once CppDFAConstantConditions
    if ( m_batchedVertices.size( ) == 0 || m_batchedIndices.size( ) == 0 )
    {
        return;
    }

    // ReSharper disable once CppDFAUnreachableCode
    constexpr uint32_t vertexAlignment = 256 / sizeof( UIVertex );
    constexpr uint32_t indexAlignment  = 256 / sizeof( uint32_t );

    const uint32_t alignedVertexOffset = ( m_totalVertexCount + vertexAlignment - 1 ) / vertexAlignment * vertexAlignment;
    const uint32_t alignedIndexOffset  = ( m_totalIndexCount + indexAlignment - 1 ) / indexAlignment * indexAlignment;

    const size_t vertexDataSize = m_batchedVertices.size( ) * sizeof( UIVertex );
    const size_t indexDataSize  = m_batchedIndices.size( ) * sizeof( uint32_t );

    if ( alignedVertexOffset + m_batchedVertices.size( ) > m_desc.MaxVertices || alignedIndexOffset + m_batchedIndices.size( ) > m_desc.MaxIndices )
    {
        spdlog::error( "ClayRenderer: Geometry exceeds buffer limits" );
        return;
    }

    memcpy( m_vertexBufferData + alignedVertexOffset * sizeof( UIVertex ), m_batchedVertices.data( ), vertexDataSize );

    const auto indexDst = reinterpret_cast<uint32_t *>( m_indexBufferData + alignedIndexOffset * sizeof( uint32_t ) );
    for ( uint32_t i = 0; i < m_batchedIndices.size( ); ++i )
    {
        indexDst[ i ] = m_batchedIndices[ i ]; // Don't add vertex offset here, use baseVertex instead
    }

    DrawBatch batch;
    batch.VertexOffset = alignedVertexOffset;
    batch.IndexOffset  = alignedIndexOffset;
    batch.IndexCount   = m_batchedIndices.size( );
    batch.TextureIndex = m_currentBatchTextureIndex;

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

    m_totalVertexCount = alignedVertexOffset + m_batchedVertices.size( );
    m_totalIndexCount  = alignedIndexOffset + m_batchedIndices.size( );

    m_batchedVertices.clear( );
    m_batchedIndices.clear( );
}

void ClayRenderer::ExecuteDrawBatches( DenOfIz_CommandList commandList ) const
{
    if ( m_drawBatches.empty( ) )
    {
        return;
    }

    DenOfIz_CommandList_BindVertexBuffer( commandList, m_vertexBuffer, 0, sizeof( UIVertex ), 0 );
    DenOfIz_CommandList_BindIndexBuffer( commandList, m_indexBuffer, DENOFIZ_INDEX_TYPE_UINT32, 0 );

    for ( const auto &batch : m_drawBatches )
    {
        if ( batch.Scissor.Enabled )
        {
            DenOfIz_CommandList_BindScissorRect( commandList, batch.Scissor.X, batch.Scissor.Y, batch.Scissor.Width, batch.Scissor.Height );
        }
        else
        {
            DenOfIz_CommandList_BindScissorRect( commandList, 0, 0, m_viewportWidth, m_viewportHeight );
        }

        if ( !m_supportsSrvArray && batch.TextureIndex < m_perTextureBindGroups.size( ) )
        {
            DenOfIz_CommandList_BindGroup( commandList, m_perTextureBindGroups[ batch.TextureIndex ] );
        }

        DenOfIz_CommandList_DrawIndexed( commandList, batch.IndexCount, 1, batch.IndexOffset, batch.VertexOffset, 0 );
    }
}

void ClayRenderer::FlushBatchedGeometry( DenOfIz_CommandList commandList )
{
    FlushCurrentBatch( );
    ExecuteDrawBatches( commandList );
}

uint32_t ClayRenderer::GetCurrentVertexCount( ) const
{
    return m_batchedVertices.size( );
}

void ClayRenderBatch::AddVertices( const UIVertexArray &vertices, const DenOfIz_UInt32Array &indices )
{
    m_renderer->AddVerticesWithDepth( vertices, indices );
}

uint32_t ClayRenderBatch::GetCurrentVertexOffset( ) const
{
    return m_renderer->GetCurrentVertexCount( );
}

void ClayRenderer::SyncFontTexturesFromClayText( )
{
    for ( uint16_t fontId = 0; fontId < 16; ++fontId )
    {
        DenOfIz_Texture fontTexture = m_clayText->GetFontTexture( fontId );
        if ( DENOFIZ_HANDLE_IS_VALID( fontTexture ) )
        {
            const uint32_t textureIndex = m_clayText->GetFontTextureIndex( fontId );
            for ( uint32_t i = 0; i < m_frameData.size( ); ++i )
            {
                FrameData &frameData = m_frameData[ i ];
                if ( textureIndex > 0 && textureIndex < frameData.Textures.size( ) )
                {
                    frameData.Textures[ textureIndex ] = fontTexture;
                }
            }
        }
    }
}
