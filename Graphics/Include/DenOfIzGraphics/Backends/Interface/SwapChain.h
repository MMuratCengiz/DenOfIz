/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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

    /** Always has the pixel size of its window: it recreates itself when the window changes, there is no resize call. */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_SwapChain )

    typedef enum
    {
        DENOFIZ_PRESENT_MODE_FIFO      = 0, /**< VSync, no tearing. Always supported. */
        DENOFIZ_PRESENT_MODE_MAILBOX   = 1, /**< No tearing, newest frame replaces a queued one. Falls back to FIFO. Metal presents it like IMMEDIATE. */
        DENOFIZ_PRESENT_MODE_IMMEDIATE = 2  /**< No VSync, may tear. Falls back to MAILBOX, then FIFO. */
    } DenOfIz_PresentMode;

    typedef enum
    {
        DENOFIZ_COLOR_SPACE_SRGB,
        DENOFIZ_COLOR_SPACE_HDR10, /**< ST.2084 PQ, Rec.2020 primaries. */
        DENOFIZ_COLOR_SPACE_SCRGB  /**< Linear, Rec.709 primaries. */
    } DenOfIz_ColorSpace;

    /** Pixels. */
    typedef struct
    {
        float X;
        float Y;
        float Width;
        float Height;
    } DenOfIz_Viewport;

    typedef struct
    {
        DenOfIz_GraphicsWindowHandle WindowHandle;
        uint32_t                     Width;  /**< Current width in pixels. Set by the swap chain, ignored on creation. */
        uint32_t                     Height; /**< Current height in pixels. Set by the swap chain, ignored on creation. */
        uint32_t                     NumBuffers;
        DenOfIz_Format               BackBufferFormat;
        DenOfIz_Format               DepthBufferFormat; /**< UNDEFINED is no depth buffer. */
        DenOfIz_CommandQueue         CommandQueue;      /**< Queue the swap chain presents on. */
        uint32_t                     ImageUsages;       /**< Extra DenOfIz_ResourceUsageFlags for the back buffers, used by Vulkan. */
        DenOfIz_PresentMode          PresentMode;
        bool                         EnableHDR;  /**< DX12 only, applied when the display is HDR10 capable. */
        DenOfIz_ColorSpace           ColorSpace; /**< Used with EnableHDR. */
    } DenOfIz_SwapChainDesc;

    DZ_API void DenOfIz_SwapChain_GetPreferredFormat( DenOfIz_SwapChain swapChain, DenOfIz_Format *outFormat );
    /**
     * Resizes the swap chain first when its size no longer matches the window. Must be called on the thread that owns the window. The
     * back buffer is owned by the swap chain and only valid until Present. On any result other than DENOFIZ_SUCCESS outBackBuffer is
     * DENOFIZ_NULL_HANDLE and the frame must be skipped (window minimized or occluded, device lost).
     */
    DZ_API DenOfIz_Result DenOfIz_SwapChain_AcquireNextImage( DenOfIz_SwapChain swapChain, DenOfIz_Texture *outBackBuffer );
    /**
     * Presents the acquired back buffer, which must be in DENOFIZ_RESOURCE_USAGE_PRESENT_BIT. Also the frame boundary for deferred
     * destruction, see DenOfIz_LogicalDevice_ReleaseDestroyedResources. Returns DENOFIZ_NOT_READY when no back buffer was acquired.
     */
    DZ_API DenOfIz_Result DenOfIz_SwapChain_Present( DenOfIz_SwapChain swapChain );
    /** NULL handle when DepthBufferFormat is UNDEFINED. Owned by the swap chain and replaced when it is resized: query it every frame. */
    DZ_API void DenOfIz_SwapChain_GetDepthBuffer( DenOfIz_SwapChain swapChain, DenOfIz_Texture *outDepthBuffer );
    DZ_API void DenOfIz_SwapChain_GetDesc( DenOfIz_SwapChain swapChain, DenOfIz_SwapChainDesc *outDesc );
    /** The whole swap chain. The pointer lives as long as the swap chain and its contents follow resizes. */
    DZ_API void DenOfIz_SwapChain_GetViewport( DenOfIz_SwapChain swapChain, const DenOfIz_Viewport **outViewport );
    /** Destroyed immediately. DX12 waits for its queue and Vulkan for the device to go idle first. */
    DZ_API void DenOfIz_SwapChain_Destroy( DenOfIz_SwapChain swapChain );

#ifdef __cplusplus
}
#endif
