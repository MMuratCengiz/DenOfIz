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

#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"

namespace DenOfIz
{
    struct DZ_API FrameSyncDesc
    {
        ILogicalDevice *Device;
        ISwapChain     *SwapChain;
        ICommandQueue  *CommandQueue;
        uint32_t        NumFrames;
    };

    /// Abstracts repetitive frame management, mostly for simple prototyping
    class FrameSync
    {
        uint32_t m_numFrames;

        std::vector<std::unique_ptr<IFence>>     m_frameFences;
        std::vector<std::unique_ptr<ISemaphore>> m_imageAvailableSemaphores;
        std::vector<std::unique_ptr<ISemaphore>> m_renderFinishedSemaphores;
        std::unique_ptr<ICommandListPool>        m_commandListPool;

        uint32_t m_currentFrame = 0;
        uint32_t m_nextFrame    = 0;

        ILogicalDevice             *m_device;
        ISwapChain                 *m_swapChain;
        ICommandQueue              *m_commandQueue;
        std::vector<ICommandList *> m_commandLists;

    public:
        DZ_API explicit FrameSync( const FrameSyncDesc &desc );
        DZ_API uint32_t      NextFrame( );
        DZ_API IFence       *GetFrameFence( uint32_t frame ) const;
        DZ_API ISemaphore   *GetPresentSignalSemaphore( uint32_t frame ) const;
        DZ_API ICommandList *GetCommandList( uint32_t frame ) const;
        DZ_API void          ExecuteCommandList( uint32_t frame, const ISemaphoreArray& additionalSemaphores = {} ) const;
        DZ_API uint32_t      AcquireNextImage( uint32_t frame ) const;
        DZ_API PresentResult Present( uint32_t imageIndex ) const;
        DZ_API void          WaitIdle( ) const;
        DZ_API ~FrameSync( ) = default;
    };
} // namespace DenOfIz
