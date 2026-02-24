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

#include <DirectXMath.h>
#include "DenOfIzExamples/Assets/AssetData.h"
#include "DenOfIzGraphics/Backends/Interface/BindGroup.h"
#include "DenOfIzGraphics/Backends/Interface/BindGroupLayout.h"
#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "Interop.h"

using namespace DirectX;

namespace DenOfIz
{
    class PerDrawBinding final
    {
        void             *m_modelMatrixMappedData = nullptr;
        DenOfIz_Buffer    m_modelMatrixBuffer     = DENOFIZ_NULL_HANDLE;
        DenOfIz_BindGroup m_bindGroup             = DENOFIZ_NULL_HANDLE;

    public:
        DZ_EXAMPLES_API PerDrawBinding( DenOfIz_LogicalDevice device, DenOfIz_BindGroupLayout layout );
        DZ_EXAMPLES_API ~PerDrawBinding( );
        DZ_EXAMPLES_API void                            Update( const XMFLOAT4X4 &modelMatrix ) const;
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_BindGroup BindGroup( ) const;
    };
} // namespace DenOfIz
