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

#include "DenOfIzGraphics/Backends/Interface/IPipeline.h"
#include "IBottomLevelAS.h"
#include "IShaderLocalData.h"
#include "ITopLevelAS.h"

namespace DenOfIz
{
    struct DZ_API HitGroupBindingDesc
    {
        HitGroupType      GeometryType       = HitGroupType::Triangles;
        int               Offset             = 0;          // Where to place this hit group in the hit group range
        InteropString     HitGroupExportName = "HitGroup"; // Must match `HitGroupExportName` provided in the ShaderDesc structure.
        IShaderLocalData *Data               = nullptr;
    };

    struct DZ_API MissBindingDesc
    {
        int               Offset = 0;
        InteropString     ShaderName;
        IShaderLocalData *Data = nullptr;
    };

    struct DZ_API RayGenerationBindingDesc
    {
        InteropString     ShaderName;
        IShaderLocalData *Data = nullptr;
    };

    struct DZ_API SBTSizeDesc
    {
        uint32_t NumRayGenerationShaders = 1;
        uint32_t NumMissShaders          = 1;
        uint32_t NumHitGroups            = 1;
    };

    struct DZ_API ShaderBindingTableDesc
    {
        IPipeline  *Pipeline = nullptr;
        SBTSizeDesc SizeDesc;
        uint32_t    MaxHitGroupDataBytes = 0;
        uint32_t    MaxMissDataBytes     = 0;
        uint32_t    MaxRayGenDataBytes   = 0;
    };

    struct DZ_API ShaderRecordDebugData
    {
        const void   *Identifier;
        uint32_t      IdentifierSize;
        uint32_t      LocalRootArgsSize;
        InteropString Name;
    };

    struct DZ_API ShaderRecordDebugDataArray
    {
        ShaderRecordDebugData *Elements;
        uint32_t               NumElements;
    };

    struct DZ_API ShaderBindingTableDebugData
    {
        size_t                     HitGroupNumBytes;
        size_t                     MissNumBytes;
        size_t                     RayGenNumBytes;
        ShaderRecordDebugDataArray RayGenerationShaders;
        ShaderRecordDebugDataArray MissShaders;
        ShaderRecordDebugDataArray HitGroups;
    };

    class DZ_API IShaderBindingTable
    {
    public:
        // TODO TBD if we want to keep this
        virtual void Resize( const SBTSizeDesc & )                                   = 0;
        virtual void BindRayGenerationShader( const RayGenerationBindingDesc &desc ) = 0;
        virtual void BindHitGroup( const HitGroupBindingDesc &desc )                 = 0;
        virtual void BindMissShader( const MissBindingDesc &desc )                   = 0;
        virtual void Build( )                                                        = 0;
        //.. omw
        virtual ~IShaderBindingTable( ) = default;

        static void PrintShaderBindingTableDebugData( const ShaderBindingTableDebugData &table );
    };
} // namespace DenOfIz
