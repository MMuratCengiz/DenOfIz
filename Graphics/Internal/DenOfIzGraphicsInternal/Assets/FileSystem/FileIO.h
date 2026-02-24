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

#include <filesystem>
#include "DenOfIzGraphics/Utilities/Common.h"

#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

namespace DenOfIz
{
    /**
     * @brief Cross-platform file I/O operations for asset management.
     * Handles platform-specific path resolution and provides consistent interface
     * for file operations across different platforms.
     */
    class FileIO
    {
    public:
        static uint64_t GetFileNumBytes( DenOfIz_StringView path );
        /**
         * @brief Read entire file into a byte array
         * @param path Path to the file (relative to resources or absolute)
         * @param buffer Contents will be written here, make
         * @return Byte array containing file contents
         * @throws std::runtime_error if file cannot be read
         * @note For binary files. Does not add null termination.
         */
        static bool ReadFile( DenOfIz_StringView path, const DenOfIz_ByteArray &buffer );

        /**
         * @brief Write byte array to file
         * @param path Path to output file
         * @param data Byte array to write
         * @throws std::runtime_error if file cannot be written
         */
        static void WriteFile( DenOfIz_StringView path, const DenOfIz_ByteArrayView &data );

        /**
         * @brief Check if file exists
         * @param path Path to check
         * @return true if file exists
         */
        static bool FileExists( DenOfIz_StringView path );

        /**
         * @brief Create directory and any needed parent directories
         * @param path Directory path to create
         * @return true if successful
         */
        static bool CreateDirectories( DenOfIz_StringView path );

        /**
         * @brief Delete file or empty directory
         * @param path Path to remove
         * @return true if successful
         */
        static bool Remove( DenOfIz_StringView path );

        /**
         * @brief Delete directory and all contents
         * @param path Directory to remove
         * @return true if successful
         */
        static bool RemoveAll( DenOfIz_StringView path );

        /**
         * @brief Get canonical absolute path, resolving any . or .. components
         * @param path Input path
         * @return Canonical absolute path
         */
        static std::string GetAbsolutePath( DenOfIz_StringView path );

        /**
         * @brief Convert path to resource path inside application bundle on macOS,
         * or return unmodified path on other platforms
         * @param path Input path
         * @return Resource path
         */
        static std::string GetResourcePath( DenOfIz_StringView path );

        static std::string PlatformResourcePath( const std::string &path );
    };

} // namespace DenOfIz
