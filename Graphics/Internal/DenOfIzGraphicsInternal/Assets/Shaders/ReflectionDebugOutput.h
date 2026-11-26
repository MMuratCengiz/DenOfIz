/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/IRootSignature.h>
#include "DxcEnumConverter.h"
#include "ShaderReflectionHelper.h"
#include "metal_irconverter/metal_irconverter.h"

namespace DenOfIz
{
    class ReflectionDebugOutput
    {
    public:
        static void DumpIRRootParameters( const std::vector<IRRootParameter1> &rootParameters, const char *prefix = "" );
        static void DumpReflectionInfo( const DenOfIz_ShaderReflectDesc &reflection );
        static void DumpBindGroupLayouts( std::stringstream &output, const DenOfIz_BindGroupLayoutDescArray &bindGroupLayouts );
        static void DumpRootConstants( std::stringstream &output, const DenOfIz_RootConstantBindingDescArray &rootConstants );
        static void DumpLocalResourceBindings( std::stringstream &output, const DenOfIz_LocalResourceBindingDescArray &resourceBindings );
    };
} // namespace DenOfIz
