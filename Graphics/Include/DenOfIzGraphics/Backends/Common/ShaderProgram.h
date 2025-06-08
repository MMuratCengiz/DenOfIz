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

#include <memory>
#include "DenOfIzGraphics/Assets/Shaders/ShaderReflectDesc.h"
#include "DenOfIzGraphics/Backends/Interface/ShaderData.h"
#include "DenOfIzGraphics/Utilities/Interop.h"

namespace DenOfIz
{

    /// <summary>
    /// This is used to bind data to raytracing shaders. With this structure you specify a register space, a stage and an entry name which will be then created as a root structure
    /// Any data you bind to this register space will be considered input for the specific ShaderStage.
    /// Note there is no difference between the hit shaders as normally they are all exported together.
    /// </summary>
    struct DZ_API ShaderRecordBindingDesc
    {
        uint32_t      RegisterSpace;
        ShaderStage   Stage;
        InteropString EntryName;
    };
    template class DZ_API InteropArray<ShaderRecordBindingDesc>;

    struct DZ_API ShaderProgramDesc
    {
        InteropArray<ShaderStageDesc> ShaderStages;
        ShaderRayTracingDesc          RayTracing;
    };

    struct DZ_API CompiledShader
    {
        // The user should clean up these!
        InteropArray<CompiledShaderStage *> Stages;
        ShaderReflectDesc                   ReflectDesc;
        ShaderRayTracingDesc                RayTracing;
    };
    template class DZ_API InteropArray<CompiledShader>;

    struct ShaderAsset;

    class ShaderProgram
    {
    public:
        DZ_API explicit ShaderProgram( ShaderProgramDesc desc );
        // Load shader program generated offline into a file
        DZ_API explicit ShaderProgram( const ShaderAsset &asset );
        DZ_API ~ShaderProgram( );

        [[nodiscard]] DZ_API InteropArray<CompiledShaderStage *> CompiledShaders( ) const;
        [[nodiscard]] DZ_API ShaderReflectDesc                   Reflect( ) const;
        [[nodiscard]] DZ_API ShaderProgramDesc                   Desc( ) const;

    private:
        class Impl;
        std::unique_ptr<Impl>         m_pImpl;
    };

} // namespace DenOfIz
