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

#include "exception"
#include "glog/logging.h"
#include "Interop.h"

#define DZ_RETURN_IF( condition )                                                                                                                                                  \
    if ( condition )                                                                                                                                                               \
    return
#define DZ_ASSERTM( exp, msg )                                                                                                                                                     \
    if ( !( exp ) )                                                                                                                                                                \
    DLOG( ERROR ) << msg
#define DZ_NOT_NULL( exp )                                                                                                                                                         \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        if ( !exp )                                                                                                                                                                \
        {                                                                                                                                                                          \
            LOG( FATAL ) << #exp " is required but was null.";                                                                                                                     \
        }                                                                                                                                                                          \
    }                                                                                                                                                                              \
    while ( 0 )

namespace DenOfIz
{
    class DZ_API NonCopyable
    {
        NonCopyable( NonCopyable const & )            = delete;
        NonCopyable( NonCopyable && )                 = delete;
        NonCopyable &operator=( NonCopyable const & ) = delete;
        NonCopyable &operator=( NonCopyable && )      = delete;

    protected:
        NonCopyable( )  = default;
        ~NonCopyable( ) = default;
    };
} // namespace DenOfIz

#ifndef DZ_SWIG
#define DZ_ARRAY_ACCESS( ElementType, DisplayName, FieldName )
#else
/// Swig creates a proxy element every time an InteropArray field is accessed, this is a way to work around this issue
#define DZ_ARRAY_ACCESS( ElementType, DisplayName, FieldName )                                                                                                                     \
    size_t Num##DisplayName##Elements( ) const                                                                                                                                     \
    {                                                                                                                                                                              \
        return FieldName.NumElements( );                                                                                                                                           \
    }                                                                                                                                                                              \
    void Set##DisplayName( size_t i, const ElementType &val )                                                                                                                      \
    {                                                                                                                                                                              \
        FieldName.SetElement( i, val );                                                                                                                                            \
    }                                                                                                                                                                              \
    const ElementType &Get##DisplayName( size_t i ) const                                                                                                                          \
    {                                                                                                                                                                              \
        return FieldName.GetElement( i );                                                                                                                                          \
    }                                                                                                                                                                              \
    void Add##DisplayName( const ElementType &val )                                                                                                                                \
    {                                                                                                                                                                              \
        FieldName.AddElement( val );                                                                                                                                               \
    }                                                                                                                                                                              \
    void Resize##DisplayName##s( size_t size )                                                                                                                                     \
    {                                                                                                                                                                              \
        FieldName.Resize( size );                                                                                                                                                  \
    }                                                                                                                                                                              \
    void Clear##DisplayName##s( )                                                                                                                                                  \
    {                                                                                                                                                                              \
        FieldName.Clear( );                                                                                                                                                        \
    }
#endif