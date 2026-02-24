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
#include "Texture.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Swap chain handle for presenting rendered frames to a window surface.
     *
     * A swap chain manages a collection of back buffers used for double or triple buffering.
     * It synchronizes frame presentation with the display's refresh rate and handles the
     * buffer rotation for smooth rendering.
     *
     * @par Backend implementations
     * - DirectX 12: IDXGISwapChain4
     * - Vulkan: VkSwapchainKHR + VkSurfaceKHR
     * - Metal: CAMetalLayer + MTKView
     * - WebGPU: WGPUSurface
     *
     * @par Typical usage
     * 1. Create swap chain via DenOfIz_LogicalDevice_CreateSwapChain()
     * 2. Each frame: acquire image, render to it, present
     * 3. Handle resize events by calling DenOfIz_SwapChain_Resize()
     * 4. Use DenOfIz_SwapChain_GetRenderTarget() to get textures for rendering
     *
     * @par Example: Frame rendering loop
     * @code
     * uint32_t imageIndex;
     * DenOfIz_SwapChain_AcquireNextImage(swapChain, &imageIndex);
     *
     * DenOfIz_Texture renderTarget;
     * DenOfIz_SwapChain_GetRenderTarget(swapChain, imageIndex, &renderTarget);
     *
     * // ... render to renderTarget ...
     *
     * DenOfIz_PresentResult result = DenOfIz_SwapChain_Present(swapChain, imageIndex);
     * if (result == DENOFIZ_PRESENT_RESULT_SUBOPTIMAL) {
     *     DenOfIz_SwapChain_Resize(swapChain, newWidth, newHeight);
     * }
     * @endcode
     *
     * @see DenOfIz_LogicalDevice_CreateSwapChain
     * @see DenOfIz_SwapChainDesc
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_SwapChain )

    /**
     * @brief Result codes returned by DenOfIz_SwapChain_Present().
     */
    typedef enum
    {
        DENOFIZ_PRESENT_RESULT_ERROR       = -1, /**< Presentation failed due to an error. */
        DENOFIZ_PRESENT_RESULT_SUCCESS     = 0,  /**< Presentation succeeded. */
        DENOFIZ_PRESENT_RESULT_TIMEOUT     = 1,  /**< Presentation timed out waiting for GPU. */
        DENOFIZ_PRESENT_RESULT_DEVICE_LOST = 2,  /**< GPU device was lost; swap chain must be recreated. */
        DENOFIZ_PRESENT_RESULT_SUBOPTIMAL  = 3   /**< Swap chain is out of date (e.g., window resized); call Resize(). */
    } DenOfIz_PresentResult;

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
     * @brief Descriptor for creating a swap chain.
     *
     * @see DenOfIz_LogicalDevice_CreateSwapChain
     */
    typedef struct
    {
        DenOfIz_GraphicsWindowHandle WindowHandle;      /**< Handle to the window surface. Required. */
        uint32_t                     Width;             /**< Initial width in pixels. Must be > 0. */
        uint32_t                     Height;            /**< Initial height in pixels. Must be > 0. */
        uint32_t                     NumBuffers;        /**< Number of back buffers (2 for double, 3 for triple buffering). */
        DenOfIz_Format               BackBufferFormat;  /**< Format of back buffer textures (e.g., DENOFIZ_FORMAT_B8G8R8A8_UNORM). */
        DenOfIz_Format               DepthBufferFormat; /**< Format for depth buffer, or DENOFIZ_FORMAT_UNDEFINED for none. */
        DenOfIz_CommandQueue         CommandQueue;      /**< Queue used for presentation. Must support presentation. */
        uint32_t                     ImageUsages;       /**< Additional DenOfIz_ResourceUsageFlags for back buffers. */
        bool                         AllowTearing;      /**< Enable variable refresh rate (VRR/FreeSync/G-Sync). */
        bool                         EnableHDR;         /**< Enable HDR output if supported by display. */
        DenOfIz_ColorSpace           ColorSpace;        /**< Color space for output. */
        DenOfIz_MSAASampleCount      SampleCount;       /**< MSAA sample count (DENOFIZ_MSAA_SAMPLE_COUNT_1 for no MSAA). */
    } DenOfIz_SwapChainDesc;

    /**
     * @brief Returns the preferred back buffer format for this swap chain.
     *
     * @param swapChain Valid swap chain handle.
     * @param[out] outFormat Receives the preferred format.
     *
     * @par Valid Usage
     * - @p swapChain must be a valid handle created by DenOfIz_LogicalDevice_CreateSwapChain()
     * - @p outFormat must not be NULL
     */
    DZ_API void DenOfIz_SwapChain_GetPreferredFormat( DenOfIz_SwapChain swapChain, DenOfIz_Format *outFormat );

    /**
     * @brief Acquires the next available back buffer image for rendering.
     *
     * Call this at the start of each frame before rendering. The returned index identifies
     * which back buffer to render to and must be passed to DenOfIz_SwapChain_Present().
     *
     * @param swapChain Valid swap chain handle.
     * @param[out] outImageIndex Receives the index of the acquired image (0 to NumBuffers-1).
     *
     * @par Valid Usage
     * - @p swapChain must be a valid handle
     * - @p outImageIndex must not be NULL
     * - Previous frame's work on the acquired buffer should be complete (use fences/semaphores)
     *
     * @see DenOfIz_SwapChain_Present
     */
    DZ_API void DenOfIz_SwapChain_AcquireNextImage( DenOfIz_SwapChain swapChain, uint32_t *outImageIndex );

    /**
     * @brief Presents the rendered image to the display.
     *
     * Call this after submitting rendering commands for the frame. On Vulkan, this operation
     * uses internal semaphores to synchronize with the GPU.
     *
     * @param swapChain Valid swap chain handle.
     * @param imageIndex Index of the image to present (from AcquireNextImage).
     * @return Result code indicating success or type of failure.
     * @retval DENOFIZ_PRESENT_RESULT_SUCCESS Presentation succeeded.
     * @retval DENOFIZ_PRESENT_RESULT_SUBOPTIMAL Swap chain is out of date; call Resize().
     * @retval DENOFIZ_PRESENT_RESULT_DEVICE_LOST GPU device lost; recreate swap chain.
     * @retval DENOFIZ_PRESENT_RESULT_TIMEOUT Presentation timed out.
     * @retval DENOFIZ_PRESENT_RESULT_ERROR Presentation failed.
     *
     * @par Valid Usage
     * - @p swapChain must be a valid handle
     * - @p imageIndex must be a value returned by DenOfIz_SwapChain_AcquireNextImage()
     * - Rendering commands for @p imageIndex must have been submitted to the GPU
     */
    DZ_API DenOfIz_PresentResult DenOfIz_SwapChain_Present( DenOfIz_SwapChain swapChain, uint32_t imageIndex );

    /**
     * @brief Resizes the swap chain back buffers.
     *
     * Call this when the window size changes or when Present() returns SUBOPTIMAL.
     * All command lists referencing the old back buffers must have completed execution
     * before calling this function.
     *
     * @param swapChain Valid swap chain handle.
     * @param width New width in pixels. Must be > 0.
     * @param height New height in pixels. Must be > 0.
     *
     * @par Valid Usage
     * - @p swapChain must be a valid handle
     * - GPU must be idle or all references to back buffers must be released
     * - @p width and @p height must be > 0
     *
     * @note After resize, render targets from GetRenderTarget() are invalidated and must be re-acquired.
     * @note Ensure you wait until device is until before resizing, gracefully manage PRESENT as it might return errors during resize.
     */
    DZ_API void DenOfIz_SwapChain_Resize( DenOfIz_SwapChain swapChain, uint32_t width, uint32_t height );

    /**
     * @brief Returns the texture resource for a swap chain back buffer.
     *
     * The returned texture can be used as a render target. The handle is owned by the
     * swap chain and must not be destroyed by the caller.
     *
     * @param swapChain Valid swap chain handle.
     * @param image Index of the back buffer (0 to NumBuffers-1).
     * @param[out] outRenderTarget Receives the texture handle.
     *
     * @par Valid Usage
     * - @p swapChain must be a valid handle
     * - @p image must be < NumBuffers specified in DenOfIz_SwapChainDesc
     * - @p outRenderTarget must not be NULL
     *
     * @warning Do not call DenOfIz_TextureResource_Destroy() on the returned handle.
     */
    DZ_API void DenOfIz_SwapChain_GetRenderTarget( DenOfIz_SwapChain swapChain, uint32_t image, DenOfIz_Texture *outRenderTarget );

    /**
     * @brief Returns the current viewport matching the swap chain dimensions.
     *
     * @param swapChain Valid swap chain handle.
     * @param[out] outViewport Receives pointer to the viewport. Valid until next Resize().
     *
     * @par Valid Usage
     * - @p swapChain must be a valid handle
     * - @p outViewport must not be NULL
     */
    DZ_API void DenOfIz_SwapChain_GetViewport( DenOfIz_SwapChain swapChain, const DenOfIz_Viewport **outViewport );

    /**
     * @brief Destroys the swap chain and releases all resources.
     *
     * @param swapChain Swap chain handle to destroy. May be DENOFIZ_NULL_HANDLE.
     *
     * @par Valid Usage
     * - All command lists referencing the swap chain's back buffers must have completed
     * - After this call, @p swapChain is invalid and must not be used
     */
    DZ_API void DenOfIz_SwapChain_Destroy( DenOfIz_SwapChain swapChain );

#ifdef __cplusplus
}
#endif
