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
    /// Manages a rotating group of command lists, commonly used in games to create a command list per frame.
    /// It can also manage synchronization objects to make sure each frame is synchronized correctly.
    /// It should be mostly used in the beginning of the frame and at the end of it. Todo provide sample
    struct CommandListRingDesc
    {
        bool    CreateSyncObjects       = true;
        uint8_t NumFrames               = 3;
        uint8_t NumCommandListsPerFrame = 1;
    };

    class CommandListRing
    {
        std::vector<std::unique_ptr<IFence>>           m_frameFences;
        std::vector<std::unique_ptr<ISemaphore>>       m_imageReadySemaphores;
        std::vector<std::unique_ptr<ISemaphore>>       m_imageRenderedSemaphores;
        std::vector<std::unique_ptr<ICommandListPool>> m_commandListPools;
        uint32_t                                       m_currentFrame = 0;
        uint32_t                                       m_frame        = 0;

        ILogicalDevice     *m_logicalDevice;
        CommandListRingDesc m_desc;

    public:
        explicit CommandListRing( ILogicalDevice *logicalDevice );
        /// Move the CommandListRing to the next frame.
        void NextFrame( );
        /// Return a command list from the pool of the current frame.
        /// @param index cannot be larger or equal @ref CommandListRingDesc::NuNumCommandListsPerFrame
        ICommandList *FrameCommandList( const uint32_t index );
        /// Will execute the command list with the created sync objects and then present the results to the swap chain.
        /// If you do not need to render instead use the Execute command
        /// @param commandList command list to execute with given sync objects
        /// @param swapChain swapChain object to present
        /// @param image the index of the swap chain image, can be acquired view @ref ISwapChain::AcquireNextImage(  )
        void ExecuteAndPresent( ICommandList *commandList, ISwapChain *swapChain, const uint32_t image ) const;
        /// Execute the last commandList in the pool of the current frame, this is different as it will notify the built-in Sync object.
        /// @param commandList the last command list in this pool
        void                   ExecuteLast( ICommandList *commandList ) const;
        void                   WaitIdle( ) const;
        [[nodiscard]] uint32_t CurrentImage( ISwapChain *swapChain ) const;
        [[nodiscard]] uint32_t CurrentFrame( ) const;
    };

} // namespace DenOfIz
