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

#include "CommonData.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Descriptor for creating a texture sampler.
     *
     * Samplers define how textures are filtered and addressed when sampled in shaders.
     *
     * @see DenOfIz_LogicalDevice_CreateSampler
     */
    typedef struct DenOfIz_SamplerDesc
    {
        DenOfIz_Filter             MagFilter;     /**< Magnification filter (when texture is larger than screen pixels). */
        DenOfIz_Filter             MinFilter;     /**< Minification filter (when texture is smaller than screen pixels). */
        DenOfIz_SamplerAddressMode AddressModeU;  /**< Addressing mode for U (horizontal) texture coordinate. */
        DenOfIz_SamplerAddressMode AddressModeV;  /**< Addressing mode for V (vertical) texture coordinate. */
        DenOfIz_SamplerAddressMode AddressModeW;  /**< Addressing mode for W (depth) texture coordinate. */
        float                      MaxAnisotropy; /**< Maximum anisotropy level (1.0 = disabled, 16.0 = max quality). */
        DenOfIz_CompareOp          CompareOp;     /**< Comparison operation for shadow/depth sampling. Use LESS/GREATER etc. to enable. NEVER(0) or ALWAYS disables comparison. */
        DenOfIz_MipmapMode         MipmapMode;    /**< Mipmap filtering mode. */
        float                      MipLodBias;    /**< Bias added to mipmap LOD calculation. */
        float                      MinLod;        /**< Minimum mipmap level to use. */
        float                      MaxLod;        /**< Maximum mipmap level to use. */
        DenOfIz_StringView         DebugName;     /**< Debug name for graphics debuggers. */
    } DenOfIz_SamplerDesc;

    /**
     * @brief Descriptor for creating a texture resource.
     *
     * @par Validation Rules
     * - Width, Height, Depth must be > 0
     * - ArraySize must be >= 1
     * - MipLevels must be >= 1
     * - Format must not be DENOFIZ_FORMAT_UNDEFINED
     * - Cube textures (CUBE_BIT): ArraySize must be a multiple of 6, Width == Height
     * - 3D textures (Depth > 1): ArraySize must be 1, no MSAA
     * - MSAA textures: MipLevels must be 1
     * - Block-compressed formats: Width/Height must be multiples of 4
     *
     * @par Usage flags (DenOfIz_TextureUsageFlags)
     * - TEXTURE_BINDING_BIT: Can be sampled in shaders
     * - STORAGE_BINDING_BIT: Can be used as UAV/storage image (read-write)
     * - RENDER_ATTACHMENT_BIT: Can be used as render target or depth-stencil (format determines which)
     * - COPY_SRC_BIT/COPY_DST_BIT: Can be source/destination of copy operations
     * - CUBE_BIT: Texture is a cubemap (ArraySize must be 6*N)
     *
     * @note Texture aspect (color/depth/stencil) is automatically derived from Format.
     *
     * @see DenOfIz_LogicalDevice_CreateTexture
     */
    typedef struct DenOfIz_TextureDesc
    {
        uint32_t                  Width;                 /**< Texture width in pixels. Must be > 0. */
        uint32_t                  Height;                /**< Texture height in pixels. Must be > 0. */
        uint32_t                  Depth;                 /**< Texture depth (1 for 2D, >1 for 3D textures). */
        uint32_t                  ArraySize;             /**< Array size (1 for non-array, 6*N for cube maps). */
        uint32_t                  MipLevels;             /**< Number of mip levels. Must be >= 1. */
        DenOfIz_Format            Format;                /**< Pixel format. Must not be UNDEFINED. */
        DenOfIz_TextureUsageFlags Usage;                 /**< Texture usage flags (TEXTURE_BINDING, STORAGE_BINDING, RENDER_ATTACHMENT, etc.). */
        DenOfIz_MSAASampleCount   MSAASampleCount;       /**< MSAA sample count (1 = no MSAA). */
        DenOfIz_HeapType          HeapType;              /**< Memory heap type (GPU, CPU_GPU, etc.). */
        DenOfIz_SamplerDesc       Sampler;               /**< Embedded sampler (optional, for combined sampler/texture). */
        DenOfIz_StringView        DebugName;             /**< Debug name for graphics debuggers. */
        DenOfIz_Float4            ClearColorHint;        /**< Optimized clear color for render targets. */
        DenOfIz_Float2            ClearDepthStencilHint; /**< Optimized clear value (X=depth, Y=stencil). */
    } DenOfIz_TextureDesc;

    /**
     * @brief Texture resource handle for GPU image data.
     *
     * Textures store image data that can be sampled in shaders, used as render targets,
     * or accessed via unordered access views. Supports 1D, 2D, 3D textures, arrays, and cube maps.
     *
     * @par Backend implementations
     * - DirectX 12: ID3D12Resource2 with D3D12_RESOURCE_DESC
     * - Vulkan: VkImage + VkImageView + VmaAllocation
     * - Metal: id<MTLTexture>
     * - WebGPU: WGPUTexture + DenOfIz_WGPUTextureView
     *
     * @par Resource state tracking
     * Textures must be in the correct state before use. Use DenOfIz_ResourceTracking
     * or manual barriers to transition states:
     * - DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT: For sampling in shaders
     * - DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT: For use as color attachment
     * - DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT: For use as depth attachment
     * - DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT / COPY_DST_BIT: For copy operations
     *
     * @see DenOfIz_LogicalDevice_CreateTexture
     * @see DenOfIz_TextureDesc
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_Texture )

    /**
     * @brief Texture sampler handle.
     *
     * Samplers define texture filtering and addressing. They are separate from textures
     * and can be reused across multiple textures.
     *
     * @par Backend implementations
     * - DirectX 12: D3D12_SAMPLER_DESC (in sampler descriptor heap)
     * - Vulkan: VkSampler
     * - Metal: id<MTLSamplerState>
     * - WebGPU: WGPUSampler
     *
     * @see DenOfIz_LogicalDevice_CreateSampler
     * @see DenOfIz_SamplerDesc
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_Sampler )

    /**
     * @brief Array of texture resource handles.
     */
    typedef struct DenOfIz_TextureArray
    {
        DenOfIz_Texture *Elements;    /**< Pointer to array of texture handles. */
        size_t           NumElements; /**< Number of elements in the array. */
    } DenOfIz_TextureArray;

    /**
     * @brief Returns the pixel format of the texture.
     *
     * @param texture Valid texture handle.
     * @param[out] outFormat Receives the texture format.
     *
     * @par Valid Usage
     * - @p texture must be a valid handle
     * - @p outFormat must not be NULL
     */
    DZ_API void DenOfIz_TextureResource_GetFormat( DenOfIz_Texture texture, DenOfIz_Format *outFormat );

    /**
     * @brief Destroys the texture and releases GPU memory.
     *
     * @param texture Texture handle to destroy. May be DENOFIZ_NULL_HANDLE.
     *
     * @par Valid Usage
     * - All command lists referencing this texture must have completed
     * - After this call, @p texture is invalid and must not be used
     *
     * @warning Do not destroy textures obtained from DenOfIz_SwapChain_GetRenderTarget().
     */
    DZ_API void DenOfIz_TextureResource_Destroy( DenOfIz_Texture texture );

    /**
     * @brief Destroys the sampler.
     *
     * @param sampler Sampler handle to destroy. May be DENOFIZ_NULL_HANDLE.
     *
     * @par Valid Usage
     * - All command lists referencing this sampler must have completed
     * - After this call, @p sampler is invalid and must not be used
     */
    DZ_API void DenOfIz_Sampler_Destroy( DenOfIz_Sampler sampler );

#ifdef __cplusplus
}
#endif
