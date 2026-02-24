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

#include "DenOfIzGraphics/Backends/Interface/BindGroupLayout.h"
#include "DenOfIzGraphics/Backends/Interface/InputLayout.h"
#include "DenOfIzGraphics/Backends/Interface/RayTracing/LocalRootSignature.h"
#include "DenOfIzGraphics/Backends/Interface/RootSignature.h"

struct DenOfIz_ShaderReflectDesc
{
    DenOfIz_InputLayoutDesc              InputLayout;
    DenOfIz_BindGroupLayoutDescArray     BindGroupLayouts;
    DenOfIz_RootConstantBindingDescArray RootConstants;
    /// Local data layouts for each shader, index matched with the ShaderDescs provided in the ShaderProgramDesc.
    DenOfIz_LocalRootSignatureDescArray LocalRootSignatures;
    /// Thread group sizes for compute, mesh, and task shaders.
    DenOfIz_ThreadGroupDescArray ThreadGroups;
};
