/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "BindGroupLayout.h"
#include "DenOfIzGraphics/Handle.h"
#include "ShaderData.h"
#include "Texture.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_RootSignature )

    typedef enum DenOfIz_RootSignatureType
    {
        DENOFIZ_ROOT_SIGNATURE_TYPE_GRAPHICS,
        DENOFIZ_ROOT_SIGNATURE_TYPE_COMPUTE
    } DenOfIz_RootSignatureType;

    typedef struct DenOfIz_RootConstantBindingDesc
    {
        DenOfIz_StringView       Name;
        uint32_t                 Binding;
        uint64_t                 NumBytes;
        DenOfIz_ShaderStageFlags Stages;
    } DenOfIz_RootConstantBindingDesc;

    typedef struct DenOfIz_RootConstantBindingDescArray
    {
        DenOfIz_RootConstantBindingDesc *Elements;
        uint32_t                         NumElements;
    } DenOfIz_RootConstantBindingDescArray;

    typedef struct DenOfIz_RootSignatureDesc
    {
        DenOfIz_BindGroupLayoutArray         BindGroupLayouts;
        DenOfIz_RootConstantBindingDescArray RootConstants;
    } DenOfIz_RootSignatureDesc;

    DZ_API void DenOfIz_RootSignature_Destroy( DenOfIz_RootSignature rootSignature );

#ifdef __cplusplus
}
#endif
