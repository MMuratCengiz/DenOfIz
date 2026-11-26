/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "CommonData.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_SamplerDesc
    {
        DenOfIz_Filter             MagFilter;
        DenOfIz_Filter             MinFilter;
        DenOfIz_SamplerAddressMode AddressModeU;
        DenOfIz_SamplerAddressMode AddressModeV;
        DenOfIz_SamplerAddressMode AddressModeW;
        float                      MaxAnisotropy; /**< Zero disables anisotropic filtering. */
        DenOfIz_CompareOp          CompareOp;     /**< Comparison sampler unless NEVER (zero). */
        DenOfIz_MipmapMode         MipmapMode;
        float                      MipLodBias;
        float                      MinLod;
        float                      MaxLod; /**< Zero samples mip 0 only. */
        DenOfIz_StringView         DebugName;
    } DenOfIz_SamplerDesc;

    /**
     * Width, ArraySize and MipLevels must be at least 1, Format must not be UNDEFINED. Height 0 is a 1D texture, Depth above 1 a 3D
     * texture (no array, no MSAA). CUBE_BIT requires Width == Height and an ArraySize that is a multiple of 6. Multisampled textures have
     * one mip level and no STORAGE_BINDING. Block compressed formats require Width and Height to be multiples of 4 and allow no
     * STORAGE_BINDING.
     */
    typedef struct DenOfIz_TextureDesc
    {
        uint32_t                  Width;
        uint32_t                  Height;
        uint32_t                  Depth;
        uint32_t                  ArraySize;
        uint32_t                  MipLevels;
        DenOfIz_Format            Format;
        DenOfIz_TextureUsageFlags Usage;
        DenOfIz_MSAASampleCount   MSAASampleCount;
        DenOfIz_HeapType          HeapType;
        DenOfIz_SamplerDesc       Sampler;
        DenOfIz_StringView        DebugName;
        DenOfIz_Float4            ClearColorHint;        /**< Optimized clear value of DX12 render targets. */
        DenOfIz_Float2            ClearDepthStencilHint; /**< Optimized clear value of DX12 depth targets, X depth, Y stencil. */
    } DenOfIz_TextureDesc;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_Texture )
    DENOFIZ_DEFINE_HANDLE( DenOfIz_Sampler )

    typedef struct DenOfIz_TextureArray
    {
        DenOfIz_Texture *Elements;
        size_t           NumElements;
    } DenOfIz_TextureArray;

    DZ_API void DenOfIz_Texture_GetFormat( DenOfIz_Texture texture, DenOfIz_Format *outFormat );
    DZ_API void DenOfIz_Texture_GetDesc( DenOfIz_Texture texture, DenOfIz_TextureDesc *outDesc );
    /**
     * Release is deferred until the GPU is done with the texture, see DenOfIz_LogicalDevice_ReleaseDestroyedResources. Never destroy a
     * swap chain's textures.
     */
    DZ_API void DenOfIz_Texture_Destroy( DenOfIz_Texture texture );
    /** Release is deferred until the GPU is done with the sampler, see DenOfIz_LogicalDevice_ReleaseDestroyedResources. */
    DZ_API void DenOfIz_Sampler_Destroy( DenOfIz_Sampler sampler );

#ifdef __cplusplus
}
#endif
