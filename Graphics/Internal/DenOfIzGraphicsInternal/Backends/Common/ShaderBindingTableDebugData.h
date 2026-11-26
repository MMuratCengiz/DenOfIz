/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace DenOfIz
{
    struct ShaderRecordDebugData
    {
        const void *Identifier;
        uint32_t    IdentifierSize;
        uint32_t    LocalRootArgsSize;
        std::string Name;
    };

    struct ShaderBindingTableDebugData
    {
        size_t                             HitGroupNumBytes;
        size_t                             MissNumBytes;
        size_t                             RayGenNumBytes;
        std::vector<ShaderRecordDebugData> RayGenerationShaders;
        std::vector<ShaderRecordDebugData> MissShaders;
        std::vector<ShaderRecordDebugData> HitGroups;
    };

    inline void PrintShaderBindingTableDebugData( const ShaderBindingTableDebugData &table )
    {
        std::stringstream ss;

        auto PrintRecords = [ &ss ]( const std::string &tableName, const std::vector<ShaderRecordDebugData> &records, uint32_t recordSize )
        {
            ss << "|--------------------------------------------------------------------\n";
            ss << "|Shader table - " << tableName << ": " << recordSize << " | " << records.size( ) * recordSize << " bytes\n";

            for ( size_t i = 0; i < records.size( ); i++ )
            {
                const auto &record = records[ i ];
                ss << "| [" << i << "]: " << record.Name << ", " << record.IdentifierSize << " + " << record.LocalRootArgsSize << " bytes\n";
            }
            ss << "|--------------------------------------------------------------------\n";
        };

        PrintRecords( "RayGenShaderTable", table.RayGenerationShaders, table.RayGenNumBytes );
        PrintRecords( "MissShaderTable", table.MissShaders, table.MissNumBytes );
        PrintRecords( "HitGroupShaderTable", table.HitGroups, table.HitGroupNumBytes );
    }
} // namespace DenOfIz
