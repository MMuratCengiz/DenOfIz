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

#include "DenOfIzGraphics/Assets/Serde/Asset.h"
#include "DenOfIzGraphics/Assets/Shaders/ShaderReflectDesc.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_ShaderStageAsset
    {
        DenOfIz_ShaderStageFlags     Stage;
        DenOfIz_StringView           EntryPoint;
        DenOfIz_ByteArray            DXIL;
        DenOfIz_ByteArray            MSL;
        DenOfIz_ByteArray            SPIRV;
        DenOfIz_ByteArray            WGSL;
        DenOfIz_ByteArray            Reflection;
        DenOfIz_RayTracingShaderDesc RayTracing;
    } DenOfIz_ShaderStageAsset;

    typedef struct DenOfIz_ShaderStageAssetArray
    {
        DenOfIz_ShaderStageAsset *Elements;
        uint32_t                  NumElements;
    } DenOfIz_ShaderStageAssetArray;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_ShaderAsset )

    DZ_API DenOfIz_ShaderAsset           DenOfIz_ShaderAsset_Create( );
    DZ_API void                          DenOfIz_ShaderAsset_Destroy( DenOfIz_ShaderAsset shaderAsset );
    DZ_API DenOfIz_AssetHeader           DenOfIz_ShaderAsset_Header( DenOfIz_ShaderAsset shaderAsset );
    DZ_API uint64_t                      DenOfIz_ShaderAsset_Magic( DenOfIz_ShaderAsset shaderAsset );
    DZ_API uint32_t                      DenOfIz_ShaderAsset_Version( DenOfIz_ShaderAsset shaderAsset );
    DZ_API uint64_t                      DenOfIz_ShaderAsset_NumBytes( DenOfIz_ShaderAsset shaderAsset );
    DZ_API DenOfIz_StringView            DenOfIz_ShaderAsset_Path( DenOfIz_ShaderAsset shaderAsset );
    DZ_API size_t                        DenOfIz_ShaderAsset_NumStages( DenOfIz_ShaderAsset shaderAsset );
    DZ_API DenOfIz_ShaderStageAsset     *DenOfIz_ShaderAsset_GetStage( DenOfIz_ShaderAsset shaderAsset, size_t index );
    DZ_API DenOfIz_ShaderStageAsset     *DenOfIz_ShaderAsset_AddStage( DenOfIz_ShaderAsset shaderAsset );
    DZ_API void                          DenOfIz_ShaderAsset_SetStageEntryPoint( DenOfIz_ShaderAsset shaderAsset, size_t index, DenOfIz_StringView entryPoint );
    DZ_API void                          DenOfIz_ShaderAsset_ReserveStages( DenOfIz_ShaderAsset shaderAsset, size_t capacity );
    DZ_API void                          DenOfIz_ShaderAsset_ClearStages( DenOfIz_ShaderAsset shaderAsset );
    DZ_API DenOfIz_ShaderReflectDesc    *DenOfIz_ShaderAsset_GetReflectDesc( DenOfIz_ShaderAsset shaderAsset );
    DZ_API void                          DenOfIz_ShaderAsset_SetReflectDesc( DenOfIz_ShaderAsset shaderAsset, const DenOfIz_ShaderReflectDesc *desc );
    DZ_API DenOfIz_ShaderRayTracingDesc *DenOfIz_ShaderAsset_GetRayTracing( DenOfIz_ShaderAsset shaderAsset );
    DZ_API void                          DenOfIz_ShaderAsset_SetVersion( DenOfIz_ShaderAsset shaderAsset, uint32_t version );
    DZ_API void                          DenOfIz_ShaderAsset_SetNumBytes( DenOfIz_ShaderAsset shaderAsset, uint64_t numBytes );
    DZ_API void                          DenOfIz_ShaderAsset_SetPath( DenOfIz_ShaderAsset shaderAsset, DenOfIz_StringView path );
    DZ_API DenOfIz_StringView            DenOfIz_ShaderAsset_StoreString( DenOfIz_ShaderAsset shaderAsset, DenOfIz_StringView str );
    DZ_API DenOfIz_StringView            DenOfIz_ShaderAsset_Extension( );

#ifdef __cplusplus
}
#endif
