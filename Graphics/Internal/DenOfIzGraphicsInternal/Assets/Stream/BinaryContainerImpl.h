/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <iosfwd>
#include <ostream>
#include <sstream>

#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

namespace DenOfIz
{
    class BinaryContainerImpl
    {
        mutable std::string m_cachedData;
        std::stringstream   m_stream;

    public:
        BinaryContainerImpl( );
        ~BinaryContainerImpl( );

        std::iostream        *Stream( );
        DenOfIz_ByteArrayView GetData( ) const;
    };
} // namespace DenOfIz
