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

#include "DenOfIzGraphics/Backends/Interface/RayTracing/IShaderBindingTable.h"
#include <sstream>
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

void PrintRecordDebugData( std::stringstream &ss, const std::string &tableName, const ShaderRecordDebugDataArray &records, uint32_t recordSize )
{
    ss << "|--------------------------------------------------------------------\n";
    ss << "|Shader table - " << tableName << ": " << recordSize << " | " << records.NumElements * recordSize << " bytes\n";

    for ( size_t i = 0; i < records.NumElements; i++ )
    {
        const auto &record = records.Elements[ i ];
        ss << "| [" << i << "]: " << record.Name.Get( ) << ", " << record.IdentifierSize << " + " << record.LocalRootArgsSize << " bytes\n";
    }
    ss << "|--------------------------------------------------------------------\n";
}

void IShaderBindingTable::PrintShaderBindingTableDebugData( const ShaderBindingTableDebugData &table )
{
    std::stringstream ss;

    PrintRecordDebugData( ss, "RayGenShaderTable", table.RayGenerationShaders, table.RayGenNumBytes );
    PrintRecordDebugData( ss, "MissShaderTable", table.MissShaders, table.MissNumBytes );
    PrintRecordDebugData( ss, "HitGroupShaderTable", table.HitGroups, table.HitGroupNumBytes );

    spdlog::info( "\n {}", ss.str( ) );
}
