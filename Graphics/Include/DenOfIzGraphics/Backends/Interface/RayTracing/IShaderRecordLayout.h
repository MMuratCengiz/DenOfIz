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

#include <DenOfIzGraphics/Backends/Interface/CommonData.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>
#include <DenOfIzGraphics/Backends/Interface/RayTracing/RayTracingData.h>

namespace DenOfIz
{
    struct DZ_API ShaderRecordBindingDesc
    {
        InteropString               Name;
        DescriptorBufferBindingType Type;
        uint32_t                    Binding;
        uint32_t                    NumBytes;
        uint32_t                    Location; // Used for Metal
    };

    struct DZ_API ShaderRecordLayoutDesc
    {
        /***
         * Currently the idea is that register spaces will have corresponding with the shader stages.
         * This could change as I am not sure if this causes a lot of "secret" register spaces.
         */
        ShaderStage                           Stage;
        InteropArray<ShaderRecordBindingDesc> Bindings;
    };

    ///
    /// \brief Layout specification for shader records. This equates to LocalRootSignature in DXR. No interface since it is quite API specific.
    class DZ_API IShaderRecordLayout
    {
    public:
        virtual ~IShaderRecordLayout( ) = default;
    };
} // namespace DenOfIz
