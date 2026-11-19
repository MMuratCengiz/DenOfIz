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

#include "CommandQueue.h"
#include "CommonData.h"
#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Result.h"
#include "Texture.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Swap chain handle for presenting rendered frames to a window surface.
     *
     * The swap chain always has the pixel size of its window: it recreates itself when the window changes, there is no resize call.
     *
     * @par Backend implementations
     * - DirectX 12: IDXGISwapChain4
     * - Vulkan: VkSwapchainKHR + VkSurfaceKHR
     * - Metal: CAMetalLayer + MTKView
     * - WebGPU: WGPUSurface
     *
     * @par Example: Frame rendering loop
     * @code
     * DenOfIz_Texture backBuffer;
     * if (DenOfIz_SwapChain_AcquireNextImage(swapChain, &backBuffer) != DENOFIZ_SUCCESS) {
     *     return; // skip the frame
     * }
     *
     * // ... render to backBuffer, submit ...
     *
     * DenOfIz_SwapChain_Present(swapChain);
     * @endcode
     *
     * @see DenOfIz_LogicalDevice_CreateSwapChain
     * @see DenOfIz_SwapChainDesc
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_SwapChain )

    /**
     * @brief How presented frames are synchronized with the display.
     */
    typedef enum
    {
        DENOFIZ_PRESENT_MODE_FIFO      = 0, /**< VSync, no tearing. Always supported. */
        DENOFIZ_PRESENT_MODE_MAILBOX   = 1, /**< No tearing, newest frame replaces a queued one. Falls back to FIFO. */
        DENOFIZ_PRESENT_MODE_IMMEDIATE = 2  /**< No VSync, may tear. Falls back to MAILBOX, then FIFO. */
    } DenOfIz_PresentMode;

    /**
     * @brief Color space for swap chain output.
     */
    typedef enum
    {
        DENOFIZ_COLOR_SPACE_SRGB,  /**< Standard sRGB color space (SDR). */
        DENOFIZ_COLOR_SPACE_HDR10, /**< HDR10 (ST.2084 PQ curve, Rec.2020 primaries). */
        DENOFIZ_COLOR_SPACE_SCRGB  /**< Linear scRGB (extended sRGB, for HDR on Windows). */
    } DenOfIz_ColorSpace;

    /**
     * @brief Viewport rectangle for rendering.
     */
    typedef struct
    {
        float X;      /**< Left edge of the viewport in pixels. */
        float Y;      /**< Top edge of the viewport in pixels. */
        float Width;  /**< Width of the viewport in pixels. */
        float Height; /**< Height of the viewport in pixels. */
    } DenOfIz_Viewport;

    /**
     * @brief Descriptor of a swap chain.
     *
     * @see DenOfIz_LogicalDevice_CreateSwapChain
     * @see DenOfIz_SwapChain_GetDesc
     */
    typedef struct
    {
        DenOfIz_GraphicsWindowHandle WindowHandle;      /**< Handle to the window surface. Required. */
        uint32_t                     Width;             /**< Current width in pixels. Set by the swap chain, ignored on creation. */
        uint32_t                     Height;            /**< Current height in pixels. Set by the swap chain, ignored on creation. */
        uint32_t                     NumBuffers;        /**< Number of back buffers (2 for double, 3 for triple buffering). */
        DenOfIz_Format               BackBufferFormat;  /**< Format of back buffer textures (e.g., DENOFIZ_FORMAT_B8G8R8A8_UNORM). */
        DenOfIz_Format               DepthBufferFormat; /**< Format of the swap chain depth buffer, or DENOFIZ_FORMAT_UNDEFINED for none. */
        DenOfIz_CommandQueue         CommandQueue;      /**< Queue used for presentation. Must support presentation. */
        uint32_t                     ImageUsages;       /**< Additional DenOfIz_ResourceUsageFlags for back buffers. */
        DenOfIz_PresentMode          PresentMode;       /**< Synchronization with the display. */
        bool                         EnableHDR;         /**< Enable HDR output if supported by display. */
        DenOfIz_ColorSpace           ColorSpace;        /**< Color space for output. */
    } DenOfIz_SwapChainDesc;

    /**
     * @brief Returns the preferred back buffer format for this swap chain.
     */
    DZ_API void DenOfIz_SwapChain_GetPreferredFormat( DenOfIz_SwapChain swapChain, DenOfIz_Format *outFormat );

    /**
     * @brief Acquires the back buffer to render this frame to.
     *
     * Resizes the swap chain first when its size no longer matches the window. Must be called on the thread that owns the window.
     *
     * @param[out] outBackBuffer Receives the back buffer. Owned by the swap chain and only valid until DenOfIz_SwapChain_Present().
     * @return DENOFIZ_SUCCESS when a back buffer was acquired. On any other value @p outBackBuffer is DENOFIZ_NULL_HANDLE and the frame must be
     * skipped (window minimized or occluded, device lost).
     */
    DZ_API DenOfIz_Result DenOfIz_SwapChain_AcquireNextImage( DenOfIz_SwapChain swapChain, DenOfIz_Texture *outBackBuffer );

    /**
     * @brief Presents the acquired back buffer. The back buffer must have been transitioned to DENOFIZ_RESOURCE_USAGE_PRESENT_BIT.
     *
     * @return DENOFIZ_SUCCESS, DENOFIZ_NOT_READY when no back buffer was acquired, or DENOFIZ_ERROR_DEVICE_LOST.
     */
    DZ_API DenOfIz_Result DenOfIz_SwapChain_Present( DenOfIz_SwapChain swapChain );

    /**
     * @brief Returns the depth buffer of the swap chain, DENOFIZ_NULL_HANDLE when DepthBufferFormat is DENOFIZ_FORMAT_UNDEFINED.
     *
     * Owned by the swap chain and replaced when the swap chain is resized: query it every frame.
     */
    DZ_API void DenOfIz_SwapChain_GetDepthBuffer( DenOfIz_SwapChain swapChain, DenOfIz_Texture *outDepthBuffer );

    /**
     * @brief Returns the current description of the swap chain, including its current size.
     */
    DZ_API void DenOfIz_SwapChain_GetDesc( DenOfIz_SwapChain swapChain, DenOfIz_SwapChainDesc *outDesc );

    /**
     * @brief Returns the viewport covering the whole swap chain. Valid until the swap chain is resized.
     */
    DZ_API void DenOfIz_SwapChain_GetViewport( DenOfIz_SwapChain swapChain, const DenOfIz_Viewport **outViewport );

    /**
     * @brief Destroys the swap chain. May be DENOFIZ_NULL_HANDLE.
     */
    DZ_API void DenOfIz_SwapChain_Destroy( DenOfIz_SwapChain swapChain );

#ifdef __cplusplus
}
#endif
