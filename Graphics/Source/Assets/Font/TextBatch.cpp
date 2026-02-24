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

#include "DenOfIzGraphicsInternal/Assets/Font/TextBatch.h"
#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h"
#include "DenOfIzGraphics/Support/ResourceTracking.h"
#include "DenOfIzGraphicsInternal/Assets/Font/TextLayoutCache.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

#include <memory>
#include <vector>

using namespace DenOfIz;

class TextBatch::Impl
{
public:
    struct FontShaderUniforms
    {
        DenOfIz_Float4x4 Projection;
        DenOfIz_Float4   TextColor;
        DenOfIz_Float4   TextureSizeParams;
    };

    TextBatchDesc         Desc;
    DenOfIz_LogicalDevice LogicalDevice = DENOFIZ_NULL_HANDLE;
    class Font           *Font          = nullptr;

    uint32_t                 MaxVertices        = 4096;
    uint32_t                 MaxIndices         = 4096;
    uint32_t                 CurrentVertexCount = 0;
    uint32_t                 CurrentIndexCount  = 0;
    std::vector<GlyphVertex> GlyphVertices;
    std::vector<uint32_t>    IndexData;

    DenOfIz_BufferDesc VertexBufferDesc{ };
    DenOfIz_BufferDesc IndexBufferDesc{ };

    DenOfIz_BindGroup BindGroup                = DENOFIZ_NULL_HANDLE;
    DenOfIz_Texture   Atlas                    = DENOFIZ_NULL_HANDLE;
    DenOfIz_Buffer    VertexBuffer             = DENOFIZ_NULL_HANDLE;
    Byte             *VertexBufferMappedMemory = nullptr;
    DenOfIz_Buffer    IndexBuffer              = DENOFIZ_NULL_HANDLE;
    Byte             *IndexBufferMappedMemory  = nullptr;

    DenOfIz_Buffer      UniformBuffer     = DENOFIZ_NULL_HANDLE;
    FontShaderUniforms *UniformBufferData = nullptr;

    std::vector<std::unique_ptr<TextLayout>> TextLayouts;
    uint32_t                                 CurrentTextLayoutIndex = 0;
    DenOfIz_Float4x4                         ProjectionMatrix{ };

    mutable std::vector<std::unique_ptr<TextLayout>> MeasureTextLayouts;
    mutable uint32_t                                 CurrentMeasureLayoutIndex = 0;

    DenOfIz_ResourceTracking ResourceTracking = DENOFIZ_NULL_HANDLE;
    DenOfIz_Sampler          FontSampler      = DENOFIZ_NULL_HANDLE;

    std::unique_ptr<TextLayoutCache> TextLayoutCachePtr;

    void UpdateBuffers( );
    void InitializeAtlas( );
};

TextBatch::TextBatch( const TextBatchDesc &desc ) : m_impl( std::make_unique<Impl>( ) )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( desc.LogicalDevice ) )
    {
        spdlog::error( "TextBatch::TextBatch LogicalDevice cannot be null" );
        return;
    }
    if ( desc.Font == nullptr )
    {
        spdlog::error( "TextBatch::TextBatch Font cannot be null" );
        return;
    }
    m_impl->Desc          = desc;
    m_impl->Font          = desc.Font;
    m_impl->LogicalDevice = desc.LogicalDevice;

    DenOfIz_ResourceTracking_Create( &m_impl->ResourceTracking );

    m_impl->VertexBufferDesc           = { };
    m_impl->VertexBufferDesc.NumBytes  = m_impl->MaxVertices * sizeof( GlyphVertex );
    m_impl->VertexBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_VERTEX_BIT;
    m_impl->VertexBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    m_impl->VertexBufferDesc.DebugName = DENOFIZ_STRING( "Font Vertex Buffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_impl->LogicalDevice, &m_impl->VertexBufferDesc, &m_impl->VertexBuffer );
    void *vertexMappedMemory = nullptr;
    DenOfIz_Buffer_MapMemory( m_impl->VertexBuffer, &vertexMappedMemory );
    m_impl->VertexBufferMappedMemory = static_cast<Byte *>( vertexMappedMemory );

    m_impl->IndexBufferDesc           = { };
    m_impl->IndexBufferDesc.NumBytes  = m_impl->MaxIndices * sizeof( uint32_t );
    m_impl->IndexBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_INDEX_BIT;
    m_impl->IndexBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    m_impl->IndexBufferDesc.DebugName = DENOFIZ_STRING( "Font Index Buffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_impl->LogicalDevice, &m_impl->IndexBufferDesc, &m_impl->IndexBuffer );
    void *indexMappedMemory = nullptr;
    DenOfIz_Buffer_MapMemory( m_impl->IndexBuffer, &indexMappedMemory );
    m_impl->IndexBufferMappedMemory = static_cast<Byte *>( indexMappedMemory );

    DenOfIz_BufferDesc uniformBufferDesc{ };
    uniformBufferDesc.NumBytes                  = 3 * sizeof( Impl::FontShaderUniforms );
    uniformBufferDesc.Usage                     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    uniformBufferDesc.HeapType                  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    uniformBufferDesc.DebugName                 = DENOFIZ_STRING( "Font Uniform Buffer" );
    uniformBufferDesc.StructureDesc.NumElements = 1;
    uniformBufferDesc.StructureDesc.Stride      = sizeof( Impl::FontShaderUniforms );
    DenOfIz_LogicalDevice_CreateBuffer( m_impl->LogicalDevice, &uniformBufferDesc, &m_impl->UniformBuffer );
    void *uniformMappedMemory = nullptr;
    DenOfIz_Buffer_MapMemory( m_impl->UniformBuffer, &uniformMappedMemory );
    m_impl->UniformBufferData = static_cast<Impl::FontShaderUniforms *>( uniformMappedMemory );

    DenOfIz_SamplerDesc samplerDesc{ };
    samplerDesc.MagFilter     = DENOFIZ_FILTER_LINEAR;
    samplerDesc.MinFilter     = DENOFIZ_FILTER_LINEAR;
    samplerDesc.AddressModeU  = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.AddressModeV  = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.AddressModeW  = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.MipLodBias    = 0.0f;
    samplerDesc.MaxAnisotropy = 0.0f;
    samplerDesc.MinLod        = 0.0f;
    samplerDesc.MaxLod        = 0.0f;
    samplerDesc.MipmapMode    = DENOFIZ_MIPMAP_MODE_LINEAR;
    samplerDesc.DebugName     = DENOFIZ_STRING( "MSDF Font Sampler" );
    DenOfIz_LogicalDevice_CreateSampler( m_impl->LogicalDevice, &samplerDesc, &m_impl->FontSampler );

    DenOfIz_BindGroupDesc bindGroupDesc{ };
    bindGroupDesc.Layout = m_impl->Desc.BindGroupLayout;

    DenOfIz_LogicalDevice_CreateBindGroup( m_impl->LogicalDevice, &bindGroupDesc, &m_impl->BindGroup );
    m_impl->TextLayoutCachePtr = std::make_unique<TextLayoutCache>( );
    m_impl->InitializeAtlas( );
}

TextBatch::~TextBatch( )
{
    if ( m_impl->UniformBufferData && DENOFIZ_HANDLE_IS_VALID( m_impl->UniformBuffer ) )
    {
        DenOfIz_Buffer_UnmapMemory( m_impl->UniformBuffer );
    }
    if ( m_impl->VertexBufferMappedMemory && DENOFIZ_HANDLE_IS_VALID( m_impl->VertexBuffer ) )
    {
        DenOfIz_Buffer_UnmapMemory( m_impl->VertexBuffer );
    }
    if ( m_impl->IndexBufferMappedMemory && DENOFIZ_HANDLE_IS_VALID( m_impl->IndexBuffer ) )
    {
        DenOfIz_Buffer_UnmapMemory( m_impl->IndexBuffer );
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_impl->UniformBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_impl->UniformBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_impl->VertexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_impl->VertexBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_impl->IndexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_impl->IndexBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_impl->Atlas ) )
    {
        DenOfIz_TextureResource_Destroy( m_impl->Atlas );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_impl->FontSampler ) )
    {
        DenOfIz_Sampler_Destroy( m_impl->FontSampler );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_impl->BindGroup ) )
    {
        DenOfIz_BindGroup_Destroy( m_impl->BindGroup );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_impl->ResourceTracking ) )
    {
        DenOfIz_ResourceTracking_Destroy( m_impl->ResourceTracking );
    }
}

void TextBatch::BeginBatch( ) const
{
    m_impl->GlyphVertices.clear( );
    m_impl->IndexData.clear( );
    m_impl->CurrentVertexCount     = 0;
    m_impl->CurrentIndexCount      = 0;
    m_impl->CurrentTextLayoutIndex = 0;
}

void TextBatch::AddText( const DenOfIz_AddTextDesc &desc ) const
{
    DenOfIz_AddTextDesc modifiedParams = desc;

    const float baseSize       = static_cast<float>( DenOfIz_FontAsset_InitialFontSize( m_impl->Font->Asset( ) ) );
    const float targetSize     = desc.FontSize > 0 ? desc.FontSize : baseSize;
    const auto  effectiveScale = targetSize / baseSize;

    const uint64_t    textHash   = TextLayoutCache::HashString( desc.Text );
    const TextLayout *textLayout = m_impl->TextLayoutCachePtr->GetOrCreate( textHash, 0, static_cast<uint32_t>( targetSize ), m_impl->Font, desc.Text );

    if ( desc.HorizontalCenter || desc.VerticalCenter )
    {
        if ( desc.HorizontalCenter )
        {
            modifiedParams.X -= textLayout->GetTextWidth( ) / 2.0f;
        }

        if ( desc.VerticalCenter )
        {
            const DenOfIz_FontMetrics metrics    = DenOfIz_FontAsset_GetMetrics( m_impl->Font->Asset( ) );
            const float               textHeight = ( metrics.Ascent + metrics.Descent ) * effectiveScale;
            modifiedParams.Y -= textHeight / 2.0f;
        }
    }

    const float adjustedY = modifiedParams.Y;

    const TextVertexAllocationInfo allocInfo              = textLayout->GetVertexAllocationInfo( );
    const size_t                   requiredVertexCapacity = m_impl->CurrentVertexCount + allocInfo.VertexCount;
    const size_t                   requiredIndexCapacity  = m_impl->CurrentIndexCount + allocInfo.IndexCount;

    if ( m_impl->GlyphVertices.capacity( ) < requiredVertexCapacity )
    {
        m_impl->GlyphVertices.reserve( requiredVertexCapacity + 1024 );
    }
    if ( m_impl->IndexData.capacity( ) < requiredIndexCapacity )
    {
        m_impl->IndexData.reserve( requiredIndexCapacity + 1024 );
    }
    m_impl->GlyphVertices.resize( requiredVertexCapacity );
    m_impl->IndexData.resize( requiredIndexCapacity );

    GenerateTextVerticesDesc generateDesc{ };
    generateDesc.StartPosition   = { modifiedParams.X, adjustedY };
    generateDesc.Color           = modifiedParams.Color;
    generateDesc.OutVertices     = m_impl->GlyphVertices.data( ) + m_impl->CurrentVertexCount;
    generateDesc.OutIndices      = m_impl->IndexData.data( ) + m_impl->CurrentIndexCount;
    generateDesc.BaseVertexIndex = m_impl->CurrentVertexCount;
    generateDesc.BaseIndexOffset = m_impl->CurrentIndexCount;
    generateDesc.Scale           = effectiveScale;
    generateDesc.LetterSpacing   = desc.LetterSpacing;
    generateDesc.LineHeight      = desc.LineHeight;

    textLayout->GenerateTextVertices( generateDesc );

    m_impl->CurrentVertexCount += allocInfo.VertexCount;
    m_impl->CurrentIndexCount += allocInfo.IndexCount;

    if ( m_impl->CurrentVertexCount > m_impl->MaxVertices || m_impl->CurrentIndexCount > m_impl->MaxIndices )
    {
        m_impl->MaxVertices = std::max( m_impl->MaxVertices * 2, m_impl->CurrentVertexCount );
        m_impl->MaxIndices  = std::max( m_impl->MaxIndices * 2, m_impl->CurrentIndexCount );
    }
}

void TextBatch::EndBatch( DenOfIz_CommandList commandList ) const
{
    if ( m_impl->CurrentVertexCount == 0 || m_impl->CurrentIndexCount == 0 || !m_impl->Font )
    {
        return;
    }

    m_impl->UpdateBuffers( );

    Impl::FontShaderUniforms *uniforms = m_impl->UniformBufferData;
    uniforms->Projection               = m_impl->ProjectionMatrix;
    uniforms->TextColor                = DenOfIz_Float4{ 1.0f, 1.0f, 1.0f, 1.0f };

    const DenOfIz_FontAsset fontAsset = m_impl->Font->Asset( );
    uniforms->TextureSizeParams = DenOfIz_Float4{ static_cast<float>( DenOfIz_FontAsset_AtlasWidth( fontAsset ) ), static_cast<float>( DenOfIz_FontAsset_AtlasHeight( fontAsset ) ),
                                                  DENOFIZ_FONT_MSDF_PIXEL_RANGE, static_cast<float>( DENOFIZ_ANTI_ALIASING_MODE_GRAYSCALE ) };

    DenOfIz_CommandList_BindGroup( commandList, m_impl->BindGroup );
    DenOfIz_CommandList_BindVertexBuffer( commandList, m_impl->VertexBuffer, 0, 0, 0 );
    DenOfIz_CommandList_BindIndexBuffer( commandList, m_impl->IndexBuffer, DENOFIZ_INDEX_TYPE_UINT32, 0 );
    DenOfIz_CommandList_DrawIndexed( commandList, m_impl->CurrentIndexCount, 1, 0, 0, 0 );
}

void TextBatch::SetProjectionMatrix( const DenOfIz_Float4x4 &projectionMatrix ) const
{
    m_impl->ProjectionMatrix = projectionMatrix;
}

DenOfIz_Float2 TextBatch::MeasureText( DenOfIz_StringView text, const DenOfIz_AddTextDesc &desc ) const
{
    const float baseSize       = static_cast<float>( DenOfIz_FontAsset_InitialFontSize( m_impl->Font->Asset( ) ) );
    const float targetSize     = desc.FontSize > 0 ? desc.FontSize : baseSize;
    const auto  effectiveScale = targetSize / baseSize;

    if ( !text.Chars || text.NumChars == 0 )
    {
        return DenOfIz_Float2{ 0.0f, 0.0f };
    }

    const size_t      length     = text.NumChars;
    const uint64_t    textHash   = TextLayoutCache::HashString( text );
    const TextLayout *textLayout = m_impl->TextLayoutCachePtr->GetOrCreate( textHash, 0, static_cast<uint32_t>( targetSize ), m_impl->Font, text );

    float textWidth  = textLayout->GetTextWidth( );
    float textHeight = textLayout->GetTextHeight( );
    if ( desc.LetterSpacing > 0 && length > 0 )
    {
        textWidth += static_cast<float>( desc.LetterSpacing ) * static_cast<float>( length - 1 );
    }

    if ( desc.LineHeight > 0 )
    {
        textHeight = static_cast<float>( desc.LineHeight );
    }
    else
    {
        const DenOfIz_FontMetrics metrics = DenOfIz_FontAsset_GetMetrics( m_impl->Font->Asset( ) );
        textHeight                        = static_cast<float>( metrics.Ascent + metrics.Descent ) * effectiveScale;
    }
    return DenOfIz_Float2{ textWidth, textHeight };
}

void TextBatch::Impl::UpdateBuffers( )
{
    if ( VertexBufferDesc.NumBytes < CurrentVertexCount * sizeof( GlyphVertex ) )
    {
        if ( VertexBufferMappedMemory && DENOFIZ_HANDLE_IS_VALID( VertexBuffer ) )
        {
            DenOfIz_Buffer_UnmapMemory( VertexBuffer );
            VertexBufferMappedMemory = nullptr;
        }

        if ( DENOFIZ_HANDLE_IS_VALID( VertexBuffer ) )
        {
            DenOfIz_Buffer_Destroy( VertexBuffer );
        }

        VertexBufferDesc.NumBytes                  = MaxVertices * sizeof( GlyphVertex );
        VertexBufferDesc.StructureDesc.NumElements = MaxVertices;
        DenOfIz_LogicalDevice_CreateBuffer( LogicalDevice, &VertexBufferDesc, &VertexBuffer );
        void *vertexMappedMemory = nullptr;
        DenOfIz_Buffer_MapMemory( VertexBuffer, &vertexMappedMemory );
        VertexBufferMappedMemory = static_cast<Byte *>( vertexMappedMemory );
    }

    if ( IndexBufferDesc.NumBytes < CurrentIndexCount * sizeof( uint32_t ) )
    {
        if ( IndexBufferMappedMemory && DENOFIZ_HANDLE_IS_VALID( IndexBuffer ) )
        {
            DenOfIz_Buffer_UnmapMemory( IndexBuffer );
            IndexBufferMappedMemory = nullptr;
        }

        if ( DENOFIZ_HANDLE_IS_VALID( IndexBuffer ) )
        {
            DenOfIz_Buffer_Destroy( IndexBuffer );
        }

        IndexBufferDesc.NumBytes = MaxIndices * sizeof( uint32_t );
        DenOfIz_LogicalDevice_CreateBuffer( LogicalDevice, &IndexBufferDesc, &IndexBuffer );
        void *indexMappedMemory = nullptr;
        DenOfIz_Buffer_MapMemory( IndexBuffer, &indexMappedMemory );
        IndexBufferMappedMemory = static_cast<Byte *>( indexMappedMemory );
    }

    memcpy( VertexBufferMappedMemory, GlyphVertices.data( ), CurrentVertexCount * sizeof( GlyphVertex ) );
    memcpy( IndexBufferMappedMemory, IndexData.data( ), CurrentIndexCount * sizeof( uint32_t ) );
}

void TextBatch::Impl::InitializeAtlas( )
{
    DenOfIz_CommandQueueDesc commandQueueDesc{ };
    commandQueueDesc.QueueType = DENOFIZ_QUEUE_TYPE_GRAPHICS;

    DenOfIz_CommandQueue commandQueue;
    DenOfIz_LogicalDevice_CreateCommandQueue( LogicalDevice, &commandQueueDesc, &commandQueue );

    DenOfIz_CommandListPoolDesc commandListPoolDesc{ };
    commandListPoolDesc.CommandQueue    = commandQueue;
    commandListPoolDesc.NumCommandLists = 1;

    DenOfIz_CommandListPool commandListPool;
    DenOfIz_LogicalDevice_CreateCommandListPool( LogicalDevice, &commandListPoolDesc, &commandListPool );
    DenOfIz_CommandListArray commandLists;
    DenOfIz_CommandListPool_GetCommandLists( commandListPool, &commandLists );
    DenOfIz_CommandList commandList = commandLists.Elements[ 0 ];
    DenOfIz_CommandList_Begin( commandList );

    const DenOfIz_FontAsset fontAsset = Desc.Font->Asset( );

    DenOfIz_PhysicalDevice deviceInfo;
    DenOfIz_LogicalDevice_DeviceInfo( LogicalDevice, &deviceInfo );
    const auto alignedPitch = Utilities::Align( DenOfIz_FontAsset_AtlasWidth( fontAsset ) * DENOFIZ_FONT_ASSET_NUM_CHANNELS, deviceInfo.Constants.BufferTextureRowAlignment );
    const auto alignedSlice = Utilities::Align( DenOfIz_FontAsset_AtlasHeight( fontAsset ), deviceInfo.Constants.BufferTextureAlignment );

    DenOfIz_BufferDesc stagingDesc{ };
    stagingDesc.NumBytes  = alignedPitch * alignedSlice;
    stagingDesc.Usage     = DENOFIZ_BUFFER_USAGE_COPY_SRC_BIT;
    stagingDesc.DebugName = DENOFIZ_STRING( "Font MSDF Atlas Staging Buffer" );
    stagingDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU;
    DenOfIz_Buffer fontAtlasStagingBuffer;
    DenOfIz_LogicalDevice_CreateBuffer( LogicalDevice, &stagingDesc, &fontAtlasStagingBuffer );

    DenOfIz_TextureDesc textureDesc{ };
    textureDesc.Width     = DenOfIz_FontAsset_AtlasWidth( fontAsset );
    textureDesc.Height    = DenOfIz_FontAsset_AtlasHeight( fontAsset );
    textureDesc.Depth     = 1;
    textureDesc.ArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Format    = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Usage     = DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT | DENOFIZ_TEXTURE_USAGE_COPY_DST_BIT;
    textureDesc.DebugName = DENOFIZ_STRING( "Font MTSDF Atlas Texture" );
    DenOfIz_LogicalDevice_CreateTexture( LogicalDevice, &textureDesc, &Atlas );
    DenOfIz_ResourceTracking_TrackTexture( ResourceTracking, Atlas, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_BindGroup_BeginUpdate( BindGroup );
    DenOfIz_BindGroup_Cbv( BindGroup, 0, UniformBuffer );
    DenOfIz_BindGroup_SrvTexture( BindGroup, 0, Atlas );
    DenOfIz_BindGroup_Sampler( BindGroup, 0, FontSampler );
    DenOfIz_BindGroup_EndUpdate( BindGroup );
    DenOfIz_ResourceTracking_TrackBuffer( ResourceTracking, fontAtlasStagingBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_LoadAtlasIntoGpuTextureDesc loadDesc{ };
    loadDesc.Device        = LogicalDevice;
    loadDesc.StagingBuffer = fontAtlasStagingBuffer;
    loadDesc.CommandList   = commandList;
    loadDesc.Texture       = Atlas;
    DenOfIz_FontAssetReader_LoadAtlasIntoGpuTexture( fontAsset, &loadDesc );

    DenOfIz_ResourceTracking_TransitionTexture( ResourceTracking, commandList, Atlas, DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_Format atlasFormat;
    DenOfIz_TextureResource_GetFormat( Atlas, &atlasFormat );

    DenOfIz_CopyBufferToTextureDesc copyDesc{ };
    copyDesc.SrcBuffer  = fontAtlasStagingBuffer;
    copyDesc.DstTexture = Atlas;
    copyDesc.RowPitch   = DenOfIz_FontAsset_AtlasWidth( fontAsset ) * 4;
    copyDesc.Format     = atlasFormat;

    DenOfIz_CommandList_CopyBufferToTexture( commandList, &copyDesc );

    DenOfIz_ResourceTracking_TransitionTexture( ResourceTracking, commandList, Atlas, DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( commandList );
    DenOfIz_ExecuteCommandListsDesc executeDesc{ };
    executeDesc.CommandLists.Elements    = &commandList;
    executeDesc.CommandLists.NumElements = 1;
    DenOfIz_CommandQueue_ExecuteCommandLists( commandQueue, &executeDesc );
    DenOfIz_CommandQueue_WaitIdle( commandQueue );

    DenOfIz_Buffer_Destroy( fontAtlasStagingBuffer );
    DenOfIz_CommandListPool_Destroy( commandListPool );
    DenOfIz_CommandQueue_Destroy( commandQueue );
}
