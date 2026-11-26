/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "BindGroupLayout.h"
#include "Buffer.h"
#include "DenOfIzGraphics/Handle.h"
#include "RayTracing/TopLevelAS.h"
#include "Texture.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Shader resources of one register space, laid out by a DenOfIz_BindGroupLayout and bound with DenOfIz_CommandList_BindGroup.
     *
     * Bind resources between BeginUpdate and EndUpdate. binding is the register number (b, t, u or s) of a DenOfIz_BindingDesc in the
     * layout. Metal keeps only the bindings of the latest update, so bind everything the shaders use in each update. Updates overwrite the
     * group in place: don't update a group used by command lists the GPU has not finished.
     *
     * Cbv needs DENOFIZ_BUFFER_USAGE_UNIFORM_BIT, a UAV buffer DENOFIZ_BUFFER_USAGE_STORAGE_BIT, an SRV texture
     * DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT and a UAV texture DENOFIZ_TEXTURE_USAGE_STORAGE_BINDING_BIT.
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_BindGroup )

    typedef struct DenOfIz_BindGroupDesc
    {
        DenOfIz_BindGroupLayout Layout;
    } DenOfIz_BindGroupDesc;

    typedef struct DenOfIz_BindBufferDesc
    {
        uint32_t       Binding;
        DenOfIz_Buffer Resource;
        size_t         ResourceOffset; /**< Bytes. For CbvWithDesc a multiple of DenOfIz_DeviceConstants.ConstantBufferAlignment. */
    } DenOfIz_BindBufferDesc;

    DZ_API void DenOfIz_BindGroup_BeginUpdate( DenOfIz_BindGroup bindGroup );
    DZ_API void DenOfIz_BindGroup_Cbv( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Buffer resource );
    DZ_API void DenOfIz_BindGroup_CbvWithDesc( DenOfIz_BindGroup bindGroup, const DenOfIz_BindBufferDesc *desc );
    DZ_API void DenOfIz_BindGroup_SrvBuffer( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Buffer resource );
    DZ_API void DenOfIz_BindGroup_SrvBufferWithDesc( DenOfIz_BindGroup bindGroup, const DenOfIz_BindBufferDesc *desc );
    DZ_API void DenOfIz_BindGroup_SrvTopLevelAS( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_TopLevelAS accelerationStructure );
    DZ_API void DenOfIz_BindGroup_SrvTexture( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Texture resource );
    DZ_API void DenOfIz_BindGroup_SrvTextureMip( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Texture resource, uint32_t mipLevel );

    /** Binds resources to elements 0 to NumElements - 1 of the array at binding. */
    DZ_API void DenOfIz_BindGroup_SrvArray( DenOfIz_BindGroup bindGroup, uint32_t binding, const DenOfIz_TextureArray *resources );

    /** outBindGroup is not written and may be NULL. */
    DZ_API void DenOfIz_BindGroup_SrvArrayIndex( DenOfIz_BindGroup bindGroup, uint32_t binding, uint32_t arrayIndex, DenOfIz_Texture resource, DenOfIz_BindGroup *outBindGroup );
    DZ_API void DenOfIz_BindGroup_UavBuffer( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Buffer resource );
    DZ_API void DenOfIz_BindGroup_UavBufferWithDesc( DenOfIz_BindGroup bindGroup, const DenOfIz_BindBufferDesc *desc );
    DZ_API void DenOfIz_BindGroup_UavTexture( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Texture resource );
    DZ_API void DenOfIz_BindGroup_UavTextureMip( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Texture resource, uint32_t mipLevel );
    DZ_API void DenOfIz_BindGroup_Sampler( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Sampler sampler );
    DZ_API void DenOfIz_BindGroup_EndUpdate( DenOfIz_BindGroup bindGroup );

    /** Safe while command lists using it are in flight, see DenOfIz_LogicalDevice_ReleaseDestroyedResources. */
    DZ_API void DenOfIz_BindGroup_Destroy( DenOfIz_BindGroup bindGroup );

#ifdef __cplusplus
}
#endif
