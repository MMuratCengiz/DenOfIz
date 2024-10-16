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

#include "Interop.h"
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include <DenOfIzGraphics/Renderer/Assets/AssetData.h>
#include <DirectXMath.h>

namespace DenOfIz
{
    using namespace DirectX;

    class SphereAsset final
    {
    private:
        XMFLOAT4X4                       m_modelMatrix = { };
        std::unique_ptr<IBufferResource> m_modelBuffer;
        std::unique_ptr<AssetData>       m_assetData;
        std::unique_ptr<MaterialData>    m_materialData;

    public:
        DZ_EXAMPLES_API            SphereAsset( ILogicalDevice *device, BatchResourceCopy *batchResourceCopy );
        DZ_EXAMPLES_API void       Translate( XMFLOAT4 translation );
        DZ_EXAMPLES_API void       Rotate( XMFLOAT4 rotation );
        DZ_EXAMPLES_API void       Scale( XMFLOAT4 scale );
        DZ_EXAMPLES_API AssetData *Data( ) const;
        DZ_EXAMPLES_API XMFLOAT4X4 ModelMatrix( ) const;
    };
} // namespace DenOfIz
