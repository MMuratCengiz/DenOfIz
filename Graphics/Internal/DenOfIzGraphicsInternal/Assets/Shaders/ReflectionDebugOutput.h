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

#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include "DxcEnumConverter.h"
#include "ShaderReflectionHelper.h"
#include "metal_irconverter/metal_irconverter.h"

namespace DenOfIz
{
    class ReflectionDebugOutput
    {
    public:
        static void DumpIRRootParameters( const std::vector<IRRootParameter1> &rootParameters, const char *prefix = "" );
        static void DumpReflectionInfo( const ShaderReflectDesc &reflection );
        static void DumpResourceBindings( std::stringstream &output, const ResourceBindingDescArray &resourceBindings );
        static void DumpRootSignature( std::stringstream &output, const RootSignatureDesc &sig );
        static void DumpStructFields( std::stringstream &output, const InteropArray<ReflectionResourceField> &fields );
    };
} // namespace DenOfIz
