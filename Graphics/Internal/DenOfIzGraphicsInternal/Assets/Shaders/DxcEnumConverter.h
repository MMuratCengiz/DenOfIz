/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphics/Backends/Interface/ShaderData.h>
#include <metal_irconverter/metal_irconverter.h>
#include <string>

#ifdef _WIN32
#include <wrl/client.h>
#include "DenOfIzGraphics/Utilities/Common_Windows.h"
#else
#define __EMULATE_UUID
#include "WinAdapter.h"
#endif

#include "dxcapi.h"

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
        static DenOfIz_ResourceBindingType ReflectTypeToBufferBindingType( D3D_SHADER_INPUT_TYPE type );
        static uint32_t                    ReflectTypeToRootSignatureType( D3D_SHADER_INPUT_TYPE type, D3D_SRV_DIMENSION dimension );
        static std::string                 GetBindingTypeString( DenOfIz_ResourceBindingType type );
        static std::string                 GetStagesString( DenOfIz_ShaderStageFlags stages );
        // MSL specific
#if defined( _WIN32 ) || defined( __APPLE__ ) // TODO metal shader converter on linux: not yet supported
        static IRShaderVisibility    ShaderStageToShaderVisibility( DenOfIz_ShaderStageFlags stage );
        static IRRootParameterType   BindingTypeToIRRootParameterType( const DenOfIz_ResourceBindingType &type );
        static IRRootParameterType   IRDescriptorRangeTypeToIRRootParameterType( const IRDescriptorRangeType &type );
        static IRDescriptorRangeType ShaderTypeToIRDescriptorType( const D3D_SHADER_INPUT_TYPE &type );
#endif
    };
} // namespace DenOfIz
