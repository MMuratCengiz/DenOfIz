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

#include "DenOfIzGraphicsInternal/UI/ClayTextCache.h"
#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h"
#include "DenOfIzGraphics/Backends/Interface/CommandList.h"
#include "DenOfIzGraphics/Backends/Interface/CommandListPool.h"
#include "DenOfIzGraphics/Backends/Interface/CommandQueue.h"
#include "DenOfIzGraphics/Support/ResourceTracking.h"
#include "DenOfIzGraphicsInternal/Assets/Font/EmbeddedFonts.h"
#include "DenOfIzGraphicsInternal/Assets/Font/FontLibrary.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

using namespace DenOfIz;

ClayTextCache::ClayTextCache( const ClayTextCacheDesc &desc ) : m_logicalDevice( desc.LogicalDevice ), m_maxTextures( desc.MaxTextures )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( m_logicalDevice ) )
    {
        spdlog::error( "ClayText: LogicalDevice cannot be null" );
        return;
    }

    m_textures.resize( m_maxTextures, DENOFIZ_NULL_HANDLE );
    m_textureFontFlags.resize( m_maxTextures, false );

    static FontLibrary defaultFontLibrary;
    static auto        defaultFont = defaultFontLibrary.LoadFont( { EmbeddedFonts::GetJetbrainsMono( ) } );
    AddFont( 0, defaultFont );
}

ClayTextCache::~ClayTextCache( )
{
    for ( auto &pair : m_fonts )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( pair.second.Atlas ) )
        {
            DenOfIz_TextureResource_Destroy( pair.second.Atlas );
        }
    }
}

void ClayTextCache::AddFont( const uint16_t fontId, Font *font )
{
    if ( font == nullptr )
    {
        spdlog::error( "ClayText::AddFont: Font cannot be null" );
        return;
    }

    auto &fontData   = m_fonts[ fontId ];
    fontData.FontPtr = font;
    InitializeFontAtlas( &fontData );
}

void ClayTextCache::RemoveFont( const uint16_t fontId )
{
    const auto it = m_fonts.find( fontId );
    if ( it != m_fonts.end( ) )
    {
        if ( it->second.TextureIndex > 0 && it->second.TextureIndex < m_textures.size( ) )
        {
            m_textures[ it->second.TextureIndex ]         = DENOFIZ_NULL_HANDLE;
            m_textureFontFlags[ it->second.TextureIndex ] = false;
        }
        if ( DENOFIZ_HANDLE_IS_VALID( it->second.Atlas ) )
        {
            DenOfIz_TextureResource_Destroy( it->second.Atlas );
        }
        m_fonts.erase( it );
    }
}

// Note keep DenOfIz::Font here to disambiguate in Linux
DenOfIz::Font *ClayTextCache::GetFont( const uint16_t fontId ) const
{
    const auto it = m_fonts.find( fontId );
    if ( it != m_fonts.end( ) )
    {
        return it->second.FontPtr;
    }
    return nullptr;
}

DenOfIz_ClayDimensions ClayTextCache::MeasureText( const DenOfIz_StringView &text, const Clay_TextElementConfig &desc ) const
{
    DenOfIz_ClayDimensions result{ };
    result.Width  = 0;
    result.Height = 0;

    const auto it = m_fonts.find( desc.fontId );
    if ( it == m_fonts.end( ) || it->second.FontPtr == nullptr )
    {
        return result;
    }

    Font *font = it->second.FontPtr;

    const auto  baseSize   = static_cast<float>( DenOfIz_FontAsset_InitialFontSize( font->Asset( ) ) );
    const float targetSize = desc.fontSize > 0 ? desc.fontSize : baseSize;

    const TextLayout *layout = GetOrCreateShapedText( text, desc.fontId, static_cast<uint32_t>( targetSize ), font );

    const auto size = layout->GetTextSize( );
    result.Width    = size.X;
    result.Height   = size.Y;

    return result;
}

TextLayout *ClayTextCache::GetOrCreateShapedText( const Clay_RenderCommand *command, Font *font ) const
{
    const auto &data       = command->renderData.text;
    const float targetSize = data.fontSize > 0 ? data.fontSize : static_cast<float>( DenOfIz_FontAsset_InitialFontSize( font->Asset( ) ) );
    return GetOrCreateShapedText( DenOfIz_StringView( data.stringContents.chars, data.stringContents.length ), data.fontId, static_cast<uint32_t>( targetSize ), font );
}

TextLayout *ClayTextCache::GetOrCreateShapedText( const DenOfIz_StringView &text, const uint16_t fontId, const uint32_t fontSize, Font *font ) const
{
    const uint64_t textHash = TextLayoutCache::HashString( text );
    return m_textLayoutCache.GetOrCreate( textHash, fontId, fontSize, font, text );
}

CachedTextVertices *ClayTextCache::GetOrCreateTextVertices( const TextVertexCacheKey &key ) const
{
    return m_textVertexCache.GetOrCreateCachedTextVertices( key, m_currentFrame );
}

uint32_t ClayTextCache::GetFontTextureIndex( const uint16_t fontId ) const
{
    const auto it = m_fonts.find( fontId );
    if ( it != m_fonts.end( ) )
    {
        return it->second.TextureIndex;
    }
    return 0;
}

DenOfIz_Texture ClayTextCache::GetFontTexture( const uint16_t fontId ) const
{
    const auto it = m_fonts.find( fontId );
    if ( it != m_fonts.end( ) && DENOFIZ_HANDLE_IS_VALID( it->second.Atlas ) )
    {
        return it->second.Atlas;
    }
    return DENOFIZ_NULL_HANDLE;
}

void ClayTextCache::GetAllFontTextures( std::vector<DenOfIz_Texture> &outTextures ) const
{
    outTextures.clear( );
    outTextures.reserve( m_textures.size( ) );

    for ( size_t i = 0; i < m_textures.size( ); ++i )
    {
        if ( m_textureFontFlags[ i ] && DENOFIZ_HANDLE_IS_VALID( m_textures[ i ] ) )
        {
            outTextures.push_back( m_textures[ i ] );
        }
    }
}

void ClayTextCache::UpdateFrame( const uint32_t currentFrame ) const
{
    m_currentFrame = currentFrame;
}

void ClayTextCache::ClearCaches( ) const
{
    m_textLayoutCache.Clear( );
    m_textVertexCache.Clear( );
}

uint64_t ClayTextCache::HashString( const DenOfIz_StringView &text )
{
    return TextLayoutCache::HashString( text );
}

void ClayTextCache::InitializeFontAtlas( ClayTextFontData *fontData )
{
    if ( fontData == nullptr || fontData->FontPtr == nullptr )
    {
        return;
    }

    const DenOfIz_FontAsset fontAsset = fontData->FontPtr->Asset( );
    if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
    {
        spdlog::error( "Font asset is null" );
        return;
    }

    DenOfIz_TextureDesc textureDesc{ };
    textureDesc.Width     = DenOfIz_FontAsset_AtlasWidth( fontAsset );
    textureDesc.Height    = DenOfIz_FontAsset_AtlasHeight( fontAsset );
    textureDesc.Format    = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Usage     = DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT | DENOFIZ_TEXTURE_USAGE_COPY_DST_BIT;
    textureDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Depth     = 1;
    textureDesc.DebugName = DENOFIZ_STRING( "ClayText Font Atlas Texture" );
    DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &textureDesc, &fontData->Atlas );

    DenOfIz_ByteArray atlasData = DenOfIz_FontAsset_AtlasData( fontAsset );
    if ( atlasData.NumElements > 0 )
    {
        DenOfIz_CommandQueueDesc commandQueueDesc{ };
        commandQueueDesc.QueueType = DENOFIZ_QUEUE_TYPE_GRAPHICS;

        DenOfIz_CommandQueue commandQueue;
        DenOfIz_LogicalDevice_CreateCommandQueue( m_logicalDevice, &commandQueueDesc, &commandQueue );

        DenOfIz_CommandListPoolDesc commandListPoolDesc{ };
        commandListPoolDesc.CommandQueue    = commandQueue;
        commandListPoolDesc.NumCommandLists = 1;

        DenOfIz_CommandListPool commandListPool;
        DenOfIz_LogicalDevice_CreateCommandListPool( m_logicalDevice, &commandListPoolDesc, &commandListPool );
        DenOfIz_CommandListArray commandLists;
        DenOfIz_CommandListPool_GetCommandLists( commandListPool, &commandLists );
        DenOfIz_CommandList commandList = commandLists.Elements[ 0 ];
        DenOfIz_CommandList_Begin( commandList );

        DenOfIz_PhysicalDevice deviceInfo;
        DenOfIz_LogicalDevice_DeviceInfo( m_logicalDevice, &deviceInfo );
        const auto alignedPitch = Utilities::Align( DenOfIz_FontAsset_AtlasWidth( fontAsset ) * DENOFIZ_FONT_ASSET_NUM_CHANNELS, deviceInfo.Constants.BufferTextureRowAlignment );
        const auto alignedSlice = Utilities::Align( DenOfIz_FontAsset_AtlasHeight( fontAsset ), deviceInfo.Constants.BufferTextureAlignment );

        DenOfIz_BufferDesc stagingDesc{ };
        stagingDesc.NumBytes  = alignedPitch * alignedSlice;
        stagingDesc.Usage     = DENOFIZ_BUFFER_USAGE_COPY_SRC_BIT;
        stagingDesc.DebugName = DENOFIZ_STRING( "ClayText Font MSDF Atlas Staging Buffer" );
        stagingDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU;
        DenOfIz_Buffer stagingBuffer;
        DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &stagingDesc, &stagingBuffer );

        DenOfIz_ResourceTracking resourceTracking;
        DenOfIz_ResourceTracking_Create( &resourceTracking );
        DenOfIz_ResourceTracking_TrackTexture( resourceTracking, fontData->Atlas, DENOFIZ_QUEUE_TYPE_GRAPHICS );
        DenOfIz_ResourceTracking_TrackBuffer( resourceTracking, stagingBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );

        DenOfIz_LoadAtlasIntoGpuTextureDesc loadDesc{ };
        loadDesc.Device        = m_logicalDevice;
        loadDesc.StagingBuffer = stagingBuffer;
        loadDesc.CommandList   = commandList;
        loadDesc.Texture       = fontData->Atlas;
        DenOfIz_FontAssetReader_LoadAtlasIntoGpuTexture( fontAsset, &loadDesc );

        DenOfIz_ResourceTracking_TransitionTexture( resourceTracking, commandList, fontData->Atlas, DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

        DenOfIz_Format format;
        DenOfIz_TextureResource_GetFormat( fontData->Atlas, &format );

        DenOfIz_CopyBufferToTextureDesc copyDesc{ };
        copyDesc.SrcBuffer  = stagingBuffer;
        copyDesc.DstTexture = fontData->Atlas;
        copyDesc.RowPitch   = DenOfIz_FontAsset_AtlasWidth( fontAsset ) * 4; // 4 bytes per pixel (RGBA)
        copyDesc.Format     = format;

        DenOfIz_CommandList_CopyBufferToTexture( commandList, &copyDesc );

        DenOfIz_ResourceTracking_TransitionTexture( resourceTracking, commandList, fontData->Atlas, DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

        DenOfIz_CommandList_End( commandList );
        DenOfIz_ExecuteCommandListsDesc executeDesc{ };
        executeDesc.CommandLists.Elements    = &commandList;
        executeDesc.CommandLists.NumElements = 1;
        DenOfIz_CommandQueue_ExecuteCommandLists( commandQueue, &executeDesc );
        DenOfIz_CommandQueue_WaitIdle( commandQueue );

        DenOfIz_ResourceTracking_Destroy( resourceTracking );
        DenOfIz_Buffer_Destroy( stagingBuffer );
        DenOfIz_CommandListPool_Destroy( commandListPool );
        DenOfIz_CommandQueue_Destroy( commandQueue );
    }

    fontData->TextureIndex = RegisterTexture( fontData->Atlas );
}

uint32_t ClayTextCache::RegisterTexture( DenOfIz_Texture texture )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
    {
        return 0;
    }

    for ( uint32_t i = 1; i < m_textures.size( ); ++i )
    {
        if ( m_textures[ i ] == texture )
        {
            return i;
        }
    }

    for ( uint32_t i = 1; i < m_textures.size( ); ++i )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( m_textures[ i ] ) )
        {
            m_textures[ i ]         = texture;
            m_textureFontFlags[ i ] = true;
            return i;
        }
    }

    spdlog::error( "ClayText: Exceeded maximum texture count" );
    return 0;
}
