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
#include "DenOfIzGraphics/Backends/Interface/CommandList.h"
#include "DenOfIzGraphics/Backends/Interface/Texture.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_ResourceTracking )

    typedef struct DenOfIz_TransitionBufferDesc
    {
        DenOfIz_Buffer             Buffer;
        DenOfIz_ResourceUsageFlags NewUsage;
        DenOfIz_QueueType          QueueType;
    } DenOfIz_TransitionBufferDesc;

    typedef struct DenOfIz_TransitionBufferDescArray
    {
        DenOfIz_TransitionBufferDesc *Elements;
        size_t                        NumElements;
    } DenOfIz_TransitionBufferDescArray;

    typedef struct DenOfIz_TransitionTextureDesc
    {
        DenOfIz_Texture            Texture;
        DenOfIz_ResourceUsageFlags NewUsage;
        DenOfIz_QueueType          QueueType;
    } DenOfIz_TransitionTextureDesc;

    typedef struct DenOfIz_TransitionTextureDescArray
    {
        DenOfIz_TransitionTextureDesc *Elements;
        size_t                         NumElements;
    } DenOfIz_TransitionTextureDescArray;

    typedef struct DenOfIz_BatchTransitionDesc
    {
        DenOfIz_TransitionBufferDescArray  Buffers;
        DenOfIz_TransitionTextureDescArray Textures;
    } DenOfIz_BatchTransitionDesc;

    DZ_API void DenOfIz_ResourceTracking_Create( DenOfIz_ResourceTracking *outTracking );
    DZ_API void DenOfIz_ResourceTracking_TrackBuffer( DenOfIz_ResourceTracking tracking, DenOfIz_Buffer buffer, DenOfIz_QueueType queueType );
    DZ_API void DenOfIz_ResourceTracking_TrackTexture( DenOfIz_ResourceTracking tracking, DenOfIz_Texture texture, DenOfIz_QueueType queueType );
    DZ_API void DenOfIz_ResourceTracking_UntrackBuffer( DenOfIz_ResourceTracking tracking, DenOfIz_Buffer buffer );
    DZ_API void DenOfIz_ResourceTracking_UntrackTexture( DenOfIz_ResourceTracking tracking, DenOfIz_Texture texture );
    DZ_API void DenOfIz_ResourceTracking_TransitionBuffer( DenOfIz_ResourceTracking tracking, DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint32_t newUsage,
                                                           DenOfIz_QueueType queueType );
    DZ_API void DenOfIz_ResourceTracking_TransitionTexture( DenOfIz_ResourceTracking tracking, DenOfIz_CommandList commandList, DenOfIz_Texture texture, uint32_t newUsage,
                                                            DenOfIz_QueueType queueType );
    DZ_API void DenOfIz_ResourceTracking_BatchTransition( DenOfIz_ResourceTracking tracking, DenOfIz_CommandList commandList, const DenOfIz_BatchTransitionDesc *desc );
    DZ_API void DenOfIz_ResourceTracking_NotifyTextureTransition( DenOfIz_ResourceTracking tracking, DenOfIz_Texture texture, uint32_t newUsage );
    DZ_API void DenOfIz_ResourceTracking_NotifyBufferTransition( DenOfIz_ResourceTracking tracking, DenOfIz_Buffer buffer, uint32_t newUsage );
    DZ_API void DenOfIz_ResourceTracking_Destroy( DenOfIz_ResourceTracking tracking );

#ifdef __cplusplus
}
#endif
