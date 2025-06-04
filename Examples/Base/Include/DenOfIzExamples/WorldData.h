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

#include "DenOfIzExamples/Assets/AssetData.h"
#include "Camera.h"
#include "PerMaterialBinding.h"

namespace DenOfIz
{
    using namespace DirectX;

    struct RenderItem
    {
        XMFLOAT4X4 Model;
        AssetData *Data;
    };

    struct MaterialBatch
    {
        const PerMaterialBinding *MaterialBinding;
        std::vector<RenderItem>   RenderItems;

        MaterialBatch( PerMaterialBinding *materialBinding, const MaterialData *material ) : MaterialBinding( materialBinding )
        {
            MaterialBinding->Update( material );
        }
    };

    struct RenderBatch
    {
        std::vector<MaterialBatch> MaterialBatches;
    };

    struct WorldData
    {
        RenderBatch RenderBatch;
        Camera     *Camera;
        float       DeltaTime;
    };
} // namespace DenOfIz
