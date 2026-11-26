/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Backends/Interface/Texture.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "LocalRootSignature.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Resources of one shader record, laid out by a DenOfIz_LocalRootSignature. Set every binding between Begin and End (Metal forgets
     * the previous ones at Begin), then bind it into a shader binding table: the table copies the record as it is, rebind after changing
     * it. Do not change it while the GPU uses it. Bindings are the shader register numbers.
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_ShaderLocalData )

    typedef struct
    {
        DenOfIz_LocalRootSignature Layout; /**< Must outlive the local data. */
    } DenOfIz_ShaderLocalDataDesc;

    DZ_API void DenOfIz_ShaderLocalData_Begin( DenOfIz_ShaderLocalData localData );
    DZ_API void DenOfIz_ShaderLocalData_CbvBuffer( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Buffer bufferResource );
    /** Copies constant data inline into the record. data must be exactly the binding's NumBytes. */
    DZ_API void DenOfIz_ShaderLocalData_CbvData( DenOfIz_ShaderLocalData localData, uint32_t binding, const DenOfIz_ByteArrayView *data );
    DZ_API void DenOfIz_ShaderLocalData_SrvBuffer( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Buffer bufferResource );
    DZ_API void DenOfIz_ShaderLocalData_SrvTexture( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Texture textureResource );
    DZ_API void DenOfIz_ShaderLocalData_UavBuffer( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Buffer bufferResource );
    DZ_API void DenOfIz_ShaderLocalData_UavTexture( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Texture textureResource );
    DZ_API void DenOfIz_ShaderLocalData_Sampler( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Sampler sampler );
    DZ_API void DenOfIz_ShaderLocalData_End( DenOfIz_ShaderLocalData localData );
    /**
     * Release is deferred until the GPU is done with it, see DenOfIz_LogicalDevice_ReleaseDestroyedResources. Shader binding tables it was
     * bound into still reference its descriptors and must not be used afterwards.
     */
    DZ_API void DenOfIz_ShaderLocalData_Destroy( DenOfIz_ShaderLocalData localData );

#ifdef __cplusplus
}
#endif
