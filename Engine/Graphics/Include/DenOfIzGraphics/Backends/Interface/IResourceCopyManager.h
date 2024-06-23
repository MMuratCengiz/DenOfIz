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

#include "IBufferResource.h"
#include "ITextureResource.h"
#include "ISemaphore.h"

namespace DenOfIz
{
    struct CopyBufferInfo
    {
        IBufferResource* src;
        IBufferResource* dst;
        uint32_t srcOffset;
        uint32_t dstOffset;
        /// 0 means copy the whole buffer
        uint32_t size;
    };

    struct CopyBufferToTextureInfo
    {
        IBufferResource* src;
        ITextureResource* dst;
        uint32_t srcOffset = 0;
        uint32_t dstOffset = 0;
        /// 0 means copy the whole buffer
        uint32_t size = 0;
    };

    class IResourceCopyManager
    {
    public:
        virtual ~IResourceCopyManager() = default;

        virtual void Begin() = 0;
        virtual void CopyBuffer(const CopyBufferInfo& copyInfo) = 0;
        virtual void CopyBufferToTexture(const CopyBufferToTextureInfo& copyInfo) = 0;
        virtual void End(const ISemaphore* notify) = 0;
    };
}