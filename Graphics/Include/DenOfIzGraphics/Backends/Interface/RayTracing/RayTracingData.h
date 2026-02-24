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

#include <cstdint>

/**
 * @brief Type of geometry in a hit group for ray tracing.
 */
typedef enum
{
    DENOFIZ_HIT_GROUP_TYPE_TRIANGLES, /**< Standard triangle mesh geometry with closest-hit shader. */
    DENOFIZ_HIT_GROUP_TYPE_AABBS      /**< Procedural geometry using AABBs with intersection shader. */
} DenOfIz_HitGroupType;

/**
 * @brief Flags controlling acceleration structure build behavior.
 *
 * These flags configure how BLAS and TLAS structures are built, trading off between
 * build time, trace performance, memory usage, and updateability.
 */
typedef enum
{
    DENOFIZ_AS_BUILD_NONE_BIT              = 1 << 1, /**< No special flags. */
    DENOFIZ_AS_BUILD_ALLOW_UPDATE_BIT      = 1 << 2, /**< Allow in-place updates (required for dynamic transforms). */
    DENOFIZ_AS_BUILD_ALLOW_COMPACTION_BIT  = 1 << 3, /**< Allow post-build compaction to reduce memory. */
    DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT = 1 << 4, /**< Prefer faster ray traversal over faster builds. */
    DENOFIZ_AS_BUILD_PREFER_FAST_BUILD_BIT = 1 << 5, /**< Prefer faster builds over faster ray traversal. */
    DENOFIZ_AS_BUILD_LOW_MEMORY_BIT        = 1 << 6, /**< Minimize memory at cost of build/trace performance. */
    DENOFIZ_AS_BUILD_FAST_TRACE_BIT        = 1 << 7, /**< Optimize for fastest trace (may use more memory). */
    DENOFIZ_AS_BUILD_FAST_BUILD_BIT        = 1 << 8, /**< Optimize for fastest build (may reduce trace quality). */
    DENOFIZ_AS_BUILD_MINIMIZE_MEMORY_BIT   = 1 << 9, /**< Use minimum memory footprint. */
    DENOFIZ_AS_BUILD_PERFORM_UPDATE_BIT    = 1 << 10 /**< Perform update rather than full rebuild (requires ALLOW_UPDATE). */
} DenOfIz_ASBuildFlagBits;
typedef uint32_t DenOfIz_ASBuildFlags;
