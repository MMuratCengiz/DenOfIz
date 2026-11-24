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

#include "DenOfIzGraphicsInternal/UI/UIRenderer.h"

#include <array>
#include <cmath>
#include <cstring>
#include <string>
#include "DenOfIzGraphics/Assets/Font/Font.h"
#include "DenOfIzGraphics/Backends/Interface/CommandListPool.h"
#include "DenOfIzGraphics/Backends/Interface/CommandQueue.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/EmbeddedShaders.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

// Include last due to DirectXMath
#include "DenOfIzGraphicsInternal/Utilities/InteropMathConverter.h"

using namespace DenOfIz;
using namespace DirectX;

namespace
{
    constexpr uint32_t MinVertexCapacity = 16384;
    constexpr uint32_t MinIndexCapacity  = 24576;
    constexpr uint32_t MinStateCapacity  = 256;
    constexpr uint64_t ShapeCacheAge     = 60;

    uint32_t GrowCapacity( const uint32_t current, const uint32_t required, const uint32_t minimum )
    {
        uint32_t capacity = std::max( current, minimum );
        while ( capacity < required )
        {
            capacity *= 2;
        }
        return capacity;
    }
} // namespace

UIRenderer::UIRenderer( const UIRendererDesc &desc ) : m_desc( desc ), m_device( desc.LogicalDevice )
{
    DenOfIz_PhysicalDevice deviceInfo{ };
    DenOfIz_LogicalDevice_DeviceInfo( m_device, &deviceInfo );
    m_supportsSrvArray = deviceInfo.Capabilities.SrvArray;

    if ( DENOFIZ_HANDLE_IS_VALID( desc.ResourceTracking ) )
    {
        m_resourceTracking = desc.ResourceTracking;
    }
    else
    {
        DenOfIz_ResourceTracking_Create( &m_resourceTracking );
        m_ownsTracking = true;
    }

    m_shaderProgram = m_supportsSrvArray ? EmbeddedShaders::GetUIArrayShaderProgram( ) : EmbeddedShaders::GetUISingleShaderProgram( );

    DenOfIz_ShaderReflectDesc reflectDesc{ };
    DenOfIz_ShaderProgram_Reflect( m_shaderProgram, &reflectDesc );
    m_bindGroupLayouts.resize( reflectDesc.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < reflectDesc.BindGroupLayouts.NumElements; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroupLayout( m_device, &reflectDesc.BindGroupLayouts.Elements[ i ], &m_bindGroupLayouts[ i ] );
    }
    DenOfIz_RootSignatureDesc rootSignatureDesc{ };
    rootSignatureDesc.BindGroupLayouts.Elements    = m_bindGroupLayouts.data( );
    rootSignatureDesc.BindGroupLayouts.NumElements = static_cast<uint32_t>( m_bindGroupLayouts.size( ) );
    rootSignatureDesc.RootConstants                = reflectDesc.RootConstants;
    DenOfIz_LogicalDevice_CreateRootSignature( m_device, &rootSignatureDesc, &m_rootSignature );
    DenOfIz_LogicalDevice_CreateInputLayout( m_device, &reflectDesc.InputLayout, &m_inputLayout );

    m_pipeline        = CreatePipeline( desc.DepthFormat );
    m_texturePipeline = desc.DepthFormat == DENOFIZ_FORMAT_UNDEFINED ? m_pipeline : CreatePipeline( DENOFIZ_FORMAT_UNDEFINED );

    DenOfIz_SamplerDesc samplerDesc{ };
    samplerDesc.MagFilter    = DENOFIZ_FILTER_LINEAR;
    samplerDesc.MinFilter    = DENOFIZ_FILTER_LINEAR;
    samplerDesc.MipmapMode   = DENOFIZ_MIPMAP_MODE_LINEAR;
    samplerDesc.AddressModeU = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.AddressModeV = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.AddressModeW = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.CompareOp    = DENOFIZ_COMPARE_OP_ALWAYS;
    DenOfIz_LogicalDevice_CreateSampler( m_device, &samplerDesc, &m_sampler );

    DenOfIz_TextureDesc nullDesc{ };
    nullDesc.Width     = 1;
    nullDesc.Height    = 1;
    nullDesc.Depth     = 1;
    nullDesc.MipLevels = 1;
    nullDesc.ArraySize = 1;
    nullDesc.Format    = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
    nullDesc.Usage     = DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT;
    nullDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    nullDesc.DebugName = DENOFIZ_STRING( "UI Null Texture" );
    DenOfIz_LogicalDevice_CreateTexture( m_device, &nullDesc, &m_nullTexture );

    m_alignedUniforms = Utilities::Align( sizeof( Uniforms ), 256 );
    DenOfIz_BufferDesc uniformDesc{ };
    uniformDesc.NumBytes  = m_alignedUniforms * desc.NumFrames;
    uniformDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    uniformDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    uniformDesc.DebugName = DENOFIZ_STRING( "UI Uniforms" );
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &uniformDesc, &m_uniformBuffer );
    DenOfIz_Buffer_MapMemory( m_uniformBuffer, reinterpret_cast<void **>( &m_uniformData ) );

    DenOfIz_CommandQueueDesc queueDesc{ };
    queueDesc.QueueType = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    DenOfIz_LogicalDevice_CreateCommandQueue( m_device, &queueDesc, &m_queue );
    DenOfIz_CommandListPoolDesc poolDesc{ };
    poolDesc.CommandQueue    = m_queue;
    poolDesc.NumCommandLists = desc.NumFrames;
    DenOfIz_LogicalDevice_CreateCommandListPool( m_device, &poolDesc, &m_commandListPool );

    CreateFrameResources( );
}

UIRenderer::~UIRenderer( )
{
    DenOfIz_LogicalDevice_WaitIdle( m_device );
    for ( FrameData &frame : m_frames )
    {
        if ( frame.VertexData )
        {
            DenOfIz_Buffer_UnmapMemory( frame.VertexBuffer );
        }
        if ( frame.IndexData )
        {
            DenOfIz_Buffer_UnmapMemory( frame.IndexBuffer );
        }
        if ( frame.StateData )
        {
            DenOfIz_Buffer_UnmapMemory( frame.StateBuffer );
        }
        for ( const DenOfIz_Buffer buffer : { frame.VertexBuffer, frame.IndexBuffer, frame.StateBuffer } )
        {
            if ( DENOFIZ_HANDLE_IS_VALID( buffer ) )
            {
                DenOfIz_Buffer_Destroy( buffer );
            }
        }
        for ( const DenOfIz_BindGroup group : frame.SlotGroups )
        {
            if ( DENOFIZ_HANDLE_IS_VALID( group ) )
            {
                DenOfIz_BindGroup_Destroy( group );
            }
        }
        for ( const DenOfIz_BindGroup group : { frame.Constants, frame.Textures } )
        {
            if ( DENOFIZ_HANDLE_IS_VALID( group ) )
            {
                DenOfIz_BindGroup_Destroy( group );
            }
        }
        if ( DENOFIZ_HANDLE_IS_VALID( frame.ColorTarget ) )
        {
            DenOfIz_ResourceTracking_UntrackTexture( m_resourceTracking, frame.ColorTarget );
            DenOfIz_Texture_Destroy( frame.ColorTarget );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( frame.Fence ) )
        {
            DenOfIz_Fence_Destroy( frame.Fence );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( frame.Semaphore ) )
        {
            DenOfIz_Semaphore_Destroy( frame.Semaphore );
        }
    }
    if ( m_uniformData )
    {
        DenOfIz_Buffer_UnmapMemory( m_uniformBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_uniformBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_uniformBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_sampler ) )
    {
        DenOfIz_Sampler_Destroy( m_sampler );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_nullTexture ) )
    {
        DenOfIz_Texture_Destroy( m_nullTexture );
    }
    if ( m_texturePipeline != m_pipeline && DENOFIZ_HANDLE_IS_VALID( m_texturePipeline ) )
    {
        DenOfIz_Pipeline_Destroy( m_texturePipeline );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_pipeline ) )
    {
        DenOfIz_Pipeline_Destroy( m_pipeline );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_rootSignature ) )
    {
        DenOfIz_RootSignature_Destroy( m_rootSignature );
    }
    for ( const DenOfIz_BindGroupLayout layout : m_bindGroupLayouts )
    {
        DenOfIz_BindGroupLayout_Destroy( layout );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_inputLayout ) )
    {
        DenOfIz_InputLayout_Destroy( m_inputLayout );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_commandListPool ) )
    {
        DenOfIz_CommandListPool_Destroy( m_commandListPool );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_queue ) )
    {
        DenOfIz_CommandQueue_Destroy( m_queue );
    }
    if ( m_ownsTracking )
    {
        DenOfIz_ResourceTracking_Destroy( m_resourceTracking );
    }
}

DenOfIz_Pipeline UIRenderer::CreatePipeline( const DenOfIz_Format depthFormat ) const
{
    DenOfIz_PipelineDesc pipelineDesc{ };
    pipelineDesc.RootSignature                         = m_rootSignature;
    pipelineDesc.InputLayout                           = m_inputLayout;
    pipelineDesc.ShaderProgram                         = m_shaderProgram;
    pipelineDesc.BindPoint                             = DENOFIZ_BIND_POINT_GRAPHICS;
    pipelineDesc.Graphics.PrimitiveTopology            = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
    pipelineDesc.Graphics.CullMode                     = DENOFIZ_CULL_MODE_NONE;
    pipelineDesc.Graphics.FillMode                     = DENOFIZ_FILL_MODE_SOLID;
    pipelineDesc.Graphics.DepthTest.Enable             = false;
    pipelineDesc.Graphics.DepthTest.Write              = false;
    pipelineDesc.Graphics.DepthTest.CompareOp          = DENOFIZ_COMPARE_OP_ALWAYS;
    pipelineDesc.Graphics.DepthStencilAttachmentFormat = depthFormat;

    DenOfIz_RenderTargetDesc renderTarget{ };
    renderTarget.Format                      = m_desc.RenderTargetFormat;
    renderTarget.Blend.Enable                = true;
    renderTarget.Blend.SrcBlend              = DENOFIZ_BLEND_SRC_ALPHA;
    renderTarget.Blend.DstBlend              = DENOFIZ_BLEND_INV_SRC_ALPHA;
    renderTarget.Blend.BlendOp               = DENOFIZ_BLEND_OP_ADD;
    renderTarget.Blend.SrcBlendAlpha         = DENOFIZ_BLEND_ONE;
    renderTarget.Blend.DstBlendAlpha         = DENOFIZ_BLEND_INV_SRC_ALPHA;
    renderTarget.Blend.BlendOpAlpha          = DENOFIZ_BLEND_OP_ADD;
    renderTarget.Blend.RenderTargetWriteMask = 0x0F;
    pipelineDesc.Graphics.RenderTargets.Elements    = &renderTarget;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;

    DenOfIz_Pipeline pipeline = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreatePipeline( m_device, &pipelineDesc, &pipeline );
    return pipeline;
}

void UIRenderer::CreateFrameResources( )
{
    m_frames.resize( m_desc.NumFrames );
    DenOfIz_CommandListArray commandLists{ };
    DenOfIz_CommandListPool_GetCommandLists( m_commandListPool, &commandLists );
    for ( uint32_t i = 0; i < m_desc.NumFrames; ++i )
    {
        FrameData &frame  = m_frames[ i ];
        frame.CommandList = i < commandLists.NumElements ? commandLists.Elements[ i ] : DENOFIZ_NULL_HANDLE;
        DenOfIz_LogicalDevice_CreateFence( m_device, &frame.Fence );
        DenOfIz_LogicalDevice_CreateSemaphore( m_device, &frame.Semaphore );

        DenOfIz_BindGroupDesc constantsDesc{ };
        constantsDesc.Layout = m_bindGroupLayouts[ 1 ];
        DenOfIz_LogicalDevice_CreateBindGroup( m_device, &constantsDesc, &frame.Constants );
        EnsureStateCapacity( i, MinStateCapacity, 0 );

        if ( m_supportsSrvArray )
        {
            DenOfIz_BindGroupDesc texturesDesc{ };
            texturesDesc.Layout = m_bindGroupLayouts[ 0 ];
            DenOfIz_LogicalDevice_CreateBindGroup( m_device, &texturesDesc, &frame.Textures );
        }
        EnsureGeometryCapacity( frame, MinVertexCapacity, MinIndexCapacity, 0, 0 );
    }
}

void UIRenderer::EnsureGeometryCapacity( FrameData &frame, const uint32_t vertexCount, const uint32_t indexCount, const uint32_t keepVertices, const uint32_t keepIndices ) const
{
    if ( vertexCount > frame.VertexCapacity )
    {
        const uint32_t capacity = GrowCapacity( frame.VertexCapacity, vertexCount, MinVertexCapacity );
        DenOfIz_BufferDesc desc{ };
        desc.NumBytes  = static_cast<uint64_t>( capacity ) * sizeof( UIVertex );
        desc.Usage     = DENOFIZ_BUFFER_USAGE_VERTEX_BIT;
        desc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
        desc.DebugName = DENOFIZ_STRING( "UI Vertices" );
        DenOfIz_Buffer buffer = DENOFIZ_NULL_HANDLE;
        uint8_t       *data   = nullptr;
        DenOfIz_LogicalDevice_CreateBuffer( m_device, &desc, &buffer );
        DenOfIz_Buffer_MapMemory( buffer, reinterpret_cast<void **>( &data ) );
        if ( DENOFIZ_HANDLE_IS_VALID( frame.VertexBuffer ) )
        {
            std::memcpy( data, frame.VertexData, static_cast<size_t>( keepVertices ) * sizeof( UIVertex ) );
            DenOfIz_Buffer_UnmapMemory( frame.VertexBuffer );
            DenOfIz_Buffer_Destroy( frame.VertexBuffer );
        }
        frame.VertexBuffer   = buffer;
        frame.VertexData     = data;
        frame.VertexCapacity = capacity;
    }
    if ( indexCount > frame.IndexCapacity )
    {
        const uint32_t capacity = GrowCapacity( frame.IndexCapacity, indexCount, MinIndexCapacity );
        DenOfIz_BufferDesc desc{ };
        desc.NumBytes  = static_cast<uint64_t>( capacity ) * sizeof( uint32_t );
        desc.Usage     = DENOFIZ_BUFFER_USAGE_INDEX_BIT;
        desc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
        desc.DebugName = DENOFIZ_STRING( "UI Indices" );
        DenOfIz_Buffer buffer = DENOFIZ_NULL_HANDLE;
        uint8_t       *data   = nullptr;
        DenOfIz_LogicalDevice_CreateBuffer( m_device, &desc, &buffer );
        DenOfIz_Buffer_MapMemory( buffer, reinterpret_cast<void **>( &data ) );
        if ( DENOFIZ_HANDLE_IS_VALID( frame.IndexBuffer ) )
        {
            std::memcpy( data, frame.IndexData, static_cast<size_t>( keepIndices ) * sizeof( uint32_t ) );
            DenOfIz_Buffer_UnmapMemory( frame.IndexBuffer );
            DenOfIz_Buffer_Destroy( frame.IndexBuffer );
        }
        frame.IndexBuffer   = buffer;
        frame.IndexData     = data;
        frame.IndexCapacity = capacity;
    }
}

void UIRenderer::BindFrameConstants( const uint32_t frameIndex ) const
{
    const FrameData       &frame = m_frames[ frameIndex ];
    DenOfIz_BindBufferDesc bindUniforms{ };
    bindUniforms.Resource       = m_uniformBuffer;
    bindUniforms.ResourceOffset = frameIndex * m_alignedUniforms;
    DenOfIz_BindGroup_BeginUpdate( frame.Constants );
    DenOfIz_BindGroup_CbvWithDesc( frame.Constants, &bindUniforms );
    DenOfIz_BindGroup_SrvBuffer( frame.Constants, 0, frame.StateBuffer );
    DenOfIz_BindGroup_EndUpdate( frame.Constants );
}

void UIRenderer::EnsureStateCapacity( const uint32_t frameIndex, const uint32_t recordCount, const uint32_t keepRecords )
{
    FrameData &frame = m_frames[ frameIndex ];
    if ( recordCount <= frame.StateCapacity )
    {
        return;
    }
    const uint32_t     capacity = GrowCapacity( frame.StateCapacity, recordCount, MinStateCapacity );
    DenOfIz_BufferDesc desc{ };
    desc.NumBytes                  = static_cast<uint64_t>( capacity ) * sizeof( UIStateRecord );
    desc.HeapType                  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    desc.StructureDesc.NumElements = capacity;
    desc.StructureDesc.Stride      = sizeof( UIStateRecord );
    desc.DebugName                 = DENOFIZ_STRING( "UI States" );
    DenOfIz_Buffer buffer          = DENOFIZ_NULL_HANDLE;
    UIStateRecord *data            = nullptr;
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &desc, &buffer );
    DenOfIz_Buffer_MapMemory( buffer, reinterpret_cast<void **>( &data ) );
    if ( DENOFIZ_HANDLE_IS_VALID( frame.StateBuffer ) )
    {
        std::memcpy( data, frame.StateData, static_cast<size_t>( keepRecords ) * sizeof( UIStateRecord ) );
        DenOfIz_Buffer_UnmapMemory( frame.StateBuffer );
        DenOfIz_Buffer_Destroy( frame.StateBuffer );
    }
    data[ 0 ]           = UIStateRecord{ { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } };
    frame.StateBuffer   = buffer;
    frame.StateData     = data;
    frame.StateCapacity = capacity;
    BindFrameConstants( frameIndex );
}

uint32_t UIRenderer::AllocateRecord( )
{
    if ( m_recordCount >= m_target->StateCapacity )
    {
        EnsureStateCapacity( m_targetIndex, m_recordCount + 1, m_recordCount );
    }
    return m_recordCount++;
}

void UIRenderer::PushState( const DrawState &state )
{
    DrawState next = state;
    next.Record    = 0;
    if ( next.ClipCount > 0 || !UIMath::IsIdentityLayer( next.Layer ) )
    {
        next.Record = AllocateRecord( );
        const UILayerCommand &layer = next.Layer;
        m_target->StateData[ next.Record ] =
            UIStateRecord{ { layer.BiasR, layer.BiasG, layer.BiasB, layer.Scale },
                           { layer.Opacity, static_cast<float>( next.ClipCount ), 0.0f, 0.0f },
                           { static_cast<float>( next.Clips[ 0 ] ), static_cast<float>( next.Clips[ 1 ] ), static_cast<float>( next.Clips[ 2 ] ), static_cast<float>( next.Clips[ 3 ] ) } };
    }
    m_stateStack.push_back( next );
    m_stateBits = next.Record << UITextureSlots::Bits;
}

UIVertex *UIRenderer::AllocateVertices( const uint32_t count )
{
    if ( m_vertexCount + count > m_target->VertexCapacity )
    {
        EnsureGeometryCapacity( *m_target, m_vertexCount + count, 0, m_vertexCount, m_indexCount );
    }
    UIVertex *vertices = reinterpret_cast<UIVertex *>( m_target->VertexData ) + m_vertexCount;
    m_vertexCount += count;
    return vertices;
}

uint32_t *UIRenderer::AllocateIndices( const uint32_t count )
{
    if ( m_indexCount + count > m_target->IndexCapacity )
    {
        EnsureGeometryCapacity( *m_target, 0, m_indexCount + count, m_vertexCount, m_indexCount );
    }
    uint32_t *indices = reinterpret_cast<uint32_t *>( m_target->IndexData ) + m_indexCount;
    m_indexCount += count;
    m_batches.back( ).IndexCount += count;
    return indices;
}

void UIRenderer::EnsureColorTarget( FrameData &frame, const uint32_t width, const uint32_t height ) const
{
    if ( DENOFIZ_HANDLE_IS_VALID( frame.ColorTarget ) && frame.TargetWidth == width && frame.TargetHeight == height )
    {
        return;
    }
    if ( DENOFIZ_HANDLE_IS_VALID( frame.ColorTarget ) )
    {
        DenOfIz_ResourceTracking_UntrackTexture( m_resourceTracking, frame.ColorTarget );
        DenOfIz_Texture_Destroy( frame.ColorTarget );
    }
    DenOfIz_TextureDesc desc{ };
    desc.Width     = width;
    desc.Height    = height;
    desc.Depth     = 1;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format    = m_desc.RenderTargetFormat;
    desc.Usage     = DENOFIZ_TEXTURE_USAGE_COPY_SRC_BIT | DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT | DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT;
    desc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    desc.DebugName = DENOFIZ_STRING( "UI Color Target" );
    DenOfIz_LogicalDevice_CreateTexture( m_device, &desc, &frame.ColorTarget );
    DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, frame.ColorTarget, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    frame.TargetWidth  = width;
    frame.TargetHeight = height;
}

uint32_t UIRenderer::SlotOf( const DenOfIz_Texture texture )
{
    for ( uint32_t slot = UITextureSlots::FirstImage; slot < m_nextImageSlot; ++slot )
    {
        if ( m_slots[ slot ] == texture )
        {
            return slot;
        }
    }
    if ( m_nextImageSlot >= UITextureSlots::Count )
    {
        if ( !m_slotsExhaustedReported )
        {
            spdlog::error( "UI: more than {} distinct images in one frame, the rest are not drawn", UITextureSlots::Count - UITextureSlots::FirstImage );
            m_slotsExhaustedReported = true;
        }
        return UINT32_MAX;
    }
    m_slots[ m_nextImageSlot ] = texture;
    return m_nextImageSlot++;
}

DenOfIz_Float4 UIRenderer::ShadeColor( const DenOfIz_UIColor &color ) const
{
    constexpr float inv = 1.0f / 255.0f;
    return DenOfIz_Float4{ color.R * inv, color.G * inv, color.B * inv, color.A * inv };
}

void UIRenderer::SetScissor( const Scissor &scissor )
{
    Batch &batch = m_batches.back( );
    if ( batch.IndexCount == 0 )
    {
        batch.Clip       = scissor;
        batch.FirstIndex = m_indexCount;
        return;
    }
    Batch next;
    next.FirstIndex = m_indexCount;
    next.Clip       = scissor;
    next.Slot       = m_supportsSrvArray ? 0 : batch.Slot;
    m_batches.push_back( next );
}

void UIRenderer::StartBatch( const uint32_t slot )
{
    if ( m_supportsSrvArray || slot == 0 )
    {
        return;
    }
    Batch &batch = m_batches.back( );
    if ( batch.Slot == slot )
    {
        return;
    }
    if ( batch.Slot == 0 || batch.IndexCount == 0 )
    {
        batch.Slot = slot;
        return;
    }
    Batch next;
    next.FirstIndex = m_indexCount;
    next.Clip       = batch.Clip;
    next.Slot       = slot;
    m_batches.push_back( next );
}

void UIRenderer::EmitQuad( const float x0, const float y0, const float x1, const float y1, const float u0, const float v0, const float u1, const float v1,
                           const DenOfIz_UIColor &color, const uint32_t slot )
{
    StartBatch( slot );
    const DenOfIz_Float4 c       = ShadeColor( color );
    const uint32_t       index   = ( m_supportsSrvArray ? slot : ( slot == 0 ? 0 : slot < UITextureSlots::FirstImage ? 1 : 2 ) ) | m_stateBits;
    const uint32_t       base     = m_vertexCount;
    UIVertex            *vertices = AllocateVertices( 4 );
    vertices[ 0 ]                 = UIVertex{ { x0, y0, 0.0f }, { u0, v0 }, c, index };
    vertices[ 1 ]                 = UIVertex{ { x1, y0, 0.0f }, { u1, v0 }, c, index };
    vertices[ 2 ]                 = UIVertex{ { x1, y1, 0.0f }, { u1, v1 }, c, index };
    vertices[ 3 ]                 = UIVertex{ { x0, y1, 0.0f }, { u0, v1 }, c, index };
    uint32_t *indices             = AllocateIndices( 6 );
    indices[ 0 ]                  = base;
    indices[ 1 ]                  = base + 1;
    indices[ 2 ]                  = base + 2;
    indices[ 3 ]                  = base;
    indices[ 4 ]                  = base + 2;
    indices[ 5 ]                  = base + 3;
}

void UIRenderer::EmitShape( const UIShape &shape, const DenOfIz_UIRect &rect, const DenOfIz_UIColor &color, const uint32_t slot, const DenOfIz_Float4 *uv )
{
    StartBatch( slot );
    const uint32_t       index    = ( m_supportsSrvArray ? slot : ( slot == 0 ? 0 : slot < UITextureSlots::FirstImage ? 1 : 2 ) ) | m_stateBits;
    const uint32_t       base     = m_vertexCount;
    const float          invW     = rect.Width > 0.0f ? 1.0f / rect.Width : 0.0f;
    const float          invH     = rect.Height > 0.0f ? 1.0f / rect.Height : 0.0f;
    const DenOfIz_Float4 full     = ShadeColor( color );
    const auto           count    = static_cast<uint32_t>( shape.Vertices.size( ) );
    UIVertex            *vertices = AllocateVertices( count );
    for ( uint32_t i = 0; i < count; ++i )
    {
        const UIShapeVertex &v      = shape.Vertices[ i ];
        UIVertex            &vertex = vertices[ i ];
        vertex.Position             = DenOfIz_Float3{ rect.X + v.X, rect.Y + v.Y, 0.0f };
        vertex.Color                = DenOfIz_Float4{ full.X, full.Y, full.Z, full.W * v.Coverage };
        vertex.TextureIndex         = index;
        if ( uv )
        {
            const float u   = std::clamp( v.X * invW, 0.0f, 1.0f );
            const float t   = std::clamp( v.Y * invH, 0.0f, 1.0f );
            vertex.TexCoord = DenOfIz_Float2{ uv->X + ( uv->Z - uv->X ) * u, uv->Y + ( uv->W - uv->Y ) * t };
        }
        else
        {
            vertex.TexCoord = DenOfIz_Float2{ 0.0f, 0.0f };
        }
    }
    const auto indexCount = static_cast<uint32_t>( shape.Indices.size( ) );
    uint32_t  *indices    = AllocateIndices( indexCount );
    for ( uint32_t i = 0; i < indexCount; ++i )
    {
        indices[ i ] = base + shape.Indices[ i ];
    }
}

void UIRenderer::EmitText( const UICommand &command )
{
    const UITextLayout &layout = *command.Text.Layout;
    const auto         &quads  = layout.GetQuads( );
    if ( quads.empty( ) )
    {
        return;
    }
    const uint32_t slot = layout.TextureSlot;
    if ( !DENOFIZ_HANDLE_IS_VALID( m_slots[ slot ] ) )
    {
        return;
    }
    StartBatch( slot );
    const float          s          = command.Text.Scale;
    const float          lineHeight = layout.LineHeight * s;
    const DenOfIz_Float4 color      = ShadeColor( command.Text.Color );
    const uint32_t       index      = ( m_supportsSrvArray ? slot : 1 ) | m_stateBits;
    const float          clipTop    = m_clipPoints.Y;
    const float          clipBottom = m_clipPoints.Y + m_clipPoints.Height;

    uint32_t line    = UINT32_MAX;
    float    lineX   = 0.0f;
    float    lineY   = 0.0f;
    bool     visible = true;
    for ( const UIGlyphQuad &quad : quads )
    {
        if ( quad.Line != line )
        {
            line              = quad.Line;
            const float top   = command.Rect.Y + static_cast<float>( line ) * lineHeight;
            if ( top > clipBottom )
            {
                break;
            }
            visible = top + lineHeight >= clipTop;
            lineX   = command.Rect.X + layout.AlignOffset( line, command.Text.BoxWidth, command.Text.Align ) * s;
            lineY   = std::round( ( top + layout.Baseline * s ) * m_pixelsPerPoint ) / m_pixelsPerPoint;
        }
        if ( !visible )
        {
            continue;
        }
        const float x0   = lineX + quad.X0 * s;
        const float x1   = lineX + quad.X1 * s;
        const float y0   = lineY + quad.Y0 * s;
        const float y1   = lineY + quad.Y1 * s;
        const uint32_t base     = m_vertexCount;
        UIVertex      *vertices = AllocateVertices( 4 );
        vertices[ 0 ]           = UIVertex{ { x0, y0, 0.0f }, { quad.U0, quad.V0 }, color, index };
        vertices[ 1 ]           = UIVertex{ { x1, y0, 0.0f }, { quad.U1, quad.V0 }, color, index };
        vertices[ 2 ]           = UIVertex{ { x1, y1, 0.0f }, { quad.U1, quad.V1 }, color, index };
        vertices[ 3 ]           = UIVertex{ { x0, y1, 0.0f }, { quad.U0, quad.V1 }, color, index };
        uint32_t *indices       = AllocateIndices( 6 );
        indices[ 0 ]            = base;
        indices[ 1 ]            = base + 1;
        indices[ 2 ]            = base + 2;
        indices[ 3 ]            = base;
        indices[ 4 ]            = base + 2;
        indices[ 5 ]            = base + 3;
    }
}

void UIRenderer::Build( const std::vector<UICommand> &commands, const uint32_t frameIndex, const float pixelWidth, const float pixelHeight, const float pixelsPerPoint )
{
    ++m_frameCounter;
    m_pixelsPerPoint = pixelsPerPoint;
    m_target         = &m_frames[ frameIndex ];
    m_targetIndex    = frameIndex;
    m_vertexCount    = 0;
    m_indexCount     = 0;
    m_recordCount    = 1;
    m_stateBits      = 0;
    m_batches.clear( );
    m_clipStack.clear( );
    m_stateStack.clear( );
    m_stateStack.push_back( DrawState{ } );
    m_slots.fill( DENOFIZ_NULL_HANDLE );
    for ( uint32_t font = 0; font < UITextureSlots::MaxFonts; ++font )
    {
        m_slots[ UITextureSlots::FirstFont + font ] = m_desc.Text->GetAtlas( font );
    }
    m_nextImageSlot = UITextureSlots::FirstImage;

    const Scissor viewport{ 0, 0, static_cast<int32_t>( pixelWidth ), static_cast<int32_t>( pixelHeight ) };
    m_clipStack.push_back( viewport );
    m_clipPoints = DenOfIz_UIRect{ 0.0f, 0.0f, pixelWidth / pixelsPerPoint, pixelHeight / pixelsPerPoint };
    m_batches.push_back( Batch{ 0, 0, viewport, 0 } );

    const float feather = 1.0f / pixelsPerPoint;
    for ( const UICommand &command : commands )
    {
        switch ( command.Type )
        {
        case UICommandType::PushClip:
        {
            const Scissor &parent = m_clipStack.back( );
            const auto     left   = static_cast<float>( parent.X );
            const auto     top    = static_cast<float>( parent.Y );
            const auto     right  = static_cast<float>( parent.X + parent.Width );
            const auto     bottom = static_cast<float>( parent.Y + parent.Height );
            const auto     x0     = static_cast<int32_t>( std::clamp( std::floor( command.Rect.X * pixelsPerPoint ), left, right ) );
            const auto     y0     = static_cast<int32_t>( std::clamp( std::floor( command.Rect.Y * pixelsPerPoint ), top, bottom ) );
            const auto     x1     = static_cast<int32_t>( std::clamp( std::ceil( ( command.Rect.X + command.Rect.Width ) * pixelsPerPoint ), left, right ) );
            const auto     y1     = static_cast<int32_t>( std::clamp( std::ceil( ( command.Rect.Y + command.Rect.Height ) * pixelsPerPoint ), top, bottom ) );
            const Scissor  clip{ x0, y0, std::max( 0, x1 - x0 ), std::max( 0, y1 - y0 ) };
            m_clipStack.push_back( clip );
            m_clipPoints = DenOfIz_UIRect{ clip.X / pixelsPerPoint, clip.Y / pixelsPerPoint, clip.Width / pixelsPerPoint, clip.Height / pixelsPerPoint };
            SetScissor( clip );
            if ( !command.Clip.Rounded )
            {
                const DrawState current = m_stateStack.back( );
                m_stateStack.push_back( current );
                break;
            }
            const uint32_t          record = AllocateRecord( );
            const DenOfIz_UICorners radius = UIMath::ScaleCorners( command.Clip.Radius, pixelsPerPoint );
            m_target->StateData[ record ]  = UIStateRecord{ { command.Rect.X * pixelsPerPoint, command.Rect.Y * pixelsPerPoint, ( command.Rect.X + command.Rect.Width ) * pixelsPerPoint,
                                                              ( command.Rect.Y + command.Rect.Height ) * pixelsPerPoint },
                                                            { radius.TopLeft, radius.TopRight, radius.BottomRight, radius.BottomLeft },
                                                            { 0.0f, 0.0f, 0.0f, 0.0f } };
            DrawState next = m_stateStack.back( );
            for ( uint32_t i = std::min( next.ClipCount, UIMaxRoundedClips - 1 ); i > 0; --i )
            {
                next.Clips[ i ] = next.Clips[ i - 1 ];
            }
            next.Clips[ 0 ] = record;
            next.ClipCount  = std::min( next.ClipCount + 1, UIMaxRoundedClips );
            PushState( next );
            break;
        }
        case UICommandType::PopClip:
        {
            if ( m_clipStack.size( ) > 1 )
            {
                m_clipStack.pop_back( );
            }
            if ( m_stateStack.size( ) > 1 )
            {
                m_stateStack.pop_back( );
            }
            m_stateBits         = m_stateStack.back( ).Record << UITextureSlots::Bits;
            const Scissor &clip = m_clipStack.back( );
            m_clipPoints        = DenOfIz_UIRect{ clip.X / pixelsPerPoint, clip.Y / pixelsPerPoint, clip.Width / pixelsPerPoint, clip.Height / pixelsPerPoint };
            SetScissor( clip );
            break;
        }
        case UICommandType::PushLayer:
        {
            DrawState next = m_stateStack.back( );
            next.Layer     = UIMath::ComposeLayer( next.Layer, command.Layer );
            PushState( next );
            break;
        }
        case UICommandType::PopLayer:
            if ( m_stateStack.size( ) > 1 )
            {
                m_stateStack.pop_back( );
            }
            m_stateBits = m_stateStack.back( ).Record << UITextureSlots::Bits;
            break;
        case UICommandType::Fill:
            if ( !UIMath::Intersects( command.Rect, m_clipPoints ) )
            {
                break;
            }
            if ( UIMath::HasCorners( command.Fill.Radius ) )
            {
                EmitShape( m_shapes.GetFill( command.Rect.Width, command.Rect.Height, command.Fill.Radius, feather, m_frameCounter ), command.Rect, command.Fill.Color, 0, nullptr );
            }
            else
            {
                EmitQuad( command.Rect.X, command.Rect.Y, command.Rect.X + command.Rect.Width, command.Rect.Y + command.Rect.Height, 0.0f, 0.0f, 0.0f, 0.0f, command.Fill.Color, 0 );
            }
            break;
        case UICommandType::Border:
            if ( UIMath::Intersects( command.Rect, m_clipPoints ) )
            {
                EmitShape( m_shapes.GetBorder( command.Rect.Width, command.Rect.Height, command.Border.Widths, command.Border.Radius, feather, m_frameCounter ), command.Rect,
                           command.Border.Color, 0, nullptr );
            }
            break;
        case UICommandType::Image:
        {
            if ( !UIMath::Intersects( command.Rect, m_clipPoints ) )
            {
                break;
            }
            const uint32_t slot = SlotOf( command.Image.Texture );
            if ( slot == UINT32_MAX )
            {
                break;
            }
            const DenOfIz_Float4 &uv = command.Image.UV;
            if ( UIMath::HasCorners( command.Image.Radius ) )
            {
                EmitShape( m_shapes.GetFill( command.Rect.Width, command.Rect.Height, command.Image.Radius, feather, m_frameCounter ), command.Rect, command.Image.Tint, slot, &uv );
            }
            else
            {
                EmitQuad( command.Rect.X, command.Rect.Y, command.Rect.X + command.Rect.Width, command.Rect.Y + command.Rect.Height, uv.X, uv.Y, uv.Z, uv.W, command.Image.Tint, slot );
            }
            break;
        }
        case UICommandType::Text:
            EmitText( command );
            break;
        }
    }

    if ( m_frameCounter % 30 == 0 )
    {
        m_shapes.Evict( m_frameCounter, ShapeCacheAge );
    }
}

void UIRenderer::Upload( const uint32_t frameIndex, const float pixelWidth, const float pixelHeight )
{
    FrameData &frame = m_frames[ frameIndex ];
    const float    width  = pixelWidth / m_pixelsPerPoint;
    const float    height = pixelHeight / m_pixelsPerPoint;
    const XMMATRIX ortho  = XMMatrixOrthographicOffCenterLH( 0.0f, width, height, 0.0f, 0.0f, 1.0f );
    XMFLOAT4X4     projection;
    XMStoreFloat4x4( &projection, ortho );

    Uniforms uniforms{ };
    uniforms.Projection = InteropMathConverter::Float_4x4FromXMFLOAT4X4( projection );
    uniforms.ScreenSize = DenOfIz_Float4{ pixelWidth, pixelHeight, m_pixelsPerPoint, 0.0f };
    const UIFontFace *face = m_desc.Text->GetFace( 0 );
    uniforms.FontParams     = DenOfIz_Float4{ face->AtlasWidth, face->AtlasHeight, DENOFIZ_FONT_MSDF_PIXEL_RANGE, static_cast<float>( UITextureSlots::FirstImage ) };
    std::memcpy( m_uniformData + frameIndex * m_alignedUniforms, &uniforms, sizeof( Uniforms ) );

    if ( m_supportsSrvArray )
    {
        if ( !frame.BoundValid || frame.Bound != m_slots )
        {
            std::array<DenOfIz_Texture, UITextureSlots::Count> textures{ };
            for ( uint32_t i = 0; i < UITextureSlots::Count; ++i )
            {
                textures[ i ] = DENOFIZ_HANDLE_IS_VALID( m_slots[ i ] ) ? m_slots[ i ] : m_nullTexture;
            }
            DenOfIz_TextureArray array{ };
            array.Elements    = textures.data( );
            array.NumElements = UITextureSlots::Count;
            DenOfIz_BindGroup_BeginUpdate( frame.Textures );
            DenOfIz_BindGroup_SrvArray( frame.Textures, 0, &array );
            DenOfIz_BindGroup_Sampler( frame.Textures, 0, m_sampler );
            DenOfIz_BindGroup_EndUpdate( frame.Textures );
            frame.Bound      = m_slots;
            frame.BoundValid = true;
        }
        return;
    }

    for ( const Batch &batch : m_batches )
    {
        const uint32_t        slot    = batch.Slot;
        const DenOfIz_Texture texture = DENOFIZ_HANDLE_IS_VALID( m_slots[ slot ] ) ? m_slots[ slot ] : m_nullTexture;
        if ( !DENOFIZ_HANDLE_IS_VALID( frame.SlotGroups[ slot ] ) )
        {
            DenOfIz_BindGroupDesc groupDesc{ };
            groupDesc.Layout = m_bindGroupLayouts[ 0 ];
            DenOfIz_LogicalDevice_CreateBindGroup( m_device, &groupDesc, &frame.SlotGroups[ slot ] );
            frame.SlotTextures[ slot ] = DENOFIZ_NULL_HANDLE;
        }
        if ( frame.SlotTextures[ slot ] != texture )
        {
            DenOfIz_BindGroup_BeginUpdate( frame.SlotGroups[ slot ] );
            DenOfIz_BindGroup_SrvTexture( frame.SlotGroups[ slot ], 0, texture );
            DenOfIz_BindGroup_Sampler( frame.SlotGroups[ slot ], 0, m_sampler );
            DenOfIz_BindGroup_EndUpdate( frame.SlotGroups[ slot ] );
            frame.SlotTextures[ slot ] = texture;
        }
    }
}

void UIRenderer::Record( const uint32_t frameIndex, const DenOfIz_CommandList commandList, const DenOfIz_Pipeline pipeline, const float pixelWidth, const float pixelHeight )
{
    if ( m_indexCount == 0 )
    {
        return;
    }
    const FrameData &frame = m_frames[ frameIndex ];
    DenOfIz_CommandList_BindViewport( commandList, 0.0f, 0.0f, pixelWidth, pixelHeight );
    DenOfIz_CommandList_BindPipeline( commandList, pipeline );
    DenOfIz_CommandList_BindGroup( commandList, frame.Constants );
    if ( m_supportsSrvArray )
    {
        DenOfIz_CommandList_BindGroup( commandList, frame.Textures );
    }
    DenOfIz_CommandList_BindVertexBuffer( commandList, frame.VertexBuffer, 0, sizeof( UIVertex ), 0 );
    DenOfIz_CommandList_BindIndexBuffer( commandList, frame.IndexBuffer, DENOFIZ_INDEX_TYPE_UINT32, 0 );

    uint32_t boundSlot = UINT32_MAX;
    for ( const Batch &batch : m_batches )
    {
        if ( batch.IndexCount == 0 || batch.Clip.Width <= 0 || batch.Clip.Height <= 0 )
        {
            continue;
        }
        DenOfIz_CommandList_BindScissorRect( commandList, static_cast<float>( batch.Clip.X ), static_cast<float>( batch.Clip.Y ), static_cast<float>( batch.Clip.Width ),
                                             static_cast<float>( batch.Clip.Height ) );
        if ( !m_supportsSrvArray && batch.Slot != boundSlot )
        {
            DenOfIz_CommandList_BindGroup( commandList, frame.SlotGroups[ batch.Slot ] );
            boundSlot = batch.Slot;
        }
        DenOfIz_CommandList_DrawIndexed( commandList, batch.IndexCount, 1, batch.FirstIndex, 0, 0 );
    }
}

void UIRenderer::Render( const std::vector<UICommand> &commands, const uint32_t frameIndex, const DenOfIz_CommandList commandList, const float pixelWidth, const float pixelHeight,
                         const float pixelsPerPoint )
{
    if ( frameIndex >= m_frames.size( ) )
    {
        spdlog::error( "UI: frame index {} is out of range, NumFrames is {}", frameIndex, m_frames.size( ) );
        return;
    }
    Build( commands, frameIndex, pixelWidth, pixelHeight, pixelsPerPoint );
    Upload( frameIndex, pixelWidth, pixelHeight );
    Record( frameIndex, commandList, m_pipeline, pixelWidth, pixelHeight );
}

void UIRenderer::RenderToTexture( const std::vector<UICommand> &commands, const uint32_t frameIndex, const uint32_t pixelWidth, const uint32_t pixelHeight, const float pixelsPerPoint,
                                  const DenOfIz_SemaphoreArray *waitSemaphores, DenOfIz_UIRenderResult *outResult )
{
    if ( frameIndex >= m_frames.size( ) )
    {
        spdlog::error( "UI: frame index {} is out of range, NumFrames is {}", frameIndex, m_frames.size( ) );
        return;
    }
    FrameData &frame = m_frames[ frameIndex ];
    if ( frame.Submitted )
    {
        DenOfIz_Fence_Wait( frame.Fence );
    }
    EnsureColorTarget( frame, std::max( 1u, pixelWidth ), std::max( 1u, pixelHeight ) );

    const auto width  = static_cast<float>( frame.TargetWidth );
    const auto height = static_cast<float>( frame.TargetHeight );
    Build( commands, frameIndex, width, height, pixelsPerPoint );
    Upload( frameIndex, width, height );

    const DenOfIz_CommandList commandList = frame.CommandList;
    DenOfIz_CommandList_Begin( commandList );
    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, frame.ColorTarget, DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_RenderingAttachmentDesc colorAttachment{ };
    colorAttachment.Resource   = frame.ColorTarget;
    colorAttachment.LoadOp     = DENOFIZ_LOAD_OP_CLEAR;
    colorAttachment.StoreOp    = DENOFIZ_STORE_OP_STORE;
    colorAttachment.ClearColor = DenOfIz_Float4{ 0.0f, 0.0f, 0.0f, 0.0f };

    DenOfIz_RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &colorAttachment;
    renderingDesc.RTAttachments.NumElements = 1;
    renderingDesc.NumLayers                 = 1;
    renderingDesc.RenderAreaWidth           = width;
    renderingDesc.RenderAreaHeight          = height;
    DenOfIz_CommandList_BeginRendering( commandList, &renderingDesc );
    DenOfIz_CommandList_BindScissorRect( commandList, 0.0f, 0.0f, width, height );
    Record( frameIndex, commandList, m_texturePipeline, width, height );
    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, frame.ColorTarget, DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    DenOfIz_CommandList_End( commandList );

    DenOfIz_ExecuteCommandListsDesc executeDesc{ };
    executeDesc.CommandLists.Elements        = &frame.CommandList;
    executeDesc.CommandLists.NumElements     = 1;
    executeDesc.Signal                       = frame.Fence;
    executeDesc.SignalSemaphores.Elements    = &frame.Semaphore;
    executeDesc.SignalSemaphores.NumElements = 1;
    if ( waitSemaphores )
    {
        executeDesc.WaitSemaphores = *waitSemaphores;
    }
    DenOfIz_CommandQueue_ExecuteCommandLists( m_queue, &executeDesc );
    frame.Submitted = true;

    if ( outResult )
    {
        outResult->Texture   = frame.ColorTarget;
        outResult->Semaphore = frame.Semaphore;
    }
}
