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

#include "Inter.h"

namespace DenOfIz
{
    class EmbeddedFonts
    {
    public:
        DZ_API static FontAsset *GetInterVar( )
        {
            static FontAsset interVar = InterVar( );
            return &interVar;
        }

    private:
        static const InteropArray<Byte>& InterDataAggr( )
        {
            static InteropArray<Byte> data;
            static std::mutex         dataMutex;
            if ( data.NumElements( ) == 0 )
            {
                std::lock_guard lock( dataMutex );
                if ( data.NumElements( ) == 0 )
                {
                    const size_t size0     = Inter::Data0.size( );
                    const size_t size1     = Inter::Data1.size( );
                    const size_t size2     = Inter::Data2.size( );
                    const size_t size3     = Inter::Data3.size( );
                    const size_t totalSize = size0 + size1 + size2 + size3;

                    std::vector<Byte> vData;
                    vData.reserve( totalSize );

                    vData.insert( vData.end( ), Inter::Data0.begin( ), Inter::Data0.end( ) );
                    vData.insert( vData.end( ), Inter::Data1.begin( ), Inter::Data1.end( ) );
                    vData.insert( vData.end( ), Inter::Data2.begin( ), Inter::Data2.end( ) );
                    vData.insert( vData.end( ), Inter::Data3.begin( ), Inter::Data3.end( ) );

                    data.MemCpy( vData.data( ), totalSize );
                }
            }
            return data;
        }

        static FontAsset InterVar( )
        {
            BinaryReader    binaryReader( InterDataAggr( ) );
            FontAssetReader reader( { &binaryReader } );
            return reader.Read( );
        }
    };
} // namespace DenOfIz
