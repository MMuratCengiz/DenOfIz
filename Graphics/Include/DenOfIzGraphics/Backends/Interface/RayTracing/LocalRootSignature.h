/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/RootSignature.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_LocalResourceBindingDesc
    {
        uint32_t                        Binding;
        uint32_t                        RegisterSpace;
        DenOfIz_ResourceDescriptorFlags Descriptor;
        DenOfIz_ShaderStageFlags        Stages;
        int                             ArraySize;
        uint64_t                        NumBytes; /**< Size of a constant buffer binding, its data is stored inline in the shader record. */
    } DenOfIz_LocalResourceBindingDesc;

    typedef struct DenOfIz_LocalResourceBindingDescArray
    {
        DenOfIz_LocalResourceBindingDesc *Elements;
        uint32_t                          NumElements;
    } DenOfIz_LocalResourceBindingDescArray;

    /**
     * Layout of the per shader record resources of a ray tracing shader, filled through DenOfIz_ShaderLocalData. Reflected per stage into
     * DenOfIz_ShaderReflectDesc::LocalRootSignatures, see DenOfIz_RayTracingShaderDesc::LocalBindings.
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_LocalRootSignature )

    typedef struct DenOfIz_LocalRootSignatureDesc
    {
        DenOfIz_LocalResourceBindingDescArray ResourceBindings;
    } DenOfIz_LocalRootSignatureDesc;

    typedef struct DenOfIz_LocalRootSignatureDescArray
    {
        DenOfIz_LocalRootSignatureDesc *Elements;
        uint32_t                        NumElements;
    } DenOfIz_LocalRootSignatureDescArray;

    typedef struct DenOfIz_LocalRootSignatureArray
    {
        DenOfIz_LocalRootSignature *Elements;
        size_t                      NumElements;
    } DenOfIz_LocalRootSignatureArray;

    /**
     * Release is deferred until the GPU is done with it, see DenOfIz_LogicalDevice_ReleaseDestroyedResources. ShaderLocalData objects
     * created with it reference it and must be destroyed first.
     */
    DZ_API void DenOfIz_LocalRootSignature_Destroy( DenOfIz_LocalRootSignature localRootSignature );

#ifdef __cplusplus
}
#endif
