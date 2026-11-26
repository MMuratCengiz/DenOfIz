/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/BindGroupLayout.h"
#include "DenOfIzGraphics/Backends/Interface/InputLayout.h"
#include "DenOfIzGraphics/Backends/Interface/RayTracing/LocalRootSignature.h"
#include "DenOfIzGraphics/Backends/Interface/RootSignature.h"

struct DenOfIz_ShaderReflectDesc
{
    DenOfIz_InputLayoutDesc              InputLayout;
    DenOfIz_BindGroupLayoutDescArray     BindGroupLayouts;
    DenOfIz_RootConstantBindingDescArray RootConstants;
    /** One per stage, indexed like DenOfIz_ShaderProgramDesc::ShaderStages. */
    DenOfIz_LocalRootSignatureDescArray LocalRootSignatures;
    /** One per stage, indexed like DenOfIz_ShaderProgramDesc::ShaderStages. Set for compute, mesh and task stages, zero for others. */
    DenOfIz_ThreadGroupDescArray ThreadGroups;
};
