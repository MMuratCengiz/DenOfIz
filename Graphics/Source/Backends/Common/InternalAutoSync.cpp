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
