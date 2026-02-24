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
