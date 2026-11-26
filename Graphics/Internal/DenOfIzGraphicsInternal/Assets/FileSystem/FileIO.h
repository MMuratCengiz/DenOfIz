/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <filesystem>
#include "DenOfIzGraphics/Utilities/Common.h"

#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

namespace DenOfIz
{
    class FileIO
    {
    public:
        static uint64_t GetFileNumBytes( DenOfIz_StringView path );

        static bool        ReadFile( DenOfIz_StringView path, const DenOfIz_ByteArray &buffer );
        static void        WriteFile( DenOfIz_StringView path, const DenOfIz_ByteArrayView &data );
        static bool        FileExists( DenOfIz_StringView path );
        static bool        CreateDirectories( DenOfIz_StringView path );
        static bool        Remove( DenOfIz_StringView path );
        static bool        RemoveAll( DenOfIz_StringView path );
        static std::string GetAbsolutePath( DenOfIz_StringView path );
        static std::string GetResourcePath( DenOfIz_StringView path );

        static std::string PlatformResourcePath( const std::string &path );
    };

} // namespace DenOfIz
