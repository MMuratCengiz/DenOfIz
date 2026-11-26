/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <vector>
#include "DenOfIzExamples/Assets/AssetData.h"
#include "FreeCamera.h"
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
        RenderBatch Batch;
        FreeCamera *Camera;
        float       DeltaTime;
    };
} // namespace DenOfIz
