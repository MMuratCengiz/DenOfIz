/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetWriterHelpers.h"

using namespace DenOfIz;

void AssetWriterHelpers::WriteAssetDataStream( const DenOfIz_BinaryWriter writer, const DenOfIz_AssetDataStream &stream )
{
    DenOfIz_BinaryWriter_WriteUInt64( writer, stream.Offset );
    DenOfIz_BinaryWriter_WriteUInt64( writer, stream.NumBytes );
}
