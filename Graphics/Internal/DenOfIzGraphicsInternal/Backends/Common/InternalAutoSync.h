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

#pragma once

#include <mutex>
#include <unordered_map>
#include <unordered_set>
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
    };
} // namespace DenOfIz
