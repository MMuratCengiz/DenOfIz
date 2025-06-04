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

#include <DenOfIzGraphics/Backends/Interface/ReflectionData.h>
#include <DenOfIzGraphics/Backends/Interface/ShaderData.h>
#include <metal_irconverter/metal_irconverter.h>
#ifndef _WIN32
#define interface struct
#endif
#include <directx/d3d12shader.h>
#ifndef _WIN32
#undef interface
#endif

namespace DenOfIz
{

    class DxcEnumConverter final
    {
    public:
        static ResourceBindingType   ReflectTypeToBufferBindingType( D3D_SHADER_INPUT_TYPE type );
        static ReflectionBindingType ReflectTypeToBufferReflectionBindingType( D3D_SHADER_INPUT_TYPE type );
        static ResourceDescriptor    ReflectTypeToRootSignatureType( D3D_SHADER_INPUT_TYPE type, D3D_SRV_DIMENSION dimension );
        static std::string           GetFieldTypeString( ReflectionFieldType type );
        static std::string           GetBindingTypeString( ResourceBindingType type );
        static std::string           GetStagesString( const InteropArray<ShaderStage> &stages );
        static ReflectionFieldType   VariableTypeToReflectionType( const D3D_SHADER_VARIABLE_TYPE &type );
        // MSL specific
#if defined( _WIN32 ) || defined( __APPLE__ ) // TODO metal shader converter on linux: not yet supported
        static IRShaderVisibility    ShaderStageToShaderVisibility( ShaderStage stage );
        static IRRootParameterType   BindingTypeToIRRootParameterType( const ResourceBindingType &type );
        static IRRootParameterType   IRDescriptorRangeTypeToIRRootParameterType( const IRDescriptorRangeType &type );
        static IRDescriptorRangeType ShaderTypeToIRDescriptorType( const D3D_SHADER_INPUT_TYPE &type );
#endif
    };
} // namespace DenOfIz
