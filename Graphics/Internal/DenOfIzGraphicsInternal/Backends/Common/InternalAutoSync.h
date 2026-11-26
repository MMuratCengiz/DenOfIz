/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "DenOfIzGraphicsInternal/Backends/Interface/ISwapChain.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"

namespace DenOfIz
{
    class InternalAutoSync
    {
        std::mutex                                   m_mutex;
        std::unordered_set<ITexture *>               m_swapChainTextures;
        std::unordered_map<ISwapChain *, ITexture *> m_swapChainCurrentTextures;
        std::unordered_set<ISwapChain *>             m_activeSwapChains;

    public:
        explicit InternalAutoSync( bool autoIssueBarriers );
        ~InternalAutoSync( ) = default;

        void NewTextureResource( ITexture *texture, bool isSwapChainTexture = false );
        void RemoveTextureResource( ITexture *texture );
        void AcquireImage( ISwapChain *swapChain, ITexture *texture );
        void BeginRendering( ITexture *renderTarget );
        bool NeedsSwapChainSync( ISwapChain *&outSwapChain );
        void ClearCommandListSync( );
        void CollectActiveSwapChains( std::vector<ISwapChain *> &outSwapChains );
        void ClearSwapChainSync( ISwapChain *swapChain );
        void RemoveSwapChain( ISwapChain *swapChain );
    };
} // namespace DenOfIz
