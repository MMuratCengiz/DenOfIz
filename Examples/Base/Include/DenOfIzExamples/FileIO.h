/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "Interop.h"

namespace DenOfIz
{
    class FileIO
    {
    public:
        static DZ_EXAMPLES_API bool FileExists( const DenOfIz_StringView &path );
    };
} // namespace DenOfIz
