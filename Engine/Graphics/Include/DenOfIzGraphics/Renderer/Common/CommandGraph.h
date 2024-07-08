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
    /// Command graph is a collection of command lists that are executed in parallel, synchronization is managed automatically
    struct CommandGraphDesc
    {
        uint32_t ThreadCount = 1;
        uint32_t FrameCount  = 3;
    };

    class CommandGraph
    {
    protected:
        CommandGraphDesc                           m_desc;
        std::vector<std::unique_ptr<IFence>>       m_frameFences;
        std::vector<std::unique_ptr<ICommandList>> m_commandLists;
        int                                        m_frame = -1; // -1 = first frame
        ILogicalDevice                            *m_logicalDevice;

    public:
        CommandGraph(ILogicalDevice *logicalDevice) : m_logicalDevice(logicalDevice)
        {
        }

        virtual

            void
            NextFrame()
        {
            m_frame = (m_frame + 1) % m_desc.FrameCount;
        }

        inline uint32_t GetCurrentFrame() const
        {
            return m_frame;
        }
    };

} // namespace DenOfIz
