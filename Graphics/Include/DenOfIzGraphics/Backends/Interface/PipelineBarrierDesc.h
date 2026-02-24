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

#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Backends/Interface/RayTracing/BottomLevelAS.h"
#include "DenOfIzGraphics/Backends/Interface/RayTracing/TopLevelAS.h"
#include "DenOfIzGraphics/Backends/Interface/Texture.h"

#undef MemoryBarrier

typedef struct DenOfIz_TextureBarrierDesc
{
    DenOfIz_Texture            Resource;
    DenOfIz_ResourceUsageFlags OldState;
    DenOfIz_ResourceUsageFlags NewState;
    bool                       EnableQueueBarrier;
    DenOfIz_QueueType          SourceQueue;
    DenOfIz_QueueType          DestinationQueue;
    bool                       EnableSubresourceBarrier;
    uint32_t                   MipLevel;
    uint32_t                   ArrayLayer;
} DenOfIz_TextureBarrierDesc;

typedef struct DenOfIz_TextureBarrierDescArray
{
    DenOfIz_TextureBarrierDesc const *Elements;
    size_t                            NumElements;
} DenOfIz_TextureBarrierDescArray;

typedef struct DenOfIz_BufferBarrierDesc
{
    DenOfIz_Buffer             Resource;
    DenOfIz_ResourceUsageFlags OldState;
    DenOfIz_ResourceUsageFlags NewState;
} DenOfIz_BufferBarrierDesc;

typedef struct DenOfIz_BufferBarrierDescArray
{
    DenOfIz_BufferBarrierDesc const *Elements;
    size_t                           NumElements;
} DenOfIz_BufferBarrierDescArray;

typedef struct DenOfIz_MemoryBarrierDesc
{
    DenOfIz_BottomLevelAS      BottomLevelAS;
    DenOfIz_TopLevelAS         TopLevelAS;
    DenOfIz_Buffer             BufferResource;
    DenOfIz_Texture            TextureResource;
    DenOfIz_ResourceUsageFlags OldState;
    DenOfIz_ResourceUsageFlags NewState;
} DenOfIz_MemoryBarrierDesc;

typedef struct DenOfIz_MemoryBarrierDescArray
{
    DenOfIz_MemoryBarrierDesc const *Elements;
    size_t                           NumElements;
} DenOfIz_MemoryBarrierDescArray;

typedef struct DenOfIz_PipelineBarrierDesc
{
    DenOfIz_BufferBarrierDescArray  BufferBarriers;
    DenOfIz_TextureBarrierDescArray TextureBarriers;
    DenOfIz_MemoryBarrierDescArray  MemoryBarriers;
} DenOfIz_PipelineBarrierDesc;
