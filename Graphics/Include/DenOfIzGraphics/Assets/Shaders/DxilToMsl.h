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
#include "DenOfIzGraphics/Backends/Interface/ShaderData.h"
#include "DenOfIzGraphics/Utilities/Interop.h"

namespace DenOfIz
{
    struct DZ_API DxilToMslDesc
    {
        ShaderStageDescArray     Shaders;
        CompiledShaderStageArray DXILShaders;
        ShaderRayTracingDesc     RayTracing;
        ByteArrayArray          *OutMSLShaders;
    };

    class DxilToMsl
    {
    public:
        DZ_API DxilToMsl( );
        DZ_API ~DxilToMsl( );
        DZ_API void Convert( const DxilToMslDesc &desc ) const;

    private:
        class Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
} // namespace DenOfIz
