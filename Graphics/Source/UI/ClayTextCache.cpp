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
#include "DenOfIzGraphics/Assets/Font/FontLibrary.h"
#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h"
#include "DenOfIzGraphics/Backends/Interface/ICommandList.h"
#include "DenOfIzGraphics/Backends/Interface/ICommandListPool.h"
#include "DenOfIzGraphics/Backends/Interface/ICommandQueue.h"
#include "DenOfIzGraphics/Renderer/Sync/ResourceTracking.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Embedded/EmbeddedFonts.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

ClayTextCache::ClayTextCache( const ClayTextCacheDesc &desc ) : m_logicalDevice( desc.LogicalDevice ), m_maxTextures( desc.MaxTextures )
{
    if ( m_logicalDevice == nullptr )
    {
        spdlog::error( "ClayText: LogicalDevice cannot be null" );
        return;
    }

    m_textures.resize( m_maxTextures, nullptr );
    m_textureFontFlags.resize( m_maxTextures, false );

    static FontLibrary defaultFontLibrary;
    static auto        defaultFont = defaultFontLibrary.LoadFont( { EmbeddedFonts::GetInterVar( ) } );
    AddFont( 0, defaultFont );
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
            m_textures[ it->second.TextureIndex ]         = nullptr;
            m_textureFontFlags[ it->second.TextureIndex ] = false;
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

ClayDimensions ClayTextCache::MeasureText( const InteropString &text, const Clay_TextElementConfig &desc ) const
{
    return MeasureTextDirect( text.Get( ), text.NumChars( ), desc.fontId, desc.fontSize );
}

ClayDimensions ClayTextCache::MeasureTextDirect( const char *text, const size_t length, const uint16_t fontId, const uint16_t fontSize ) const
{
    ClayDimensions result{ };
    result.Width  = 0;
    result.Height = 0;

    const auto it = m_fonts.find( fontId );
    if ( it == m_fonts.end( ) || it->second.FontPtr == nullptr )
    {
        return result;
    }

    Font *font = it->second.FontPtr;

    const auto  baseSize   = static_cast<float>( font->Asset( )->InitialFontSize );
    const float targetSize = fontSize > 0 ? fontSize * m_dpiScale : baseSize;

    const TextLayout *layout = GetOrCreateShapedTextDirect( text, length, fontId, static_cast<uint32_t>( targetSize ), font );

    const auto size = layout->GetTextSize( );
    result.Width    = size.X / m_dpiScale;
    result.Height   = size.Y / m_dpiScale;

    return result;
}

TextLayout *ClayTextCache::GetOrCreateShapedText( const Clay_RenderCommand *command, Font *font ) const
{
    const auto &data       = command->renderData.text;
    const float targetSize = data.fontSize > 0 ? data.fontSize * m_dpiScale : static_cast<float>( font->Asset( )->InitialFontSize );
    return GetOrCreateShapedTextDirect( data.stringContents.chars, data.stringContents.length, data.fontId, static_cast<uint32_t>( targetSize ), font );
}

TextLayout *ClayTextCache::GetOrCreateShapedTextDirect( const char *text, const size_t length, const uint16_t fontId, const uint32_t fontSize, Font *font ) const
{
    const uint64_t textHash = TextLayoutCache::HashString( text, length );
    return m_textLayoutCache.GetOrCreate( textHash, fontId, fontSize, font, text, length, m_currentFrame );
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

ITextureResource *ClayTextCache::GetFontTexture( const uint16_t fontId ) const
{
    const auto it = m_fonts.find( fontId );
    if ( it != m_fonts.end( ) && it->second.Atlas )
    {
        return it->second.Atlas.get( );
    }
    return nullptr;
}

void ClayTextCache::GetAllFontTextures( std::vector<ITextureResource *> &outTextures ) const
{
    outTextures.clear( );
    outTextures.reserve( m_textures.size( ) );

    for ( size_t i = 0; i < m_textures.size( ); ++i )
    {
        if ( m_textureFontFlags[ i ] && m_textures[ i ] != nullptr )
        {
            outTextures.push_back( m_textures[ i ] );
        }
    }
}

void ClayTextCache::UpdateFrame( const uint32_t currentFrame ) const
{
    m_currentFrame = currentFrame;
}

void ClayTextCache::CleanupCaches( const uint32_t maxLayoutAge, const uint32_t maxVertexAge ) const
{
    m_textLayoutCache.Cleanup( m_currentFrame, maxLayoutAge );
    m_textVertexCache.Cleanup( m_currentFrame, maxVertexAge );
}

void ClayTextCache::ClearCaches( ) const
{
    m_textLayoutCache.Clear( );
    m_textVertexCache.Clear( );
}

void ClayTextCache::SetDpiScale( const float dpiScale )
{
    m_dpiScale = dpiScale;
}

float ClayTextCache::GetDpiScale( ) const
{
    return m_dpiScale;
}

uint64_t ClayTextCache::HashString( const char *str, const size_t length )
{
    return TextLayoutCache::HashString( str, length );
}

void ClayTextCache::InitializeFontAtlas( ClayTextFontData *fontData )
{
    if ( fontData == nullptr || fontData->FontPtr == nullptr )
    {
        return;
    }

    const auto *fontAsset = fontData->FontPtr->Asset( );
    if ( fontAsset == nullptr )
    {
        spdlog::error( "Font asset is null" );
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
    textureDesc.DebugName    = "ClayText Font Atlas Texture";
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
        stagingDesc.NumBytes     = alignedPitch * alignedSlice;
        stagingDesc.Descriptor   = BitSet( ResourceDescriptor::Buffer );
        stagingDesc.InitialUsage = ResourceUsage::CopySrc;
        stagingDesc.DebugName    = "ClayText Font MSDF Atlas Staging Buffer";
        stagingDesc.HeapType     = HeapType::CPU;
        auto stagingBuffer       = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( stagingDesc ) );

        ResourceTracking resourceTracking;
        resourceTracking.TrackTexture( fontData->Atlas.get( ), ResourceUsage::ShaderResource );
        resourceTracking.TrackBuffer( stagingBuffer.get( ), ResourceUsage::CopySrc );

        LoadAtlasIntoGpuTextureDesc loadDesc{ };
        loadDesc.Device        = m_logicalDevice;
        loadDesc.StagingBuffer = stagingBuffer.get( );
        loadDesc.CommandList   = commandList;
        loadDesc.Texture       = fontData->Atlas.get( );
        FontAssetReader::LoadAtlasIntoGpuTexture( *fontAsset, loadDesc );

        BatchTransitionDesc batchTransitionDesc{ commandList };
        batchTransitionDesc.TransitionTexture( fontData->Atlas.get( ), ResourceUsage::CopyDst );
        resourceTracking.BatchTransition( batchTransitionDesc );

        CopyBufferToTextureDesc copyDesc{ };
        copyDesc.SrcBuffer  = stagingBuffer.get( );
        copyDesc.DstTexture = fontData->Atlas.get( );
        copyDesc.RowPitch   = fontAsset->AtlasWidth * 4; // 4 bytes per pixel (RGBA)
        copyDesc.Format     = fontData->Atlas->GetFormat( );

        commandList->CopyBufferToTexture( copyDesc );

        batchTransitionDesc = BatchTransitionDesc{ commandList };
        batchTransitionDesc.TransitionTexture( fontData->Atlas.get( ), ResourceUsage::ShaderResource );
        resourceTracking.BatchTransition( batchTransitionDesc );

        commandList->End( );
        ExecuteCommandListsDesc executeDesc{ };
        executeDesc.CommandLists = { commandList };
        commandQueue->ExecuteCommandLists( executeDesc );
        commandQueue->WaitIdle( );
    }

    fontData->TextureIndex = RegisterTexture( fontData->Atlas.get( ) );
}

uint32_t ClayTextCache::RegisterTexture( ITextureResource *texture )
{
    if ( texture == nullptr )
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
        if ( m_textures[ i ] == nullptr )
        {
            m_textures[ i ]         = texture;
            m_textureFontFlags[ i ] = true;
            return i;
        }
    }

    spdlog::error( "ClayText: Exceeded maximum texture count" );
    return 0;
}
