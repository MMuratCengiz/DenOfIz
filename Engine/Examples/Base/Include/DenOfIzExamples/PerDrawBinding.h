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

#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include <DenOfIzGraphics/Renderer/Assets/AssetData.h>
#include <DirectXMath.h>

using namespace DirectX;

namespace DenOfIz
{
    class PerDrawBinding final
    {
        constexpr static uint8_t            RegisterSpace = 2;
        void                               *m_modelMatrixMappedData;
        std::unique_ptr<IBufferResource>    m_modelMatrixBuffer;
        std::unique_ptr<IResourceBindGroup> m_bindGroup;

    public:
        PerDrawBinding( ILogicalDevice *device, IRootSignature *rootSignature );
        ~PerDrawBinding( );
        void                              Update( const XMFLOAT4X4 &modelMatrix ) const;
        [[nodiscard]] IResourceBindGroup *BindGroup( ) const;
    };
} // namespace DenOfIz
