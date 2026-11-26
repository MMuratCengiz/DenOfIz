/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Assets/Serde/Asset.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"

namespace DenOfIz
{
    class AssetWriterHelpers
    {
    public:
        static void WriteAssetDataStream( const DenOfIz_BinaryWriter writer, const DenOfIz_AssetDataStream &stream );
    };
} // namespace DenOfIz
