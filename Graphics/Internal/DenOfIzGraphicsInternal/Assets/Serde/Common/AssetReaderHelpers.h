/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Assets/Serde/Asset.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"

namespace DenOfIz
{
    class AssetReaderHelpers
    {
    public:
        static DenOfIz_AssetDataStream ReadAssetDataStream( DenOfIz_BinaryReader reader );
    };
} // namespace DenOfIz
