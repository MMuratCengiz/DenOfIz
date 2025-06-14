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

#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Backends/Interface/IRootSignature.h"
#include "DenOfIzGraphics/Backends/Interface/RayTracing/RayTracingData.h"
#include "DenOfIzGraphics/Backends/Interface/ShaderData.h"

namespace DenOfIz
{
    struct DZ_API LocalRootSignatureDesc
    {
        ResourceBindingDescArray ResourceBindings{ };
    };

    struct DZ_API LocalRootSignatureDescArray
    {
        LocalRootSignatureDesc *Elements;
        uint32_t                NumElements;
    };

    ///
    /// @brief Layout specification for shader records. This equates to LocalRootSignature in DXR. No interface since it is quite API specific.
    class DZ_API ILocalRootSignature
    {
    public:
        virtual ~ILocalRootSignature( ) = default;
    };

    struct DZ_API ILocalRootSignatureArray
    {
        ILocalRootSignature **Elements;
        uint32_t              NumElements;
    };
} // namespace DenOfIz
