/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

namespace DenOfIz
{
    class StringStore
    {
        std::vector<std::unique_ptr<char[]>> m_blocks;
        char                                *m_currentPtr     = nullptr;
        size_t                               m_bytesRemaining = 0;

    public:
        static constexpr size_t s_defaultBlockSize = 8192;

        StringStore( ) = default;

        StringStore( const StringStore & )            = delete;
        StringStore &operator=( const StringStore & ) = delete;

        StringStore( StringStore && )            = default;
        StringStore &operator=( StringStore && ) = default;

        DenOfIz_StringView Store( const std::string &str );
        DenOfIz_StringView Store( const char *cString );
        DenOfIz_StringView Store( const char *str, size_t len );

    private:
        void AllocateNewBlock( size_t minSize );
    };
} // namespace DenOfIz
