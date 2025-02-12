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

#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>

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

        uint64_t m_currentFrame = 0;
        uint64_t m_nextFrame    = 0;

        ILogicalDevice             *m_device;
        ISwapChain                 *m_swapChain;
        ICommandQueue              *m_commandQueue;
        std::vector<ICommandList *> m_commandLists;

    public:
        explicit FrameSync( const FrameSyncDesc &desc );
        uint64_t      NextFrame( );
        IFence       *GetFrameFence( uint64_t frame ) const;
        ISemaphore   *GetPresentSignalSemaphore( uint64_t frame ) const;
        ICommandList *GetCommandList( uint64_t frame ) const;
        void          ExecuteCommandList( uint64_t frame ) const;
        uint32_t      AcquireNextImage( uint64_t frame ) const;
        PresentResult Present( uint32_t imageIndex ) const;
        void          WaitIdle( ) const;
        ~FrameSync( ) = default;
    };
} // namespace DenOfIz
