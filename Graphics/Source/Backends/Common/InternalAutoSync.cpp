/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Common/InternalAutoSync.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

InternalAutoSync::InternalAutoSync( bool autoIssueBarriers )
{
}

void InternalAutoSync::NewTextureResource( ITexture *texture, bool isSwapChainTexture )
{
    std::lock_guard lock( m_mutex );
    if ( isSwapChainTexture )
    {
        m_swapChainTextures.insert( texture );
    }
}

void InternalAutoSync::RemoveTextureResource( ITexture *texture )
{
    std::lock_guard lock( m_mutex );
    m_swapChainTextures.erase( texture );
    for ( auto it = m_swapChainCurrentTextures.begin( ); it != m_swapChainCurrentTextures.end( ); )
    {
        if ( it->second == texture )
        {
            it = m_swapChainCurrentTextures.erase( it );
        }
        else
        {
            ++it;
        }
    }
}

void InternalAutoSync::AcquireImage( ISwapChain *swapChain, ITexture *texture )
{
    std::lock_guard lock( m_mutex );

    m_swapChainCurrentTextures[ swapChain ] = texture;
    m_swapChainTextures.insert( texture );
    m_activeSwapChains.insert( swapChain );
}

void InternalAutoSync::BeginRendering( ITexture *renderTarget )
{
    std::lock_guard lock( m_mutex );
    if ( m_swapChainTextures.contains( renderTarget ) )
    {
        for ( const auto &[ swapChain, texture ] : m_swapChainCurrentTextures )
        {
            if ( texture == renderTarget )
            {
                m_activeSwapChains.insert( swapChain );
                return;
            }
        }
    }
}

bool InternalAutoSync::NeedsSwapChainSync( ISwapChain *&outSwapChain )
{
    std::lock_guard lock( m_mutex );
    if ( !m_activeSwapChains.empty( ) )
    {
        outSwapChain = *m_activeSwapChains.begin( );
        return true;
    }
    outSwapChain = nullptr;
    return false;
}

void InternalAutoSync::ClearCommandListSync( )
{
    std::lock_guard lock( m_mutex );
    m_activeSwapChains.clear( );
}

void InternalAutoSync::CollectActiveSwapChains( std::vector<ISwapChain *> &outSwapChains )
{
    std::lock_guard lock( m_mutex );
    outSwapChains.assign( m_activeSwapChains.begin( ), m_activeSwapChains.end( ) );
}

void InternalAutoSync::ClearSwapChainSync( ISwapChain *swapChain )
{
    std::lock_guard lock( m_mutex );
    m_activeSwapChains.erase( swapChain );
}

void InternalAutoSync::RemoveSwapChain( ISwapChain *swapChain )
{
    std::lock_guard lock( m_mutex );
    m_activeSwapChains.erase( swapChain );
    if ( const auto it = m_swapChainCurrentTextures.find( swapChain ); it != m_swapChainCurrentTextures.end( ) )
    {
        m_swapChainTextures.erase( it->second );
        m_swapChainCurrentTextures.erase( it );
    }
}
