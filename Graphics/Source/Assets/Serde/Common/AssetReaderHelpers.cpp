/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetReaderHelpers.h"

using namespace DenOfIz;

DenOfIz_AssetDataStream AssetReaderHelpers::ReadAssetDataStream( DenOfIz_BinaryReader reader )
{
    DenOfIz_AssetDataStream stream;
    stream.Offset   = DenOfIz_BinaryReader_ReadUInt64( reader );
    stream.NumBytes = DenOfIz_BinaryReader_ReadUInt64( reader );
    return stream;
}
