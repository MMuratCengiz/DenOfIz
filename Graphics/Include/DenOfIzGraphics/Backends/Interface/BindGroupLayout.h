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
