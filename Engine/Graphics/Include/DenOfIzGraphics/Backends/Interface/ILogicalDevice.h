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

#include <DenOfIzCore/Common.h>

namespace DenOfIz
{
    struct PhysicalDeviceCapabilities
    {
        bool DedicatedTransferQueue;
        bool RayTracing;
        bool ComputeShaders;
    };

    struct PhysicalDeviceProperties
    {
        bool IsDedicated;
        uint32_t MemoryAvailableInMb;
    };

    struct PhysicalDeviceInfo
    {
        long Id;
        std::string Name;
        PhysicalDeviceProperties Properties;
        PhysicalDeviceCapabilities Capabilities;
    };

    class ILogicalDevice
    {
    public:
        virtual ~ILogicalDevice() = default;

    private:
        virtual void CreateDevice( SDL_Window *window ) = 0;
        virtual std::vector<PhysicalDeviceInfo> ListPhysicalDevices() = 0;
        virtual void LoadPhysicalDevice( const PhysicalDeviceInfo &device ) = 0;
        virtual void WaitIdle() = 0;
    };

}
