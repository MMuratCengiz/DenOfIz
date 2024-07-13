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

#include <future>
#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
#include "TextureLoader.h"
#include "DenOfIzCore/Utilities.h"

namespace DenOfIz
{
    struct CopyToGpuBufferDesc
    {
        IBufferResource *DstBuffer;
        const void      *Data;
        size_t           NumBytes;
    };

    struct CopyDataToTextureDesc
    {
        ITextureResource *DstTexture;
        const void       *Data;
        size_t            NumBytes;
    };

    struct LoadTextureDesc
    {
        std::string       File;
        ITextureResource *DstTexture;
    };

    class BatchResourceCopy
    {
    private:
        ILogicalDevice *m_device;

        std::unique_ptr<ICommandListPool> m_commandListPool;
        ICommandList                     *m_copyCommandList;

        std::unique_ptr<IFence>                       m_executeFence;
        std::mutex                                    m_resourceCleanLock;
        std::vector<std::unique_ptr<IBufferResource>> m_resourcesToClean;
        std::future<void>                             m_cleanResourcesFuture;

    public:
        BatchResourceCopy(ILogicalDevice *device);
        ~BatchResourceCopy();

        void Begin();
        void CopyToGPUBuffer(const CopyToGpuBufferDesc &copyDesc);
        void CopyBufferRegion(const CopyBufferRegionDesc &copyDesc);
        void CopyTextureRegion(const CopyTextureRegionDesc &copyDesc);
        void CopyBufferToTexture(const CopyDataToTextureDesc &copyDesc);
        void LoadTexture(const LoadTextureDesc &loadDesc); // TODO !IMPROVEMENT! Temp needs to be elsewhere
        void End(ISemaphore *notify);
        void CleanResources();
    };
} // namespace DenOfIz
