/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "ShaderData.h"
#include "Texture.h"

#ifdef __cplusplus
extern "C"
{
#endif
    DENOFIZ_DEFINE_HANDLE( DenOfIz_BindGroupLayout )

    typedef struct DenOfIz_BindGroupLayoutArray
    {
        DenOfIz_BindGroupLayout *Elements;
        uint32_t                 NumElements;
    } DenOfIz_BindGroupLayoutArray;

    typedef struct DenOfIz_BindingDesc
    {
        uint32_t                        Binding;
        DenOfIz_ResourceDescriptorFlags Descriptor;
        DenOfIz_ShaderStageFlags        Stages;
        uint32_t                        ArraySize;
        bool                            IsBindless;
    } DenOfIz_BindingDesc;

    typedef struct DenOfIz_BindingDescArray
    {
        DenOfIz_BindingDesc *Elements;
        uint32_t             NumElements;
    } DenOfIz_BindingDescArray;

    typedef struct DenOfIz_BindGroupLayoutDesc
    {
        uint32_t                 RegisterSpace;
        DenOfIz_BindingDescArray Bindings;
    } DenOfIz_BindGroupLayoutDesc;

    typedef struct DenOfIz_BindGroupLayoutDescArray
    {
        DenOfIz_BindGroupLayoutDesc *Elements;
        uint32_t                     NumElements;
    } DenOfIz_BindGroupLayoutDescArray;

    DZ_API void                               DenOfIz_BindGroupLayout_Destroy( DenOfIz_BindGroupLayout bindGroupLayout );
    DZ_API const DenOfIz_BindGroupLayoutDesc *DenOfIz_BindGroupLayout_GetDesc( DenOfIz_BindGroupLayout bindGroupLayout );
#ifdef __cplusplus
}
#endif
