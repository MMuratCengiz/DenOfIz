/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <cstdint>

typedef enum
{
    DENOFIZ_HIT_GROUP_TYPE_TRIANGLES,
    DENOFIZ_HIT_GROUP_TYPE_AABBS /**< Procedural primitives, hit groups require an intersection shader. */
} DenOfIz_HitGroupType;

typedef enum
{
    DENOFIZ_AS_BUILD_NONE_BIT              = 1 << 1, /**< No effect, like zero. */
    DENOFIZ_AS_BUILD_ALLOW_UPDATE_BIT      = 1 << 2, /**< Required to refit a TLAS with DenOfIz_CommandList_UpdateTopLevelAS. */
    DENOFIZ_AS_BUILD_ALLOW_COMPACTION_BIT  = 1 << 3, /**< DX12 and Vulkan. */
    DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT = 1 << 4, /**< DX12 and Vulkan. */
    DENOFIZ_AS_BUILD_PREFER_FAST_BUILD_BIT = 1 << 5,
    DENOFIZ_AS_BUILD_LOW_MEMORY_BIT        = 1 << 6, /**< Vulkan only, MINIMIZE_MEMORY is the DX12 equivalent. */
    DENOFIZ_AS_BUILD_FAST_TRACE_BIT        = 1 << 7, /**< Ignored, see PREFER_FAST_TRACE. */
    DENOFIZ_AS_BUILD_FAST_BUILD_BIT        = 1 << 8, /**< Ignored, see PREFER_FAST_BUILD. */
    DENOFIZ_AS_BUILD_MINIMIZE_MEMORY_BIT   = 1 << 9, /**< DX12 only, LOW_MEMORY is the Vulkan equivalent. */
    DENOFIZ_AS_BUILD_PERFORM_UPDATE_BIT    = 1 << 10 /**< DX12 only. UpdateTopLevelAS sets it itself. */
} DenOfIz_ASBuildFlagBits;
typedef uint32_t DenOfIz_ASBuildFlags;
