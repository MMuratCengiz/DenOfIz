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

#include <DenOfIzGraphics/Backends/Interface/ILock.h>
#include <DenOfIzGraphics/Backends/Interface/IResource.h>
#include <DenOfIzGraphics/Backends/Interface/ISemaphore.h>
#include <boost/noncopyable.hpp>

namespace DenOfIz
{

    struct SwapChainCreateInfo
    {
        uint32_t Width = 0; // 0 means that it will be set to the window width
        uint32_t Height = 0; // 0 means that it will be set to the window height
        uint32_t BufferCount = 3;
        ImageFormat BackBufferFormat = ImageFormat::R8G8B8A8Unorm;
        ImageFormat DepthBufferFormat = ImageFormat::D32Float;
    };

    struct Viewport
    {
        float X = 0.0f;
        float Y = 0.0f;
        float Width = 0.0f;
        float Height = 0.0f;
    };

    class ISwapChain : boost::noncopyable
    {
    public:
        virtual ~ISwapChain() = default;

        virtual ImageFormat GetPreferredFormat() = 0;
        virtual uint32_t AcquireNextImage(ISemaphore *imageReadySemaphore) = 0;
        virtual void Resize(uint32_t width, uint32_t height) = 0;

        virtual IImageResource *GetRenderTarget(uint32_t frame) = 0;
        virtual Viewport GetViewport() = 0;
    };

} // namespace DenOfIz
